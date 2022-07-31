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
bool fastecc_benchmark_main(ECC_bench_params params, uint8_t* buffer);
bool wirehair_benchmark_main(ECC_bench_params params, uint8_t* buffer);

// Extra workspace used by each library on top of place required for original data
size_t leopard_extra_space(ECC_bench_params params);
size_t fastecc_extra_space(ECC_bench_params params);

// Write benchmark results to logfile
void write_to_logfile(const char* operation, int invocations, double microseconds_per_call, double megabytes_per_second);


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
        double microseconds_per_call = double(TotalUsec) / Invocations;
        double megabytes_per_second = bytes_processed_per_call / microseconds_per_call;
        printf("  %s: %.0lf usec, %.0lf MB/s\n", operation, microseconds_per_call, megabytes_per_second);
        write_to_logfile(operation, Invocations, microseconds_per_call, megabytes_per_second);
    }

    uint64_t t0 = 0;
    uint64_t Invocations = 0;
    uint64_t TotalUsec = 0;
    uint64_t MaxCallUsec = 0;
    uint64_t MinCallUsec = 0;
};


// Round x up to 2^i
inline uint64_t NextPow2(uint64_t x)
{
	if (x == 0)  return 0;
	if (x == 1)  return 1;
	int i = 1;
	for (x--; x/=2; i++)
		;
	return uint64_t(1) << i;
}
