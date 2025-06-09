// ota_cmd.c
// This file contains the implementation of OTA command handling. 
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#include "ota_cmd.h"
#include "aes_cmac_impl.h"
#include "ota_flash_layout.h"
#include "eeprom_flags.h"
#include "ota_async_event.h"

#ifndef OTA_GATT_AES128_KEY_BYTES
#error "OTA module needs a 128-bit AES-CMAC Key defined in platformio.ini or build CFLAGS!"
#error "This is required for secure OTA operations and cannot be omitted."
#else
__attribute__((aligned(8))) const uint8_t ota_aes128_key[16] = OTA_GATT_AES128_KEY_BYTES;
#endif

// 16 bytes for buffer AES-CMAC, 16 bytes for io_buffer AES-CMAC, 16 bytes for challenge, 16 bytes for result
__attribute__((aligned(8))) static char aes_cmac_challenge_full_buffer[16 + 16 + 16 + 16]; 
__attribute__((aligned(8))) static char aes_cmac_temp_cmd_buffer[OTA_CMD_ARGS_MAX_LEN];

// OTA command argument lengths for each command
const uint8_t ota_cmd_args_length_table[OTA_CMD_OPCODE_MAX] = {
    OTA_CMD_ARGS_READ_LEN,    // Read command length
    OTA_CMD_ARGS_PROGRAM_LEN, // Program command length
    OTA_CMD_ARGS_ERASE_LEN,   // Erase command length
    OTA_CMD_ARGS_VERIFY_LEN,  // Verify command length
};

/**
 * @brief Check if the OTA request is valid
 *
 * @param buffer Pointer to the buffer containing the OTA command
 * @param length Length of the OTA command in the buffer
 * 
 * @return bStatus_t Result of the validation
 */
bStatus_t ota_cmd_is_valid(const uint8_t *buffer, uint32_t length) {
    if (length < sizeof(uint8_t)) {
        return ATT_ERR_INVALID_VALUE; // Buffer too short to contain a valid command
    }

    uint8_t opcode = buffer[0]; // First byte is the command opcode
    if (opcode >= OTA_CMD_OPCODE_MAX) {
        return ATT_ERR_INVALID_VALUE; // Invalid command opcode
    }

    uint32_t expected_length = ota_cmd_args_length_table[opcode];
    if (length != expected_length + sizeof(uint8_t)) {
        return ATT_ERR_INVALID_VALUE_SIZE; // Buffer too short for the expected command length
    }

    return SUCCESS; // Valid OTA command
}

/**
 * @brief Check if the OTA command is authenticated by AES-CMAC Signature
 * 
 * @param buffer Pointer to the buffer containing the OTA command (must already be checked by caller)
 * @param length Length of the OTA command in the buffer (must already be checked by caller)
 * @param io_buffer Pointer to the IO buffer where the command data is stored (must be aligned)
 * @param io_buffer_length Length of the IO buffer (must already be checked by caller)
 * @param challenge Pointer to the challenge data used for authentication
 * @param challenge_length Length of the challenge data
 * @param token Pointer to the token data used for authentication
 * @param token_length Length of the token data
 * 
 * @return bStatus_t Result of the authentication check
 */
bStatus_t ota_cmd_is_authenticated(
    const uint8_t *buffer, 
    uint32_t length, 
    const uint8_t *io_buffer, 
    uint32_t io_buffer_length, 
    const uint8_t *challenge, 
    uint32_t challenge_length, 
    const uint8_t *token, 
    uint32_t token_length
) {
    if(token_length != 16) {
        return ATT_ERR_INVALID_VALUE_SIZE; // Token must be 16 bytes for AES-CMAC
    }
    if(challenge_length != 16) {
        return ATT_ERR_INVALID_VALUE_SIZE; // Challenge must be 16 bytes for AES-CMAC
    }

    // Prepare the full buffer for AES-CMAC calculation
    tmos_memset(aes_cmac_challenge_full_buffer, 0, sizeof(aes_cmac_challenge_full_buffer));
    
    // 1. Calculate the AES-CMAC of the command buffer
    // Copy the command buffer to the aligned temporary command buffer for AES-CMAC Hardware Acceleration
    tmos_memcpy(aes_cmac_temp_cmd_buffer, buffer, length);
    AES_CMAC(
        (uint8_t *)ota_aes128_key, 
        (uint8_t *)aes_cmac_temp_cmd_buffer, 
        length, 
        (uint8_t *)aes_cmac_challenge_full_buffer
    );

    // 2. Calculate the AES-CMAC of the IO buffer (Already aligned)
    // If the IO buffer empty, we set the AES-CMAC to zero
    if(io_buffer_length != 0) {
        AES_CMAC(
            (uint8_t *)ota_aes128_key, 
            (uint8_t *)io_buffer, 
            io_buffer_length, 
            (uint8_t *)(aes_cmac_challenge_full_buffer + 16)
        );
    } else {
        tmos_memset(aes_cmac_challenge_full_buffer + 16, 0, 16);
    }

    // 3. Copy the challenge to the full buffer
    tmos_memcpy(
        aes_cmac_challenge_full_buffer + 16 + 16, 
        challenge, 
        challenge_length
    );

    // 4. Calculate the AES-CMAC of the full buffer
    // Store the result in the last 16 bytes of the buffer
    AES_CMAC(
        (uint8_t *)ota_aes128_key, 
        (uint8_t *)aes_cmac_challenge_full_buffer, 
        16 + 16 + 16, 
        (uint8_t *)(aes_cmac_challenge_full_buffer + 16 + 16 + 16) 
    );

    // 5. Compare the calculated MAC with the provided token
    if(tmos_memcmp(
        token, 
        aes_cmac_challenge_full_buffer + 16 + 16 + 16, 
        16
    ) != 0) {
        return ATT_ERR_INSUFFICIENT_AUTHEN;
    }

    return SUCCESS;
}

