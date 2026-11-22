

# File x430\_types.h

[**File List**](files.md) **>** [**inc**](dir_bfccd401955b95cf8c75461437045ac0.md) **>** [**x430\_types.h**](x430__types_8h.md)

[Go to the documentation of this file](x430__types_8h.md)


```C++
/*******************************************************************************
 * DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
 *
 * Project:     DYNAMIXEL-X430-Control-Table-Emulator
 * Description: Public types: callbacks, configuration structures and the
 *              opaque, caller-owned instances (bus, device, simulator).
 *
 * Author:      Anton Vynohradov
 * Email:       avynohradov@systemfromscratch.com
 *
 * License:     Apache License 2.0
 *
 * Copyright (c) 2026 Anton Vynohradov
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

#ifndef X430_TYPES_H
#define X430_TYPES_H

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <stddef.h>
#include <stdint.h>

#include "x430_config.h" /* first: sets DXL2_MAX_PACKET_SIZE for dxl2.h */
#include "dxl2.h"
#include "x430_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==============================================================================
 * INSTANCES (FORWARD DECLARATIONS)
 * =============================================================================*/

struct x430_bus;
struct x430_device;

/* ==============================================================================
 * PLATFORM PORT
 * =============================================================================*/

typedef void (*x430_lock_fn)(void* context);

typedef uint32_t (*x430_millis_fn)(void* context);

typedef void (*x430_delay_fn)(void* context, uint32_t us);

struct x430_port
{
    void* context;          
    x430_lock_fn lock;      
    x430_lock_fn unlock;    
    x430_millis_fn millis;  
    x430_delay_fn delay_us; 
};

/* ==============================================================================
 * APPLICATION HOOKS
 * =============================================================================*/

typedef uint8_t (*x430_write_hook)(struct x430_device* device, uint16_t address, uint8_t size,
                                   int32_t value, void* user_data);

typedef void (*x430_read_hook)(struct x430_device* device, uint16_t address, uint16_t length,
                               void* user_data);

typedef uint8_t (*x430_event_hook)(struct x430_device* device, enum x430_event event,
                                   void* user_data);

typedef int (*x430_nvm_read_hook)(struct x430_device* device, enum x430_nvm_slot slot,
                                  uint16_t offset, uint8_t* data, uint16_t length, void* user_data);

typedef int (*x430_nvm_write_hook)(struct x430_device* device, enum x430_nvm_slot slot,
                                   uint16_t offset, const uint8_t* data, uint16_t length,
                                   void* user_data);

struct x430_hooks
{
    x430_write_hook write;         
    x430_read_hook read;           
    x430_event_hook event;         
    x430_nvm_read_hook nvm_read;   
    x430_nvm_write_hook nvm_write; 
};

/* ==============================================================================
 * CONFIGURATION TYPES
 * =============================================================================*/

struct x430_bus_config
{
    dxl2_write_fn write;          
    void* write_context;          
    const struct x430_port* port; 
};

struct x430_device_config
{
    uint8_t id;
    uint8_t firmware_version;
    uint32_t model_information;
    const struct x430_hooks* hooks;
    void* user_data;
};

struct x430_sim_config
{
    uint16_t input_voltage; 
    uint8_t temperature;    
};

/* ==============================================================================
 * INSTANCES
 * =============================================================================*/

#define X430_NVM_EEPROM_SIZE (X430_NVM_HEADER_SIZE + X430_RAM_START + 2U)
#define X430_NVM_BACKUP_SIZE (X430_NVM_HEADER_SIZE + X430_IMAGE_SIZE + 2U)

#if defined(UINTPTR_MAX) && (UINTPTR_MAX > 0xFFFFFFFFU)
#define X430_POINTER_BYTES 8U
#else
#define X430_POINTER_BYTES 4U
#endif

#define X430_ALIGN_UP(n, a) (((((n) + (a)) - 1U) / (a)) * (a))

#define X430_DXL2_LINK_BYTES                                                                      \
    X430_ALIGN_UP(                                                                                \
        (4U * X430_POINTER_BYTES) + X430_ALIGN_UP(DXL2_MAX_PACKET_SIZE, X430_POINTER_BYTES) +     \
            X430_POINTER_BYTES + X430_ALIGN_UP(4U, X430_POINTER_BYTES) + X430_POINTER_BYTES + 8U, \
        X430_POINTER_BYTES)

#define X430_BUS_BYTES                                  \
    (X430_DXL2_LINK_BYTES + (3U * X430_POINTER_BYTES) + \
     (X430_MAX_DEVICES_U * ((2U * X430_POINTER_BYTES) + 8U)) + 16U + DXL2_MAX_PACKET_SIZE)

#define X430_DEVICE_BYTES \
    ((3U * X430_POINTER_BYTES) + 17U + X430_IMAGE_SIZE + X430_REG_WRITE_SIZE_U)

#define X430_SIM_BYTES (24U + X430_POINTER_BYTES)

#define X430_BUS_WORDS ((X430_BUS_BYTES + 7U) / 8U)
#define X430_DEVICE_WORDS ((X430_DEVICE_BYTES + 7U) / 8U)
#define X430_SIM_WORDS ((X430_SIM_BYTES + 7U) / 8U)

struct x430_bus
{
    uint64_t opaque[X430_BUS_WORDS]; 
};

struct x430_device
{
    uint64_t opaque[X430_DEVICE_WORDS]; 
};

struct x430_sim
{
    uint64_t opaque[X430_SIM_WORDS]; 
};

#ifdef __cplusplus
}
#endif

#endif /* X430_TYPES_H */
```


