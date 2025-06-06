// ota_eeprom_offsets.h
// This file contains the definitions for the EEPROM data structures used in the OTA bootloader of the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __OTA_EEPROM_STRUCTS_H__
#define __OTA_EEPROM_STRUCTS_H__

#include "CH58x_common.h"

typedef enum _current_flash_bank_t
{
    FLASH_BANK_A = 0xa5a5a5a5,
    FLASH_BANK_B = 0x5a5a5a5a,
    FLASH_BANK_FAIL_BOOT = 0xdeadbeef,
} current_flash_bank_t;

typedef enum _flash_mode_flag_t
{
    FLASH_MODE_FLAG_OK = 0,
    FLASH_MODE_FLAG_FLASHED,
    FLASH_MODE_FLAG_FIRSTBOOT,
    FLASH_MODE_FLAG_MAX = 0xff,
} flash_mode_flag_t;

typedef enum _boot_reason_code_t
{
    REASON_NORMAL = 0,
    REASON_FALLBACK_BOOT,
    REASON_MAX = 0xff,
} boot_reason_code_t;

typedef struct _bootloader_flash_eeprom_data_t
{
    uint32_t current_flash_bank; // Current flash bank
    uint8_t flash_mode_flag; // Flash mode flag
    uint8_t boot_reason_code; // Boot reason code
    uint8_t reserved[2]; // Reserved for future use (Padding)
} bootloader_flash_eeprom_data_t;

#endif // __OTA_EEPROM_STRUCTS_H__
