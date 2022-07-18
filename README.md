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

SIMD usage:
- CM256, Leopard and Wirehair provides AVX2/SSSE3/Neon64/Neon-optimized code paths
- Intel ISA-L provides AVX512/AVX2/AVX/SSSE3/Neon/SVE/VSX-optimized code paths
- FastECC provides AVX2/SSE4-optimized code paths

So far, the benchmark is single-threaded. Leopard and FastECC have built-in OpenMP support, which may be enabled in later benchmark versions.


## Results

Notes:
- 80+20 means 80 data blocks and 20 parity blocks
- Every speed is represented by the best runs among multiple experiments
- Encoding speeds are measured in terms of original data processed
- Decoding speeds are measured in terms of recovered data produced:
  - first test recovers single block, so `speed = one block size / time`
  - second test recovers as much blocks as code can do, so `speed = size of all parity blocks / time`
- Block sizes for each run were optimized to fit all data into L3 cache, but no more than 64 KB


### Results on i7-8665U (skylake running at 3.3-4.5 GHz)

CM256:

| AVX2     | Encoding all | Decoding one  | Decoding all  |
| -------: | -----------: | ------------: | ------------: |
| 200+50   |     643 MB/s |      156 MB/s |      159 MB/s |
| 50+50    |     635 MB/s |      636 MB/s |      716 MB/s |
| 80+20    |    1650 MB/s |      403 MB/s |      413 MB/s |
| 20+20    |    1606 MB/s |     1562 MB/s |     1880 MB/s |

| SSSE3    | Encoding all | Decoding one  | Decoding all  |
| -------: | -----------: | ------------: | ------------: |
| 200+50   |     336 MB/s |       84 MB/s |       86 MB/s |
| 50+50    |     346 MB/s |      339 MB/s |      352 MB/s |
| 80+20    |     882 MB/s |      212 MB/s |      219 MB/s |
| 20+20    |     892 MB/s |      866 MB/s |      892 MB/s |
