// eeprom_flags.h
// This file contains the functions and definitions for managing EEPROM flags in the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __EEPROM_FLAGS_H__
#define __EEPROM_FLAGS_H__

#include "ota_common.h"
#include "ota_eeprom_offsets.h"
#include "ota_eeprom_structs.h"

const char *ota_flash_bank_to_string(current_flash_bank_t bank);
const char *ota_flash_mode_flag_to_string(flash_mode_flag_t flag);
const char *ota_boot_reason_code_to_string(boot_reason_code_t code);
void ota_get_eeprom_flags(void);
void ota_print_eeprom_flags(void);
void ota_set_flags_current_flash_bank(current_flash_bank_t bank);
void ota_set_flags_flash_mode_flag(flash_mode_flag_t flag);
void ota_set_flags_boot_reason_code(boot_reason_code_t code);
current_flash_bank_t ota_get_flags_current_flash_bank(void);
flash_mode_flag_t ota_get_flags_flash_mode_flag(void);
boot_reason_code_t ota_get_flags_boot_reason_code(void);
const char *ota_get_flags_current_flash_bank_string(void);
const char *ota_get_flags_flash_mode_flag_string(void);
const char *ota_get_flags_boot_reason_code_string(void);
void ota_save_eeprom_flags(void);
void ota_assert_boot_ok(void);

#endif // __EEPROM_FLAGS_H__
