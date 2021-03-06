//
// Benchmarking Leopard library: https://github.com/catid/leopard
//

#include <cstdio>
#include <memory>

#include "common.h"

#include "LeopardFF8.cpp"
#undef LEO_MUL_128
#undef LEO_MULADD_128
#undef LEO_MUL_256
#undef LEO_MULADD_256
#undef LEO_IFFTB_256
#undef LEO_IFFTB_128
#undef LEO_FFTB_256
#undef LEO_FFTB_128
#include "LeopardFF16.cpp"
#include "LeopardCommon.cpp"
#include "leopard.cpp"


// Extra workspace used by the library on top of place required for original data
size_t leopard_extra_space(ECC_bench_params params)
{
    size_t encode_work_count = leo_encode_work_count(params.OriginalCount, params.RecoveryCount);
    size_t decode_work_count = leo_decode_work_count(params.OriginalCount, params.RecoveryCount);
    return params.BlockBytes * (encode_work_count + decode_work_count);
}


// Perform single encoding operation, return false if it fails
bool leopard_benchmark_encode(
    ECC_bench_params params,
    size_t encode_work_count,
    void** original_data,
    void** parity_data,
    OperationTimer& encode_time)
{
    // Generate recovery data
    encode_time.BeginCall();
    LeopardResult encodeResult = leo_encode(
        params.BlockBytes,
        params.OriginalCount,
        params.RecoveryCount,
        encode_work_count,
        original_data,
        parity_data
    );
    encode_time.EndCall();

    if (encodeResult != Leopard_Success)
    {
        printf("  leo_encode failed: %s\n", leo_result_string(encodeResult));
        return false;
    }

    return true;
}


// Perform single decoding operation, return false if it fails
bool leopard_benchmark_decode(
    ECC_bench_params params,
    size_t decode_work_count,
    void** originalFileData_losing_one,
    void** recoveryBlocks,
    void** decoderWorkArea,
    OperationTimer& decode_time)
{
    decode_time.BeginCall();
    LeopardResult decodeResult = leo_decode(
        params.BlockBytes,
        params.OriginalCount,
        params.RecoveryCount,
        decode_work_count,
        originalFileData_losing_one,
        recoveryBlocks,
        decoderWorkArea);
    decode_time.EndCall();

    if (decodeResult != Leopard_Success)
    {
        printf("  leo_decode-one failed: %s\n", leo_result_string(decodeResult));
        return false;
    }

    return true;
}


// Benchmark library and print results, return false if anything failed
bool leopard_benchmark_main(ECC_bench_params params, uint8_t* buffer)
{
    // Total encode/decode times
    OperationTimer encode_time, decode_one_time, decode_all_time;

    if (leo_init()) {
        printf("leo_init failed\n");
        return false;
    }

    size_t encode_work_count = leo_encode_work_count(params.OriginalCount, params.RecoveryCount);
    size_t decode_work_count = leo_decode_work_count(params.OriginalCount, params.RecoveryCount);

    if (encode_work_count == 0)  // 0 means unsupported data+parity combination
        return false;

    // Print CPU SIMD extensions used to accelerate library in this run
    // (depends on compilation options such as -mavx2 and actual CPU)
    printf("Leopard (%s, %d-bit):\n",
#ifndef GF256_TARGET_MOBILE
#  ifdef GF256_TRY_AVX2
        leopard::CpuHasAVX2? "avx2":
#  endif
        leopard::CpuHasSSSE3? "ssse3":
#endif
#if defined(GF256_TRY_NEON)
        leopard::CpuHasNeon64? "neon64":
        leopard::CpuHasNeon? "neon":
#endif
        "", sizeof(size_t)*8);

    // Pointers to data
    std::vector<uint8_t*> original_data(params.OriginalCount);
    std::vector<uint8_t*> original_data_losing_one(params.OriginalCount);
    std::vector<uint8_t*> original_data_losing_most_possible(params.OriginalCount);
    std::vector<uint8_t*> encode_work_data(encode_work_count);
    std::vector<uint8_t*> decode_work_data(decode_work_count);

    for (unsigned i = 0; i < params.OriginalCount; ++i) {
        original_data[i] = buffer;
        // Lose only the first block
        original_data_losing_one[i] = (i==0? nullptr : buffer);
        // Lose up to RecoveryCount blocks
        original_data_losing_most_possible[i] = (i < params.RecoveryCount? nullptr : buffer);
        buffer += params.BlockBytes;
    }
    for (unsigned i = 0; i < encode_work_count; ++i) {
        encode_work_data[i] = buffer;
        buffer += params.BlockBytes;
    }
    for (unsigned i = 0; i < decode_work_count; ++i) {
        decode_work_data[i] = buffer;
        buffer += params.BlockBytes;
    }

    // It's exactly like original_data[] bit with the first block lost
    // so we have to repair it
    original_data_losing_one[0] = nullptr;

    void** originalFileData = (void**)&original_data[0];
    void** recoveryBlocks   = (void**)&encode_work_data[0];   // recovery data written here
    void** decoderWorkArea  = (void**)&decode_work_data[0];
    void** originalFileData_losing_one = (void**)&original_data_losing_one[0];
    void** originalFileData_losing_most_possible = (void**)&original_data_losing_most_possible[0];

    // Repeat benchmark multiple times to improve its accuracy
    for (int trial = 0; trial < params.Trials; ++trial)
    {
        if (! leopard_benchmark_encode(params, encode_work_count,
                originalFileData, recoveryBlocks, encode_time)) {
            return false;
        }
        if (! leopard_benchmark_decode(params, decode_work_count,
                originalFileData_losing_one, recoveryBlocks, decoderWorkArea, decode_one_time)) {
            return false;
        }
        if (! leopard_benchmark_decode(params, decode_work_count,
                originalFileData_losing_most_possible, recoveryBlocks, decoderWorkArea, decode_all_time)) {
            return false;
        }
    }

    // Benchmark reports for each operation
    encode_time.Print("encode", params.OriginalFileBytes());
    decode_one_time.Print("decode one", params.BlockBytes);
    decode_all_time.Print("decode all", params.RecoveryDataBytes());

    return true;
}
