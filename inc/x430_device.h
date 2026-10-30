/**
 * @file x430_device.h
 * @brief One emulated DYNAMIXEL X430 actuator: Control Table and its rules.
 *
 * Project: sfs-dynamixel-x430 - DYNAMIXEL X430 Control Table Emulator
 * Author: Anton Vynohradov
 * Email: avynohradov@systemfromscratch.com
 *
 * @copyright Copyright (c) 2026 Anton Vynohradov
 * @license Apache License 2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @defgroup x430_device Device
 * @ingroup x430
 * @brief Control Table of one DYNAMIXEL ID and the rules of the real device.
 *
 * @details A device holds the Control Table image of the model selected at
 *          build time and enforces what a real X430 enforces: access rights,
 *          value ranges, the EEPROM lock while torque is on, Indirect
 *          Address/Data, REG_WRITE/ACTION, Bus Watchdog, hardware error
 *          shutdown, Startup Configuration, factory reset, reboot and
 *          Control Table backup.
 *
 *          The device does not move anything. The application (or the
 *          simulator, @ref x430_sim) connects it to the actuator through
 *          @ref x430_hooks:
 *
 *          @li **write**: every accepted register write, before it is stored.
 *              The application applies Goal Position, Torque Enable, ... to
 *              its motor and may refuse with a DXL2_ERR_* code.
 *          @li **read**: before data is read; the application may refresh live
 *              registers (Present Position, ...). Alternatively it updates them
 *              periodically with @ref x430_device_set.
 *          @li **event**: reboot, factory reset, clear multi-turn, Bus Watchdog
 *              timeout, Baud Rate change, hardware error shutdown.
 *          @li **nvm_load / nvm_store**: persistence of the EEPROM area and of
 *              the Control Table backup. The library adds a header and a CRC;
 *              the application stores opaque byte blocks.
 *
 *          A device is served by a bus (@ref x430_bus), which decodes the
 *          instructions. Functions marked "lock-free" do not take the bus lock:
 *          call them from hooks or between @ref x430_bus_lock and
 *          @ref x430_bus_unlock.
 * @{
 */

#ifndef X430_DEVICE_H
#define X430_DEVICE_H

/** @name Includes */
/** @{ */

#include <stdbool.h>
#include <stdint.h>

#include "x430_config.h"
#include "x430_defs.h"
#include "x430_model.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** @} */

/** @name Return values and options */
/** @{ */

/** Success of a function that returns int. */
#define X430_OK 0
/** Invalid argument or state. */
#define X430_EINVAL (-1)

/** x430_device_write option: packet addressed by the Secondary ID (EEPROM is locked). */
#define X430_WRITE_SECONDARY 0x01U
/** x430_device_write option: validate only, change nothing, call no hook. */
#define X430_WRITE_VALIDATE_ONLY 0x02U

/** Size of the persistent block header: magic, model, format, slot, payload size. */
#define X430_NVM_HEADER_SIZE 10U
/** Size of the EEPROM block passed to nvm_store / requested by nvm_load. */
#define X430_NVM_EEPROM_SIZE (X430_NVM_HEADER_SIZE + X430_RAM_START + 2U)
/** Size of the Control Table backup block. */
#define X430_NVM_BACKUP_SIZE (X430_NVM_HEADER_SIZE + X430_TABLE_SIZE + 2U)

/** @} */

/** @name Type definitions */
/** @{ */

struct x430_bus;
struct x430_device;

/** @brief Notifications from the device to the application. */
enum x430_event
{
    /** REBOOT executed: RAM area reset, hardware errors cleared. Reset the motor state. */
    X430_EVENT_REBOOT = 0,
    /** FACTORY_RESET executed: EEPROM back to factory values (then like REBOOT). */
    X430_EVENT_FACTORY_RESET,
    /**
     * CLEAR (multi-turn) requested while the device is not moving. Return
     * DXL2_ERR_NONE after bringing Present Position into one turn, or an error.
     * Without an event hook the library wraps Present Position itself.
     */
    X430_EVENT_CLEAR_MULTI_TURN,
    /** Bus Watchdog expired: stop the actuator (Bus Watchdog now reads -1). */
    X430_EVENT_BUS_WATCHDOG,
    /** Baud Rate (8) changed; delivered after the status packet was sent. */
    X430_EVENT_BAUD_RATE,
    /** A hardware error listed in Shutdown (63) switched the torque off. */
    X430_EVENT_TORQUE_SHUTDOWN,
    /** BACKUP restore copied the backup into the Control Table. */
    X430_EVENT_BACKUP_RESTORED
};

