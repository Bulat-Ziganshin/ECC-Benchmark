g++ -mavx2 -o bench_avx2 -mtune=skylake -O2 -s main.cpp cm256.cpp -I../external/cm256/include ../external/cm256/src/cm256.cpp
g++ -msse4 -o bench_sse4 -mtune=skylake -O2 -s main.cpp cm256.cpp -I../external/cm256/include ../external/cm256/src/cm256.cpp
