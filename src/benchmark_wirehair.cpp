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
            encoder,     // Pointer to codec from wirehair_encoder_create()
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


// Perform single operation decoding single lost block, return false if it fails
bool wirehair_benchmark_decode_one_block(
    ECC_bench_params params,
    uint8_t* originalFileData,
    uint8_t* recoveryBlocks,
    WirehairCodec& decoder)
{
    // Create decoder
    decoder = wirehair_decoder_create(
        decoder,                     // Codec object to reuse
        params.OriginalFileBytes(),  // Bytes in the message to decode
        params.BlockBytes);          // Bytes in each encoded block

    if (!decoder) {
        printf("wirehair_decoder_create failed\n");
        return false;
    }

    auto blockSize = params.BlockBytes;


    // Simulate loss of the first data block,
    // using instead as much recovery blocks as required by the codec
    for (int blockId = 1; blockId < params.OriginalCount + params.RecoveryCount; ++blockId)
    {
        auto blockPtr = originalFileData + blockId * blockSize;

        // Attempt decode
        WirehairResult decodeResult = wirehair_decode(
            decoder,     // Pointer to codec from wirehair_decoder_create()
            blockId,     // ID number of received block
            blockPtr,    // Pointer to block data
            blockSize);  // Number of bytes in the data block

        // If decoder returns success:
        if (decodeResult == Wirehair_Success) {
            // Decoder has enough data to recover now
            goto recover;
        }

        if (decodeResult != Wirehair_NeedMore) {
            printf("wirehair_decode failed: %s\n", wirehair_result_string(decodeResult));
            return false;
        }
    }

    printf("wirehair_benchmark_decode_one_block failed: not enough data for recovery\n");
    return false;


recover:
    // Now let's recover the first data block
    auto blockId  = 0;
    auto blockPtr = originalFileData;

    uint32_t writeLen = 0;
    WirehairResult recoverResult = wirehair_recover_block(
        decoder,    // Pointer to codec from wirehair_decoder_create()
        blockId,    // ID of the block to reconstruct
        blockPtr,   // Pointer to block data
        &writeLen   // Set to the number of data bytes in the block
    );

    if (recoverResult != Wirehair_Success  ||  writeLen != blockSize) {
        printf("wirehair_recover_block failed: %s\n", wirehair_result_string(recoverResult));
        return false;
    }

    return true;
}


// Perform single operation decoding single lost block, return false if it fails
bool wirehair_benchmark_decode_all_blocks(
    ECC_bench_params params,
    uint8_t* originalFileData,
    uint8_t* recoveryBlocks,
    WirehairCodec& decoder)
{
    // Create decoder
    decoder = wirehair_decoder_create(
        decoder,                     // Codec object to reuse
        params.OriginalFileBytes(),  // Bytes in the message to decode
        params.BlockBytes);          // Bytes in each encoded block

    if (!decoder) {
        printf("wirehair_decoder_create failed\n");
        return false;
    }

    auto blockSize = params.BlockBytes;


    // Simulate loss of the first data block,
    // using instead as much recovery blocks as required by the codec
    for (int blockId = 1; blockId < params.OriginalCount + params.RecoveryCount; ++blockId)
    {
        auto blockPtr = originalFileData + blockId * blockSize;

        // Attempt decode
        WirehairResult decodeResult = wirehair_decode(
            decoder,     // Pointer to codec from wirehair_decoder_create()
            blockId,     // ID number of received block
            blockPtr,    // Pointer to block data
            blockSize);  // Number of bytes in the data block

        // If decoder returns success:
        if (decodeResult == Wirehair_Success) {
            // Decoder has enough data to recover now
            goto recover;
        }

        if (decodeResult != Wirehair_NeedMore) {
            printf("wirehair_decode failed: %s\n", wirehair_result_string(decodeResult));
            return false;
        }
    }

    printf("wirehair_benchmark_decode_all_blocks failed: not enough data for recovery\n");
    return false;


recover:
    // Now let's recover entire buffer (even if we need only the first data block)
    WirehairResult recoverResult = wirehair_recover(
        decoder,                    // Pointer to codec from wirehair_decoder_create()
        originalFileData,           // Buffer where reconstructed message will be written
        params.OriginalFileBytes()  // Bytes in the message
    );

    if (recoverResult != Wirehair_Success) {
        printf("wirehair_recover_block failed: %s\n", wirehair_result_string(recoverResult));
        return false;
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
    WirehairCodec encoder = nullptr, decoder_one = nullptr, decoder_all = nullptr;

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
        decode_one_time.BeginCall();
        if (! wirehair_benchmark_decode_one_block(params, originalFileData, recoveryBlocks, decoder_one)) {
            return false;
        }
        decode_one_time.EndCall();
        decode_all_time.BeginCall();
        if (! wirehair_benchmark_decode_all_blocks(params, originalFileData, recoveryBlocks, decoder_all)) {
            return false;
        }
        decode_all_time.EndCall();
    }

    // Benchmark reports for each operation
    encode_time.Print("encode", params.OriginalFileBytes());
    decode_one_time.Print("decode one", params.BlockBytes);
    decode_all_time.Print("decode all", params.BlockBytes);

    return true;
}
