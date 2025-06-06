// ota_gatt_profile.h
// This file contains the definitions for the OTA GATT profile of the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __OTA_GATT_PROFILE_H__
#define __OTA_GATT_PROFILE_H__

#include "ota_common.h"

#ifndef OTA_GATT_AES128_KEY
#error "OTA module needs a 128-bit AES-CMAC Key defined in platformio.ini or build CFLAGS!"
#error "This is required for secure OTA operations and cannot be omitted."
#endif

// GATT Profile Service UUID
#define OTA_GATT_PROFILE_SERV_UUID 0xFFE0

// OTA GATT Profile Characteristic UUIDs
#define OTA_GATT_PROFILE_CHAR_UUID_MAIN 0xFFE1
#define OTA_GATT_PROFILE_CHAR_UUID_BUFFER 0xFFE2
#define OTA_GATT_PROFILE_CHAR_UUID_CHALLENGE 0xFFE3
#define OTA_GATT_PROFILE_CHAR_UUID_TOKEN 0xFFE4

// Key Profile Services bit fields
#define OTA_GATT_PROFILE_SERVICES 0x00000001

// OTA IO Buffer Size
#define OTA_IO_BUFFER_SIZE 512

void OTA_GattProfile_AddService(uint32_t services);

#endif // __OTA_GATT_PROFILE_H__
