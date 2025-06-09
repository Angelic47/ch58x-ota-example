// ota_async_event.h
// This file contains the definitions and declarations for asynchronous OTA events.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __OTA_ASYNC_EVENT_H__
#define __OTA_ASYNC_EVENT_H__

#include "ota_common.h"

#define OTA_ASYNC_EVENT_ERASE 0x0001 // Event for asynchronous erase operation
#define OTA_ASYNC_EVENT_VERIFY 0x0002 // Event for asynchronous verify operation

// The main routine to process OTA events
uint16_t ota_process_event(uint8_t task_id, uint16_t events);

// Function to check if OTA operations are currently busy
uint32_t ota_is_busy_flag(void);

// Function to get the status of the last asynchronous OTA event
bStatus_t ota_get_async_event_status(void);

// Function to start an asynchronous erase operation
bStatus_t ota_start_async_erase(uint32_t address, uint32_t length);

// Function to start an asynchronous verify operation
bStatus_t ota_start_async_verify(uint32_t address, uint32_t length, uint8_t *buffer, uint32_t *buffer_length);

// Function to initialize the OTA asynchronous event system
bStatus_t ota_async_event_init(void);

#endif // __OTA_ASYNC_EVENT_H__