/** @brief Persistent storage slots. */
enum x430_nvm_slot
{
    X430_NVM_EEPROM = 0, /**< EEPROM area, X430_NVM_EEPROM_SIZE bytes. */
    X430_NVM_BACKUP = 1  /**< Control Table backup, X430_NVM_BACKUP_SIZE bytes. */
};

/**
 * @brief Application hooks. Every member may be NULL.
 *
 * @details Hooks run in the bus context with the bus lock held. They may call
 *          the lock-free device functions, but not the bus functions.
 */
struct x430_hooks
{
    /**
     * @brief A validated register write, before the value is stored.
     * @param device Device written.
     * @param address First address of the register.
     * @param size Register size in bytes.
     * @param value New value (sign-extended for signed registers).
     * @param user_data x430_device_config::user_data.
     * @return DXL2_ERR_NONE to accept, another DXL2_ERR_* code to refuse
     *         (the instruction stops at this register).
     */
    uint8_t (*write)(struct x430_device* device, uint16_t address, uint8_t size, int32_t value,
                     void* user_data);

    /**
     * @brief Data is about to be read.
     * @details Called once per contiguous run of real addresses (Indirect Data
     *          is resolved to the addresses it points at).
     */
    void (*read)(struct x430_device* device, uint16_t address, uint16_t length, void* user_data);

    /**
     * @brief Device event (@ref x430_event).
     * @return DXL2_ERR_NONE, or an error code where the event can be refused
     *         (X430_EVENT_CLEAR_MULTI_TURN); ignored otherwise.
     */
    uint8_t (*event)(struct x430_device* device, enum x430_event event, void* user_data);

    /**
     * @brief Load a persistent block.
     * @return 0 when @p size bytes were read, negative when there is no data.
     */
    int (*nvm_load)(struct x430_device* device, enum x430_nvm_slot slot, uint8_t* data,
                    uint16_t size, void* user_data);

    /**
     * @brief Store a persistent block (called after the change, may be slow).
     * @return 0 on success, negative on failure (reported as Result Fail).
     */
    int (*nvm_store)(struct x430_device* device, enum x430_nvm_slot slot, const uint8_t* data,
                     uint16_t size, void* user_data);
};

/** @brief Device configuration passed to @ref x430_device_init. */
struct x430_device_config
{
    /** Factory ID of this device (0 = model default 1). Stored EEPROM wins at startup. */
    uint8_t id;
    /** Firmware Version (6); 0 = X430_FIRMWARE_VERSION. */
    uint8_t firmware_version;
    /** Model Information (2), free for the application (e.g. driver/motor type). */
    uint32_t model_information;
    /** Application hooks; NULL = pure register device. */
    const struct x430_hooks* hooks;
    /** Passed to every hook. */
    void* user_data;
};

/** @brief Device state. Treat the members as private; use the functions below. */
struct x430_device
{
    uint8_t table[X430_TABLE_SIZE];        /**< Control Table image, little endian. */
    const struct x430_model* model;        /**< Generated model descriptor. */
    struct x430_device_config config;      /**< Copy of the configuration. */
    struct x430_bus* bus;                  /**< Owning bus, NULL before attach. */
    uint8_t reg_data[X430_REG_WRITE_SIZE]; /**< REG_WRITE data. */
    uint16_t reg_address;                  /**< REG_WRITE address. */
    uint16_t reg_length;                   /**< REG_WRITE length, 0 = none. */
    bool baud_changed;                     /**< Deferred X430_EVENT_BAUD_RATE. */
    bool initialized;                      /**< x430_device_init succeeded. */
};

