#include <cstdio>
#include <memory>

#include "../src/gf256.cpp"

#include "common.h"


// Perform single encoding operation, return false if it fails
bool cm256_benchmark_encode(
    ECC_bench_params params,
    uint8_t* originalFileData,
    uint8_t* recoveryBlocks,
    uint64_t& encode_time)
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

    return true;
}


// Perform single operation decoding single lost block, return false if it fails
bool cm256_benchmark_decode_one_block(
    ECC_bench_params params,
    uint8_t* originalFileData,
    uint8_t* recoveryBlocks,
    uint64_t& decode_time)
{
    // Pointers to data
    cm256_block blocks[256];

    // Initialize the indices
    for (int i = 0; i < params.OriginalCount; ++i)
    {
        blocks[i].Block = originalFileData + i * params.BlockBytes;
        blocks[i].Index = cm256_get_original_block_index(params, i);
    }

    //// Simulate loss of data, subsituting a recovery block in its place ////
    int lostBlock = params.RecoveryCount==1? 0 : 1;  // Since recovery block #0 recovers much faster using just XORs
    blocks[0].Block = recoveryBlocks + lostBlock * params.BlockBytes; // A recovery block
    blocks[0].Index = cm256_get_recovery_block_index(params, lostBlock); // A recovery block index
    //// Simulate loss of data, subsituting a recovery block in its place ////

    uint64_t t0 = siamese::GetTimeUsec();
    if (cm256_decode(params, blocks))
    {
        printf("  cm256_decode failed\n");
        return false;
    }
    uint64_t t1 = siamese::GetTimeUsec();
    decode_time += t1 - t0;

    // blocks[0].Index will now be = lostBlock
    // and blocks[0].Block overwritten with recovered data

    return true;
}


// Perform single operation decoding as much blocks as possible, return false if it fails
bool cm256_benchmark_decode_all_blocks(
    ECC_bench_params params,
    uint8_t* originalFileData,
    uint8_t* recoveryBlocks,
    uint64_t& decode_time)
{
    // Pointers to data
    cm256_block blocks[256];

    // Initialize the indices for recovery operation
    for (int i = 0; i < params.OriginalCount; ++i)
    {
        if (i < params.RecoveryCount) {
            // Simulate loss of data, subsituting a recovery block in its place
            blocks[i].Block = recoveryBlocks + i * params.BlockBytes;       // recovery block
            blocks[i].Index = cm256_get_recovery_block_index(params, i);    // recovery block index
        } else {
            blocks[i].Block = originalFileData + i * params.BlockBytes;     // data block
            blocks[i].Index = cm256_get_original_block_index(params, i);    // data block index
        }
    }

    uint64_t t0 = siamese::GetTimeUsec();
    if (cm256_decode(params, blocks))
    {
        printf("  cm256_decode failed\n");
        return false;
    }
    uint64_t t1 = siamese::GetTimeUsec();
    decode_time += t1 - t0;

    // For each i,
    //   blocks[i].Index will now be = cm256_get_original_block_index(params, i)
    //   and blocks[i].Block overwritten with recovered data of this block

    return true;
}


// Print the benchmark results, return false if anything failed
bool cm256_benchmark_print_results(
    ECC_bench_params params,
    uint8_t* originalFileData,
    uint8_t* recoveryBlocks)
{
    // Total encode/decode times
    uint64_t encode_time = 0, decode_one_time = 0, decode_all_time = 0;

    if (cm256_init()) {
        printf("cm256_init failed\n");
        return false;
    }

    // Print CPU SIMD extensions used to accelerate CM256 in this run
    // (depends on compilation options such as -mavx2 and actual CPU)
    printf("CM256 (%s, %d-bit):\n",
#ifndef GF256_TARGET_MOBILE
#  ifdef GF256_TRY_AVX2
        CpuHasAVX2? "avx2":
#  endif
        CpuHasSSSE3? "ssse3":
#endif
#if defined(GF256_TRY_NEON)
        CpuHasNeon64? "neon64":
        CpuHasNeon? "neon":
#endif
        "", sizeof(size_t)*8);

    // Repeat benchmark multiple times to improve its accuracy
    for (int trial = 0; trial < params.Trials; ++trial)
    {
        if (! cm256_benchmark_encode(params, originalFileData, recoveryBlocks, encode_time)) {
            return false;
        }
        if (! cm256_benchmark_decode_one_block(params, originalFileData, recoveryBlocks, decode_one_time)) {
            return false;
        }
        if (! cm256_benchmark_encode(params, originalFileData, recoveryBlocks, encode_time)) {
            return false;
        }
        if (! cm256_benchmark_decode_all_blocks(params, originalFileData, recoveryBlocks, decode_all_time)) {
            return false;
        }
    }

    {
        const double opusec = double(encode_time) / (params.Trials * 2);
        const double mbps = params.OriginalFileBytes() / opusec;
        printf("  encode: %.0lf usec, %.0lf MB/s\n", opusec, mbps);
    }
    {
        const double opusec = double(decode_one_time) / params.Trials;
        const double mbps = params.BlockBytes / opusec;
        printf("  decode one: %.0lf usec, %.0lf MB/s\n", opusec, mbps);
    }
    {
        const double opusec = double(decode_all_time) / params.Trials;
        const double mbps = params.RecoveryCount * params.BlockBytes / opusec;
        printf("  decode all: %.0lf usec, %.0lf MB/s\n", opusec, mbps);
    }

    return true;
}


// Benchmark CM256 library, return false if anything failed.
// This function allocates and initializes memory buffers used in encode/decode
bool cm256_benchmark_main(ECC_bench_params params)
{
    // Allocate the original file data and recovery data
    auto original = std::make_unique<uint8_t[]>(params.OriginalFileBytes());
    auto recovery = std::make_unique<uint8_t[]>(params.RecoveryCount * params.BlockBytes);
    auto originalFileData = original.get();
    auto recoveryBlocks = recovery.get();

    // Fill the original file data
    for (size_t i = 0; i < params.OriginalFileBytes(); ++i) {
        originalFileData[i] = (uint8_t)((i*123456791) >> 13);
    }

    return cm256_benchmark_print_results(params, originalFileData, recoveryBlocks);
}
