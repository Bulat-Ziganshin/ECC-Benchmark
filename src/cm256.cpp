#include <cstdio>

#include "cm256.h"
#include "common.h"


// Benchmark CM256 library, return false if anything failed
bool bench_cm256(ECC_bench_params params)
{
    if (cm256_init()) {
        printf("cm256_init failed\n");
        return false;
    }

    // Allocate and fill the original file data
    uint8_t* originalFileData = new uint8_t[params.OriginalFileBytes()];
    memset(originalFileData, 1, params.OriginalFileBytes());

    // Pointers to data
    cm256_block blocks[256];
    for (int i = 0; i < params.OriginalCount; ++i)
    {
        blocks[i].Block = originalFileData + i * params.BlockBytes;
    }

    // Recovery data
    uint8_t* recoveryBlocks = new uint8_t[params.RecoveryCount * params.BlockBytes];

    // Generate recovery data
    if (cm256_encode(params, blocks, recoveryBlocks))
    {
        printf("cm256_encode failed\n");
        return false;
    }

    // Initialize the indices
    for (int i = 0; i < params.OriginalCount; ++i)
    {
        blocks[i].Index = cm256_get_original_block_index(params, i);
    }

    //// Simulate loss of data, subsituting a recovery block in its place ////
    blocks[0].Block = recoveryBlocks; // First recovery block
    blocks[0].Index = cm256_get_recovery_block_index(params, 0); // First recovery block index
    //// Simulate loss of data, subsituting a recovery block in its place ////

    if (cm256_decode(params, blocks))
    {
        printf("cm256_decode failed\n");
        return false;
    }

    // blocks[0].Index will now be 0.

    delete[] originalFileData;
    delete[] recoveryBlocks;
    return true;
}
