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
#include <stdint.h>

#include "x430.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==============================================================================
 * DEFINES & MACROS
 * =============================================================================*/

/** @brief Value of the ready field after a successful initialization. A zeroed or
 *         never initialized instance holds another value: every function but the
 *         *_init() functions then returns ::X430_ERR_STATE. */
#define X430_READY 0x58343330U

/* ==============================================================================
 * TYPE DEFINITIONS
 * =============================================================================*/

/** @brief State behind the opaque ::x430_bus. Its layout may change in any release. */
struct x430_bus_ctx
{
    struct dxl2_link link;                         /**< DXL2-C parser/encoder. */
    struct x430_bus_config config;                 /**< Copy of the configuration. */
    struct x430_device* devices[X430_MAX_DEVICES]; /**< Attached devices. */
    uint32_t ready;                                /**< ::X430_READY after x430_bus_init(). */
    uint32_t last_rx_ms;                           /**< Time of the last instruction packet. */
    uint8_t device_count;                          /**< Entries in devices. */
    bool write_failed;                             /**< The writer failed since the last call. */
    uint8_t data[DXL2_MAX_PARAMETER_SIZE];         /**< Status data scratch buffer. */
#if X430_FEATURE_FAST_READ
    uint8_t raw[DXL2_MAX_PACKET_SIZE]; /**< Fast read response packet. */
#endif
};

/** @brief State behind the opaque ::x430_device. Its layout may change in any release. */
struct x430_device_ctx
{
    struct x430_device_config config;      /**< Copy of the configuration. */
    struct x430_bus* bus;                  /**< Owning bus, NULL before attach. */
    uint32_t ready;                        /**< ::X430_READY after x430_device_init(). */
    uint16_t reg_address;                  /**< REG_WRITE address. */
    uint16_t reg_length;                   /**< REG_WRITE length, 0 = none. */
    bool baud_changed;                     /**< Deferred ::X430_EVENT_BAUD_RATE. */
    uint8_t table[X430_TABLE_SIZE];        /**< Control Table image, little endian. */
    uint8_t reg_data[X430_REG_WRITE_SIZE]; /**< REG_WRITE data. */
    uint8_t nvm[X430_NVM_BUFFER_SIZE];     /**< Persistent block buffer. */
};

/** @brief State behind the opaque ::x430_sim. Its layout may change in any release. */
struct x430_sim_ctx
{
    int64_t shaft_mpulse;          /**< Shaft position [1/1000 pulse]. */
    int64_t velocity_mpulse;       /**< Shaft velocity [1/1000 pulse per second]. */
    struct x430_device* device;    /**< Simulated device. */
    struct x430_sim_config config; /**< Copy of the configuration. */
    uint32_t ready;                /**< ::X430_READY after x430_sim_init(). */
};

/* ==============================================================================
 * INLINE FUNCTIONS
 * =============================================================================*/

/**
 * @brief  State of a bus.
 * @note   MISRA C:2012 Rule 11.3, deviation DEV-01 (docs/misra.md): ::x430_bus is the
 *         opaque storage of ::x430_bus_ctx; size and alignment are checked in x430_bus.c.
 */
static inline struct x430_bus_ctx* x430_bus_ctx_of(struct x430_bus* bus)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (struct x430_bus_ctx*)bus;
}

/** @brief Read-only variant of x430_bus_ctx_of(). */
static inline const struct x430_bus_ctx* x430_bus_ctx_of_const(const struct x430_bus* bus)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (const struct x430_bus_ctx*)bus;
}

/**
 * @brief  State of a device.
 * @note   MISRA C:2012 Rule 11.3, deviation DEV-01 (docs/misra.md); checked in x430_device.c.
 */
static inline struct x430_device_ctx* x430_device_ctx_of(struct x430_device* device)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (struct x430_device_ctx*)device;
}

/** @brief Read-only variant of x430_device_ctx_of(). */
static inline const struct x430_device_ctx*
x430_device_ctx_of_const(const struct x430_device* device)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (const struct x430_device_ctx*)device;
}

/**
 * @brief  State of a simulator.
 * @note   MISRA C:2012 Rule 11.3, deviation DEV-01 (docs/misra.md); checked in x430_sim.c.
 */
static inline struct x430_sim_ctx* x430_sim_ctx_of(struct x430_sim* sim)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (struct x430_sim_ctx*)sim;
}

/* ==============================================================================
 * INTERNAL FUNCTIONS: x430_bus.c
 * =============================================================================*/

/** @brief True when another device on @p bus uses @p id (@p bus may be NULL). */
bool x430_bus_id_taken(const struct x430_bus* bus, const struct x430_device* except, uint8_t id);

/** @brief Milliseconds from the port of @p bus; 0 without bus or clock. */
uint32_t x430_bus_millis(const struct x430_bus* bus);

/* ==============================================================================
 * INTERNAL FUNCTIONS: x430_device.c (initialized devices only, no checks)
 * =============================================================================*/

/** @brief Value of the register that starts at @p address, 0 when none starts there. */
int32_t x430_device_value(const struct x430_device* device, uint16_t address);

/** @brief Store @p value into the register that starts at @p address (none: no effect). */
void x430_device_store(struct x430_device* device, uint16_t address, int32_t value);

/** @brief One byte of the Control Table image. */
uint8_t x430_device_byte(const struct x430_device* device, uint16_t address);

/** @brief Status error alert: DXL2_ERR_ALERT_BIT when Hardware Error Status is not 0. */
uint8_t x430_device_alert(const struct x430_device* device);

/** @brief Bus Watchdog check; @p idle_ms is the time since the last instruction packet. */
void x430_device_watchdog(struct x430_device* device, uint32_t idle_ms);

/** @brief Bus of the device, NULL when it is not attached. */
struct x430_bus* x430_device_bus(const struct x430_device* device);

/** @brief Records the bus of a device on x430_bus_attach(). */
void x430_device_attach(struct x430_device* device, struct x430_bus* bus);

/** @brief True for a device that completed x430_device_init() (device may be NULL). */
bool x430_device_ready(const struct x430_device* device);

/** @brief Copies @p length bytes (no <string.h>). */
void x430_copy(uint8_t* destination, const uint8_t* source, uint16_t length);

/** @brief Sets @p length bytes to @p value (no <string.h>). */
void x430_fill(uint8_t* destination, uint8_t value, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* X430_INTERNAL_H */
