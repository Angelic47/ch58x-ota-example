// eeprom_flags.c
// This file contains the implementation of functions to manage EEPROM flags in the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#include "libota.h"
#include "ota_eeprom_offsets.h"
#include "ota_eeprom_structs.h"

current_flash_bank_t current_flash_bank;
flash_mode_flag_t flash_mode_flag;
boot_reason_code_t boot_reason_code;
uint32_t eeprom_already_read = 0;

__attribute__((aligned(8))) bootloader_flash_eeprom_data_t eeprom_data;

const char *ota_flash_bank_to_string(current_flash_bank_t bank)
{
    switch (bank)
    {
        case FLASH_BANK_A:
            return "A";
        case FLASH_BANK_B:
            return "B";
        case FLASH_BANK_FAIL_BOOT:
            return "Fail Boot";
        default:
            return "Unknown (First Initialization Maybe)";
    }
}

const char *ota_flash_mode_flag_to_string(flash_mode_flag_t flag)
{
    switch (flag)
    {
        case FLASH_MODE_FLAG_OK:
            return "OK";
        case FLASH_MODE_FLAG_FLASHED:
            return "Flashed";
        case FLASH_MODE_FLAG_FIRSTBOOT:
            return "First Boot";
        default:
            return "Unknown";
    }
}

const char *ota_boot_reason_code_to_string(boot_reason_code_t code)
{
    switch (code)
    {
        case REASON_NORMAL:
            return "Normal";
        case REASON_FALLBACK_BOOT:
            return "Fallback Boot";
        default:
            return "Unknown";
    }
}

void ota_get_eeprom_flags(void)
{
    EEPROM_READ(OTA_EEPROM_FLASH_OFFSET_FLAGS, (uint32_t *)&eeprom_data, OTA_EEPROM_FLASH_READ_LEN);
    current_flash_bank = eeprom_data.current_flash_bank;
    flash_mode_flag = eeprom_data.flash_mode_flag;
    boot_reason_code = eeprom_data.boot_reason_code;
    eeprom_already_read = 1; // Set the flag to indicate EEPROM has been read
}

void ota_print_eeprom_flags(void)
{
    if (!eeprom_already_read)
        ota_get_eeprom_flags();
    LOG("EEPROM Flags:\r\n");
    LOG(" - Current Flash Bank: %s\r\n", ota_flash_bank_to_string(current_flash_bank));
    LOG(" - Flash Mode Flag: %s\r\n", ota_flash_mode_flag_to_string(flash_mode_flag));
    LOG(" - Boot Reason Code: %s\r\n", ota_boot_reason_code_to_string(boot_reason_code));
}

void ota_set_flags_current_flash_bank(current_flash_bank_t bank)
{
    if (!eeprom_already_read)
        ota_get_eeprom_flags();
    current_flash_bank = bank;
}

void ota_set_flags_flash_mode_flag(flash_mode_flag_t flag)
{
    if (!eeprom_already_read)
        ota_get_eeprom_flags();
    flash_mode_flag = flag;
}

void ota_set_flags_boot_reason_code(boot_reason_code_t code)
{
    if (!eeprom_already_read)
        ota_get_eeprom_flags();
    boot_reason_code = code;
}

void ota_save_eeprom_flags(void)
{
    if (!eeprom_already_read)
        ota_get_eeprom_flags();
    
    // Update the EEPROM data structure with the current flags
    eeprom_data.current_flash_bank = current_flash_bank;
    eeprom_data.flash_mode_flag = flash_mode_flag;
    eeprom_data.boot_reason_code = boot_reason_code;
    
    // Erase the EEPROM page
    // 256 bytes is the size of the EEPROM page
    EEPROM_ERASE(OTA_EEPROM_FLASH_OFFSET_FLAGS, OTA_EEPROM_FLASH_ERASE_SIZE);
    
    // Write the updated flags to EEPROM
    EEPROM_WRITE(OTA_EEPROM_FLASH_OFFSET_FLAGS, (uint32_t *)&eeprom_data, OTA_EEPROM_FLASH_READ_LEN);
}

void ota_assert_boot_ok(void)
{
    // This function is called to assert that the boot is OK
    // It sets the flash mode flag to OK and saves the flags to EEPROM
    if (!eeprom_already_read)
        ota_get_eeprom_flags();
    
    if (flash_mode_flag != FLASH_MODE_FLAG_FIRSTBOOT)
        return; // No need to assert if not in first boot mode
    
    flash_mode_flag = FLASH_MODE_FLAG_OK; // Set to OK
    ota_save_eeprom_flags(); // Save the updated flags to EEPROM
}
