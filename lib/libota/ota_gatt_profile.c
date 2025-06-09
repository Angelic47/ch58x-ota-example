// ota_gatt_profile.c
// This file contains the implementation of the OTA GATT profile for the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#include "ota_gatt_profile.h"
#include "eeprom_flags.h"
#include "ota_async_event.h"
#include "ota_cmd.h"

// GATT Profile Service UUID
const uint8_t otaProfileServiceUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(OTA_GATT_PROFILE_SERV_UUID), 
    HI_UINT16(OTA_GATT_PROFILE_SERV_UUID)
};

// Characteristic 1 UUID
const uint8_t otaProfileChar1UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(OTA_GATT_PROFILE_CHAR_UUID_MAIN),
    HI_UINT16(OTA_GATT_PROFILE_CHAR_UUID_MAIN)
};

// Characteristic 2 UUID
const uint8_t otaProfileChar2UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(OTA_GATT_PROFILE_CHAR_UUID_BUFFER),
    HI_UINT16(OTA_GATT_PROFILE_CHAR_UUID_BUFFER)
};

// Characteristic 3 UUID
const uint8_t otaProfileChar3UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(OTA_GATT_PROFILE_CHAR_UUID_CHALLENGE),
    HI_UINT16(OTA_GATT_PROFILE_CHAR_UUID_CHALLENGE)
};

// Characteristic 4 UUID
const uint8_t otaProfileChar4UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(OTA_GATT_PROFILE_CHAR_UUID_TOKEN),
    HI_UINT16(OTA_GATT_PROFILE_CHAR_UUID_TOKEN)
};

// Characteristic 5 UUID
const uint8_t otaProfileChar5UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(OTA_GATT_PROFILE_CHAR_UUID_FLASH_BANK),
    HI_UINT16(OTA_GATT_PROFILE_CHAR_UUID_FLASH_BANK)
};

// Characteristic 6 UUID
const uint8_t otaProfileChar6UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(OTA_GATT_PROFILE_CHAR_UUID_FLASH_BANK_READABLE),
    HI_UINT16(OTA_GATT_PROFILE_CHAR_UUID_FLASH_BANK_READABLE)
};

// Characteristic 7 UUID
const uint8_t otaProfileChar7UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(OTA_GATT_PROFILE_CHAR_UUID_FLASH_MODE),
    HI_UINT16(OTA_GATT_PROFILE_CHAR_UUID_FLASH_MODE)
};

// Characteristic 8 UUID
const uint8_t otaProfileChar8UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(OTA_GATT_PROFILE_CHAR_UUID_FLASH_MODE_READABLE),
    HI_UINT16(OTA_GATT_PROFILE_CHAR_UUID_FLASH_MODE_READABLE)
};

// Characteristic 9 UUID
const uint8_t otaProfileChar9UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(OTA_GATT_PROFILE_CHAR_UUID_BOOT_REASON),
    HI_UINT16(OTA_GATT_PROFILE_CHAR_UUID_BOOT_REASON)
};

// Characteristic 10 UUID
const uint8_t otaProfileChar10UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(OTA_GATT_PROFILE_CHAR_UUID_BOOT_REASON_READABLE),
    HI_UINT16(OTA_GATT_PROFILE_CHAR_UUID_BOOT_REASON_READABLE)
};

// GATT Profile Service attributes
static const gattAttrType_t otaProfileService = {
    .len = ATT_BT_UUID_SIZE,
    .uuid = otaProfileServiceUUID
};

// Characteristic 1 Properties
static uint8_t otaProfileChar1Props = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// Characteristic 1 User Description
static uint8_t otaProfileChar1UserDesc[] = "OTA Command Control & Status Readback";

// Characteristic 2 Properties
static uint8_t otaProfileChar2Props = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// Characteristic 2 Value
__attribute__((aligned(8))) static uint8_t otaProfileChar2Val[OTA_IO_BUFFER_SIZE] = {0};
static uint32_t otaProfileChar2Len = 0;
static uint32_t otaProfileChar2IsWritting = 0;

// Characteristic 2 User Description
static uint8_t otaProfileChar2UserDesc[] = "OTA Buffer";

// Characteristic 3 Properties
static uint8_t otaProfileChar3Props = GATT_PROP_READ;

