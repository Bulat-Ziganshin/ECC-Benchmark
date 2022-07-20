//
// Benchmarking FastECC library: https://github.com/Bulat-Ziganshin/FastECC
//
// Unfortunately, this early version of the library doesn't export ready-to-use encoder
// so we literally copied this code from RS.cpp:
//
// Implementation of the Reed-Solomon algo in O(N*log(N)) using Number-Theoretical Transform in GF(p)
//

#include <cstdio>
#include <cmath>
#include <memory>

#include "common.h"

#include "GF(p).cpp"
#include "ntt.cpp"


// Extra workspace used by the library on top of place required for original data
size_t fastecc_extra_space(ECC_bench_params params)
{
    return params.BlockBytes * params.OriginalCount;
}


// Benchmark encoding using the Reed-Solomon algo
template <typename T, T P>
void EncodeReedSolomon (size_t N, size_t SIZE, T **data)
{
    // 1. iNTT: polynomial interpolation. We find coefficients of order-N polynomial describing the source data
    MFA_NTT<T,P> (data, N, SIZE, true);
    // Now we should divide results by N in order to get coefficients, but we combined this operation with the multiplication below

    // Now we can evaluate the polynomial at 2*N points.
    // Points with even index will contain the source data,
    // while points with odd indexes may be used as ECC data.
    // But more efficient approach is to compute only odd-indexed points.
    // This is accomplished by the following steps:

    // 2. Multiply the polynomial coefficients by root(2*N)**i
    T root_2N = GF_Root<T,P>(2*N),  inv_N = GF_Inv<T,P>(N);
    #pragma omp parallel for
    for (ptrdiff_t i=0; i<N; i++) {
        T root_i = GF_Mul<T,P> (inv_N, GF_Pow<T,P>(root_2N,i));    // root_2N**i / N (combine division by N with multiplication by powers of the root)
        T* __restrict__ block = data[i];
        for (size_t k=0; k<SIZE; k++) {         // cycle over SIZE elements of the single block
            block[k] = GF_Mul<T,P> (block[k], root_i);
        }
    }

    // 3. NTT: polynomial evaluation. This evaluates the modified polynomial at root(N)**i points,
    // that is equivalent to evaluation of the original polynomial at root(2*N)**(2*i+1) points.
    MFA_NTT<T,P> (data, N, SIZE, false);

    // Further optimization: in order to compute only even-indexed points,
    // it's enough to compute order-N/2 NTT of data[i]+data[i+N/2]. And so on...
}


template <typename T, T P>
bool fastecc_benchmark_specialize(ECC_bench_params params, uint8_t* buffer)
{
    // Total encode/decode times
    OperationTimer encode_time, decode_one_time, decode_all_time;

    size_t N = NextPow2( std::max( params.OriginalCount, params.RecoveryCount));   // NTT order
    size_t SIZE = params.BlockBytes / sizeof(T);

    // Use extra space because algorithm overwrites data in-place
    T *data0 = (T*) (buffer + params.OriginalFileBytes());

    // Fill space with values < P (larger values are incompatible with FastECC algorithm)
    for (size_t i=0; i<N*SIZE; i++) {
        data0[i] = (i < P? i : i%P);
    }

    T **data = new T* [N];      // pointers to blocks
    for (size_t i=0; i<N; i++)
        data[i] = data0 + i*SIZE;

    printf("FastECC 0x%llx %d-bit\n", (unsigned long long)P, sizeof(T)*8);

    // Repeat benchmark multiple times to improve its accuracy
    for (int trial = 0; trial < params.Trials; ++trial)
    {
        // Generate recovery data
        encode_time.BeginCall();
        EncodeReedSolomon<T,P> (N, SIZE, data);
        encode_time.EndCall();
    }

    // Benchmark reports for each operation
    encode_time.Print("encode", params.OriginalFileBytes());

    return true;
}


// Benchmark library and print results, return false if anything failed
bool fastecc_benchmark_main(ECC_bench_params params, uint8_t* buffer)
{
    return fastecc_benchmark_specialize<uint32_t,0xFFF00001> (params, buffer);
}
