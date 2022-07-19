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
bool leopard_benchmark_main(ECC_bench_params params, uint8_t* buffer);

// Extra workspace used by library on top of place required for original and parity data
size_t leopard_extra_space(ECC_bench_params params);


//-----------------------------------------------------------------------------
class OperationTimer
{
public:
    void BeginCall()
    {
        t0 = siamese::GetTimeUsec();
    }
    void EndCall()
    {
        const uint64_t t1 = siamese::GetTimeUsec();
        const uint64_t delta = t1 - t0;
        if (++Invocations == 1)
            MaxCallUsec = MinCallUsec = delta;
        else if (MaxCallUsec < delta)
            MaxCallUsec = delta;
        else if (MinCallUsec > delta)
            MinCallUsec = delta;
        TotalUsec += delta;
        t0 = 0;
    }
    void Reset()
    {
        t0 = 0;
        Invocations = 0;
        TotalUsec = 0;
    }
    void Print(const char* operation, uint64_t bytes_processed_per_call)
    {
        const double microseconds_per_call = double(TotalUsec) / Invocations;
        const double megabytes_per_second = bytes_processed_per_call / microseconds_per_call;
        printf("  %s: %.0lf usec, %.0lf MB/s\n", operation, microseconds_per_call, megabytes_per_second);
    }

    uint64_t t0 = 0;
    uint64_t Invocations = 0;
    uint64_t TotalUsec = 0;
    uint64_t MaxCallUsec = 0;
    uint64_t MinCallUsec = 0;
};