// Characteristic 3 Value
static uint8_t otaProfileChar3Val[16] = {0};
static const uint32_t otaProfileChar3Len = 16; // Fixed length for AES-CMAC challenge

// Characteristic 3 User Description
static uint8_t otaProfileChar3UserDesc[] = "OTA AES-CMAC Signature Challenge";

// Characteristic 4 Properties
static uint8_t otaProfileChar4Props = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// Characteristic 4 Value
static uint8_t otaProfileChar4Val[16] = {0};
static uint32_t otaProfileChar4Len = 0;
static uint32_t otaProfileChar4IsWritting = 0;

// Characteristic 4 User Description
static uint8_t otaProfileChar4UserDesc[] = "OTA AES-CMAC Signature Token";

// Characteristic 5 Properties
static uint8_t otaProfileChar5Props = GATT_PROP_READ;

// Characteristic 5 User Description
static uint8_t otaProfileChar5UserDesc[] = "OTA Flash Bank";

// Characteristic 6 Properties
static uint8_t otaProfileChar6Props = GATT_PROP_READ;

// Characteristic 6 User Description
static uint8_t otaProfileChar6UserDesc[] = "OTA Flash Bank (Readable String)";

// Characteristic 7 Properties
static uint8_t otaProfileChar7Props = GATT_PROP_READ;

// Characteristic 7 User Description
static uint8_t otaProfileChar7UserDesc[] = "OTA Flash Mode";

// Characteristic 8 Properties
static uint8_t otaProfileChar8Props = GATT_PROP_READ;

// Characteristic 8 User Description
static uint8_t otaProfileChar8UserDesc[] = "OTA Flash Mode (Readable String)";

// Characteristic 9 Properties
static uint8_t otaProfileChar9Props = GATT_PROP_READ;

// Characteristic 9 User Description
static uint8_t otaProfileChar9UserDesc[] = "OTA Boot Reason";

// Characteristic 10 Properties
static uint8_t otaProfileChar10Props = GATT_PROP_READ;

// Characteristic 10 User Description
static uint8_t otaProfileChar10UserDesc[] = "OTA Boot Reason (Readable String)";

// Profile Attributes Table
static gattAttribute_t otaProfileAttrTbl[] = {
    // Service Declaration
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = primaryServiceUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = (uint8_t *)&otaProfileService,
    },
    // Characteristic 1 Declaration
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = characterUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = &otaProfileChar1Props,
    },
    // Characteristic 1 Value
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = otaProfileChar1UUID,
        },
        .permissions = GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        .handle = 0, // Will be assigned by the stack
        .pValue = NULL, 
    },
    // Characteristic 1 User Description
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = charUserDescUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar1UserDesc,
    },
    // Characteristic 2 Declaration
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = characterUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = &otaProfileChar2Props,
    },
    // Characteristic 2 Value
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = otaProfileChar2UUID,
        },
        .permissions = GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar2Val,
    },
    // Characteristic 2 User Description
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = charUserDescUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar2UserDesc,
    },
    // Characteristic 3 Declaration
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = characterUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = &otaProfileChar3Props,
    },
    // Characteristic 3 Value
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = otaProfileChar3UUID,
        },
        .permissions = GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar3Val,
    },
    // Characteristic 3 User Description
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = charUserDescUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar3UserDesc,
    },
    // Characteristic 4 Declaration
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = characterUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = &otaProfileChar4Props,
    },
    // Characteristic 4 Value
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = otaProfileChar4UUID,
        },
        .permissions = GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar4Val,
    },
    // Characteristic 4 User Description
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = charUserDescUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar4UserDesc,
    },
    // Characteristic 5 Declaration
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = characterUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = &otaProfileChar5Props,
    },
    // Characteristic 5 Value
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = otaProfileChar5UUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = NULL,
    },
    // Characteristic 5 User Description
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = charUserDescUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar5UserDesc,
    },
    // Characteristic 6 Declaration
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = characterUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = &otaProfileChar6Props,
    },
    // Characteristic 6 Value
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = otaProfileChar6UUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = NULL,
    },
    // Characteristic 6 User Description
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = charUserDescUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar6UserDesc,
    },
    // Characteristic 7 Declaration
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = characterUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = &otaProfileChar7Props,
    },
    // Characteristic 7 Value
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = otaProfileChar7UUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = NULL,
    },
    // Characteristic 7 User Description
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = charUserDescUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar7UserDesc,
    },
    // Characteristic 8 Declaration
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = characterUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = &otaProfileChar8Props,
    },
    // Characteristic 8 Value
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = otaProfileChar8UUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = NULL,
    },
    // Characteristic 8 User Description
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = charUserDescUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar8UserDesc,
    },
    // Characteristic 9 Declaration
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = characterUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = &otaProfileChar9Props,
    },
    // Characteristic 9 Value
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = otaProfileChar9UUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = NULL,
    },
    // Characteristic 9 User Description
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = charUserDescUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar9UserDesc,
    },
    // Characteristic 10 Declaration
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = characterUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = &otaProfileChar10Props,
    },
    // Characteristic 10 Value
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = otaProfileChar10UUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = NULL,
    },
    // Characteristic 10 User Description
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = charUserDescUUID,
        },
        .permissions = GATT_PERMIT_READ,
        .handle = 0, // Will be assigned by the stack
        .pValue = otaProfileChar10UserDesc,
    }
};

