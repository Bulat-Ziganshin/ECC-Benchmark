#include <cstdio>
#include <memory>

#include "../src/gf256.cpp"

#include "common.h"


// Perform encoding and decoding operations, return false if anything failed
bool cm256_run_trials(
    ECC_bench_params params,
    uint8_t* originalFileData,
    uint8_t* recoveryBlocks,
    uint64_t& encode_time,
    uint64_t& decode_time)
{
    // Repeat benchmark multiple times to improve its accuracy
    for (int trial = 0; trial < params.Trials; ++trial)
    {
        // Pointers to data
        cm256_block blocks[256];
        for (int i = 0; i < params.OriginalCount; ++i)
        {
            blocks[i].Block = originalFileData + i * params.BlockBytes;
        }

        uint64_t t0 = siamese::GetTimeUsec();
        // Generate recovery data
        if (cm256_encode(params, blocks, recoveryBlocks))
        {
            printf("  cm256_encode failed\n");
            return false;
        }
        uint64_t t1 = siamese::GetTimeUsec();
        encode_time += t1 - t0;

        // Initialize the indices
        for (int i = 0; i < params.OriginalCount; ++i)
        {
            blocks[i].Index = cm256_get_original_block_index(params, i);
        }

        //// Simulate loss of data, subsituting a recovery block in its place ////
        blocks[0].Block = recoveryBlocks; // First recovery block
        blocks[0].Index = cm256_get_recovery_block_index(params, 0); // First recovery block index
        //// Simulate loss of data, subsituting a recovery block in its place ////

        t0 = siamese::GetTimeUsec();
        if (cm256_decode(params, blocks))
        {
            printf("  cm256_decode failed\n");
            return false;
        }
        t1 = siamese::GetTimeUsec();
        decode_time += t1 - t0;

        // blocks[0].Index will now be 0.
    }

    return true;
}


// Print the benchmark results, return false if anything failed
bool cm256_print_results(
    ECC_bench_params params,
    uint8_t* originalFileData,
    uint8_t* recoveryBlocks)
{
    uint64_t encode_time = 0, decode_time = 0;  // Total encode/decode times

    printf("CM256 (%s, %d-bit):\n",
#ifdef GF256_TRY_AVX2
        CpuHasAVX2? "avx2":
#endif
        CpuHasSSSE3? "ssse3":
#if defined(GF256_TRY_NEON)
        CpuHasNeon64? "neon64":
        CpuHasNeon? "neon":
#endif
        "", sizeof(size_t)*8);

    if (! cm256_run_trials(params, originalFileData, recoveryBlocks, encode_time, decode_time))
    {
        return false;
    }

    {
        const double opusec = double(encode_time) / params.Trials;
        const double mbps = params.OriginalFileBytes() / opusec;
        printf("  %.0lf usec, %.0lf MB/s\n", opusec, mbps);
    }
    {
        const double opusec = double(decode_time) / params.Trials;
        const double mbps = params.BlockBytes / opusec;
        printf("  %.0lf usec, %.0lf MB/s\n", opusec, mbps);
    }

    return true;
}


// Benchmark CM256 library, return false if anything failed.
// This function allocates and initializes memory buffers used in encode/decode
bool cm256_benchmark_main(ECC_bench_params params)
{
    if (cm256_init()) {
        printf("cm256_init failed\n");
        return false;
    }

    // Allocate the original file data and recovery data
    auto original = std::make_unique<uint8_t[]>(params.OriginalFileBytes());
    auto recovery = std::make_unique<uint8_t[]>(params.RecoveryCount * params.BlockBytes);
    auto originalFileData = original.get();
    auto recoveryBlocks = recovery.get();

    // Fill the original file data
    for (size_t i = 0; i < params.OriginalFileBytes(); ++i) {
        originalFileData[i] = (uint8_t)((i*123456791) >> 13);
    }

    return cm256_print_results(params, originalFileData, recoveryBlocks);
}
