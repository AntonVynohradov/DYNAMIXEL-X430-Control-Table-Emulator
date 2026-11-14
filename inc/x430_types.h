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

/**
 * @defgroup x430_types Types
 * @ingroup x430
 * @brief Callbacks, configuration structures and the opaque instances.
 * @{
 */

/* ==============================================================================
 * INSTANCES (FORWARD DECLARATIONS)
 * =============================================================================*/

struct x430_bus;
struct x430_device;

/* ==============================================================================
 * PLATFORM PORT
 * =============================================================================*/

/**
 * @brief  Take or release the bus lock (x430_port::lock, x430_port::unlock).
 * @param  context  x430_port::context.
 */
typedef void (*x430_lock_fn)(void* context);

/**
 * @brief  Free-running millisecond counter (x430_port::millis).
 * @param  context  x430_port::context.
 * @return Milliseconds; wraps at 2^32.
 */
typedef uint32_t (*x430_millis_fn)(void* context);

/**
 * @brief  Busy or sleeping delay (x430_port::delay_us).
 * @param  context  x430_port::context.
 * @param  us       Delay in microseconds.
 */
typedef void (*x430_delay_fn)(void* context, uint32_t us);

/**
 * @brief Platform services. Every member may be NULL; the library then works
 *        without that service:
 *
 * | Member   | Used for                                 | NULL means                  |
 * |----------|------------------------------------------|-----------------------------|
 * | lock     | mutual exclusion of the Control Tables   | single context (bare metal) |
 * | unlock   | (pair of lock)                           |                             |
 * | millis   | Realtime Tick (120), Bus Watchdog (98)   | tick 0, no watchdog timeout |
 * | delay_us | Return Delay Time (9) before a status    | status sent at once         |
 *
 * The bus keeps a pointer to the port: keep it alive as long as the bus.
 */
struct x430_port
{
    void* context;          /**< Passed to every function. */
    x430_lock_fn lock;      /**< Take the bus lock. */
    x430_lock_fn unlock;    /**< Release the bus lock. */
    x430_millis_fn millis;  /**< Free-running millisecond counter. */
    x430_delay_fn delay_us; /**< Delay before a status packet. */
};

/* ==============================================================================
 * APPLICATION HOOKS
 * =============================================================================*/

/**
 * @brief  A validated register write, before the value is stored (x430_hooks::write).
 * @param  device     Device written.
 * @param  address    First address of the register.
 * @param  size       Register size in bytes (1, 2 or 4).
 * @param  value      New value (sign-extended for signed registers).
 * @param  user_data  x430_device_config::user_data.
 * @return DXL2_ERR_NONE to accept, another DXL2_ERR_* code to refuse (the
 *         instruction stops at this register and the host gets that error).
 */
typedef uint8_t (*x430_write_hook)(struct x430_device* device, uint16_t address, uint8_t size,
                                   int32_t value, void* user_data);

/**
 * @brief  Data is about to be read (x430_hooks::read). Called once per contiguous
 *         run of real addresses (Indirect Data is resolved to the addresses it
 *         points at). The hook may refresh live registers with x430_device_set().
 * @param  device     Device read.
 * @param  address    First real address of the run.
 * @param  length     Bytes in the run.
 * @param  user_data  x430_device_config::user_data.
 */
typedef void (*x430_read_hook)(struct x430_device* device, uint16_t address, uint16_t length,
                               void* user_data);

/**
 * @brief  Device event (x430_hooks::event).
 * @param  device     Device.
 * @param  event      What happened.
 * @param  user_data  x430_device_config::user_data.
 * @return DXL2_ERR_NONE, or a DXL2_ERR_* code where the event can be refused
 *         (::X430_EVENT_CLEAR_MULTI_TURN); ignored otherwise.
 */
typedef uint8_t (*x430_event_hook)(struct x430_device* device, enum x430_event event,
                                   void* user_data);

