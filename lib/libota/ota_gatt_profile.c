// ota_gatt_profile.c
// This file contains the implementation of the OTA GATT profile for the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#include "ota_gatt_profile.h"

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

// GATT Profile Service attributes
static const gattAttrType_t otaProfileService = {
    .len = ATT_BT_UUID_SIZE,
    .uuid = otaProfileServiceUUID
};

// Characteristic 1 Properties
static uint8_t otaProfileChar1Props = GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// Characteristic 1 Value
static uint8_t otaProfileChar1Val = 0;

// Characteristic 1 User Description
static uint8_t otaProfileChar1UserDesc[] = "OTA Command";

// Characteristic 2 Properties
static uint8_t otaProfileChar2Props = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// Characteristic 2 Value
static uint8_t otaProfileChar2Val[OTA_IO_BUFFER_SIZE] = {0};
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

// Profile Attributes Table
static gattAttribute_t otaProfileAttrTbl[] = {
    // Service Declaration
    {
        .type = {
            .len = ATT_BT_UUID_SIZE,
            .uuid = primaryServiceUUID,
        }
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
        .pValue = &otaProfileChar1Val,
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
    }
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

bStatus_t OTAProfile_AddService(uint32_t services)
{
    uint8_t status = SUCCESS;

    if(services & OTA_GATT_PROFILE_SERVICES)
    {
        // Register the service with the GATT server
        status = GATTServApp_RegisterService(
            otaProfileAttrTbl, 
            GATT_NUM_ATTRS(otaProfileAttrTbl), 
            GATT_MAX_ENCRYPT_KEY_SIZE,
            &otaProfileCBs
        );
    }

    return status;
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

    switch(uuid)
    {
        case OTA_GATT_PROFILE_CHAR_UUID_BUFFER:
            // Read the OTA IO buffer
            if(otaProfileChar2Len == 0)
            {
                *pLen = 0;
                return SUCCESS; // No data to read
            }
            if(offset >= otaProfileChar2Len)
            {
                *pLen = 0;
                return ATT_ERR_INVALID_OFFSET; // Offset is beyond the length of the buffer
            }
            *pLen = MIN(maxLen, otaProfileChar2Len - offset);
            tmos_memcpy(pValue, &otaProfileChar2Val[offset], *pLen);
            return SUCCESS;
        case OTA_GATT_PROFILE_CHAR_UUID_CHALLENGE:
            // Read the OTA challenge token
            if(offset >= otaProfileChar3Len)
            {
                *pLen = 0;
                return ATT_ERR_INVALID_OFFSET; // Offset is beyond the length of the challenge token
            }
            *pLen = MIN(maxLen, otaProfileChar3Len - offset);
            tmos_memcpy(pValue, &otaProfileChar3Val[offset], *pLen);
            return SUCCESS;
        case OTA_GATT_PROFILE_CHAR_UUID_TOKEN:
            // Read the OTA token
            if(offset >= otaProfileChar4Len)
            {
                *pLen = 0;
                return ATT_ERR_INVALID_OFFSET; // Offset is beyond the length of the token
            }
            *pLen = MIN(maxLen, otaProfileChar4Len - offset);
            tmos_memcpy(pValue, &otaProfileChar4Val[offset], *pLen);
            return SUCCESS;
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
        case ATT_PREP_WRITE_REQ:
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
        case ATT_EXEC_WRITE_REQ:
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

    switch(uuid)
    {
        case OTA_GATT_PROFILE_CHAR_UUID_MAIN:
            // Handle OTA command
            // todo: Implement command handling logic
            return SUCCESS;
        case OTA_GATT_PROFILE_CHAR_UUID_BUFFER:
            // Write to the OTA IO buffer
            return OTA_PrepareWrite_Handler(
                otaProfileChar2Val, 
                &newOtaProfileChar2Len, 
                &otaProfileChar2IsWritting,
                OTA_IO_BUFFER_SIZE, 
                pValue, 
                len, 
                offset,
                method
            );
        case OTA_GATT_PROFILE_CHAR_UUID_TOKEN:
            // Write the signature token
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
}
