#ifndef INC_CRYPT_H_
#define INC_CRYPT_H_
/*
# License
This software is distributed under two licenses, choose whichever you like.

## MIT License
Copyright (c) 2025 Takuro Sakai

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Public Domain
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/
#include <array>
#include <cstdint>
#ifdef _DEBUG
#    include <iostream>
#endif
#include <bit>

namespace cthash
{

struct sha256_state
{
    // Round constants
    static constexpr uint32_t K[64] = {
        0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL,
        0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL,
        0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL,
        0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
        0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 0x983e5152UL,
        0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
        0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL,
        0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
        0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL,
        0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 0x19a4c116UL, 0x1e376c08UL,
        0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL,
        0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
        0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL};
    uint64_t length_;
    uint64_t bitlen_;
    uint32_t state_[8];
    uint8_t data_[64];

    static constexpr uint32_t choose(uint32_t e, uint32_t f, uint32_t g)
    {
        return (e & f) ^ (~e & g);
    }

    static constexpr uint32_t majority(uint32_t a, uint32_t b, uint32_t c)
    {
        return (a & (b | c)) | (b & c);
    }

    static constexpr uint32_t rotr(uint32_t x, uint32_t n)
    {
        return (x >> n) | (x << (32 - n));
    }

    static constexpr uint32_t sigma0(uint32_t x)
    {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }

    static constexpr uint32_t sigma1(uint32_t x)
    {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }

    constexpr void compress()
    {
        uint32_t m[64];
        // Copy block into first 16 words m[0..15] of the message schedule array
        for(int32_t i = 0, j = 0; i < 16; ++i, j += 4) {
            m[i] = (data_[j] << 24) | (data_[j + 1] << 16) | (data_[j + 2] << 8) | (data_[j + 3]);
        }

        // Extend the first 16 words into the remaining 48 words m[16..63] of the message schedule array
        for(int32_t i = 16; i < 64; ++i) {
            m[i] = sigma1(m[i - 2]) + m[i - 7] + sigma0(m[i - 15]) + m[i - 16];
        }

        uint32_t state[8];
        for(int32_t i = 0; i < 8; ++i) {
            state[i] = state_[i];
        }

        for(int32_t i = 0; i < 64; ++i) {
            uint32_t s1 = rotr(state[4], 6) ^ rotr(state[4], 11) ^ rotr(state[4], 25);
            uint32_t ch = choose(state[4], state[5], state[6]);
            uint32_t temp1 = m[i] + K[i] + state[7] + ch + s1;
            uint32_t s0 = rotr(state[0], 2) ^ rotr(state[0], 13) ^ rotr(state[0], 22);
            uint32_t maj = majority(state[0], state[1], state[2]);
            uint32_t temp2 = s0 + maj;

            state[7] = state[6];
            state[6] = state[5];
            state[5] = state[4];
            state[4] = state[3] + temp1;
            state[3] = state[2];
            state[2] = state[1];
            state[1] = state[0];
            state[0] = temp1 + temp2;
        }

        // Add the compressed chunk to the current hash value
        for(int32_t i = 0; i < 8; ++i) {
            state_[i] += state[i];
        }
    }

    constexpr void init()
    {
        length_ = 0;
        bitlen_ = 0;
        state_[0] = 0x6A09E667UL;
        state_[1] = 0xBB67AE85UL;
        state_[2] = 0x3C6EF372UL;
        state_[3] = 0xA54FF53AUL;
        state_[4] = 0x510E527FUL;
        state_[5] = 0x9B05688CUL;
        state_[6] = 0x1F83D9ABUL;
        state_[7] = 0x5BE0CD19UL;
    }

    constexpr void update(size_t size, const uint8_t* src)
    {
        const size_t block_size = 64;
        const uint8_t* data = src;

        for(size_t i = 0; i < size; ++i) {
            data_[length_++] = data[i];
            if(block_size == length_) {
                compress();
                // End of the block
                bitlen_ += 8 * block_size;
                length_ = 0;
            }
        }
    }

    constexpr void memset(uint8_t* mem, uint8_t x, size_t size)
    {
        for(size_t i = 0; i < size; ++i) {
            mem[i] = x;
        }
    }

    constexpr std::array<uint8_t, 32> finalize()
    {
        uint64_t l = length_;
        uint8_t end = length_ < 56 ? 56 : 64;

        data_[l++] = 0x80; // Append a bit 1
        while(l < end) {
            data_[l++] = 0x00U; // Pad with zeros
        }

        if(56 <= length_) {
            compress();
            sha256_state::memset(data_, 0, 56);
        }

        // Append to the padding the total message's length in bits and compress.
        bitlen_ += length_ * 8;
        data_[63] = static_cast<uint8_t>(bitlen_);
        data_[62] = static_cast<uint8_t>(bitlen_ >> 8);
        data_[61] = static_cast<uint8_t>(bitlen_ >> 16);
        data_[60] = static_cast<uint8_t>(bitlen_ >> 24);
        data_[59] = static_cast<uint8_t>(bitlen_ >> 32);
        data_[58] = static_cast<uint8_t>(bitlen_ >> 40);
        data_[57] = static_cast<uint8_t>(bitlen_ >> 48);
        data_[56] = static_cast<uint8_t>(bitlen_ >> 56);
        compress();

        // Produces hash in the big-endian order
        std::array<uint8_t, 32> hash;
        for(uint8_t i = 0; i < 4; ++i) {
            for(uint8_t j = 0; j < 8; ++j) {
                hash[i + (j * 4)] = (state_[j] >> (24 - i * 8)) & 0x000000ffU;
            }
        }
        return hash;
    }
};

template<size_t N>
constexpr std::array<uint8_t, 32> sha256_string(const uint8_t (&src)[N])
{
    static_assert(0 < N);
    sha256_state state;
    state.init();
    // Do not include the null terminator in the hash
    state.update(N - 1, src);
    return state.finalize();
}

template<size_t N>
constexpr std::array<uint8_t, 32> sha256_data(const uint8_t (&src)[N])
{
    static_assert(0 < N);
    sha256_state state;
    state.init();
    state.update(N, src);
    return state.finalize();
}

#define CHACHA_ROTL(a, b) (((a) << (b)) | ((a) >> (32 - (b))))
#define CHACHA_QR(a, b, c, d) ( \
    a += b, d ^= a, d = CHACHA_ROTL(d, 16), \
    c += d, b ^= c, b = CHACHA_ROTL(b, 12), \
    a += b, d ^= a, d = CHACHA_ROTL(d, 8), \
    c += d, b ^= c, b = CHACHA_ROTL(b, 7))

constexpr void chacha20_block(uint32_t out[16], const uint32_t in[16])
{
    uint32_t x[16];
    for(int32_t i = 0; i < 16; ++i) {
        x[i] = in[i];
    }
    for(int32_t i = 0; i < 10; ++i) {
        // Odd round
        CHACHA_QR(x[0], x[4], x[8], x[12]);  // Column 0
        CHACHA_QR(x[1], x[5], x[9], x[13]);  // Column 1
        CHACHA_QR(x[2], x[6], x[10], x[14]); // Column 2
        CHACHA_QR(x[3], x[7], x[11], x[15]); // Column 3
        // Even round
        CHACHA_QR(x[0], x[5], x[10], x[15]); // Diagonal 0
        CHACHA_QR(x[1], x[6], x[11], x[12]); // Diagonal 1
        CHACHA_QR(x[2], x[7], x[8], x[13]);  // Diagonal 2
        CHACHA_QR(x[3], x[4], x[9], x[14]);  // Diagonal 3
    }
    for(int32_t i = 0; i < 16; ++i) {
        out[i] = x[i] + in[i];
    }
}

constexpr uint64_t to_uint64(uint32_t low, uint32_t high)
{
    return (static_cast<uint64_t>(high) << 32) | static_cast<uint64_t>(low);
}

constexpr void to_uint32(uint32_t& low, uint32_t& high, uint64_t x)
{
    low = static_cast<uint32_t>(x & 0xFFFFFFFFUL);
    high = static_cast<uint32_t>((x >> 32) & 0xFFFFFFFFUL);
}

constexpr void chacha20_encrypt(uint32_t input[16], uint32_t size, uint8_t* message)
{
    uint32_t output[16] = {};
    uint8_t* p = message;
    uint64_t counter = to_uint64(input[12], input[13]);
    for(;;) {
        chacha20_block(output, input);
        counter += 1;
        if(size <= 64) {
            uint8_t* pb = p;
            uint32_t* po = output;
            uint32_t blocks = size >> 2;
            uint32_t count = blocks << 2;
            uint32_t remain = size - count;

            for(uint32_t i = 0, j = 0; j < count; ++i, j += 4) {
                pb[j + 0] = pb[j + 0] ^ ((po[i] >> 0) & 0xFFU);
                pb[j + 1] = pb[j + 1] ^ ((po[i] >> 8) & 0xFFU);
                pb[j + 2] = pb[j + 2] ^ ((po[i] >> 16) & 0xFFU);
                pb[j + 3] = pb[j + 3] ^ ((po[i] >> 24) & 0xFFU);
            }

            switch(remain) {
            case 1: {
                pb[count + 0] = pb[count + 0] ^ ((po[blocks] >> 0) & 0xFFU);
            } break;
            case 2: {
                pb[count + 0] = pb[count + 0] ^ ((po[blocks] >> 0) & 0xFFU);
                pb[count + 1] = pb[count + 1] ^ ((po[blocks] >> 8) & 0xFFU);
            } break;
            case 3: {
                pb[count + 0] = pb[count + 0] ^ ((po[blocks] >> 0) & 0xFFU);
                pb[count + 1] = pb[count + 1] ^ ((po[blocks] >> 8) & 0xFFU);
                pb[count + 2] = pb[count + 2] ^ ((po[blocks] >> 16) & 0xFFU);
            } break;
            }
            break;
        } else {
            for(uint32_t i = 0, j = 0; i < 16; ++i, j += 4) {
                p[j + 0] ^= ((output[i] >> 0) & 0xFFU);
                p[j + 1] ^= ((output[i] >> 8) & 0xFFU);
                p[j + 2] ^= ((output[i] >> 16) & 0xFFU);
                p[j + 3] ^= ((output[i] >> 24) & 0xFFU);
            }
            size -= 64;
            p += (16 * 4);
        }
    }
    to_uint32(input[12], input[13], counter);
}

static constexpr uint32_t chacha20_iv_32[4] = {0x61707865U, 0x3320646eU, 0x79622d32U, 0x6b206574U};
static constexpr uint32_t chacha20_nonce[2] = {0x55783743U, 0x421d5279U};

constexpr void chacha20_encrypt(const std::array<uint8_t, 32>& key, uint32_t size, uint8_t* message)
{
    uint32_t input[16];

    for(int32_t i = 0; i < 4; ++i) {
        input[i] = chacha20_iv_32[i];
    }

    for(int32_t i = 0, j = 0; i < 8; ++i, j += 4) {
        input[4 + i] = (key[j + 0] << 24) | (key[j + 1] << 16) | (key[j + 2] << 8) | (key[j + 3]);
    }

    input[12] = 0;
    input[13] = 0;
    input[14] = chacha20_nonce[0];
    input[15] = chacha20_nonce[1];
    chacha20_encrypt(input, size, message);
}

template<size_t N>
constexpr std::array<uint8_t, N> chacha20_encrypt(const std::array<uint8_t, 32>& key, const std::array<uint8_t, N>& src)
{
    std::array<uint8_t, N> message = src;
    chacha20_encrypt(key, N, &message[0]);
    return message;
}

template<size_t N>
constexpr std::array<uint8_t, N> chacha20_encrypt(const std::array<uint8_t, 32>& key, const uint8_t (&src)[N])
{
    uint32_t input[16];

    for(int32_t i = 0; i < 4; ++i) {
        input[i] = chacha20_iv_32[i];
    }

    for(int32_t i = 0, j = 0; i < 8; ++i, j += 4) {
        input[4 + i] = (key[j + 0] << 24) | (key[j + 1] << 16) | (key[j + 2] << 8) | (key[j + 3]);
    }

    input[12] = 0;
    input[13] = 0;
    input[14] = chacha20_nonce[0];
    input[15] = chacha20_nonce[1];
    std::array<uint8_t, N> message;
    std::copy(src, src + N, message.begin());
    chacha20_encrypt(input, N, &message[0]);
    return message;
}

//--- BLAKE3
static constexpr uint32_t BLAKE3_KEY_LEN = 32;
static constexpr uint32_t BLAKE3_OUT_LEN = 32;
static constexpr uint32_t BLAKE3_BLOCK_LEN = 64;
static constexpr uint32_t BLAKE3_CHUNK_LEN = 1024;
static constexpr uint32_t BLAKE3_MAX_DEPTH = 54;

static constexpr uint32_t MAX_SIMD_DEGREE = 16;
static constexpr uint32_t MAX_SIMD_DEGREE_OR_2 = 2;

static constexpr uint32_t IV[8] = {0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL,
                               0xA54FF53AUL, 0x510E527FUL, 0x9B05688CUL,
                               0x1F83D9ABUL, 0x5BE0CD19UL};

static constexpr uint8_t MSG_SCHEDULE[7][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {2, 6, 3, 10, 7, 0, 4, 13, 1, 11, 12, 5, 9, 14, 15, 8},
    {3, 4, 10, 12, 13, 2, 7, 14, 6, 5, 9, 0, 11, 15, 8, 1},
    {10, 7, 12, 9, 14, 3, 13, 15, 4, 0, 11, 2, 5, 8, 1, 6},
    {12, 13, 9, 11, 15, 10, 14, 8, 7, 2, 5, 3, 0, 1, 6, 4},
    {9, 14, 11, 5, 8, 12, 15, 1, 13, 3, 0, 10, 2, 6, 4, 7},
    {11, 15, 5, 0, 1, 9, 8, 6, 14, 10, 2, 12, 3, 4, 7, 13},
};

enum blake3_flags
{
    CHUNK_START = 1 << 0,
    CHUNK_END = 1 << 1,
    PARENT = 1 << 2,
    ROOT = 1 << 3,
    KEYED_HASH = 1 << 4,
    DERIVE_KEY_CONTEXT = 1 << 5,
    DERIVE_KEY_MATERIAL = 1 << 6,
};

    // This struct is a private implementation detail. It has to be here because
// it's part of blake3_hasher below.
struct blake3_chunk_state
{
    uint32_t cv[8];
    uint64_t chunk_counter;
    uint8_t buf[BLAKE3_BLOCK_LEN];
    uint8_t buf_len;
    uint8_t blocks_compressed;
    uint8_t flags;
};

struct blake3_hasher
{
    uint32_t key[8];
    blake3_chunk_state chunk;
    uint8_t cv_stack_len;
    // The stack size is MAX_DEPTH + 1 because we do lazy merging. For example,
    // with 7 chunks, we have 3 entries in the stack. Adding an 8th chunk
    // requires a 4th entry, rather than merging everything down to 1, because we
    // don't know whether more input is coming. This is different from how the
    // reference implementation does things.
    uint8_t cv_stack[(BLAKE3_MAX_DEPTH + 1) * BLAKE3_OUT_LEN];
};

constexpr uint16_t load16(const uint8_t* x)
{
    return (((uint16_t)x[0]) << 0)
    | (((uint16_t)x[1])<<8);
}

constexpr uint32_t load32(const uint8_t* x)
{
    return (((uint32_t)x[0])<<0)
    | (((uint32_t)x[1])<<8)
    | (((uint32_t)x[2])<<16)
           | (((uint32_t)x[3]) << 24);
}

constexpr uint64_t load64(const uint8_t* x)
{
    return (((uint64_t)x[0])<<0)
    | (((uint64_t)x[1])<<8)
    | (((uint64_t)x[2])<<16)
    | (((uint64_t)x[3])<<24)
    | (((uint64_t)x[4])<<32)
    | (((uint64_t)x[5])<<40)
    | (((uint64_t)x[6])<<48)
           | (((uint64_t)x[7]) << 56);
}

constexpr void store16(uint8_t* dst, uint16_t x)
{
    dst[0] = (uint8_t)(x>>0);
    dst[1] = (uint8_t)(x>>8);
}

constexpr void store32(uint8_t* dst, uint32_t x)
{
    dst[0] = (uint8_t)(x >> 0);
    dst[1] = (uint8_t)(x >> 8);
    dst[2] = (uint8_t)(x >> 16);
    dst[3] = (uint8_t)(x >> 24);
}

constexpr void store64(uint8_t* dst, uint64_t x)
{
    dst[0] = (uint8_t)(x >> 0);
    dst[1] = (uint8_t)(x >> 8);
    dst[2] = (uint8_t)(x >> 16);
    dst[3] = (uint8_t)(x >> 24);
    dst[4] = (uint8_t)(x >> 32);
    dst[5] = (uint8_t)(x >> 40);
    dst[6] = (uint8_t)(x >> 48);
    dst[7] = (uint8_t)(x >> 56);
}

constexpr uint32_t rotr32(const uint32_t w, const unsigned c)
{
    return (w >> c) | (w << (32 - c));
}

constexpr uint64_t rotr64(const uint64_t w, const unsigned c)
{
    return (w >> c) | (w << (64 - c));
}

constexpr uint32_t highest_one(uint64_t x)
{
    uint32_t c = 0;
    if(x & 0xffffffff00000000ULL) {
        x >>= 32;
        c += 32;
    }
    if(x & 0x00000000ffff0000ULL) {
        x >>= 16;
        c += 16;
    }
    if(x & 0x000000000000ff00ULL) {
        x >>= 8;
        c += 8;
    }
    if(x & 0x00000000000000f0ULL) {
        x >>= 4;
        c += 4;
    }
    if(x & 0x000000000000000cULL) {
        x >>= 2;
        c += 2;
    }
    if(x & 0x0000000000000002ULL) {
        c += 1;
    }
    return c;
}

constexpr uint32_t popcnt(uint64_t x)
{
    uint32_t count = 0;
    while(x != 0) {
        count += 1;
        x &= x - 1;
    }
    return count;
}

constexpr void memcpy(uint8_t* dst, const uint8_t* src, size_t size)
{
    for(size_t i=0; i<size; ++i){
        dst[i] = src[i];
    }
}

constexpr void memcpy(uint32_t* dst, const uint32_t* src, size_t size)
{
    for(size_t i = 0; i < size; ++i) {
        dst[i] = src[i];
    }
}

constexpr void memset(uint8_t* dst, uint8_t x, size_t size)
{
    for(size_t i = 0; i < size; ++i) {
        dst[i] = x;
    }
}

constexpr uint64_t round_down_to_power_of_2(uint64_t x)
{
    return 1ULL << highest_one(x | 1);
}

constexpr uint32_t counter_low(uint64_t counter)
{
    return (uint32_t)counter;
}

constexpr uint32_t counter_high(uint64_t counter)
{
    return (uint32_t)(counter >> 32);
}

constexpr void store_cv_words(uint8_t bytes_out[32], uint32_t cv_words[8])
{
    store32(&bytes_out[0 * 4], cv_words[0]);
    store32(&bytes_out[1 * 4], cv_words[1]);
    store32(&bytes_out[2 * 4], cv_words[2]);
    store32(&bytes_out[3 * 4], cv_words[3]);
    store32(&bytes_out[4 * 4], cv_words[4]);
    store32(&bytes_out[5 * 4], cv_words[5]);
    store32(&bytes_out[6 * 4], cv_words[6]);
    store32(&bytes_out[7 * 4], cv_words[7]);
}

constexpr void g(uint32_t* state, size_t a, size_t b, size_t c, size_t d, uint32_t x, uint32_t y)
{
    state[a] = state[a] + state[b] + x;
    state[d] = rotr32(state[d] ^ state[a], 16);
    state[c] = state[c] + state[d];
    state[b] = rotr32(state[b] ^ state[c], 12);
    state[a] = state[a] + state[b] + y;
    state[d] = rotr32(state[d] ^ state[a], 8);
    state[c] = state[c] + state[d];
    state[b] = rotr32(state[b] ^ state[c], 7);
}

constexpr void round_fn(uint32_t state[16], const uint32_t* msg, size_t round)
{
    // Select the message schedule based on the round.
    const uint8_t* schedule = MSG_SCHEDULE[round];

    // Mix the columns.
    g(state, 0, 4, 8, 12, msg[schedule[0]], msg[schedule[1]]);
    g(state, 1, 5, 9, 13, msg[schedule[2]], msg[schedule[3]]);
    g(state, 2, 6, 10, 14, msg[schedule[4]], msg[schedule[5]]);
    g(state, 3, 7, 11, 15, msg[schedule[6]], msg[schedule[7]]);

    // Mix the rows.
    g(state, 0, 5, 10, 15, msg[schedule[8]], msg[schedule[9]]);
    g(state, 1, 6, 11, 12, msg[schedule[10]], msg[schedule[11]]);
    g(state, 2, 7, 8, 13, msg[schedule[12]], msg[schedule[13]]);
    g(state, 3, 4, 9, 14, msg[schedule[14]], msg[schedule[15]]);
}

constexpr void compress_pre(uint32_t state[16], const uint32_t cv[8],
                         const uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags)
{
    uint32_t block_words[16];
    block_words[0] = load32(block + 4 * 0);
    block_words[1] = load32(block + 4 * 1);
    block_words[2] = load32(block + 4 * 2);
    block_words[3] = load32(block + 4 * 3);
    block_words[4] = load32(block + 4 * 4);
    block_words[5] = load32(block + 4 * 5);
    block_words[6] = load32(block + 4 * 6);
    block_words[7] = load32(block + 4 * 7);
    block_words[8] = load32(block + 4 * 8);
    block_words[9] = load32(block + 4 * 9);
    block_words[10] = load32(block + 4 * 10);
    block_words[11] = load32(block + 4 * 11);
    block_words[12] = load32(block + 4 * 12);
    block_words[13] = load32(block + 4 * 13);
    block_words[14] = load32(block + 4 * 14);
    block_words[15] = load32(block + 4 * 15);

    state[0] = cv[0];
    state[1] = cv[1];
    state[2] = cv[2];
    state[3] = cv[3];
    state[4] = cv[4];
    state[5] = cv[5];
    state[6] = cv[6];
    state[7] = cv[7];
    state[8] = IV[0];
    state[9] = IV[1];
    state[10] = IV[2];
    state[11] = IV[3];
    state[12] = counter_low(counter);
    state[13] = counter_high(counter);
    state[14] = (uint32_t)block_len;
    state[15] = (uint32_t)flags;

    round_fn(state, &block_words[0], 0);
    round_fn(state, &block_words[0], 1);
    round_fn(state, &block_words[0], 2);
    round_fn(state, &block_words[0], 3);
    round_fn(state, &block_words[0], 4);
    round_fn(state, &block_words[0], 5);
    round_fn(state, &block_words[0], 6);
}

constexpr void blake3_compress_xof(const uint32_t cv[8],
                                  const uint8_t block[BLAKE3_BLOCK_LEN],
                                  uint8_t block_len, uint64_t counter,
                                  uint8_t flags, uint8_t out[64])
{
    uint32_t state[16];
    compress_pre(state, cv, block, block_len, counter, flags);

    store32(&out[0 * 4], state[0] ^ state[8]);
    store32(&out[1 * 4], state[1] ^ state[9]);
    store32(&out[2 * 4], state[2] ^ state[10]);
    store32(&out[3 * 4], state[3] ^ state[11]);
    store32(&out[4 * 4], state[4] ^ state[12]);
    store32(&out[5 * 4], state[5] ^ state[13]);
    store32(&out[6 * 4], state[6] ^ state[14]);
    store32(&out[7 * 4], state[7] ^ state[15]);
    store32(&out[8 * 4], state[8] ^ cv[0]);
    store32(&out[9 * 4], state[9] ^ cv[1]);
    store32(&out[10 * 4], state[10] ^ cv[2]);
    store32(&out[11 * 4], state[11] ^ cv[3]);
    store32(&out[12 * 4], state[12] ^ cv[4]);
    store32(&out[13 * 4], state[13] ^ cv[5]);
    store32(&out[14 * 4], state[14] ^ cv[6]);
    store32(&out[15 * 4], state[15] ^ cv[7]);
}

constexpr size_t chunk_state_len(const blake3_chunk_state* self)
{
    return (BLAKE3_BLOCK_LEN * (size_t)self->blocks_compressed) + ((size_t)self->buf_len);
}

struct output_t
{
    uint32_t input_cv[8];
    uint64_t counter;
    uint8_t block[BLAKE3_BLOCK_LEN];
    uint8_t block_len;
    uint8_t flags;
};

constexpr output_t make_output(const uint32_t input_cv[8],
                     const uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter,
                     uint8_t flags)
{
    output_t ret;
    cthash::memcpy(ret.input_cv, input_cv, 32/4);
    cthash::memcpy(ret.block, block, BLAKE3_BLOCK_LEN);
    ret.block_len = block_len;
    ret.counter = counter;
    ret.flags = flags;
    return ret;
}

constexpr void blake3_xof_many(const uint32_t cv[8],
                     const uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks)
{
    if(outblocks == 0) {
        // The current assembly implementation always outputs at least 1 block.
        return;
    }
    for(size_t i = 0; i < outblocks; ++i) {
        blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64 * i);
    }
}

constexpr void blake3_compress_in_place(uint32_t cv[8],
                              const uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags)
{
    uint32_t state[16];
    compress_pre(state, cv, block, block_len, counter, flags);
    cv[0] = state[0] ^ state[8];
    cv[1] = state[1] ^ state[9];
    cv[2] = state[2] ^ state[10];
    cv[3] = state[3] ^ state[11];
    cv[4] = state[4] ^ state[12];
    cv[5] = state[5] ^ state[13];
    cv[6] = state[6] ^ state[14];
    cv[7] = state[7] ^ state[15];
}

constexpr size_t blake3_simd_degree(void)
{
    return 1;
}

// Given some input larger than one chunk, return the number of bytes that
// should go in the left subtree. This is the largest power-of-2 number of
// chunks that leaves at least 1 byte for the right subtree.
constexpr size_t left_subtree_len(size_t input_len)
{
    // Subtract 1 to reserve at least one byte for the right side. input_len
    // should always be greater than BLAKE3_CHUNK_LEN.
    size_t full_chunks = (input_len - 1) / BLAKE3_CHUNK_LEN;
    return round_down_to_power_of_2(full_chunks) * BLAKE3_CHUNK_LEN;
}

constexpr void hash_one_portable(const uint8_t* input, size_t blocks,
                              const uint32_t key[8], uint64_t counter,
                              uint8_t flags, uint8_t flags_start,
                              uint8_t flags_end, uint8_t out[BLAKE3_OUT_LEN])
{
    uint32_t cv[8];
    cthash::memcpy(cv, key, BLAKE3_KEY_LEN/4);
    uint8_t block_flags = flags | flags_start;
    while(blocks > 0) {
        if(blocks == 1) {
            block_flags |= flags_end;
        }
        blake3_compress_in_place(cv, input, BLAKE3_BLOCK_LEN, counter,
                                          block_flags);
        input = &input[BLAKE3_BLOCK_LEN];
        blocks -= 1;
        block_flags = flags;
    }
    store_cv_words(out, cv);
}

constexpr void blake3_hash_many(const uint8_t* const* inputs, size_t num_inputs,
                      size_t blocks, const uint32_t key[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t* out)
{
    while(num_inputs > 0) {
        hash_one_portable(inputs[0], blocks, key, counter, flags, flags_start,
                          flags_end, out);
        if(increment_counter) {
            counter += 1;
        }
        inputs += 1;
        num_inputs -= 1;
        out = &out[BLAKE3_OUT_LEN];
    }
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE parents at the same time
// on a single thread. Write out the parent chaining values and return the
// number of parents hashed. (If there's an odd input chaining value left over,
// return it as an additional output.) These parents are never the root and
// never empty; those cases use a different codepath.
constexpr size_t compress_parents_parallel(const uint8_t* child_chaining_values,
                                        size_t num_chaining_values,
                                        const uint32_t key[8], uint8_t flags,
                                        uint8_t* out)
{
    const uint8_t* parents_array[MAX_SIMD_DEGREE_OR_2];
    size_t parents_array_len = 0;
    while(num_chaining_values - (2 * parents_array_len) >= 2) {
        parents_array[parents_array_len] =
            &child_chaining_values[2 * parents_array_len * BLAKE3_OUT_LEN];
        parents_array_len += 1;
    }

    blake3_hash_many(parents_array, parents_array_len, 1, key,
                     0, // Parents always use counter 0.
                     false, flags | PARENT,
                     0, // Parents have no start flags.
                     0, // Parents have no end flags.
                     out);

    // If there's an odd child left over, it becomes an output.
    if(num_chaining_values > 2 * parents_array_len) {
        cthash::memcpy(&out[parents_array_len * BLAKE3_OUT_LEN],
               &child_chaining_values[2 * parents_array_len * BLAKE3_OUT_LEN],
               BLAKE3_OUT_LEN);
        return parents_array_len + 1;
    } else {
        return parents_array_len;
    }
}

constexpr void chunk_state_init(blake3_chunk_state* self, const uint32_t key[8], uint8_t flags)
{
    cthash::memcpy(self->cv, key, BLAKE3_KEY_LEN / 4);
    self->chunk_counter = 0;
    cthash::memset(self->buf, 0, BLAKE3_BLOCK_LEN);
    self->buf_len = 0;
    self->blocks_compressed = 0;
    self->flags = flags;
}

constexpr void chunk_state_reset(blake3_chunk_state* self, const uint32_t key[8], uint64_t chunk_counter)
{
    cthash::memcpy(self->cv, key, BLAKE3_KEY_LEN / 4);
    self->chunk_counter = chunk_counter;
    self->blocks_compressed = 0;
    cthash::memset(self->buf, 0, BLAKE3_BLOCK_LEN);
    self->buf_len = 0;
}

constexpr size_t chunk_state_fill_buf(blake3_chunk_state* self, const uint8_t* input, size_t input_len)
{
    size_t take = BLAKE3_BLOCK_LEN - ((size_t)self->buf_len);
    if(take > input_len) {
        take = input_len;
    }
    uint8_t* dest = self->buf + ((size_t)self->buf_len);
    cthash::memcpy(dest, input, take);
    self->buf_len += (uint8_t)take;
    return take;
}

constexpr uint8_t chunk_state_maybe_start_flag(const blake3_chunk_state* self)
{
    if(self->blocks_compressed == 0) {
        return CHUNK_START;
    } else {
        return 0;
    }
}

constexpr void chunk_state_update(blake3_chunk_state* self, const uint8_t* input, size_t input_len)
{
    if(self->buf_len > 0) {
        size_t take = chunk_state_fill_buf(self, input, input_len);
        input += take;
        input_len -= take;
        if(input_len > 0) {
            blake3_compress_in_place(
                self->cv, self->buf, BLAKE3_BLOCK_LEN, self->chunk_counter,
                self->flags | chunk_state_maybe_start_flag(self));
            self->blocks_compressed += 1;
            self->buf_len = 0;
            cthash::memset(self->buf, 0, BLAKE3_BLOCK_LEN);
        }
    }

    while(input_len > BLAKE3_BLOCK_LEN) {
        blake3_compress_in_place(self->cv, input, BLAKE3_BLOCK_LEN,
                                 self->chunk_counter,
                                 self->flags | chunk_state_maybe_start_flag(self));
        self->blocks_compressed += 1;
        input += BLAKE3_BLOCK_LEN;
        input_len -= BLAKE3_BLOCK_LEN;
    }

    chunk_state_fill_buf(self, input, input_len);
}

constexpr output_t chunk_state_output(const blake3_chunk_state* self)
{
    uint8_t block_flags = self->flags | chunk_state_maybe_start_flag(self) | CHUNK_END;
    return make_output(self->cv, self->buf, self->buf_len, self->chunk_counter, block_flags);
}

// Chaining values within a given chunk (specifically the compress_in_place
// interface) are represented as words. This avoids unnecessary bytes<->words
// conversion overhead in the portable implementation. However, the hash_many
// interface handles both user input and parent node blocks, so it accepts
// bytes. For that reason, chaining values in the CV stack are represented as
// bytes.
constexpr void output_chaining_value(const output_t* self, uint8_t cv[32])
{
    uint32_t cv_words[8];
    cthash::memcpy(cv_words, self->input_cv, 32/4);
    blake3_compress_in_place(cv_words, self->block, self->block_len,
                             self->counter, self->flags);
    store_cv_words(cv, cv_words);
}

// Use SIMD parallelism to hash up to MAX_SIMD_DEGREE chunks at the same time
// on a single thread. Write out the chunk chaining values and return the
// number of chunks hashed. These chunks are never the root and never empty;
// those cases use a different codepath.
constexpr size_t compress_chunks_parallel(const uint8_t* input, size_t input_len,
                                       const uint32_t key[8],
                                       uint64_t chunk_counter, uint8_t flags,
                                       uint8_t* out)
{
    const uint8_t* chunks_array[MAX_SIMD_DEGREE];
    size_t input_position = 0;
    size_t chunks_array_len = 0;
    while(input_len - input_position >= BLAKE3_CHUNK_LEN) {
        chunks_array[chunks_array_len] = &input[input_position];
        input_position += BLAKE3_CHUNK_LEN;
        chunks_array_len += 1;
    }

    blake3_hash_many(chunks_array, chunks_array_len,
                     BLAKE3_CHUNK_LEN / BLAKE3_BLOCK_LEN, key, chunk_counter,
                     true, flags, CHUNK_START, CHUNK_END, out);

    // Hash the remaining partial chunk, if there is one. Note that the empty
    // chunk (meaning the empty message) is a different codepath.
    if(input_len > input_position) {
        uint64_t counter = chunk_counter + (uint64_t)chunks_array_len;
        blake3_chunk_state chunk_state;
        chunk_state_init(&chunk_state, key, flags);
        chunk_state.chunk_counter = counter;
        chunk_state_update(&chunk_state, &input[input_position],
                           input_len - input_position);
        output_t output = chunk_state_output(&chunk_state);
        output_chaining_value(&output, &out[chunks_array_len * BLAKE3_OUT_LEN]);
        return chunks_array_len + 1;
    } else {
        return chunks_array_len;
    }
}

constexpr size_t blake3_compress_subtree_wide(const uint8_t* input, size_t input_len,
                                    const uint32_t key[8],
                                    uint64_t chunk_counter, uint8_t flags,
                                    uint8_t* out)
{
    // Note that the single chunk case does *not* bump the SIMD degree up to 2
    // when it is 1. If this implementation adds multi-threading in the future,
    // this gives us the option of multi-threading even the 2-chunk case, which
    // can help performance on smaller platforms.
    if(input_len <= blake3_simd_degree() * BLAKE3_CHUNK_LEN) {
        return compress_chunks_parallel(input, input_len, key, chunk_counter, flags,
                                        out);
    }

    // With more than simd_degree chunks, we need to recurse. Start by dividing
    // the input into left and right subtrees. (Note that this is only optimal
    // as long as the SIMD degree is a power of 2. If we ever get a SIMD degree
    // of 3 or something, we'll need a more complicated strategy.)
    size_t left_input_len = left_subtree_len(input_len);
    size_t right_input_len = input_len - left_input_len;
    const uint8_t* right_input = &input[left_input_len];
    uint64_t right_chunk_counter =
        chunk_counter + (uint64_t)(left_input_len / BLAKE3_CHUNK_LEN);

    // Make space for the child outputs. Here we use MAX_SIMD_DEGREE_OR_2 to
    // account for the special case of returning 2 outputs when the SIMD degree
    // is 1.
    uint8_t cv_array[2 * MAX_SIMD_DEGREE_OR_2 * BLAKE3_OUT_LEN];
    size_t degree = blake3_simd_degree();
    if(left_input_len > BLAKE3_CHUNK_LEN && degree == 1) {
        // The special case: We always use a degree of at least two, to make
        // sure there are two outputs. Except, as noted above, at the chunk
        // level, where we allow degree=1. (Note that the 1-chunk-input case is
        // a different codepath.)
        degree = 2;
    }
    uint8_t* right_cvs = &cv_array[degree * BLAKE3_OUT_LEN];

    // Recurse!
    size_t left_n = static_cast<size_t>(-1);
    size_t right_n = static_cast<size_t>(-1);

    left_n = blake3_compress_subtree_wide(
        input, left_input_len, key, chunk_counter, flags, cv_array);
    right_n = blake3_compress_subtree_wide(right_input, right_input_len, key,
                                           right_chunk_counter, flags, right_cvs);

    // The special case again. If simd_degree=1, then we'll have left_n=1 and
    // right_n=1. Rather than compressing them into a single output, return
    // them directly, to make sure we always have at least two outputs.
    if(left_n == 1) {
        cthash::memcpy(out, cv_array, 2 * BLAKE3_OUT_LEN);
        return 2;
    }

    // Otherwise, do one layer of parent node compression.
    size_t num_chaining_values = left_n + right_n;
    return compress_parents_parallel(cv_array, num_chaining_values, key, flags,
                                     out);
}

// Hash a subtree with compress_subtree_wide(), and then condense the resulting
// list of chaining values down to a single parent node. Don't compress that
// last parent node, however. Instead, return its message bytes (the
// concatenated chaining values of its children). This is necessary when the
// first call to update() supplies a complete subtree, because the topmost
// parent node of that subtree could end up being the root. It's also necessary
// for extended output in the general case.
//
// As with compress_subtree_wide(), this function is not used on inputs of 1
// chunk or less. That's a different codepath.
constexpr void compress_subtree_to_parent_node(const uint8_t* input, size_t input_len,
                                const uint32_t key[8], uint64_t chunk_counter,
                                uint8_t flags, uint8_t out[2 * BLAKE3_OUT_LEN])
{
    uint8_t cv_array[MAX_SIMD_DEGREE_OR_2 * BLAKE3_OUT_LEN];
    blake3_compress_subtree_wide(input, input_len, key,
                                                  chunk_counter, flags, cv_array);
    // The following loop never executes when MAX_SIMD_DEGREE_OR_2 is 2, because
    // as we just asserted, num_cvs will always be <=2 in that case. But GCC
    // (particularly GCC 8.5) can't tell that it never executes, and if NDEBUG is
    // set then it emits incorrect warnings here. We tried a few different
    // hacks to silence these, but in the end our hacks just produced different
    // warnings (see https://github.com/BLAKE3-team/BLAKE3/pull/380). Out of
    // desperation, we ifdef out this entire loop when we know it's not needed.
    cthash::memcpy(out, cv_array, 2 * BLAKE3_OUT_LEN);
}


constexpr void output_root_bytes(const output_t* self, uint64_t seek, uint8_t* out, size_t out_len)
{
    if(out_len == 0) {
        return;
    }
    uint64_t output_block_counter = seek / 64;
    size_t offset_within_block = seek % 64;
    uint8_t wide_buf[64];
    if(offset_within_block) {
        blake3_compress_xof(self->input_cv, self->block, self->block_len, output_block_counter, self->flags | ROOT, wide_buf);
        const size_t available_bytes = 64 - offset_within_block;
        const size_t bytes = out_len > available_bytes ? available_bytes : out_len;
        cthash::memcpy(out, wide_buf + offset_within_block, bytes);
        out += bytes;
        out_len -= bytes;
        output_block_counter += 1;
    }
    if(out_len / 64) {
        blake3_xof_many(self->input_cv, self->block, self->block_len, output_block_counter, self->flags | ROOT, out, out_len / 64);
    }
    output_block_counter += out_len / 64;
    out += out_len & -64;
    out_len -= out_len & -64;
    if(out_len) {
        blake3_compress_xof(self->input_cv, self->block, self->block_len, output_block_counter, self->flags | ROOT, wide_buf);
        cthash::memcpy(out, wide_buf, out_len);
    }
}

 
constexpr output_t parent_output(const uint8_t block[BLAKE3_BLOCK_LEN],
                       const uint32_t key[8], uint8_t flags)
{
    return make_output(key, block, BLAKE3_BLOCK_LEN, 0, flags | PARENT);
}

// As described in hasher_push_cv() below, we do "lazy merging", delaying
// merges until right before the next CV is about to be added. This is
// different from the reference implementation. Another difference is that we
// aren't always merging 1 chunk at a time. Instead, each CV might represent
// any power-of-two number of chunks, as long as the smaller-above-larger stack
// order is maintained. Instead of the "count the trailing 0-bits" algorithm
// described in the spec, we use a "count the total number of 1-bits" variant
// that doesn't require us to retain the subtree size of the CV on top of the
// stack. The principle is the same: each CV that should remain in the stack is
// represented by a 1-bit in the total number of chunks (or bytes) so far.
constexpr void hasher_merge_cv_stack(blake3_hasher* self, uint64_t total_len)
{
    size_t post_merge_stack_len = (size_t)popcnt(total_len);
    while(self->cv_stack_len > post_merge_stack_len) {
        uint8_t* parent_node =
            &self->cv_stack[(self->cv_stack_len - 2) * BLAKE3_OUT_LEN];
        output_t output = parent_output(parent_node, self->key, self->chunk.flags);
        output_chaining_value(&output, parent_node);
        self->cv_stack_len -= 1;
    }
}

// In reference_impl.rs, we merge the new CV with existing CVs from the stack
// before pushing it. We can do that because we know more input is coming, so
// we know none of the merges are root.
//
// This setting is different. We want to feed as much input as possible to
// compress_subtree_wide(), without setting aside anything for the chunk_state.
// If the user gives us 64 KiB, we want to parallelize over all 64 KiB at once
// as a single subtree, if at all possible.
//
// This leads to two problems:
// 1) This 64 KiB input might be the only call that ever gets made to update.
//    In this case, the root node of the 64 KiB subtree would be the root node
//    of the whole tree, and it would need to be ROOT finalized. We can't
//    compress it until we know.
// 2) This 64 KiB input might complete a larger tree, whose root node is
//    similarly going to be the root of the whole tree. For example, maybe
//    we have 196 KiB (that is, 128 + 64) hashed so far. We can't compress the
//    node at the root of the 256 KiB subtree until we know how to finalize it.
//
// The second problem is solved with "lazy merging". That is, when we're about
// to add a CV to the stack, we don't merge it with anything first, as the
// reference impl does. Instead we do merges using the *previous* CV that was
// added, which is sitting on top of the stack, and we put the new CV
// (unmerged) on top of the stack afterwards. This guarantees that we never
// merge the root node until finalize().
//
// Solving the first problem requires an additional tool,
// compress_subtree_to_parent_node(). That function always returns the top
// *two* chaining values of the subtree it's compressing. We then do lazy
// merging with each of them separately, so that the second CV will always
// remain unmerged. (That also helps us support extendable output when we're
// hashing an input all-at-once.)
constexpr void hasher_push_cv(blake3_hasher* self, uint8_t new_cv[BLAKE3_OUT_LEN], uint64_t chunk_counter)
{
    hasher_merge_cv_stack(self, chunk_counter);
    cthash::memcpy(&self->cv_stack[self->cv_stack_len * BLAKE3_OUT_LEN], new_cv,
           BLAKE3_OUT_LEN);
    self->cv_stack_len += 1;
}

constexpr void blake3_hasher_init(blake3_hasher* self)
{
    cthash::memcpy(self->key, IV, BLAKE3_KEY_LEN/4);

    cthash::memcpy(self->chunk.cv, IV, BLAKE3_KEY_LEN / 4);
    self->chunk.chunk_counter = 0;
    cthash::memset(self->chunk.buf, 0, BLAKE3_BLOCK_LEN);
    self->chunk.buf_len = 0;
    self->chunk.blocks_compressed = 0;
    self->chunk.flags = 0;

    self->cv_stack_len = 0;
}

constexpr void blake3_hasher_update(blake3_hasher* self, const uint8_t* input, size_t input_len)
{
    // Explicitly checking for zero avoids causing UB by passing a null pointer
    // to memcpy. This comes up in practice with things like:
    //   std::vector<uint8_t> v;
    //   blake3_hasher_update(&hasher, v.data(), v.size());
    if(input_len == 0) {
        return;
    }

    const uint8_t* input_bytes = input;

    // If we have some partial chunk bytes in the internal chunk_state, we need
    // to finish that chunk first.
    if(chunk_state_len(&self->chunk) > 0) {
        size_t take = BLAKE3_CHUNK_LEN - chunk_state_len(&self->chunk);
        if(take > input_len) {
            take = input_len;
        }
        chunk_state_update(&self->chunk, input_bytes, take);
        input_bytes += take;
        input_len -= take;
        // If we've filled the current chunk and there's more coming, finalize this
        // chunk and proceed. In this case we know it's not the root.
        if(input_len > 0) {
            output_t output = chunk_state_output(&self->chunk);
            uint8_t chunk_cv[32];
            output_chaining_value(&output, chunk_cv);
            hasher_push_cv(self, chunk_cv, self->chunk.chunk_counter);
            chunk_state_reset(&self->chunk, self->key, self->chunk.chunk_counter + 1);
        } else {
            return;
        }
    }

    // Now the chunk_state is clear, and we have more input. If there's more than
    // a single chunk (so, definitely not the root chunk), hash the largest whole
    // subtree we can, with the full benefits of SIMD (and maybe in the future,
    // multi-threading) parallelism. Two restrictions:
    // - The subtree has to be a power-of-2 number of chunks. Only subtrees along
    //   the right edge can be incomplete, and we don't know where the right edge
    //   is going to be until we get to finalize().
    // - The subtree must evenly divide the total number of chunks up until this
    //   point (if total is not 0). If the current incomplete subtree is only
    //   waiting for 1 more chunk, we can't hash a subtree of 4 chunks. We have
    //   to complete the current subtree first.
    // Because we might need to break up the input to form powers of 2, or to
    // evenly divide what we already have, this part runs in a loop.
    while(input_len > BLAKE3_CHUNK_LEN) {
        size_t subtree_len = round_down_to_power_of_2(input_len);
        uint64_t count_so_far = self->chunk.chunk_counter * BLAKE3_CHUNK_LEN;
        // Shrink the subtree_len until it evenly divides the count so far. We know
        // that subtree_len itself is a power of 2, so we can use a bitmasking
        // trick instead of an actual remainder operation. (Note that if the caller
        // consistently passes power-of-2 inputs of the same size, as is hopefully
        // typical, this loop condition will always fail, and subtree_len will
        // always be the full length of the input.)
        //
        // An aside: We don't have to shrink subtree_len quite this much. For
        // example, if count_so_far is 1, we could pass 2 chunks to
        // compress_subtree_to_parent_node. Since we'll get 2 CVs back, we'll still
        // get the right answer in the end, and we might get to use 2-way SIMD
        // parallelism. The problem with this optimization, is that it gets us
        // stuck always hashing 2 chunks. The total number of chunks will remain
        // odd, and we'll never graduate to higher degrees of parallelism. See
        // https://github.com/BLAKE3-team/BLAKE3/issues/69.
        while((((uint64_t)(subtree_len - 1)) & count_so_far) != 0) {
            subtree_len /= 2;
        }
        // The shrunken subtree_len might now be 1 chunk long. If so, hash that one
        // chunk by itself. Otherwise, compress the subtree into a pair of CVs.
        uint64_t subtree_chunks = subtree_len / BLAKE3_CHUNK_LEN;
        if(subtree_len <= BLAKE3_CHUNK_LEN) {
            blake3_chunk_state chunk_state;
            chunk_state_init(&chunk_state, self->key, self->chunk.flags);
            chunk_state.chunk_counter = self->chunk.chunk_counter;
            chunk_state_update(&chunk_state, input_bytes, subtree_len);
            output_t output = chunk_state_output(&chunk_state);
            uint8_t cv[BLAKE3_OUT_LEN];
            output_chaining_value(&output, cv);
            hasher_push_cv(self, cv, chunk_state.chunk_counter);
        } else {
            // This is the high-performance happy path, though getting here depends
            // on the caller giving us a long enough input.
            uint8_t cv_pair[2 * BLAKE3_OUT_LEN];
            compress_subtree_to_parent_node(input_bytes, subtree_len, self->key,
                                            self->chunk.chunk_counter,
                                            self->chunk.flags, cv_pair);
            hasher_push_cv(self, cv_pair, self->chunk.chunk_counter);
            hasher_push_cv(self, &cv_pair[BLAKE3_OUT_LEN],
                           self->chunk.chunk_counter + (subtree_chunks / 2));
        }
        self->chunk.chunk_counter += subtree_chunks;
        input_bytes += subtree_len;
        input_len -= subtree_len;
    }

    // If there's any remaining input less than a full chunk, add it to the chunk
    // state. In that case, also do a final merge loop to make sure the subtree
    // stack doesn't contain any unmerged pairs. The remaining input means we
    // know these merges are non-root. This merge loop isn't strictly necessary
    // here, because hasher_push_chunk_cv already does its own merge loop, but it
    // simplifies blake3_hasher_finalize below.
    if(input_len > 0) {
        chunk_state_update(&self->chunk, input_bytes, input_len);
        hasher_merge_cv_stack(self, self->chunk.chunk_counter);
    }
}

constexpr void blake3_hasher_finalize(const blake3_hasher* self, uint8_t* out, size_t out_len)
{
    constexpr uint64_t seek = 0;
    // Explicitly checking for zero avoids causing UB by passing a null pointer
    // to memcpy. This comes up in practice with things like:
    //   std::vector<uint8_t> v;
    //   blake3_hasher_finalize(&hasher, v.data(), v.size());
    if(out_len == 0) {
        return;
    }

    // If the subtree stack is empty, then the current chunk is the root.
    if(self->cv_stack_len == 0) {
        output_t output = chunk_state_output(&self->chunk);
        output_root_bytes(&output, seek, out, out_len);
        return;
    }
    // If there are any bytes in the chunk state, finalize that chunk and do a
    // roll-up merge between that chunk hash and every subtree in the stack. In
    // this case, the extra merge loop at the end of blake3_hasher_update
    // guarantees that none of the subtrees in the stack need to be merged with
    // each other first. Otherwise, if there are no bytes in the chunk state,
    // then the top of the stack is a chunk hash, and we start the merge from
    // that.
    output_t output;
    size_t cvs_remaining;
    if(chunk_state_len(&self->chunk) > 0) {
        cvs_remaining = self->cv_stack_len;
        output = chunk_state_output(&self->chunk);
    } else {
        // There are always at least 2 CVs in the stack in this case.
        cvs_remaining = self->cv_stack_len - 2;
        output = parent_output(&self->cv_stack[cvs_remaining * 32], self->key,
                               self->chunk.flags);
    }
    while(cvs_remaining > 0) {
        cvs_remaining -= 1;
        uint8_t parent_block[BLAKE3_BLOCK_LEN];
        cthash::memcpy(parent_block, &self->cv_stack[cvs_remaining * 32], 32);
        output_chaining_value(&output, &parent_block[32]);
        output = parent_output(parent_block, self->key, self->chunk.flags);
    }
    output_root_bytes(&output, seek, out, out_len);
}

template<size_t N>
constexpr std::array<uint8_t, 32> blake3_encrypt(const uint8_t (&src)[N])
{
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);

    blake3_hasher_update(&hasher, src, N);

    std::array<uint8_t, 32> message;
    blake3_hasher_finalize(&hasher, &message[0], 32);
    return message;
}

} // namespace cthash

#ifdef _DEBUG
template<class T, size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& ar)
{
    for(size_t i = 0; i < ar.size(); ++i) {
        os << std::hex << (int32_t)ar[i];
    }
    return os;
}
#endif

#endif // INC_CRYPT_H_
