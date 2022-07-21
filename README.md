Comparison of leading error-correcting code implementations

We plan to compare:
- O(N^2) Reed-Solomon codecs:
  - [x] [CM256](https://github.com/catid/cm256) - GF(2^8)
  - [ ] [Intel ISA-L](https://github.com/intel/isa-l) - GF(2^8)
- O(N*log(N)) Reed-Solomon codecs:
  - [x] [Leopard](https://github.com/catid/leopard) - uses [FWHT](https://en.wikipedia.org/wiki/Fast_Walsh%E2%80%93Hadamard_transform) in GF(2^8) or GF(2^16), up to 2^16 blocks, data blocks >= parity blocks
  - [x] [FastECC](https://github.com/Bulat-Ziganshin/FastECC) - uses FFT in GF(p), up to 2^20 blocks
- O(N) non-MDS codecs:
  - [ ] [Wirehair](https://github.com/catid/wirehair) - fountain code, up to 64000 data blocks

SIMD usage:
- CM256, Leopard and Wirehair provides AVX2/SSSE3/Neon64/Neon-optimized code paths
- Intel ISA-L provides AVX512/AVX2/AVX/SSSE3/Neon/SVE/VSX-optimized code paths
- FastECC provides AVX2/SSE2-optimized code paths

So far, the benchmark is single-threaded. Leopard and FastECC have built-in OpenMP support, which may be enabled in later benchmark versions.


## Results

Notes:
- 80+20 means 80 data blocks and 20 parity blocks
- Every speed is represented by the best runs among multiple experiments
- Each program run involves multiple "trials", 1000 by default, and we compute average time of trial.
And on top of them, I run the program multiple times, choosing best result for each cell in CM256 table.
Raw results after the tables are single runs, just for quick conclusions
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

Some raw data with AVX2:
```

D:\>bench_avx2.exe 200 50 16384 100
Params: data_blocks=200 parity_blocks=50 chunk_size=16384 trials=100
CM256 (avx2, 64-bit):
  encode: 5139 usec, 638 MB/s
  decode one: 114 usec, 144 MB/s
  decode all: 4627 usec, 177 MB/s
Leopard (avx2, 64-bit):
  encode: 1245 usec, 2632 MB/s
  decode one: 4490 usec, 4 MB/s
  decode all: 4309 usec, 190 MB/s
FastECC 0xfff00001 32-bit
  encode: 6996 usec, 468 MB/s

D:\>bench_avx2.exe 50 50 16384 1000
Params: data_blocks=50 parity_blocks=50 chunk_size=16384 trials=1000
CM256 (avx2, 64-bit):
  encode: 1249 usec, 656 MB/s
  decode one: 26 usec, 639 MB/s
  decode all: 1114 usec, 736 MB/s
Leopard (avx2, 64-bit):
  encode: 206 usec, 3971 MB/s
  decode one: 566 usec, 29 MB/s
  decode all: 639 usec, 1282 MB/s
FastECC 0xfff00001 32-bit
  encode: 1245 usec, 658 MB/s

D:\>bench_avx2.exe 80 20 16384 1000
Params: data_blocks=80 parity_blocks=20 chunk_size=16384 trials=1000
CM256 (avx2, 64-bit):
  encode: 813 usec, 1612 MB/s
  decode one: 43 usec, 385 MB/s
  decode all: 717 usec, 457 MB/s
Leopard (avx2, 64-bit):
  encode: 220 usec, 5959 MB/s
  decode one: 565 usec, 29 MB/s
  decode all: 587 usec, 558 MB/s
FastECC 0xfff00001 32-bit
  encode: 3072 usec, 427 MB/s

D:\>bench_avx2.exe 20 20 65536 500
Params: data_blocks=20 parity_blocks=20 chunk_size=65536 trials=500
CM256 (avx2, 64-bit):
  encode: 819 usec, 1601 MB/s
  decode one: 42 usec, 1560 MB/s
  decode all: 833 usec, 1574 MB/s
Leopard (avx2, 64-bit):
  encode: 432 usec, 3031 MB/s
  decode one: 1105 usec, 59 MB/s
  decode all: 1116 usec, 1174 MB/s
FastECC 0xfff00001 32-bit
  encode: 2023 usec, 648 MB/s
```

and with SSSE3:
```
D:\>bench_sse4.exe 200 50 16384 100
Params: data_blocks=200 parity_blocks=50 chunk_size=16384 trials=100
CM256 (ssse3, 64-bit):
  encode: 9877 usec, 332 MB/s
  decode one: 201 usec, 82 MB/s
  decode all: 9737 usec, 84 MB/s
Leopard (ssse3, 64-bit):
  encode: 2154 usec, 1521 MB/s
  decode one: 6282 usec, 3 MB/s
  decode all: 6242 usec, 131 MB/s
FastECC 0xfff00001 32-bit
  encode: 10497 usec, 312 MB/s

D:\>bench_sse4.exe 50 50 16384 1000
Params: data_blocks=50 parity_blocks=50 chunk_size=16384 trials=1000
CM256 (ssse3, 64-bit):
  encode: 2501 usec, 328 MB/s
  decode one: 51 usec, 319 MB/s
  decode all: 2476 usec, 331 MB/s
Leopard (ssse3, 64-bit):
  encode: 389 usec, 2103 MB/s
  decode one: 945 usec, 17 MB/s
  decode all: 1068 usec, 767 MB/s
FastECC 0xfff00001 32-bit
  encode: 1867 usec, 439 MB/s

D:\>bench_sse4.exe 80 20 16384 1000
Params: data_blocks=80 parity_blocks=20 chunk_size=16384 trials=1000
CM256 (ssse3, 64-bit):
  encode: 1556 usec, 842 MB/s
  decode one: 81 usec, 203 MB/s
  decode all: 1563 usec, 210 MB/s
Leopard (ssse3, 64-bit):
  encode: 383 usec, 3426 MB/s
  decode one: 953 usec, 17 MB/s
  decode all: 979 usec, 335 MB/s
FastECC 0xfff00001 32-bit
  encode: 4659 usec, 281 MB/s

D:\>bench_sse4.exe 20 20 65536 500
Params: data_blocks=20 parity_blocks=20 chunk_size=65536 trials=500
CM256 (ssse3, 64-bit):
  encode: 1506 usec, 870 MB/s
  decode one: 77 usec, 847 MB/s
  decode all: 1500 usec, 874 MB/s
Leopard (ssse3, 64-bit):
  encode: 682 usec, 1923 MB/s
  decode one: 1734 usec, 38 MB/s
  decode all: 1835 usec, 714 MB/s
FastECC 0xfff00001 32-bit
  encode: 3066 usec, 427 MB/s

```


## Conclusions

### Recovery speed

In O(N^2) RS algos, recovery of multiple blocks is just
recovery of a single block performed multiple times.
Thus, speed per block is the same (modulo setup time).
More concrete, for K data blocks, M parity blocks,
and blocksize B, encoding time is `O(K*M*B)`.
Decoding L lost blocks will take `O(K*L*B)`
(it combines K survived blocks to recompute each lost block).

But in fast RS algos, single block recovery requires almost
the same amount of work as recovery of all lost blocks
in the worst case, since FFT+IFFT steps don't depend on
the amount of blocks we are going to recover.

Thus, matrix algorithms will always be faster for recovery
of only one or few missing blocks

We can counterfight that by using matrix computations
for small recoveries in fast algos too. At least it's
possible for FastECC. This requires computation of
Newton polynomial thus O(N^2) divisions - but it probably
is still faster than O(B\*N\*log(N)) multiplications
required for full decoding.


### Precomputed tables

ISA-L API is more low-level - you can compute encoding tables
just once and use them in multiple calls. It's especially
important when we want to process a stream with many gigabytes
using just a few megabytes of memory.

Moreover, it may be possible to use CM256-computed tables with ISA-L.
They have [two advantages](https://github.com/catid/cm256#comparisons-with-other-libraries) over ISA-L tables:
- first parity block is just XOR of all data blocks
- recovery tables are computed faster

When encoding or decoding operations with the same parameters
are repeated multiple times, it can make sense to keep cache
of such tables in order to avoid costly initialization.
The most obvious example is recovery of data of missed node
in ECC-protected distributed storage like [Codex](https://github.com/status-im/nim-codex).


### Art of benchmarking

Overall, proper benchmarking is an art of its own.
AVX usually runs at slower frequencies and have weird implementation,
this means that we better skip a first millisecond of its execution
and don't mix AVX and non-AVX code.

Mobile CPUs are tend to lower freqs on load, especially on m/t load,
and after prolonged load they may further lower freq due to overheating.
So, ideally we should skip a first few trials and then measure fastest one
(when CPU had highest freq).
But afair, cpu time measure sometimes may be incorrect when thread is switched
to another core, so we have either to pin task to a single core or drop a few outliers.