// Callback function definition
static bStatus_t OTAProfile_ReadAttrCB(
    uint16_t connHandle, 
    gattAttribute_t *pAttr, 
    uint8_t *pValue, 
    uint16_t *pLen, 
    uint16_t offset, 
    uint16_t maxLen, 
    uint8_t method
);
static bStatus_t OTAProfile_WriteAttrCB(
    uint16_t connHandle, 
    gattAttribute_t *pAttr, 
    uint8_t *pValue, 
    uint16_t len, 
    uint16_t offset, 
    uint8_t method
);

gattServiceCBs_t otaProfileCBs = {
    .pfnReadAttrCB = OTAProfile_ReadAttrCB,
    .pfnWriteAttrCB = OTAProfile_WriteAttrCB,
    .pfnAuthorizeAttrCB = NULL // Not used
};

void OTAProfile_RandomNextChallenge(void)
{
    // Generate a new random challenge token
    for(uint32_t i = 0; i < otaProfileChar3Len; i += 4)
    {
        uint32_t randVal = tmos_rand();
        // Copy the random value into the challenge token
        tmos_memcpy(&otaProfileChar3Val[i], &randVal, sizeof(uint32_t));
    }
}

bStatus_t OTAProfile_AddService(void)
{
    uint8_t status;

    // Generate the first random challenge token
    OTAProfile_RandomNextChallenge();

    // Initialize the async event system
    ota_async_event_init();

    // Register the service with the GATT server
    status = GATTServApp_RegisterService(
        otaProfileAttrTbl, 
        GATT_NUM_ATTRS(otaProfileAttrTbl), 
        GATT_MAX_ENCRYPT_KEY_SIZE,
        &otaProfileCBs
    );

    return status;
}

static bStatus_t OTA_PerpareRead_Handler(
    uint8_t *pValue, 
    uint16_t *pLen, 
    uint16_t offset, 
    uint16_t maxLen, 
    const uint8_t *data, 
    uint16_t datalen
)
{
    if(datalen == 0)
    {
        *pLen = 0;
        return SUCCESS; // No data to read
    }
    if(offset >= datalen)
    {
        *pLen = 0;
        return ATT_ERR_INVALID_OFFSET; // Offset is beyond the length of the string
    }
    *pLen = MIN(maxLen, datalen - offset);
    tmos_memcpy(pValue, &data[offset], *pLen);
    return SUCCESS;
}

