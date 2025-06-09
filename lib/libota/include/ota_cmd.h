// ota_cmd.h
// This header file contains the definitions and function prototypes for OTA command handling.

#ifndef __OTA_CMD_H__
#define __OTA_CMD_H__

#include "ota_common.h"

// OTA command opcodes
typedef enum _ota_cmd_opcode_t{
    OTA_CMD_OPCODE_READ = 0,
    OTA_CMD_OPCODE_PROGRAM,
    OTA_CMD_OPCODE_ERASE,
    OTA_CMD_OPCODE_VERIFY,
    OTA_CMD_OPCODE_MAX // This is used to determine the number of commands
} ota_cmd_opcode_t;

// OTA command arguments structure
typedef struct _ota_cmd_args_read_t {
    uint32_t address; // Address to read from
    uint32_t length;  // Length of data to read
    uint8_t *buffer;    // Pointer to data buffer where read data will be stored
    uint32_t buffer_length; // Length of the data buffer
} ota_cmd_args_read_t;

typedef struct _ota_cmd_args_program_t {
    uint32_t address; // Address to program
    uint32_t length;  // Length of data to program
    uint8_t *data;    // Pointer to data buffer
} ota_cmd_args_program_t;

typedef struct _ota_cmd_args_erase_t {
    uint32_t address; // Address to erase
    uint32_t length;  // Length of data to erase
} ota_cmd_args_erase_t;

typedef struct _ota_cmd_args_verify_t {
    uint32_t address; // Address to verify
    uint32_t length;  // Length of data to verify
    uint8_t *result;  // Pointer to store verification result (Hash value)
    uint32_t *result_length; // Length of the result buffer
} ota_cmd_args_verify_t;

// OTA command arguments length for each command

// Read command: address (4 bytes) + length (4 bytes)
// buffer and buffer_length are from the IO buffer, so they are not included in the length
#define OTA_CMD_ARGS_READ_LEN (sizeof(uint32_t) + sizeof(uint32_t))

// Program command: address (4 bytes)
// length and data are from the IO buffer, so they are not included in the length
#define OTA_CMD_ARGS_PROGRAM_LEN (sizeof(uint32_t))

// Erase command: address (4 bytes) + length (4 bytes)
#define OTA_CMD_ARGS_ERASE_LEN (sizeof(uint32_t) + sizeof(uint32_t))

// Verify command: address (4 bytes) + length (4 bytes)
// data and result are from the IO buffer, so they are not included in the length
#define OTA_CMD_ARGS_VERIFY_LEN (sizeof(uint32_t) + sizeof(uint32_t))

#define OTA_CMD_ARGS_MAX_LEN (OTA_CMD_ARGS_VERIFY_LEN + sizeof(uint8_t)) // +1 for the opcode

// Table for OTA command argument lengths
extern const uint8_t ota_cmd_args_length_table[];

// 128-bit AES-CMAC Key
extern const uint8_t ota_aes128_key[16];

bStatus_t ota_cmd_handler(
    const uint8_t *buffer,
    uint32_t length,
    const uint8_t *io_buffer,
    uint32_t io_buffer_length,
    const uint8_t *challenge,
    uint32_t challenge_length,
    const uint8_t *token,
    uint32_t token_length
);

#endif
