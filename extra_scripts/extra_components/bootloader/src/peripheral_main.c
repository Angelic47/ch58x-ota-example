// peripheral_main.c
// This is the main file for the bootloader of the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#include "CH58x_common.h"
#include "peripheral_main.h"

current_flash_bank_t current_flash_bank;
flash_mode_flag_t flash_mode_flag;
boot_reason_code_t boot_reason_code;

__attribute__((aligned(8))) bootloader_flash_eeprom_data_t eeprom_data;

char *flash_bank_to_string(current_flash_bank_t bank)
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

char *flash_mode_flag_to_string(flash_mode_flag_t flag)
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

char *boot_reason_code_to_string(boot_reason_code_t code)
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

void enter_bootrom_isp(void)
{
    LOG("Entering Bootrom WCHISP...\r\n");
    LOG("mstatus=%08x\r\n", read_csr(mstatus));

    PFIC->IRER[0] = 0xffffffff;
    PFIC->IRER[1] = 0xffffffff;

    // copy bootrom WCHISP code to RAM
    memcpy((void*)0x20003800, (void*)0x000780a4, 0x2500);
    // +0xbe is the offset in the bootrom code where it checks PB22, patch it to always return 1
    *(uint16_t*)0x200038be = 0x4505; // li a0, 1
    // clear the BSS section in RAM
    memset((void*)0x20005c18, 0, 0x04a8);

    // set the stack pointer and global pointer
    __asm__("la gp, 0x20006410\r\n");
    __asm__("la sp, 0x20008000\r\n");
    write_csr(mstatus, 0x88);
    write_csr(mtvec, 0x20003801);
    write_csr(mepc, 0x20004ebc);

    // jump to the bootrom WCHISP code entry point
    __asm__("mret");
    __asm__("nop");
    __asm__("nop");
}

void bootloader_get_eeprom_flags(void)
{
    EEPROM_READ(OTA_EEPROM_FLASH_OFFSET_FLAGS, (uint32_t *)&eeprom_data, OTA_EEPROM_FLASH_READ_LEN);
    current_flash_bank = eeprom_data.current_flash_bank;
    flash_mode_flag = eeprom_data.flash_mode_flag;
    boot_reason_code = eeprom_data.boot_reason_code;
}

void bootloader_print_eeprom_flags(void)
{
    LOG("Bootloader EEPROM Flags:\r\n");
    LOG(" - Current Flash Bank: %s\r\n", flash_bank_to_string(current_flash_bank));
    LOG(" - Flash Mode Flag: %s\r\n", flash_mode_flag_to_string(flash_mode_flag));
    LOG(" - Boot Reason Code: %s\r\n", boot_reason_code_to_string(boot_reason_code));
}

