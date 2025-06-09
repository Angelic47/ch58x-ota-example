// ota_flash_layout.h
// This file contains the definitions for the OTA flash layout of the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __OTA_FLASH_LAYOUT_H__
#define __OTA_FLASH_LAYOUT_H__

#define OTA_FLASH_BANK_SIZE 0x00036000 // 216KB for each bank

#define OTA_FLASH_BANK_A_ENTRY 0x00001000
#define OTA_FLASH_BANK_B_ENTRY 0x00037000

#define OTA_FLASH_BANK_A_FULL (OTA_FLASH_BANK_A_ENTRY + OTA_FLASH_BANK_SIZE)
#define OTA_FLASH_BANK_B_FULL (OTA_FLASH_BANK_B_ENTRY + OTA_FLASH_BANK_SIZE)

#define OTA_FLASH_BANK_A_END (OTA_FLASH_BANK_A_ENTRY + OTA_FLASH_BANK_SIZE - 1)
#define OTA_FLASH_BANK_B_END (OTA_FLASH_BANK_B_ENTRY + OTA_FLASH_BANK_SIZE - 1)

#endif // __OTA_FLASH_LAYOUT_H__
