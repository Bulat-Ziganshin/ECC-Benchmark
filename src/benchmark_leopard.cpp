//
// Benchmarking Leopard library: https://github.com/catid/leopard
//

#include <cstdio>
#include <memory>

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

#include "common.h"


// Extra workspace used by library on top of place required for original and parity data
size_t leopard_extra_space(ECC_bench_params params)
{
    const size_t encode_work_count = leo_encode_work_count(params.OriginalCount, params.RecoveryCount);
    const size_t decode_work_count = leo_decode_work_count(params.OriginalCount, params.RecoveryCount);
    return params.BlockBytes * (encode_work_count + decode_work_count - params.RecoveryCount);
}


// Benchmark library and print results, return false if anything failed
bool leopard_benchmark_main(ECC_bench_params params, uint8_t* buffer)
{
    // Places for original and parity data
    auto originalFileData = buffer;
    auto recoveryBlocks   = buffer + params.OriginalFileBytes();

    // Total encode/decode times
    OperationTimer encode_time, decode_one_time, decode_all_time;

    if (leo_init()) {
        printf("leo_init failed\n");
        return false;
    }

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

    return true;
}
