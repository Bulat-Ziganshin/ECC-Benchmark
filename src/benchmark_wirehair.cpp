//
// Benchmarking Wirehair library: https://github.com/catid/wirehair
//

#include <cstdio>
#include <cmath>
#include <memory>

#include "common.h"

#include "gf256.h"
#include "WirehairTools.cpp"
#include "WirehairCodec.cpp"
#include "wirehair.cpp"


// Perform single encoding operation, return false if it fails
bool wirehair_benchmark_encode(
    ECC_bench_params params,
    uint8_t* originalFileData,
    uint8_t* recoveryBlocks,
    WirehairCodec& encoder)
{
    // Create encoder
    encoder = wirehair_encoder_create(
        encoder,                     // [Optional] Pointer to prior codec object
        originalFileData,            // Pointer to message
        params.OriginalFileBytes(),  // Bytes in the message
        params.BlockBytes);          // Bytes in an output block

    if (!encoder) {
        printf("wirehair_encoder_create failed\n");
        return false;
    }

    // Generate recovery data
    for (int i = 0; i < params.RecoveryCount; ++i)
    {
        auto blockId   = i + params.OriginalCount;
        auto blockSize = params.BlockBytes;
        auto blockPtr  = recoveryBlocks + i * blockSize;

        // Encode a packet
        uint32_t writeLen = 0;
        WirehairResult encodeResult = wirehair_encode(
            encoder,     // Pointer to codec from wirehair_encoder_init()
            blockId,     // Identifier of block to generate
            blockPtr,    // Pointer to output block data
            blockSize,   // Bytes in the output buffer
            &writeLen);  // Number of bytes written <= blockBytes

        if (encodeResult != Wirehair_Success  ||  writeLen != blockSize)
        {
            printf("wirehair_encode failed: %s\n", wirehair_result_string(encodeResult));
            return false;
        }
    }

    return true;
}


// Benchmark library and print results, return false if anything failed
bool wirehair_benchmark_main(ECC_bench_params params, uint8_t* buffer)
{
    // Initialize the library
    const WirehairResult initResult = wirehair_init();
    if (initResult != Wirehair_Success) {
        printf("wirehair_init failed: %s\n", wirehair_result_string(initResult));
        return false;
    }
    WirehairCodec encoder = nullptr;

    // Introduce himself
    printf("Wirehair (%d-bit):\n", sizeof(size_t)*8);

    // Places for original and parity data
    auto originalFileData = buffer;
    auto recoveryBlocks   = buffer + params.OriginalFileBytes();

    // Total encode/decode times
    OperationTimer encode_time, decode_one_time, decode_all_time;

    // Repeat benchmark multiple times to improve its accuracy
    for (int trial = 0; trial < params.Trials; ++trial)
    {
        encode_time.BeginCall();
        if (! wirehair_benchmark_encode(params, originalFileData, recoveryBlocks, encoder)) {
            return false;
        }
        encode_time.EndCall();
    }

    // Benchmark reports for each operation
    encode_time.Print("encode", params.OriginalFileBytes());

    return true;
}
