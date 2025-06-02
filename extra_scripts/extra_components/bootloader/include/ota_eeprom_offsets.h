// ota_eeprom_offsets.h
// This file contains the definitions for the EEPROM offsets used in the OTA bootloader of the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __OTA_EEPROM_OFFSETS_H__
#define __OTA_EEPROM_OFFSETS_H__

#include "CH58x_common.h"
#include "ota_eeprom_structs.h"

#define OTA_EEPROM_FLASH_OFFSET (0x00077000 - FLASH_ROM_MAX_SIZE)

#define OTA_EEPROM_FLASH_OFFSET_FLAGS (OTA_EEPROM_FLASH_OFFSET + 0x00)
#define OTA_EEPROM_FLASH_READ_LEN (sizeof(bootloader_flash_eeprom_data_t) / 4)
#define OTA_EEPROM_FLASH_ERASE_SIZE (EEPROM_PAGE_SIZE)

#endif // __OTA_EEPROM_OFFSETS_H__
