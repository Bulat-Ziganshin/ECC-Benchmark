#include "cm256.h"
#include "../unit_test/SiameseTools.h"


struct ECC_bench_params : cm256_encoder_params
{
    // Repeat benchmark multiple times to improve its accuracy
    int Trials;

    // Size of the original file
    size_t OriginalFileBytes() { return OriginalCount * BlockBytes;}

    // Size of the original file
    size_t RecoveryDataBytes() { return RecoveryCount * BlockBytes;}
};


// Benchmark each library and print results, return false if anything failed
bool cm256_benchmark_main(ECC_bench_params params, uint8_t* buffer);