/** @} */

/** @name Lifecycle */
/** @{ */

/**
 * @brief Initialize a device: factory values, stored EEPROM, Startup Configuration.
 * @param device Device to initialize.
 * @param config Configuration, copied.
 * @retval X430_OK Device ready.
 * @retval X430_EINVAL NULL argument or ID above 252.
 */
int x430_device_init(struct x430_device* device, const struct x430_device_config* config);

/** @} */

/** @name Lock-free register access for the application */
/** @{ */

/**
 * @brief Value of the register that starts at @p address (lock-free).
 * @return Sign-extended value, 0 when no register starts there.
 */
int32_t x430_device_get(const struct x430_device* device, uint16_t address);

/**
 * @brief Store a value into the register at @p address, ignoring access rights (lock-free).
 * @details Use it for live registers (Present Position, Moving, ...). No hook is
 *          called, no check is made.
 * @retval X430_OK Value stored.
 * @retval X430_EINVAL No register starts at @p address.
 */
int x430_device_set(struct x430_device* device, uint16_t address, int32_t value);

/** @brief Current ID (7) of the device (lock-free). */
uint8_t x430_device_id(const struct x430_device* device);

/**
 * @brief Report hardware errors (Hardware Error Status bits, lock-free).
 * @details The bits stay set until REBOOT. Bits that are enabled in Shutdown
 *          (63) switch the torque off and keep it off until REBOOT.
 */
void x430_device_raise_hardware_error(struct x430_device* device, uint8_t bits);

/** @} */

/** @name Instruction semantics (used by the bus, callable for tests and custom transports) */
/** @{ */

/**
 * @brief READ semantics: hooks, Indirect Data, Realtime Tick.
 * @return DXL2_ERR_NONE, DXL2_ERR_DATA_LENGTH or DXL2_ERR_ACCESS.
 */
uint8_t x430_device_read(struct x430_device* device, uint16_t address, uint16_t length,
                         uint8_t* data);

/**
 * @brief WRITE semantics: checks every touched register, then stores in address order.
 * @param options X430_WRITE_* bits.
 * @return DXL2_ERR_NONE or the DXL2_ERR_* code of the first failure.
 */
uint8_t x430_device_write(struct x430_device* device, uint16_t address, uint16_t length,
                          const uint8_t* data, uint8_t options);

/** @brief REG_WRITE: validate and register a write for ACTION. */
uint8_t x430_device_reg_write(struct x430_device* device, uint16_t address, uint16_t length,
                              const uint8_t* data, uint8_t options);

/** @brief ACTION: execute the registered write. Instruction Error when none. */
uint8_t x430_device_action(struct x430_device* device);

/** @brief REBOOT: RAM area reset, errors cleared, Startup Configuration applied. */
uint8_t x430_device_reboot(struct x430_device* device);

/** @brief FACTORY_RESET with option 0xFF, 0x01 or 0x02. */
uint8_t x430_device_factory_reset(struct x430_device* device, uint8_t option);

/** @brief CLEAR with option X430_CLEAR_MULTI_TURN. */
uint8_t x430_device_clear(struct x430_device* device, uint8_t option);

/** @brief Control Table BACKUP with option X430_BACKUP_STORE or X430_BACKUP_RESTORE. */
uint8_t x430_device_backup(struct x430_device* device, uint8_t option);

/** @brief Status error alert: DXL2_ERR_ALERT_BIT when Hardware Error Status is not 0. */
uint8_t x430_device_alert(const struct x430_device* device);

/**
 * @brief Bus Watchdog check.
 * @param idle_ms Time since the last instruction packet on the bus.
 */
void x430_device_watchdog(struct x430_device* device, uint32_t idle_ms);

/** @brief Deliver deferred events (Baud Rate) after the status packet. */
void x430_device_flush_events(struct x430_device* device);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* X430_DEVICE_H */

/** @} */