/**
 * @brief  Load a persistent block (x430_hooks::nvm_load).
 * @param  device     Device.
 * @param  slot       Block to load.
 * @param  data       Destination of @p size bytes.
 * @param  size       ::X430_NVM_EEPROM_SIZE or ::X430_NVM_BACKUP_SIZE.
 * @param  user_data  x430_device_config::user_data.
 * @retval 0   @p size bytes were read.
 * @retval <0  No data (the factory values stay).
 */
typedef int (*x430_nvm_load_hook)(struct x430_device* device, enum x430_nvm_slot slot,
                                  uint8_t* data, uint16_t size, void* user_data);

/**
 * @brief  Store a persistent block (x430_hooks::nvm_store), after the change; may be slow.
 * @param  device     Device.
 * @param  slot       Block to store.
 * @param  data       @p size bytes: header, payload and CRC, opaque to the application.
 * @param  size       ::X430_NVM_EEPROM_SIZE or ::X430_NVM_BACKUP_SIZE.
 * @param  user_data  x430_device_config::user_data.
 * @retval 0   Stored.
 * @retval <0  Failure, reported to the host as Result Fail.
 */
typedef int (*x430_nvm_store_hook)(struct x430_device* device, enum x430_nvm_slot slot,
                                   const uint8_t* data, uint16_t size, void* user_data);

/**
 * @brief Application hooks of a device. Every member may be NULL.
 *
 * Hooks run in the bus context with the bus lock held. They may call the
 * lock-free device functions (x430_device_get(), x430_device_set(),
 * x430_device_id(), x430_device_raise_hardware_error()), but no bus function.
 */
struct x430_hooks
{
    x430_write_hook write;         /**< Every accepted register write. */
    x430_read_hook read;           /**< Before data is read. */
    x430_event_hook event;         /**< Reboot, factory reset, watchdog, ... */
    x430_nvm_load_hook nvm_load;   /**< Load the EEPROM area or the backup. */
    x430_nvm_store_hook nvm_store; /**< Store the EEPROM area or the backup. */
};

/* ==============================================================================
 * CONFIGURATION TYPES
 * =============================================================================*/

/** @brief Bus configuration, copied by x430_bus_init(). */
struct x430_bus_config
{
    dxl2_write_fn write;          /**< Writes a complete packet to the line (required). */
    void* write_context;          /**< Passed to write. */
    const struct x430_port* port; /**< Platform services, NULL = none. */
};

/** @brief Device configuration, copied by x430_device_init(). */
struct x430_device_config
{
    /** Factory ID of this device, 0..252 (0 = model default 1). Stored EEPROM wins. */
    uint8_t id;
    /** Firmware Version (6); 0 = ::X430_FIRMWARE_VERSION. */
    uint8_t firmware_version;
    /** Model Information (2), free for the application (e.g. its firmware version). */
    uint32_t model_information;
    /** Application hooks; NULL = pure register device. */
    const struct x430_hooks* hooks;
    /** Passed to every hook. */
    void* user_data;
};

/** @brief Simulator configuration, copied by x430_sim_init(). Zero selects the default. */
struct x430_sim_config
{
    uint16_t input_voltage; /**< Present Input Voltage [0.1 V] (default 120 = 12.0 V). */
    uint8_t temperature;    /**< Present Temperature [deg C] (default 30). */
};

/* ==============================================================================
 * INSTANCES
 * =============================================================================*/

/** @brief Size of the persistent EEPROM block (nvm_load / nvm_store). */
#define X430_NVM_EEPROM_SIZE (X430_NVM_HEADER_SIZE + X430_RAM_START + 2U)
/** @brief Size of the persistent Control Table backup block. */
#define X430_NVM_BACKUP_SIZE (X430_NVM_HEADER_SIZE + X430_TABLE_SIZE + 2U)

/** @brief Bytes of the persistent block buffer inside each device. */
#if X430_FEATURE_BACKUP
#define X430_NVM_BUFFER_SIZE X430_NVM_BACKUP_SIZE
#else
#define X430_NVM_BUFFER_SIZE X430_NVM_EEPROM_SIZE
#endif

