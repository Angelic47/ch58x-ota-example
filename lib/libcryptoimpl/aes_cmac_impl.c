// aes_cmac_impl.c
// This file contains the implementation of AES-CMAC operations for the CH58x Hardware AES module.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#include "aes_cmac_impl.h"

#define AES_ENCRYPT LL_Encrypt

void xor_128(uint8_t *a, uint8_t *b, uint8_t *out) {
    for (int i = 0; i < 16; i++) {
        out[i] = a[i] ^ b[i];
    }
}

void left_shift_128(uint8_t *input, uint8_t *output) {
    uint8_t overflow = 0;
    for (int i = 15; i >= 0; i--) {
        output[i] = (input[i] << 1) | overflow;
        overflow = (input[i] & 0x80) ? 1 : 0;
    }
}

void generate_subkey(uint8_t *key, uint8_t *K1, uint8_t *K2) {
    uint8_t L[16] = {0};
    AES_ENCRYPT(key, L, L);  // L = AES-128(key, 0x000000...)

    uint8_t Rb[16] = {0};
    Rb[15] = 0x87;  // Rb = 0x000000...87 for 128-bit CMAC

    left_shift_128(L, K1);
    if (L[0] & 0x80) {
        xor_128(K1, Rb, K1);
    }

    left_shift_128(K1, K2);
    if (K1[0] & 0x80) {
        xor_128(K2, Rb, K2);
    }
}

void AES_CMAC(uint8_t *key, uint8_t *msg, uint32_t len, uint8_t *mac) {
    uint8_t K1[16], K2[16];
    generate_subkey(key, K1, K2);

    uint32_t n = (len + 15) / 16;
    int last_block_complete = (len % 16 == 0) && (len != 0);

    uint8_t M_last[16] = {0};
    uint8_t block[16] = {0};
    uint8_t X[16] = {0};  // initial vector (0x00...00)

    // Prepare M_last
    if (n == 0) {
        n = 1;
        memset(block, 0, 16);
        block[0] = 0x80;
        xor_128(block, K2, M_last);
    } else {
        if (last_block_complete) {
            xor_128(&msg[16 * (n - 1)], K1, M_last);
        } else {
            uint32_t last_len = len % 16;
            memset(block, 0, 16);
            memcpy(block, &msg[16 * (n - 1)], last_len);
            block[last_len] = 0x80;
            xor_128(block, K2, M_last);
        }
    }

    // Process all blocks except the last
    for (uint32_t i = 0; i < n - 1; i++) {
        xor_128(X, &msg[16 * i], block);
        AES_ENCRYPT(key, block, X);
    }

    // Process last block
    xor_128(X, M_last, block);
    AES_ENCRYPT(key, block, mac);
}
