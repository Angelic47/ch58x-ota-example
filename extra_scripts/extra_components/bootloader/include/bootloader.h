// bootloader.h
// Some constants and macros for the bootloader of the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#define BOOTLOADER_VERSION "1.0.0"
#define BOOTLOADER_AUTHOR "Angelic47"

#define BOOTLOADER_FLASH_BANK_A_ENTRY 0x00001000
#define BOOTLOADER_FLASH_BANK_B_ENTRY 0x00037000

#define JUMP_FLASH_BANK_A ((void (*)(void))((int *)BOOTLOADER_FLASH_BANK_A_ENTRY))
#define JUMP_FLASH_BANK_B ((void (*)(void))((int *)BOOTLOADER_FLASH_BANK_B_ENTRY))

#endif // __BOOTLOADER_H__
