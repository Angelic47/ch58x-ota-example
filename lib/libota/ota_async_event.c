// ota_async_event.c
// This file contains the implementation of asynchronous OTA events for the CH58x series microcontroller.
// Includes functions to handle Erase & Verify commands that consume a lot of time and should be run asynchronously.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#include "ota_async_event.h"
#include "sha256_impl.h"

static uint32_t ota_is_busy = 0;
static bStatus_t ota_async_event_status = SUCCESS;
static uint32_t current_offset, cmd_address, cmd_length, *data_buffer_length;
static uint8_t *data_buffer;
static uint8_t event_task_id;
static SHA256_CTX sha256_ctx;
__attribute__((aligned(8))) static uint32_t sha256_hashbuf[256]; // SHA256 temp buffer

uint32_t ota_is_busy_flag(void)
{
    return ota_is_busy;
}

bStatus_t ota_get_async_event_status(void)
{
    return ota_async_event_status;
}

bStatus_t ota_start_async_erase(uint32_t address, uint32_t length)
{
    // Set the busy flag
    ota_is_busy = 1;

    // Store the address and length for the erase operation
    ota_async_event_status = blePending; // Set status to pending
    current_offset = 0;
    cmd_address = address;
    cmd_length = length;

    // Trigger the asynchronous erase event
    return tmos_set_event(event_task_id, OTA_ASYNC_EVENT_ERASE);
}

bStatus_t ota_start_async_verify(uint32_t address, uint32_t length, uint8_t *buffer, uint32_t *buffer_length)
{
    // Set the busy flag
    ota_is_busy = 1;

    // Store the address, length, and data buffer for the verify operation
    ota_async_event_status = blePending; // Set status to pending
    current_offset = 0;
    cmd_address = address;
    cmd_length = length;
    data_buffer = buffer;
    data_buffer_length = buffer_length;

    // Initialize SHA256 context
    sha256_init(&sha256_ctx);

    // Trigger the asynchronous verify event
    return tmos_set_event(event_task_id, OTA_ASYNC_EVENT_VERIFY);
}

bStatus_t ota_start_async_reboot(void)
{
    // Set the busy flag
    ota_is_busy = 1;

    // Set the status to pending
    ota_async_event_status = blePending;

    // Trigger the asynchronous reboot event
    return tmos_set_event(event_task_id, OTA_ASYNC_EVENT_REBOOT);
}

uint16_t ota_process_event(uint8_t task_id, uint16_t events)
{
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(event_task_id)) != NULL)
        {
            // ota_process_tmos_msg((tmos_event_hdr_t *)pMsg);
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if (events & OTA_ASYNC_EVENT_ERASE) {
        // Handle asynchronous erase operation
        uint8_t status;
        uint32_t erase_length = EEPROM_BLOCK_SIZE;
        if (cmd_address - current_offset < erase_length) {
            erase_length = cmd_address - current_offset; // Adjust length if less than block size
        }
        status = FLASH_ROM_ERASE(cmd_address + current_offset, erase_length);
        if (status != SUCCESS) {
            ota_async_event_status = status; // Set the status to the error code
            ota_is_busy = 0; // Clear the busy flag
            return events ^ OTA_ASYNC_EVENT_ERASE; // Clear the event after processing
        }

        // Success
        current_offset += erase_length;
        if (current_offset >= cmd_length) {
            ota_async_event_status = SUCCESS; // Set status to success
            ota_is_busy = 0; // Clear the busy flag

            return events ^ OTA_ASYNC_EVENT_ERASE;
        }

        // Continue with the next erase operation
        return events;
    }

    if (events & OTA_ASYNC_EVENT_VERIFY) {
        // Handle asynchronous verify operation
        uint32_t process_length = 256; // limit to 256 bytes per operation as SHA256 may take a long time

        if (cmd_address - current_offset < process_length) {
            process_length = cmd_address - current_offset; // Adjust length if less than 256 bytes
        }
        FLASH_ROM_READ(cmd_address + current_offset, sha256_hashbuf, process_length);

        // Update SHA256 context with the read data
        sha256_update(&sha256_ctx, (const uint8_t *)sha256_hashbuf, process_length);
        current_offset += process_length;

        if (current_offset >= cmd_length) {
            // Finalize SHA256 hash calculation
            sha256_final(&sha256_ctx, data_buffer);
            *data_buffer_length = 32; // SHA256 produces a 32-byte hash

            ota_async_event_status = SUCCESS; // Set status to success
            ota_is_busy = 0; // Clear the busy flag

            return events ^ OTA_ASYNC_EVENT_VERIFY;
        }

        // Continue with the next read operation
        return events;
    }

    // Handle asynchronous reboot operation
    if (events & OTA_ASYNC_EVENT_REBOOT) {
        // Perform the reboot operation
        SYS_DisableAllIrq(NULL);
        mDelaymS(10);
        SYS_ResetExecute();
        // This point should not be reached, as the system will reset
        while (1) {
            // Infinite loop to prevent further execution
        }
    }

    // Discard unprocessed events
    return 0;
}

bStatus_t ota_async_event_init(void)
{
    event_task_id = TMOS_ProcessEventRegister(ota_process_event);
    if (event_task_id == 0xFF) {
        return bleMemAllocError; // Failed to register the task
    }

    return SUCCESS;
}