/**
 * @brief Check if the address and length are valid for the OTA command
 * 
 * @param address Address to check
 * @param length Length to check
 * @param current_flash_bank Current flash bank (A or B)
 * 
 * @return bStatus_t Result of the address and length check
 */
bStatus_t ota_cmd_address_length_check(
    uint32_t address, 
    uint32_t length, 
    current_flash_bank_t current_flash_bank
) {
    if(length == 0) {
        return bleInvalidRange; // Length cannot be zero
    }
    if(length > OTA_FLASH_BANK_SIZE) {
        return bleInvalidRange; // Length exceeds maximum allowed length
    }
    switch(current_flash_bank) {
        case FLASH_BANK_A:
            if(address < OTA_FLASH_BANK_A_ENTRY)
                return bleInvalidRange; // Address out of bounds for Bank A
            if(address > OTA_FLASH_BANK_A_FULL)
                return bleInvalidRange; // Address or length out of bounds for Bank A
            if((address + length) > OTA_FLASH_BANK_A_END)
                return bleInvalidRange; // Address + length exceeds Bank A bounds
            break;
        case FLASH_BANK_B:
            if(address < OTA_FLASH_BANK_B_ENTRY)
                return bleInvalidRange; // Address out of bounds for Bank B
            if(address > OTA_FLASH_BANK_B_FULL)
                return bleInvalidRange; // Address or length out of bounds for Bank B
            if((address + length) > OTA_FLASH_BANK_B_END)
                return bleInvalidRange; // Address + length exceeds Bank B bounds
            break;
        default:
            // Should not happen, invalid flash bank, indicating EEPROM is corrupted
            return ATT_ERR_UNLIKELY; 
    }
    
    return SUCCESS;
}

bStatus_t ota_cmd_do_read(ota_cmd_args_read_t *args) {
    bStatus_t status;

    // Read can be used to read from either flash bank A or B
    status = ota_cmd_address_length_check(
        args->address, 
        args->length, 
        FLASH_BANK_A
    );
    if (status != SUCCESS) {
        status = ota_cmd_address_length_check(
            args->address, 
            args->length, 
            FLASH_BANK_B
        );
    }
    if (status != SUCCESS) {
        return status; // Address or length check failed
    }

    FLASH_ROM_READ(
        args->address + args->length,
        args->buffer,
        args->length
    );

    return SUCCESS;
}

bStatus_t ota_cmd_do_program(ota_cmd_args_program_t *args) {
    bStatus_t status;

    // Write can only be used to program the flash bank that is not currently active
    status = ota_cmd_address_length_check(
        args->address, 
        args->length, 
        ota_get_flags_current_flash_bank() == FLASH_BANK_A ? FLASH_BANK_B : FLASH_BANK_A
    );

    if (status != SUCCESS) {
        return status; // Address or length check failed
    }

    status = FLASH_ROM_WRITE(
        args->address,
        args->data,
        args->length
    );

    return status;
}

bStatus_t ota_cmd_do_erase(ota_cmd_args_erase_t *args) {
    // Schedule an asynchronous erase operation
    return ota_start_async_erase(args->address, args->length);
}

bStatus_t ota_cmd_do_verify(ota_cmd_args_verify_t *args) {
    // Schedule an asynchronous verify operation
    return ota_start_async_verify(args->address, args->length, args->result, args->result_length);
}

