#include <cstdio>
#include <thread>
#include "common.h"

#include "../unit_test/SiameseTools.cpp"

#define SSE_ALIGNMENT 16
#define align_up(value, ALIGNMENT) ((((value) + ALIGNMENT - 1) / ALIGNMENT) * ALIGNMENT)


// Parse ECC parameters from cmdline
ECC_bench_params parse_cmdline(int argc, char** argv)
{
    ECC_bench_params params;

    // Number of blocks
    params.OriginalCount = 50;

    // Number of additional recovery blocks generated by encoder
    params.RecoveryCount = 50;

    // Number of bytes per file block
    params.BlockBytes = 4096;

    // Repeat benchmark multiple times to improve its accuracy
    params.Trials = 1000;

    if (argc==1) printf("Usage: bench data_blocks parity_blocks chunk_size trials\n");
    if (argc>1)  params.OriginalCount = atoi(argv[1]);
    if (argc>2)  params.RecoveryCount = atoi(argv[2]);
    if (argc>3)  params.BlockBytes    = atoi(argv[3]);
    if (argc>4)  params.Trials        = atoi(argv[4]);

    // Round up for SSE compatibility
    params.BlockBytes = align_up(params.BlockBytes, SSE_ALIGNMENT);

    printf("Params: data_blocks=%d parity_blocks=%d chunk_size=%d trials=%d\n",
        params.OriginalCount, params.RecoveryCount, params.BlockBytes, params.Trials);

    return params;
}


// Try to seize a CPU core into exclusive use by this thread
void occupy_cpu_core()
{
    // Increase process/thread priorities to ensure repeatable results
#ifdef _WIN32
    ::SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


// Benchmark all libraries using parameters provided on cmdline
int main(int argc, char** argv)
{
    // Setup benchmark configuration based on cmdline options
    ECC_bench_params params = parse_cmdline(argc, argv);

    // Alloc single buffer large enough for any operation in any tested library
    size_t bufsize = params.OriginalFileBytes() + params.RecoveryDataBytes()
                     + leopard_extra_space(params);
    auto buffer = new uint8_t[bufsize + SSE_ALIGNMENT];

    // Align buffer start to SSE-compatible boundary
    buffer = (uint8_t*) align_up(uintptr_t(buffer), SSE_ALIGNMENT);

    // Fill place allocated for the file contents with random numbers.
    // It's critical to fill it with non-repeating data
    // since many libraries rely on table lookups
    // and can get unfair speedup on repeated data.
    for (size_t i = 0; i < params.OriginalFileBytes(); ++i) {
        buffer[i] = (uint8_t)((i*123456791) >> 13);
    }

    // Benchmark each library
    occupy_cpu_core();
    cm256_benchmark_main(params, buffer);
    leopard_benchmark_main(params, buffer);

    return 0;
}
