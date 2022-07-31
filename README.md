Comparison of leading error-correcting code implementations

We plan to compare:
- O(N^2) Reed-Solomon codecs:
  - [x] [CM256](https://github.com/catid/cm256) - GF(2^8)
  - [ ] [Intel ISA-L](https://github.com/intel/isa-l) - GF(2^8)
- O(N*log(N)) Reed-Solomon codecs:
  - [x] [Leopard](https://github.com/catid/leopard) - uses [FWHT](https://en.wikipedia.org/wiki/Fast_Walsh%E2%80%93Hadamard_transform) in GF(2^8) or GF(2^16), up to 2^16 blocks, data blocks >= parity blocks
  - [x] [FastECC](https://github.com/Bulat-Ziganshin/FastECC) - uses FFT in GF(p), up to 2^20 blocks
- O(N) non-MDS codec:
  - [x] [Wirehair](https://github.com/catid/wirehair) - fountain code, up to 64000 data blocks

SIMD usage:
- CM256, Leopard and Wirehair provides AVX2/SSSE3/Neon64/Neon-optimized code paths
- Intel ISA-L provides AVX512/AVX2/AVX/SSSE3/Neon/SVE/VSX-optimized code paths
- FastECC provides AVX2/SSE2-optimized code paths

So far, the benchmark is single-threaded. Leopard and FastECC have built-in OpenMP support, which may be enabled by adding `-fopenmp` to the compilation commands.


## Results

Notes:
- 80+20 means 80 data blocks and 20 parity blocks
- Encoding speeds are measured in terms of original data processed
- Decoding speeds are measured in terms of recovered data produced:
  - first test recovers single block, so `speed = one block size / time`
  - second test recovers as much blocks as code can do, so `speed = size of all parity blocks / time`
- Each program run involves multiple "trials", 1000 by default, and we compute average time of trial
  - Formatted results are represented by the best runs among multiple experiments
  - Raw results are the single runs, just for quick comparison
- Block sizes for each run were optimized to fit all data into L3 cache, but fixed to 4 KB for large codewords
- Benchmark CPU is i7-8665U (4C/8T Skylake running at 3.3-4.5 GHz)


### Formatted results of CM256

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


### Raw results with AVX2

```
D:\>bench_avx2 200 50 16384 100
Params: data_blocks=200 parity_blocks=50 chunk_size=16384 trials=100
CM256 (avx2, 64-bit):
  encode: 5219 usec, 628 MB/s
  decode one: 109 usec, 151 MB/s
  decode all: 4677 usec, 175 MB/s
Leopard (avx2, 64-bit):
  encode: 1377 usec, 2379 MB/s
  decode one: 4574 usec, 4 MB/s
  decode all: 4401 usec, 186 MB/s
FastECC 0xfff00001 32-bit
  encode: 7129 usec, 460 MB/s
Wirehair (64-bit):
  encode: 2272 usec, 1443 MB/s
  decode one: 2506 usec, 7 MB/s
  decode all: 3061 usec, 268 MB/s

D:\>bench_avx2 50 50 16384 1000
Params: data_blocks=50 parity_blocks=50 chunk_size=16384 trials=1000
CM256 (avx2, 64-bit):
  encode: 1306 usec, 627 MB/s
  decode one: 27 usec, 606 MB/s
  decode all: 1182 usec, 693 MB/s
Leopard (avx2, 64-bit):
  encode: 228 usec, 3593 MB/s
  decode one: 599 usec, 27 MB/s
  decode all: 673 usec, 1218 MB/s
FastECC 0xfff00001 32-bit
  encode: 1324 usec, 619 MB/s
Wirehair (64-bit):
  encode: 603 usec, 1360 MB/s
  decode one: 527 usec, 31 MB/s
  decode all: 678 usec, 1209 MB/s

D:\>bench_avx2 80 20 16384 1000
Params: data_blocks=80 parity_blocks=20 chunk_size=16384 trials=1000
CM256 (avx2, 64-bit):
  encode: 851 usec, 1540 MB/s
  decode one: 44 usec, 370 MB/s
  decode all: 755 usec, 434 MB/s
Leopard (avx2, 64-bit):
  encode: 239 usec, 5485 MB/s
  decode one: 594 usec, 28 MB/s
  decode all: 620 usec, 529 MB/s
FastECC 0xfff00001 32-bit
  encode: 3227 usec, 406 MB/s
Wirehair (64-bit):
  encode: 977 usec, 1342 MB/s
  decode one: 1069 usec, 15 MB/s
  decode all: 1225 usec, 268 MB/s

D:\>bench_avx2 20 20 65536 500
Params: data_blocks=20 parity_blocks=20 chunk_size=65536 trials=500
CM256 (avx2, 64-bit):
  encode: 1230 usec, 1066 MB/s
  decode one: 62 usec, 1053 MB/s
  decode all: 1238 usec, 1059 MB/s
Leopard (avx2, 64-bit):
  encode: 586 usec, 2235 MB/s
  decode one: 1536 usec, 43 MB/s
  decode all: 1571 usec, 834 MB/s
FastECC 0xfff00001 32-bit
  encode: 2378 usec, 551 MB/s
Wirehair (64-bit):
  encode: 1643 usec, 798 MB/s
  decode one: 1579 usec, 41 MB/s
  decode all: 1808 usec, 725 MB/s
```


### Raw results with SSSE3

```
D:\>bench_sse4 200 50 16384 100
Params: data_blocks=200 parity_blocks=50 chunk_size=16384 trials=100
CM256 (ssse3, 64-bit):
  encode: 11353 usec, 289 MB/s
  decode one: 233 usec, 70 MB/s
  decode all: 11088 usec, 74 MB/s
Leopard (ssse3, 64-bit):
  encode: 2558 usec, 1281 MB/s
  decode one: 7345 usec, 2 MB/s
  decode all: 7490 usec, 109 MB/s
FastECC 0xfff00001 32-bit
  encode: 11768 usec, 278 MB/s
Wirehair (64-bit):
  encode: 2955 usec, 1109 MB/s
  decode one: 3164 usec, 5 MB/s
  decode all: 3615 usec, 227 MB/s

D:\>bench_sse4 50 50 16384 1000
Params: data_blocks=50 parity_blocks=50 chunk_size=16384 trials=1000
CM256 (ssse3, 64-bit):
  encode: 2731 usec, 300 MB/s
  decode one: 56 usec, 292 MB/s
  decode all: 2719 usec, 301 MB/s
Leopard (ssse3, 64-bit):
  encode: 460 usec, 1781 MB/s
  decode one: 1131 usec, 14 MB/s
  decode all: 1268 usec, 646 MB/s
FastECC 0xfff00001 32-bit
  encode: 2123 usec, 386 MB/s
Wirehair (64-bit):
  encode: 970 usec, 844 MB/s
  decode one: 828 usec, 20 MB/s
  decode all: 1051 usec, 780 MB/s

D:\>bench_sse4 80 20 16384 1000
Params: data_blocks=80 parity_blocks=20 chunk_size=16384 trials=1000
CM256 (ssse3, 64-bit):
  encode: 1689 usec, 776 MB/s
  decode one: 88 usec, 187 MB/s
  decode all: 1699 usec, 193 MB/s
Leopard (ssse3, 64-bit):
  encode: 436 usec, 3006 MB/s
  decode one: 1115 usec, 15 MB/s
  decode all: 1152 usec, 284 MB/s
FastECC 0xfff00001 32-bit
  encode: 4840 usec, 271 MB/s
Wirehair (64-bit):
  encode: 1192 usec, 1100 MB/s
  decode one: 1275 usec, 13 MB/s
  decode all: 1408 usec, 233 MB/s

D:\>bench_sse4 20 20 65536 500
Params: data_blocks=20 parity_blocks=20 chunk_size=65536 trials=500
CM256 (ssse3, 64-bit):
  encode: 1872 usec, 700 MB/s
  decode one: 97 usec, 674 MB/s
  decode all: 1864 usec, 703 MB/s
Leopard (ssse3, 64-bit):
  encode: 866 usec, 1514 MB/s
  decode one: 2250 usec, 29 MB/s
  decode all: 2377 usec, 551 MB/s
FastECC 0xfff00001 32-bit
  encode: 3749 usec, 350 MB/s
Wirehair (64-bit):
  encode: 2267 usec, 578 MB/s
  decode one: 2087 usec, 31 MB/s
  decode all: 2341 usec, 560 MB/s
```


### Raw results for larger codewords

```
D:\>bench_avx2 2048 2048 4096 100
Params: data_blocks=2048 parity_blocks=2048 chunk_size=4096 trials=100
Leopard (avx2, 64-bit):
  encode: 8612 usec, 974 MB/s
  decode one: 18663 usec, 0 MB/s
  decode all: 21211 usec, 395 MB/s
FastECC 0xfff00001 32-bit
  encode: 23819 usec, 352 MB/s
Wirehair (64-bit):
  encode: 8301 usec, 1011 MB/s
  decode one: 6920 usec, 1 MB/s
  decode all: 9668 usec, 868 MB/s

D:\>bench_avx2 32000 32000 4096 20
Params: data_blocks=32000 parity_blocks=32000 chunk_size=4096 trials=20
Leopard (avx2, 64-bit):
  encode: 216624 usec, 605 MB/s
  decode one: 427401 usec, 0 MB/s
  decode all: 515774 usec, 254 MB/s
FastECC 0xfff00001 32-bit
  encode: 584607 usec, 224 MB/s
Wirehair (64-bit):
  encode: 245237 usec, 534 MB/s
  decode one: 197916 usec, 0 MB/s
  decode all: 272011 usec, 482 MB/s
```

Now the same with OpenMP:
```
D:\>bench_avx2_openmp 2048 2048 4096 100
Params: data_blocks=2048 parity_blocks=2048 chunk_size=4096 trials=100
Leopard (avx2, 64-bit):
  encode: 6204 usec, 1352 MB/s
  decode one: 36027 usec, 0 MB/s
  decode all: 37741 usec, 222 MB/s
FastECC 0xfff00001 32-bit
  encode: 7182 usec, 1168 MB/s
Wirehair (64-bit):
  encode: 8446 usec, 993 MB/s
  decode one: 6943 usec, 1 MB/s
  decode all: 9709 usec, 864 MB/s

D:\>bench_avx2_openmp 32000 32000 4096 20
Params: data_blocks=32000 parity_blocks=32000 chunk_size=4096 trials=20
Leopard (avx2, 64-bit):
  encode: 206935 usec, 633 MB/s
  decode one: 880574 usec, 0 MB/s
  decode all: 963278 usec, 136 MB/s
FastECC 0xfff00001 32-bit
  encode: 209445 usec, 626 MB/s
Wirehair (64-bit):
  encode: 257553 usec, 509 MB/s
  decode one: 202161 usec, 0 MB/s
  decode all: 284040 usec, 461 MB/s
```


## Conclusions

### Encoding speed

O(N^2) algorithms encoding speed reported in THIS benchmark
is O(1/number_of_parity_words). It's why:

So-called O(N^2) algorithms really are `O(M*K)`.
It's because the RS matrix algo multiples vector of M words (input data)
by `K*M` matrix and gets vector of K words (parity),
which requires `K*M` multiplications and additions.

When you have any `O(K*M)` algo with M input words and K output words,
you can say that its speed is O(1/K) relative to input data processed
or O(1/M) relative to output data produced :slight_smile:
This benchmark reports encoding speed relative to input data size,
so matrix RS algos speed is O(1/number_of_parity_words)

As of cache effects, I optimized the chunk size for each M+K setting
to reach best results. For larger codewords it means smaller chunks
and thus a bit higher overheads, but effect was within 1%
(i.e. for 20+20 I used 64KB blocks, but even with 4KB blocks
it will be only 10% slower)


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


