// aes_cmac_impl.h
// This file contains the implementation details for AES-CMAC operations.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __AES_CMAC_IMPL_H__
#define __AES_CMAC_IMPL_H__

#ifdef CH58xBLE_ROM
#include "CH58xBLE_ROM.H"
#else
#include "CH58xBLE_LIB.h"
#endif

#include "CH58x_common.h"

/*
* Functions for AES-CMAC operations
* These functions are used to compute the AES-CMAC of a message using a given key.
* The AES-CMAC is a message authentication code based on the AES block cipher.
* It is used to ensure the integrity and authenticity of the message.
*
* Parameters:
* @key: Pointer to the AES key (16 bytes).
* @msg: Pointer to the message to be authenticated.
* @len: Length of the message in bytes.
* @mac: Pointer to the output buffer where the computed MAC will be stored (16 bytes).
*/
void AES_CMAC(uint8_t *key, uint8_t *msg, size_t len, uint8_t *mac);

#endif // __AES_CMAC_IMPL_H__