static bStatus_t OTAProfile_ReadAttrCB(
    uint16_t connHandle, 
    gattAttribute_t *pAttr, 
    uint8_t *pValue, 
    uint16_t *pLen, 
    uint16_t offset, 
    uint16_t maxLen, 
    uint8_t method
)
{
    if(pAttr->type.len != ATT_BT_UUID_SIZE)
    {
        *pLen = 0;
        return ATT_ERR_INVALID_HANDLE; // Invalid attribute handle
    }
    uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

    uint32_t _flashbank;
    const char *flashBankStr;
    const char *flashModeStr;
    const char *bootReasonStr;

    switch(uuid)
    {
        case OTA_GATT_PROFILE_CHAR_UUID_MAIN:
            // Read the OTA main characteristic
            if(maxLen < sizeof(uint8_t) + sizeof(uint8_t))
                return ATT_ERR_INVALID_VALUE_SIZE; // Ensure enough space for uint8_t
            *pLen = sizeof(uint8_t) + sizeof(uint8_t); // 1 byte for status, 1 byte for async event status
            *((uint8_t *)pValue) = ota_is_busy_flag();
            *((uint8_t *)pValue + 1) = ota_get_async_event_status();
            return SUCCESS;
        case OTA_GATT_PROFILE_CHAR_UUID_BUFFER:
            // Read the OTA IO buffer
            return OTA_PerpareRead_Handler(
                pValue, 
                pLen, 
                offset, 
                maxLen, 
                otaProfileChar2Val,
                otaProfileChar2Len
            );
        case OTA_GATT_PROFILE_CHAR_UUID_CHALLENGE:
            // Read the OTA challenge token
            return OTA_PerpareRead_Handler(
                pValue, 
                pLen, 
                offset, 
                maxLen, 
                otaProfileChar3Val,
                otaProfileChar3Len
            );
        case OTA_GATT_PROFILE_CHAR_UUID_TOKEN:
            // Read the OTA authentication token
            return OTA_PerpareRead_Handler(
                pValue, 
                pLen, 
                offset, 
                maxLen, 
                otaProfileChar4Val,
                otaProfileChar4Len
            );
        case OTA_GATT_PROFILE_CHAR_UUID_FLASH_BANK:
            // Read the OTA flash bank
            if(maxLen < sizeof(uint32_t))
                return ATT_ERR_INVALID_VALUE_SIZE; // Ensure enough space for uint32_t
            *pLen = sizeof(uint32_t);
            // Get current flash bank and fill pValue
            // Must use tmos_memcpy to copy the value to avoid RISC-V misalignment faults
            _flashbank = ota_get_flags_current_flash_bank();
            tmos_memcpy(pValue, &_flashbank, sizeof(uint32_t));
            return SUCCESS;
        case OTA_GATT_PROFILE_CHAR_UUID_FLASH_BANK_READABLE:
            // Read the OTA flash bank as a string
            flashBankStr = ota_get_flags_current_flash_bank_string(); // Function to get flash bank as string    
            return OTA_PerpareRead_Handler( 
                pValue, 
                pLen, 
                offset, 
                maxLen, 
                (const uint8_t *)flashBankStr,
                tmos_strlen((char *)flashBankStr)
            );
        case OTA_GATT_PROFILE_CHAR_UUID_FLASH_MODE:
            // Read the OTA flash mode
            if(maxLen < sizeof(uint8_t))
                return ATT_ERR_INVALID_VALUE_SIZE; // Ensure enough space for uint8_t
            *pLen = sizeof(uint8_t);
            *((uint8_t *)pValue) = ota_get_flags_flash_mode_flag(); // Function to get current flash mode
            return SUCCESS;
        case OTA_GATT_PROFILE_CHAR_UUID_FLASH_MODE_READABLE:
            // Read the OTA flash mode as a string
            flashModeStr = ota_get_flags_flash_mode_flag_string(); // Function to get flash mode as string
            return OTA_PerpareRead_Handler(
                pValue, 
                pLen, 
                offset, 
                maxLen, 
                (const uint8_t *)flashModeStr,
                tmos_strlen((char *)flashModeStr)
            );
        case OTA_GATT_PROFILE_CHAR_UUID_BOOT_REASON:
            // Read the OTA boot reason
            if(maxLen < sizeof(uint8_t))
                return ATT_ERR_INVALID_VALUE_SIZE; // Ensure enough space for uint8_t
            *pLen = sizeof(uint8_t);
            *((uint8_t *)pValue) = ota_get_flags_boot_reason_code(); // Function to get current boot reason
            return SUCCESS;
        case OTA_GATT_PROFILE_CHAR_UUID_BOOT_REASON_READABLE:
            // Read the OTA boot reason as a string
            bootReasonStr = ota_get_flags_boot_reason_code_string(); // Function to get boot reason as string
            return OTA_PerpareRead_Handler(
                pValue, 
                pLen, 
                offset, 
                maxLen, 
                (const uint8_t *)bootReasonStr,
                tmos_strlen((char *)bootReasonStr)
            );
        default:
            *pLen = 0;
            return ATT_ERR_ATTR_NOT_FOUND; // Attribute not found
    }
}

