// ota_common.h
// This file contains common definitions and includes for the CH58x series microcontroller.
// Author: Iluna Angelic47 <admin@angelic47.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef __OTA_COMMON_H__
#define __OTA_COMMON_H__

#ifdef CH58xBLE_ROM
#include "CH58xBLE_ROM.H"
#else
#include "CH58xBLE_LIB.h"
#endif

#include "CH58x_common.h"

#endif // __OTA_COMMON_H__
