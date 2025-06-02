// peripheral_main.h
// This file contains the definitions and declarations for the bootloader of the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __PERIPHERAL_MAIN_H__
#define __PERIPHERAL_MAIN_H__

#include "CH58x_common.h"
#include "ota_eeprom_offsets.h"
#include "ota_eeprom_structs.h"
#include "bootloader.h"

#ifdef DEBUG
#define LOG(X...) printf("[Bootloader] "X)
#else
#define LOG(X...)
#endif

#endif // __PERIPHERAL_MAIN_H__