void bootloader_boot(void)
{
    // bank: FAIL_BOOT, mode: <any>, reason: <any>
    if(current_flash_bank == FLASH_BANK_FAIL_BOOT)
    {
        LOG("All flash banks is asserted to be bad, entering Bootrom ISP for recovery...\r\n");
        enter_bootrom_isp();
        // Should not return from here, if it does, something went wrong
        LOG("Failed to enter Bootrom ISP!\r\n");
        return;
    }
    // bank: A or B, mode: <any>, reason: <any>
    if(current_flash_bank == FLASH_BANK_A || current_flash_bank == FLASH_BANK_B)
    {
        // bank: A or B, mode: FLASH_MODE_FLAG_FIRSTBOOT, reason: REASON_FALLBACK_BOOT
        if(flash_mode_flag == FLASH_MODE_FLAG_FIRSTBOOT && 
           boot_reason_code == REASON_FALLBACK_BOOT)
        {
            // Boot fail, all flash banks are asserted to be bad, entering Bootrom ISP for recovery
            LOG("EEPROM indicates all flash banks are bad, there is no valid flash bank to boot from.\r\n");
            eeprom_data.current_flash_bank = FLASH_BANK_FAIL_BOOT; // Set to fail boot
            eeprom_data.flash_mode_flag = FLASH_MODE_FLAG_FIRSTBOOT; // Set to first boot
            eeprom_data.boot_reason_code = REASON_FALLBACK_BOOT; // Set to fallback boot

            // Erase the EEPROM page
            // 256 bytes is the size of the EEPROM page
            EEPROM_ERASE(OTA_EEPROM_FLASH_OFFSET_FLAGS, OTA_EEPROM_FLASH_ERASE_SIZE);

            // Write the updated flags to EEPROM
            EEPROM_WRITE(OTA_EEPROM_FLASH_OFFSET_FLAGS, (uint32_t *)&eeprom_data, OTA_EEPROM_FLASH_READ_LEN);

            LOG("Entering Bootrom ISP for recovery...\r\n");
            enter_bootrom_isp();
            // Should not return from here, if it does, something went wrong
            LOG("Failed to enter Bootrom ISP. halting...\r\n");
            return;
        }
        // bank: A or B, mode: FLASH_MODE_FLAG_FIRSTBOOT, reason: <any except REASON_FALLBACK_BOOT>
        if(flash_mode_flag == FLASH_MODE_FLAG_FIRSTBOOT)
        {
            LOG("EEPROM indicates the last boot was crashed, attempting to boot into the last known good flash bank...\r\n");
            if (current_flash_bank == FLASH_BANK_A)
                current_flash_bank = FLASH_BANK_B; // Switch to bank B
            else
                current_flash_bank = FLASH_BANK_A; // Switch to bank A
            eeprom_data.current_flash_bank = current_flash_bank; // Update the current flash bank
            eeprom_data.flash_mode_flag = FLASH_MODE_FLAG_FIRSTBOOT; // Set to first boot
            eeprom_data.boot_reason_code = REASON_FALLBACK_BOOT; // Set to fallback boot

            // Erase the EEPROM page
            // 256 bytes is the size of the EEPROM page
            EEPROM_ERASE(OTA_EEPROM_FLASH_OFFSET_FLAGS, OTA_EEPROM_FLASH_ERASE_SIZE);

            // Write the updated flags to EEPROM
            EEPROM_WRITE(OTA_EEPROM_FLASH_OFFSET_FLAGS, (uint32_t *)&eeprom_data, OTA_EEPROM_FLASH_READ_LEN);

            LOG("Booting into Flash Bank %s...\r\n", flash_bank_to_string(current_flash_bank));
            if (current_flash_bank == FLASH_BANK_A)
                JUMP_FLASH_BANK_A();
            else
                JUMP_FLASH_BANK_B();
            return;
        }
        // bank: A or B, mode: FLASH_MODE_FLAG_FLASHED, reason: <any>
        if(flash_mode_flag == FLASH_MODE_FLAG_FLASHED)
        {
            LOG("EEPROM indicates new OTA has been performed, booting into the new flash bank...\r\n");
            if (current_flash_bank == FLASH_BANK_A)
                current_flash_bank = FLASH_BANK_B; // Switch to bank B
            else
                current_flash_bank = FLASH_BANK_A; // Switch to bank A
            eeprom_data.current_flash_bank = current_flash_bank; // Update the current flash bank
            eeprom_data.flash_mode_flag = FLASH_MODE_FLAG_FIRSTBOOT; // Set to first boot
            eeprom_data.boot_reason_code = FLASH_MODE_FLAG_OK; // Set to OK boot reason

            // Erase the EEPROM page
            // 256 bytes is the size of the EEPROM page
            EEPROM_ERASE(OTA_EEPROM_FLASH_OFFSET_FLAGS, OTA_EEPROM_FLASH_ERASE_SIZE);

            // Write the updated flags to EEPROM
            EEPROM_WRITE(OTA_EEPROM_FLASH_OFFSET_FLAGS, (uint32_t *)&eeprom_data, OTA_EEPROM_FLASH_READ_LEN);

            LOG("Booting into Flash Bank %s...\r\n", flash_bank_to_string(current_flash_bank));
            if (current_flash_bank == FLASH_BANK_A)
                JUMP_FLASH_BANK_A();
            else
                JUMP_FLASH_BANK_B();
            return;
        }
        // bank: A or B, mode: FLASH_MODE_FLAG_OK, reason: <any>
        if(boot_reason_code == REASON_FALLBACK_BOOT)
        {
            LOG("Warning: EEPROM indicates this is a fallback boot, last OTA may have failed.\r\n");
        }
        LOG("Booting into Flash Bank %s...\r\n", flash_bank_to_string(current_flash_bank));
        if (current_flash_bank == FLASH_BANK_A)
            JUMP_FLASH_BANK_A();
        else
            JUMP_FLASH_BANK_B();
        return;
    }
    // bank: <any>, mode: <any>, reason: <any>
    else
    {
        LOG("Detected first boot or uninitialized EEPROM, initializing EEPROM...\r\n");
        // New bootloader initialization
        eeprom_data.current_flash_bank = FLASH_BANK_A; // Default to bank A
        eeprom_data.flash_mode_flag = FLASH_MODE_FLAG_FIRSTBOOT; // Set to first boot
        eeprom_data.boot_reason_code = REASON_NORMAL; // Normal boot

        // Erase the EEPROM page
        // 256 bytes is the size of the EEPROM page
        EEPROM_ERASE(OTA_EEPROM_FLASH_OFFSET_FLAGS, OTA_EEPROM_FLASH_ERASE_SIZE);

        // Write the initial flags to EEPROM
        EEPROM_WRITE(OTA_EEPROM_FLASH_OFFSET_FLAGS, (uint32_t *)&eeprom_data, OTA_EEPROM_FLASH_READ_LEN);
        LOG("Bootloader EEPROM initialized with default values.\r\n");
        LOG("Now booting into Flash Bank A...\r\n");
        JUMP_FLASH_BANK_A();
        return;
    }
}

int main(void)
{
    #if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
        PWR_DCDCCfg(ENABLE);
    #endif
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    #if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
        GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
        GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    #endif
    #ifdef DEBUG
        GPIOA_SetBits(bTXD1);
        GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
        UART1_DefInit();
    #endif
    LOG("WCH AutoOTA Bootloader" BOOTLOADER_VERSION "\r\n");
    LOG("BuildTime: " __DATE__ " " __TIME__ "\r\n");
    LOG("Author: " BOOTLOADER_AUTHOR "\r\n");
    LOG("\r\n");
    bootloader_get_eeprom_flags();
    bootloader_print_eeprom_flags();
    LOG("\r\n");
    bootloader_boot();
    LOG("Failed to boot, should not reach here!\r\n");
    return -1; // Should not reach here, if it does, something went wrong
}