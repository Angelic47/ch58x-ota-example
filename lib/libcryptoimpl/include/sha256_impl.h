// sha256_impl.h
// This file contains the software implementation of the SHA-256 hashing algorithm.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __SHA256_IMPL_H__
#define __SHA256_IMPL_H__

#ifdef CH58xBLE_ROM
#include "CH58xBLE_ROM.H"
#else
#include "CH58xBLE_LIB.h"
#endif

#include "CH58x_common.h"


typedef struct {
    uint32_t state[8];
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
} SHA256_CTX;

/**
 * @brief Initialize the SHA-256 context.
 * 
 * This function initializes the SHA-256 context to prepare it for hashing.
 * It sets the initial state and resets the data length and bit length.
 *
 * @param ctx Pointer to the SHA256_CTX structure to be initialized.
 */
void sha256_init(SHA256_CTX * ctx);

/**
 * @brief Update the SHA-256 context with new data.
 * 
 * This function processes the input data and updates the SHA-256 context.
 * It handles the data in chunks of 64 bytes and performs the necessary transformations.
 *
 * @param ctx Pointer to the SHA256_CTX structure to be updated.
 * @param data Pointer to the input data to be hashed.
 * @param len Length of the input data in bytes.
 */
void sha256_update(SHA256_CTX * ctx, const uint8_t data[], size_t len);

/**
 * @brief Finalize the SHA-256 hash computation.
 * 
 * This function finalizes the SHA-256 hash computation and produces the final hash value.
 * It pads the input data, processes any remaining data, and outputs the final hash.
 *
 * @param ctx Pointer to the SHA256_CTX structure containing the state and data.
 * @param hash Pointer to the output buffer where the final hash will be stored (32 bytes).
 */
void sha256_final(SHA256_CTX * ctx, uint8_t hash[]);

#endif // __SHA256_IMPL_H__
