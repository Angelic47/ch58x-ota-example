// ota_common.h
// This file contains common definitions and includes for the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __OTA_COMMON_H__
#define __OTA_COMMON_H__

#ifdef CH58xBLE_ROM
#include "CH58xBLE_ROM.H"
#else
#include "CH58xBLE_LIB.h"
#endif

#include "CH58x_common.h"

#ifndef LIBOTA_BUILD_CURRENT_BANK
#define __CURRENT_BUILD_BANK_STR__ "Unknown Bank"
#define __CURRENT_BUILD_BANK__ 0xFFFFFFFF
#warning "LIBOTA_BUILD_CURRENT_BANK is not defined. Defaulting to Unknown Bank."
#else
    #if LIBOTA_BUILD_CURRENT_BANK == 0
    #define __CURRENT_BUILD_BANK_STR__ "Bank A"
    #define __CURRENT_BUILD_BANK__ 0xA5A5A5A5
    #elif LIBOTA_BUILD_CURRENT_BANK == 1
    #define __CURRENT_BUILD_BANK_STR__ "Bank B"
    #define __CURRENT_BUILD_BANK__ 0x5A5A5A5A
    #else
    #error "Invalid LIBOTA_BUILD_CURRENT_BANK value. Must be 0 (Bank A) or 1 (Bank B)."
    #endif
#endif

#endif // __OTA_COMMON_H__
