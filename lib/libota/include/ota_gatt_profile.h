// ota_gatt_profile.h
// This file contains the definitions for the OTA GATT profile of the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __OTA_GATT_PROFILE_H__
#define __OTA_GATT_PROFILE_H__

#include "ota_common.h"

// GATT Profile Service UUID
#define OTA_GATT_PROFILE_SERV_UUID 0xFFF0

// OTA GATT Profile Characteristic UUIDs
#define OTA_GATT_PROFILE_CHAR_UUID_MAIN 0xFFE1
#define OTA_GATT_PROFILE_CHAR_UUID_BUFFER 0xFFE2
#define OTA_GATT_PROFILE_CHAR_UUID_CHALLENGE 0xFFE3
#define OTA_GATT_PROFILE_CHAR_UUID_TOKEN 0xFFE4
#define OTA_GATT_PROFILE_CHAR_UUID_FLASH_BANK 0xFFE5
#define OTA_GATT_PROFILE_CHAR_UUID_FLASH_BANK_READABLE 0xFFE6
#define OTA_GATT_PROFILE_CHAR_UUID_FLASH_MODE 0xFFE7
#define OTA_GATT_PROFILE_CHAR_UUID_FLASH_MODE_READABLE 0xFFE8
#define OTA_GATT_PROFILE_CHAR_UUID_BOOT_REASON 0xFFE9
#define OTA_GATT_PROFILE_CHAR_UUID_BOOT_REASON_READABLE 0xFFEA

// Key Profile Services bit fields
#define OTA_GATT_PROFILE_SERVICES 0x00000001

// OTA IO Buffer Size
#define OTA_IO_BUFFER_SIZE 512

bStatus_t OTAProfile_AddService(void);

#endif // __OTA_GATT_PROFILE_H__
