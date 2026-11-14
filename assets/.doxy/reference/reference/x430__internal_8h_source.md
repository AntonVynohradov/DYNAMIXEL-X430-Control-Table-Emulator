

# File x430\_internal.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**x430\_internal.h**](x430__internal_8h.md)

[Go to the documentation of this file](x430__internal_8h.md)


```C++
/*******************************************************************************
 * DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
 *
 * Project:     DYNAMIXEL-X430-Control-Table-Emulator
 * Description: Private declarations: the state behind the opaque instances and
 *              the functions the modules share.
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

#ifndef X430_INTERNAL_H
#define X430_INTERNAL_H

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "x430.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==============================================================================
 * DEFINES & MACROS
 * =============================================================================*/

#define X430_READY 0x58343330U

/* ==============================================================================
 * TYPE DEFINITIONS
 * =============================================================================*/

struct x430_bus_ctx
{
    struct dxl2_link link;                         
    struct x430_bus_config config;                 
    struct x430_device* devices[X430_MAX_DEVICES]; 
    uint32_t ready;                                
    uint32_t last_rx_ms;                           
    uint8_t device_count;                          
    bool write_failed;                             
    uint8_t tx[DXL2_MAX_PACKET_SIZE];              
};

struct x430_device_ctx
{
    struct x430_device_config config;      
    struct x430_bus* bus;                  
    uint32_t ready;                        
    uint16_t reg_address;                  
    uint16_t reg_length;                   
    bool baud_changed;                     
    uint8_t table[X430_IMAGE_SIZE];        
    uint8_t reg_data[X430_REG_WRITE_SIZE]; 
};

struct x430_sim_ctx
{
    int64_t shaft_mpulse;          
    int64_t velocity_mpulse;       
    struct x430_device* device;    
    struct x430_sim_config config; 
    uint32_t ready;                
};

/* ==============================================================================
 * INLINE FUNCTIONS
 * =============================================================================*/

static inline struct x430_bus_ctx* x430_bus_ctx_of(struct x430_bus* bus)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (struct x430_bus_ctx*)bus;
}

static inline const struct x430_bus_ctx* x430_bus_ctx_of_const(const struct x430_bus* bus)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (const struct x430_bus_ctx*)bus;
}

static inline struct x430_device_ctx* x430_device_ctx_of(struct x430_device* device)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (struct x430_device_ctx*)device;
}

static inline const struct x430_device_ctx*
x430_device_ctx_of_const(const struct x430_device* device)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (const struct x430_device_ctx*)device;
}

static inline struct x430_sim_ctx* x430_sim_ctx_of(struct x430_sim* sim)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (struct x430_sim_ctx*)sim;
}

/* ==============================================================================
 * INTERNAL FUNCTIONS: x430_bus.c
 * =============================================================================*/

bool x430_bus_id_taken(const struct x430_bus* bus, const struct x430_device* except, uint8_t id);

uint32_t x430_bus_millis(const struct x430_bus* bus);

size_t x430_frame_status(uint8_t* tx, size_t capacity, uint8_t id, uint16_t count);

/* ==============================================================================
 * INTERNAL FUNCTIONS: x430_device.c (initialized devices only, no checks)
 * =============================================================================*/

int32_t x430_device_value(const struct x430_device* device, uint16_t address);

void x430_device_store(struct x430_device* device, uint16_t address, int32_t value);

uint8_t x430_device_byte(const struct x430_device* device, uint16_t address);

uint8_t x430_device_alert(const struct x430_device* device);

void x430_device_watchdog(struct x430_device* device, uint32_t idle_ms);

struct x430_bus* x430_device_bus(const struct x430_device* device);

void x430_device_attach(struct x430_device* device, struct x430_bus* bus);

bool x430_device_ready(const struct x430_device* device);

void x430_copy(uint8_t* destination, const uint8_t* source, uint16_t length);

void x430_fill(uint8_t* destination, uint8_t value, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* X430_INTERNAL_H */
```