/**
 * @brief OTA command dispatcher
 * This function dispatches the OTA command based on the opcode in the buffer.
 * It assumes that the command has already been validated and authenticated and without length checks.
 * 
 * @param buffer Pointer to the buffer containing the OTA command
 * @param length Length of the OTA command in the buffer
 * @param io_buffer Pointer to the IO buffer where the command data is stored
 * @param io_buffer_length Length of the IO buffer
 * 
 * @return bStatus_t Result of the command execution
 */
bStatus_t ota_cmd_dispatcher(
    const uint8_t *buffer,
    uint32_t length,
    const uint8_t *io_buffer,
    uint32_t io_buffer_length
) {
    ota_cmd_opcode_t opcode = buffer[0]; // First byte is the command opcode
    union {
        ota_cmd_args_read_t read_args;
        ota_cmd_args_program_t program_args;
        ota_cmd_args_erase_t erase_args;
        ota_cmd_args_verify_t verify_args;
    } args;

    switch(opcode) {
        case OTA_CMD_OPCODE_READ: 
            // Read command
            tmos_memcpy(&args.read_args.address, buffer + 1, sizeof(uint32_t));
            tmos_memcpy(&args.read_args.length, buffer + 1 + sizeof(uint32_t), sizeof(uint32_t));
            args.read_args.buffer = (uint8_t *)io_buffer; // Use the IO buffer for read data
            args.read_args.buffer_length = io_buffer_length; // Length of the IO buffer
            return ota_cmd_do_read(&args.read_args); // Call the read command handler
        case OTA_CMD_OPCODE_PROGRAM: 
            // Program command
            tmos_memcpy(&args.program_args.address, buffer + 1, sizeof(uint32_t));
            args.program_args.data = (uint8_t *)io_buffer; // Use the IO buffer for program data
            args.program_args.length = io_buffer_length; // Length of data to program is the IO buffer length
            return ota_cmd_do_program(&args.program_args); // Call the program command handler
        case OTA_CMD_OPCODE_ERASE: 
            // Erase command
            tmos_memcpy(&args.erase_args.address, buffer + 1, sizeof(uint32_t));
            tmos_memcpy(&args.erase_args.length, buffer + 1 + sizeof(uint32_t), sizeof(uint32_t));
            return ota_cmd_do_erase(&args.erase_args); // Call the erase command handler
        case OTA_CMD_OPCODE_VERIFY: 
            // Verify command
            tmos_memcpy(&args.verify_args.address, buffer + 1, sizeof(uint32_t));
            tmos_memcpy(&args.verify_args.length, buffer + 1 + sizeof(uint32_t), sizeof(uint32_t));
            args.verify_args.result = (uint8_t *)io_buffer; // Use the IO buffer for verification result
            args.verify_args.result_length = &io_buffer_length; // Length of the result buffer is the IO buffer length
            return ota_cmd_do_verify(&args.verify_args); // Call the verify command handler
        default:
            // Unknown command opcode
            // Should not happen if the command has been validated before
            return ATT_ERR_UNSUPPORTED_REQ; // Unknown command opcode
    }
}

/**
 * @brief OTA command handler
 * This function handles the OTA command by validating, authenticating, and dispatching it.
 * 
 * @param buffer Pointer to the buffer containing the OTA command
 * @param length Length of the OTA command in the buffer
 * @param io_buffer Pointer to the IO buffer where the command data is stored
 * @param io_buffer_length Length of the IO buffer
 * @param challenge Pointer to the challenge data used for authentication
 * @param challenge_length Length of the challenge data
 * @param token Pointer to the token data used for authentication
 * @param token_length Length of the token data
 * 
 * @return bStatus_t Result of the command handling
 */
bStatus_t ota_cmd_handler(
    const uint8_t *buffer,
    uint32_t length,
    const uint8_t *io_buffer,
    uint32_t io_buffer_length,
    const uint8_t *challenge,
    uint32_t challenge_length,
    const uint8_t *token,
    uint32_t token_length
) {
    // Step 1: Validate the OTA command
    bStatus_t status = ota_cmd_is_valid(buffer, length);
    if (status != SUCCESS) {
        return status; // Invalid command
    }

    // Step 2: Authenticate the OTA command
    status = ota_cmd_is_authenticated(buffer, length, io_buffer, io_buffer_length, challenge, challenge_length, token, token_length);
    if (status != SUCCESS) {
        return status; // Authentication failed
    }

    // Step 3: Dispatch the OTA command
    return ota_cmd_dispatcher(buffer, length, io_buffer, io_buffer_length);
}
