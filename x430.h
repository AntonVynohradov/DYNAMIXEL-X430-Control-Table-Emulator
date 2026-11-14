/*******************************************************************************
 * DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
 *
 * Project:     DYNAMIXEL-X430-Control-Table-Emulator
 * Description: Public API: library version, public types and all public functions.
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

#ifndef X430_H
#define X430_H

/* ==============================================================================
 * VERSION
 * =============================================================================*/

/* Set by cmake/version.cmake from the latest vX.Y.Z Git tag and stamped into this
 * file in the release package. 0.0.0 means unknown: a header taken from the
 * repository and built without CMake. Do not change these values for a release. */

#ifndef X430_VERSION_MAJOR
#define X430_VERSION_MAJOR 0
#endif

#ifndef X430_VERSION_MINOR
#define X430_VERSION_MINOR 0
#endif

#ifndef X430_VERSION_PATCH
#define X430_VERSION_PATCH 0
#endif

#ifndef X430_VERSION
#define X430_VERSION "0.0.0"
#endif

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <stddef.h>
#include <stdint.h>

#include "x430_config.h" /* first: sets DXL2_MAX_PACKET_SIZE for dxl2.h */
#include "dxl2.h"
#include "x430_defs.h"
#include "x430_model.h"
#include "x430_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup x430 DYNAMIXEL X430 Control Table Emulator API
 * @brief    Device-side emulation of DYNAMIXEL X430 actuators over Protocol 2.0:
 *           the firmware moves any motor, the host sees an X430-compatible device.
 *
 * **Parts.** A *bus* (::x430_bus) decodes the instructions of one serial line with
 * DXL2-C and answers for its *devices* (::x430_device). A device holds the Control
 * Table of the model selected at build time and enforces the rules of the real
 * actuator; the application connects it to its motor through ::x430_hooks. The
 * optional *simulator* (::x430_sim) replaces the motor.
 *
 * **Execution model.** Everything runs in one task context (an RX thread or the main
 * loop), never in an interrupt: x430_bus_receive() with the bytes from the UART,
 * x430_bus_poll() every few milliseconds (Bus Watchdog), and the live register
 * updates of the application between x430_bus_lock() and x430_bus_unlock(). Hooks
 * run inside x430_bus_receive() with the bus lock held.
 *
 * **Platform access** is done only through ::x430_bus_config::write (the bytes of a
 * status packet), the optional ::x430_port (lock, millisecond clock, delay) and the
 * hooks of each device.
 *
 * **Initialization.** Every function except the three *_init() functions returns
 * ::X430_ERR_STATE for an instance without a successful initialization and calls no
 * hook. An initialization checks every argument before it changes the instance: a
 * refused initialization leaves the instance as it was.
 * @{
 */

/* ==============================================================================
 * PUBLIC VARIABLES
 * =============================================================================*/

/**
 * @brief Hooks implementing the simulated actuator: pass them with
 *        x430_device_config::user_data pointing at an initialized ::x430_sim.
 *        All members are NULL when ::X430_FEATURE_SIMULATOR is 0.
 */
extern const struct x430_hooks x430_sim_hooks;

/* ==============================================================================
 * BUS
 * =============================================================================*/

/**
 * @brief  Initialize a bus. Calls no callback except x430_port::millis.
 * @param  bus     Bus.
 * @param  config  Configuration; copied (the port it points at is not).
 * @retval ::X430_OK         Bus ready.
 * @retval ::X430_ERR_PARAM  NULL pointer or no writer.
 */
int x430_bus_init(struct x430_bus* bus, const struct x430_bus_config* config);

/**
 * @brief  Attach an initialized device to an initialized bus. Takes the bus lock.
 * @param  bus     Bus.
 * @param  device  Device, not attached to any bus.
 * @retval ::X430_OK          Attached.
 * @retval ::X430_ERR_PARAM   NULL pointer.
 * @retval ::X430_ERR_STATE   Bus or device not initialized, or the device is attached.
 * @retval ::X430_ERR_CONFIG  Bus full (::X430_MAX_DEVICES) or the ID is used on the bus.
 */
int x430_bus_attach(struct x430_bus* bus, struct x430_device* device);

/**
 * @brief  Feed received bytes in any chunking; answers every complete instruction
 *         packet. Takes the bus lock; runs the hooks.
 * @param  bus     Bus.
 * @param  data    Received bytes (may be NULL when @p length is 0).
 * @param  length  Number of bytes.
 * @retval ::X430_OK         Bytes processed.
 * @retval ::X430_ERR_PARAM  NULL bus, or NULL data with a non-zero length.
 * @retval ::X430_ERR_STATE  Bus not initialized.
 * @retval ::X430_ERR_HAL    The writer failed for at least one status packet.
 */
int x430_bus_receive(struct x430_bus* bus, const uint8_t* data, size_t length);

/**
 * @brief  Handle one decoded instruction packet, for applications that run their own
 *         DXL2-C link. Takes the bus lock; runs the hooks.
 * @param  bus     Bus.
 * @param  packet  Packet decoded by DXL2-C. Status packets are ignored.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL pointer), ::X430_ERR_STATE (bus not
 *         initialized), ::X430_ERR_HAL (the writer failed).
 */
int x430_bus_handle_packet(struct x430_bus* bus, const struct dxl2_packet* packet);

/**
 * @brief  Periodic service: Bus Watchdog of every device. Call every few
 *         milliseconds (the watchdog unit is 20 ms). Takes the bus lock.
 * @param  bus  Bus.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_bus_poll(struct x430_bus* bus);

/**
 * @brief  Take the bus lock (x430_port::lock; nothing without it). Use it around
 *         the live register updates of the application.
 * @param  bus  Bus.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_bus_lock(struct x430_bus* bus);

/**
 * @brief  Release the bus lock (x430_port::unlock).
 * @param  bus  Bus.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_bus_unlock(struct x430_bus* bus);

/**
 * @brief  Attached device with the primary ID @p id (lock-free).
 * @param  bus     Bus.
 * @param  id      DYNAMIXEL ID.
 * @param  device  Set to the device, or to NULL when no device has that ID.
 * @retval ::X430_OK         Found.
 * @retval ::X430_ERR_PARAM  NULL pointer, or no attached device has the ID.
 * @retval ::X430_ERR_STATE  Bus not initialized.
 */
int x430_bus_find(const struct x430_bus* bus, uint8_t id, struct x430_device** device);

/* ==============================================================================
 * DEVICE: LIFECYCLE AND LOCK-FREE ACCESS
 * =============================================================================*/

/**
 * @brief  Initialize a device: factory values, stored EEPROM area (nvm_read), the RAM
 *         area and the Startup Configuration (which may call the write hook).
 *         Initialize a device before x430_bus_attach(), never while a bus serves it:
 *         the content of the instance before the call is not trusted (it may be the
 *         garbage of a stack instance), so an attachment cannot be detected.
 * @param  device  Device, not attached to a bus.
 * @param  config  Configuration; copied (the hooks it points at are not).
 * @retval ::X430_OK         Device ready.
 * @retval ::X430_ERR_PARAM  NULL pointer or ID above 252.
 */
int x430_device_init(struct x430_device* device, const struct x430_device_config* config);

/**
 * @brief  Value of the register that starts at @p address (lock-free).
 * @param  device   Device.
 * @param  address  First address of a register.
 * @param  value    Sign-extended value.
 * @retval ::X430_OK         Value read.
 * @retval ::X430_ERR_PARAM  NULL pointer, or no register starts at @p address.
 * @retval ::X430_ERR_STATE  Device not initialized.
 */
int x430_device_get(const struct x430_device* device, uint16_t address, int32_t* value);

/**
 * @brief  Store a value into the register at @p address, ignoring access rights
 *         (lock-free). For live registers (Present Position, Moving, ...): no hook is
 *         called, no range is checked.
 * @param  device   Device.
 * @param  address  First address of a register.
 * @param  value    Value; truncated to the register size.
 * @retval ::X430_OK         Value stored.
 * @retval ::X430_ERR_PARAM  NULL pointer, or no register starts at @p address.
 * @retval ::X430_ERR_STATE  Device not initialized.
 */
int x430_device_set(struct x430_device* device, uint16_t address, int32_t value);

/**
 * @brief  Current ID (7) of the device (lock-free).
 * @param  device  Device.
 * @param  id      ID.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_device_id(const struct x430_device* device, uint8_t* id);

/**
 * @brief  Report hardware errors (Hardware Error Status bits, lock-free). The bits
 *         stay set until REBOOT. Bits enabled in Shutdown (63) switch the torque off
 *         (event ::X430_EVENT_TORQUE_SHUTDOWN) and keep it off until REBOOT.
 * @param  device  Device.
 * @param  bits    ::X430_HW_ERROR_* bits.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_device_raise_hardware_error(struct x430_device* device, uint8_t bits);

/**
 * @brief  Deliver deferred events (::X430_EVENT_BAUD_RATE) after the status packet.
 *         The bus calls it; call it after x430_device_write() of your own.
 * @param  device  Device.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_device_flush_events(struct x430_device* device);

/* ==============================================================================
 * DEVICE: INSTRUCTION SEMANTICS
 * =============================================================================*/

/**
 * @brief  READ semantics: read hook, Indirect Data, Realtime Tick.
 * @param  device   Device.
 * @param  address  First address.
 * @param  length   Bytes to read.
 * @param  data     Destination of @p length bytes.
 * @param  error    Protocol result: DXL2_ERR_NONE, DXL2_ERR_DATA_LENGTH (length 0) or
 *                  DXL2_ERR_ACCESS (beyond the table).
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_device_read(struct x430_device* device, uint16_t address, uint16_t length, uint8_t* data,
                     uint8_t* error);

/**
 * @brief  WRITE semantics: checks every touched register, then calls the write hook
 *         and stores in address order; stores the EEPROM area when it changed.
 * @param  device   Device.
 * @param  address  First address.
 * @param  length   Bytes to write.
 * @param  data     @p length bytes.
 * @param  options  ::X430_WRITE_SECONDARY, ::X430_WRITE_VALIDATE_ONLY.
 * @param  error    Protocol result: DXL2_ERR_NONE or the DXL2_ERR_* of the first failure.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_device_write(struct x430_device* device, uint16_t address, uint16_t length,
                      const uint8_t* data, uint8_t options, uint8_t* error);

/**
 * @brief  REG_WRITE: validate a write and register it for ACTION.
 * @param  device   Device.
 * @param  address  First address.
 * @param  length   Bytes, at most ::X430_REG_WRITE_SIZE.
 * @param  data     @p length bytes.
 * @param  options  ::X430_WRITE_SECONDARY.
 * @param  error    Protocol result.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_device_reg_write(struct x430_device* device, uint16_t address, uint16_t length,
                          const uint8_t* data, uint8_t options, uint8_t* error);

/**
 * @brief  ACTION: execute the registered write.
 * @param  device  Device.
 * @param  error   Protocol result; DXL2_ERR_INSTRUCTION when nothing is registered.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_device_action(struct x430_device* device, uint8_t* error);

/**
 * @brief  REBOOT: RAM area reset, errors cleared, Startup Configuration applied,
 *         event ::X430_EVENT_REBOOT.
 * @param  device  Device.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_device_reboot(struct x430_device* device);

/**
 * @brief  FACTORY_RESET, then like REBOOT; event ::X430_EVENT_FACTORY_RESET.
 * @param  device  Device.
 * @param  option  ::X430_FACTORY_RESET_ALL, ::X430_FACTORY_RESET_EXCEPT_ID or
 *                 ::X430_FACTORY_RESET_EXCEPT_ID_BAUD.
 * @param  error   Protocol result; DXL2_ERR_DATA_RANGE for another option.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_device_factory_reset(struct x430_device* device, uint8_t option, uint8_t* error);

/**
 * @brief  CLEAR with option ::X430_CLEAR_MULTI_TURN (event ::X430_EVENT_CLEAR_MULTI_TURN).
 * @param  device  Device.
 * @param  option  ::X430_CLEAR_MULTI_TURN.
 * @param  error   Protocol result: DXL2_ERR_RESULT_FAIL while moving,
 *                 DXL2_ERR_DATA_RANGE for another option.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized).
 */
int x430_device_clear(struct x430_device* device, uint8_t option, uint8_t* error);

/**
 * @brief  Control Table BACKUP: store, or restore with the torque off.
 * @param  device  Device.
 * @param  option  ::X430_BACKUP_STORE or ::X430_BACKUP_RESTORE.
 * @param  error   Protocol result.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (not initialized),
 *         ::X430_ERR_UNSUPPORTED (::X430_FEATURE_BACKUP is 0).
 */
int x430_device_backup(struct x430_device* device, uint8_t option, uint8_t* error);

/* ==============================================================================
 * SIMULATOR
 * =============================================================================*/

/**
 * @brief  Prepare a simulator for @p device. Call it before x430_device_init() of a
 *         device whose hooks are ::x430_sim_hooks and whose user data is @p sim.
 * @param  sim     Simulator.
 * @param  device  Device it simulates (may be initialized later).
 * @param  config  Configuration, NULL for the defaults.
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL sim or device), ::X430_ERR_UNSUPPORTED
 *         (::X430_FEATURE_SIMULATOR is 0).
 */
int x430_sim_init(struct x430_sim* sim, struct x430_device* device,
                  const struct x430_sim_config* config);

/**
 * @brief  Advance the simulation by @p elapsed_ms one-millisecond steps and refresh the
 *         live registers. Takes the bus lock of the device when it is attached. The
 *         time is proportional to @p elapsed_ms: call it periodically.
 * @param  sim         Simulator.
 * @param  elapsed_ms  Time since the previous call [ms].
 * @retval ::X430_OK, ::X430_ERR_PARAM (NULL), ::X430_ERR_STATE (simulator or device
 *         not initialized), ::X430_ERR_UNSUPPORTED (::X430_FEATURE_SIMULATOR is 0).
 */
int x430_sim_step(struct x430_sim* sim, uint32_t elapsed_ms);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* X430_H */
