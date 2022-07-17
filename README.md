Comparison of leading error-correcting code implementations

We plan to compare:
- O(N^2) Reed-Solomon codecs:
  - [x] [CM256](https://github.com/catid/cm256) - GF(2^8)
  - [ ] [Intel ISA-L](https://github.com/intel/isa-l) - GF(2^8)
- O(N*log(N)) Reed-Solomon codecs:
  - [ ] [Leopard](https://github.com/catid/leopard) - uses [FWHT](https://en.wikipedia.org/wiki/Fast_Walsh%E2%80%93Hadamard_transform) in GF(2^8) or GF(2^16), up to 2^16 blocks, data blocks >= parity blocks
  - [ ] [FastECC](https://github.com/Bulat-Ziganshin/FastECC) - uses FFT in GF(p), up to 2^63 blocks
- O(N) non-MDS codecs:
  - [ ] [Wirehair](https://github.com/catid/wirehair) - fountain code, up to 64000 data blocks

Also:
- CM256, Leopard and Wirehair provides AVX2/SSSE3/Neon64/Neon-optimized code paths
- Intel ISA-L provides AVX512/AVX2/AVX/SSSE3/Neon/SVE/VSX-optimized code paths
- FastECC provides AVX2/SSE4-optimized code paths

So far, the benchmark is single-threaded. Leopard and FastECC have built-in OpenMP support, which may be enabled in later benchmark versions.


## Results

Notes:
- Best runs are selected
- Encoding speeds are measured in terms of original data processed
- Block sizes for each run were optimized to fit all data into L3 cache
- 80+20 means 80 data blocks and 20 parity blocks

First results on i7-8665U (skylake running at 3.3-4.5 GHz).

CM256 (avx2), encoding:
- 200+50: 640 MB/s
- 50+50: 619 MB/s
- 80+20: 1634 MB/s
- 20+20: 1595 MB/s

CM256 (ssse3), encoding:
- 200+50: 334 MB/s
- 50+50: 338 MB/s
- 80+20: 872 MB/s
- 20+20: 872 MB/s