/** @brief Size of a pointer and of size_t: 8 on 64-bit hosts, 4 on Cortex-M. */
#if defined(UINTPTR_MAX) && (UINTPTR_MAX > 0xFFFFFFFFU)
#define X430_POINTER_BYTES 8U
#else
#define X430_POINTER_BYTES 4U
#endif

/** @brief @p n rounded up to a multiple of @p a. */
#define X430_ALIGN_UP(n, a) (((((n) + (a)) - 1U) / (a)) * (a))

/**
 * @brief Bytes of struct dxl2_link of DXL2-C 1.0.0 (configuration, parser,
 *        packet, counters), from DXL2_MAX_PACKET_SIZE. Plain arithmetic so that
 *        the instance size needs no sizeof (static analysis); x430_bus.c checks
 *        it at compile time.
 */
#define X430_DXL2_LINK_BYTES                                                                  \
    X430_ALIGN_UP((4U * X430_POINTER_BYTES) +                                                 \
                      X430_ALIGN_UP(DXL2_MAX_PACKET_SIZE, X430_POINTER_BYTES) +               \
                      X430_POINTER_BYTES + X430_ALIGN_UP(DXL2_MAX_PACKET_SIZE - 6U, 4U) + 8U, \
                  X430_POINTER_BYTES)

/** @brief Bytes of the Fast Sync/Bulk Read response buffer of a bus. */
#if X430_FEATURE_FAST_READ
#define X430_FAST_READ_BYTES DXL2_MAX_PACKET_SIZE
#else
#define X430_FAST_READ_BYTES 0U
#endif

/** @brief Private state of a bus in bytes; x430_bus.c checks it at compile time. */
#define X430_BUS_BYTES                                                           \
    (X430_DXL2_LINK_BYTES + (3U * X430_POINTER_BYTES) +                          \
     (X430_MAX_DEVICES_U * X430_POINTER_BYTES) + 10U + DXL2_MAX_PARAMETER_SIZE + \
     X430_FAST_READ_BYTES)

/** @brief Private state of a device in bytes; x430_device.c checks it at compile time. */
#define X430_DEVICE_BYTES                                                        \
    ((3U * X430_POINTER_BYTES) + 17U + X430_TABLE_SIZE + X430_REG_WRITE_SIZE_U + \
     X430_NVM_BUFFER_SIZE)

/** @brief Private state of a simulator in bytes; x430_sim.c checks it at compile time. */
#define X430_SIM_BYTES (24U + X430_POINTER_BYTES)

/** @brief Size of ::x430_bus in 64-bit words. */
#define X430_BUS_WORDS ((X430_BUS_BYTES + 7U) / 8U)
/** @brief Size of ::x430_device in 64-bit words. */
#define X430_DEVICE_WORDS ((X430_DEVICE_BYTES + 7U) / 8U)
/** @brief Size of ::x430_sim in 64-bit words. */
#define X430_SIM_WORDS ((X430_SIM_BYTES + 7U) / 8U)

/**
 * @brief One serial line with up to ::X430_MAX_DEVICES emulated devices.
 *        Allocated by the application (static, stack or pool) and initialized
 *        by x430_bus_init(). The content is private: use the functions of x430.h.
 */
struct x430_bus
{
    uint64_t opaque[X430_BUS_WORDS]; /**< Private state. */
};

/**
 * @brief One emulated actuator (one DYNAMIXEL ID): Control Table image and rules.
 *        Allocated by the application and initialized by x430_device_init().
 *        The content is private: use the functions of x430.h.
 */
struct x430_device
{
    uint64_t opaque[X430_DEVICE_WORDS]; /**< Private state. */
};

/**
 * @brief Kinematic model of an X430 behind a device. Allocated by the application
 *        and initialized by x430_sim_init(). The content is private.
 */
struct x430_sim
{
    uint64_t opaque[X430_SIM_WORDS]; /**< Private state. */
};

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* X430_TYPES_H */