static bStatus_t OTA_PrepareWrite_Handler(
    uint8_t *buffer,
    uint32_t *newBufLen,
    uint32_t *isWriting,
    uint32_t bufferMaxLen,
    uint8_t *pValue, 
    uint16_t len, 
    uint16_t offset,
    uint8_t method
)
{
    switch(method)
    {
        case ATT_PREPARE_WRITE_REQ:
            // Prepare write request
            // Check if the length is valid
            if(len + offset > bufferMaxLen)
            {
                return ATT_ERR_INVALID_VALUE_SIZE; // Length exceeds buffer size
            }
            if((*isWriting) == 0)
            {
                // If offset is 0, we are starting a new write operation
                tmos_memset(buffer, 0, bufferMaxLen); // Clear the buffer
                *newBufLen = 0; // Reset the buffer length
                *isWriting = 1; // Mark as writing
            }
            // Copy the value into the buffer at the specified offset
            if((offset + len) > *newBufLen)
            {
                *newBufLen = offset + len; // Update the new buffer length
            }
            tmos_memcpy(&buffer[offset], pValue, len);
            return SUCCESS; // Return success
        case ATT_EXECUTE_WRITE_REQ:
            // Execute write request
            if((*isWriting) == 0)
            {
                return ATT_ERR_PREPARE_QUEUE_FULL; // No write operation in progress
            }
            // Finalize the write operation
            *isWriting = 0; // Reset the writing flag
            return SUCCESS;
        case ATT_WRITE_REQ:
        case ATT_WRITE_CMD:
            // Handle direct write request
            if(len > bufferMaxLen)
            {
                return ATT_ERR_INVALID_VALUE_SIZE; // Length exceeds buffer size
            }
            if(offset != 0)
            {
                return ATT_ERR_INVALID_OFFSET; // Offset must be zero for direct write
            }
            tmos_memcpy(buffer, pValue, len); // Copy the value directly into the buffer
            *newBufLen = len; // Update the new buffer length
            *isWriting = 0; // Reset the writing flag
            return SUCCESS; // Return success
        default:
            return ATT_ERR_INVALID_PDU; // Invalid method
    }
}

static bStatus_t OTAProfile_WriteAttrCB(
    uint16_t connHandle, 
    gattAttribute_t *pAttr, 
    uint8_t *pValue, 
    uint16_t len, 
    uint16_t offset, 
    uint8_t method
)
{
    if(pAttr->type.len != ATT_BT_UUID_SIZE)
    {
        return ATT_ERR_INVALID_HANDLE; // Invalid attribute handle
    }
    uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
    bStatus_t status;

    if(ota_is_busy_flag())
    {
        // Still handling an OTA asynchronous event, cannot perform new write
        return ATT_ERR_WRITE_NOT_PERMITTED; 
    }

    switch(uuid)
    {
        case OTA_GATT_PROFILE_CHAR_UUID_MAIN:
            // Handle OTA command
            status = ota_cmd_handler(
                pValue, 
                len, 
                otaProfileChar2Val,
                otaProfileChar2Len,
                otaProfileChar3Val,
                otaProfileChar3Len,
                otaProfileChar4Val,
                otaProfileChar4Len
            );
            break;
        case OTA_GATT_PROFILE_CHAR_UUID_BUFFER:
            // Write to the OTA IO buffer
            status = OTA_PrepareWrite_Handler(
                otaProfileChar2Val, 
                &otaProfileChar2Len, 
                &otaProfileChar2IsWritting,
                OTA_IO_BUFFER_SIZE, 
                pValue, 
                len, 
                offset,
                method
            );
            break;
        case OTA_GATT_PROFILE_CHAR_UUID_TOKEN:
            // Write the signature token
            // Directly return, this should not affect the challenge token
            return OTA_PrepareWrite_Handler(
                otaProfileChar4Val, 
                &otaProfileChar4Len, 
                &otaProfileChar4IsWritting,
                sizeof(otaProfileChar4Val), 
                pValue, 
                len, 
                offset,
                method
            );
        default:
            return ATT_ERR_ATTR_NOT_FOUND; // Attribute not found
    }
    OTAProfile_RandomNextChallenge();
    return status;
}
