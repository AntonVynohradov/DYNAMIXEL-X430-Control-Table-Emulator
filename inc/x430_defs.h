/*******************************************************************************
 * DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
 *
 * Project:     DYNAMIXEL-X430-Control-Table-Emulator
 * Description: Constants of the public API: status codes, write options,
 *              events, persistent slots, Control Table addresses and values.
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

/**
 * @defgroup x430_defs Definitions
 * @ingroup x430
 * @brief Status codes, options, events and the addresses, operating modes and
 *        bit fields of the X430 Control Table.
 *
 * @details Control Table values come from the ROBOTIS e-Manual of the X430
 *          models (https://emanual.robotis.com/docs/en/dxl/x/). Which registers
 *          a model really has is decided by its generated table: XL/XC models
 *          have no Current Limit (38) and Goal Current (102) and report
 *          Present Load at 126; XW models have no LED (65).
 * @{
 */

#ifndef X430_DEFS_H
#define X430_DEFS_H

/** @name Status codes (return values of every x430_* function) */
/** @{ */

/** @brief Success. */
#define X430_OK 0
/** @brief NULL pointer or a value outside its documented range. */
#define X430_ERR_PARAM (-1)
/** @brief Call not allowed in the current state (instance not initialized, device attached). */
#define X430_ERR_STATE (-2)
/** @brief Values that are valid alone but not together (bus full, ID already used on the bus). */
#define X430_ERR_CONFIG (-3)
/** @brief The byte writer of the bus (x430_bus_config::write) returned an error. */
#define X430_ERR_HAL (-4)
/** @brief The function belongs to a feature that is not compiled in (X430_FEATURE_*). */
#define X430_ERR_UNSUPPORTED (-5)

/** @} */

/** @name Options of x430_device_write() and x430_device_reg_write() */
/** @{ */

/** @brief Packet addressed by the Secondary ID: the EEPROM area is locked. */
#define X430_WRITE_SECONDARY 0x01U
/** @brief Validate only: change nothing and call no hook. */
#define X430_WRITE_VALIDATE_ONLY 0x02U

/** @} */

/** @name Events and persistent storage */
/** @{ */

/** @brief Notifications from a device to the application (x430_hooks::event). */
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

/** @brief Persistent storage slots (x430_hooks::nvm_load, x430_hooks::nvm_store). */
enum x430_nvm_slot
{
    X430_NVM_EEPROM = 0, /**< EEPROM area, ::X430_NVM_EEPROM_SIZE bytes. */
    X430_NVM_BACKUP = 1  /**< Control Table backup, ::X430_NVM_BACKUP_SIZE bytes. */
};

/** @brief Size of the persistent block header: magic, model, format, slot, payload size. */
#define X430_NVM_HEADER_SIZE 10U

/** @} */

/** @name Control Table: EEPROM area */
/** @{ */

#define X430_ADDR_MODEL_NUMBER 0U
#define X430_ADDR_MODEL_INFORMATION 2U
#define X430_ADDR_FIRMWARE_VERSION 6U
#define X430_ADDR_ID 7U
#define X430_ADDR_BAUD_RATE 8U
#define X430_ADDR_RETURN_DELAY_TIME 9U
#define X430_ADDR_DRIVE_MODE 10U
#define X430_ADDR_OPERATING_MODE 11U
#define X430_ADDR_SECONDARY_ID 12U
#define X430_ADDR_PROTOCOL_TYPE 13U
#define X430_ADDR_HOMING_OFFSET 20U
#define X430_ADDR_MOVING_THRESHOLD 24U
#define X430_ADDR_TEMPERATURE_LIMIT 31U
#define X430_ADDR_MAX_VOLTAGE_LIMIT 32U
#define X430_ADDR_MIN_VOLTAGE_LIMIT 34U
#define X430_ADDR_PWM_LIMIT 36U
#define X430_ADDR_CURRENT_LIMIT 38U
#define X430_ADDR_VELOCITY_LIMIT 44U
#define X430_ADDR_MAX_POSITION_LIMIT 48U
#define X430_ADDR_MIN_POSITION_LIMIT 52U
#define X430_ADDR_STARTUP_CONFIGURATION 60U
#define X430_ADDR_SHUTDOWN 63U

/** First address of the RAM area; addresses below belong to the EEPROM area. */
#define X430_RAM_START 64U

/** @} */

/** @name Control Table: RAM area */
/** @{ */

#define X430_ADDR_TORQUE_ENABLE 64U
#define X430_ADDR_LED 65U
#define X430_ADDR_STATUS_RETURN_LEVEL 68U
#define X430_ADDR_REGISTERED_INSTRUCTION 69U
#define X430_ADDR_HARDWARE_ERROR_STATUS 70U
#define X430_ADDR_VELOCITY_I_GAIN 76U
#define X430_ADDR_VELOCITY_P_GAIN 78U
#define X430_ADDR_POSITION_D_GAIN 80U
#define X430_ADDR_POSITION_I_GAIN 82U
#define X430_ADDR_POSITION_P_GAIN 84U
#define X430_ADDR_FEEDFORWARD_2ND_GAIN 88U
#define X430_ADDR_FEEDFORWARD_1ST_GAIN 90U
#define X430_ADDR_BUS_WATCHDOG 98U
#define X430_ADDR_GOAL_PWM 100U
#define X430_ADDR_GOAL_CURRENT 102U
#define X430_ADDR_GOAL_VELOCITY 104U
#define X430_ADDR_PROFILE_ACCELERATION 108U
#define X430_ADDR_PROFILE_VELOCITY 112U
#define X430_ADDR_GOAL_POSITION 116U
#define X430_ADDR_REALTIME_TICK 120U
#define X430_ADDR_MOVING 122U
#define X430_ADDR_MOVING_STATUS 123U
#define X430_ADDR_PRESENT_PWM 124U
/** Present Current (XM/XH/XD/XW) or Present Load (XL/XC), same address. */
#define X430_ADDR_PRESENT_CURRENT 126U
#define X430_ADDR_PRESENT_LOAD 126U
#define X430_ADDR_PRESENT_VELOCITY 128U
#define X430_ADDR_PRESENT_POSITION 132U
#define X430_ADDR_VELOCITY_TRAJECTORY 136U
#define X430_ADDR_POSITION_TRAJECTORY 140U
#define X430_ADDR_PRESENT_INPUT_VOLTAGE 144U
#define X430_ADDR_PRESENT_TEMPERATURE 146U
#define X430_ADDR_BACKUP_READY 147U

/** @} */

/** @name Operating Mode (11) values */
/** @{ */

#define X430_MODE_CURRENT 0U
#define X430_MODE_VELOCITY 1U
#define X430_MODE_POSITION 3U
#define X430_MODE_EXTENDED_POSITION 4U
#define X430_MODE_CURRENT_BASED_POSITION 5U
#define X430_MODE_PWM 16U

/** @} */

/** @name Drive Mode (10) bits */
/** @{ */

/** Reverse mode: positive goals turn clockwise. */
#define X430_DRIVE_MODE_REVERSE 0x01U
/** Time-based profile: Profile Velocity/Acceleration are times in ms. */
#define X430_DRIVE_MODE_TIME_BASED 0x04U
/** Torque On by Goal Update: writing a goal value enables torque. */
#define X430_DRIVE_MODE_TORQUE_ON_BY_GOAL 0x08U

/** @} */

/** @name Startup Configuration (60) bits */
/** @{ */

#define X430_STARTUP_TORQUE_ON 0x01U
#define X430_STARTUP_RAM_RESTORE 0x02U

/** @} */

/** @name Hardware Error Status (70) and Shutdown (63) bits */
/** @{ */

#define X430_HW_ERROR_INPUT_VOLTAGE 0x01U
#define X430_HW_ERROR_OVERHEATING 0x04U
#define X430_HW_ERROR_MOTOR_ENCODER 0x08U
#define X430_HW_ERROR_ELECTRICAL_SHOCK 0x10U
#define X430_HW_ERROR_OVERLOAD 0x20U

/** @} */

/** @name Moving Status (123) bits */
/** @{ */

#define X430_MOVING_STATUS_IN_POSITION 0x01U
#define X430_MOVING_STATUS_PROFILE_ONGOING 0x02U
#define X430_MOVING_STATUS_FOLLOWING_ERROR 0x08U
/** Velocity profile type, bits 4-5. */
#define X430_MOVING_STATUS_PROFILE_STEP 0x00U
#define X430_MOVING_STATUS_PROFILE_RECTANGULAR 0x10U
#define X430_MOVING_STATUS_PROFILE_TRIANGULAR 0x20U
#define X430_MOVING_STATUS_PROFILE_TRAPEZOIDAL 0x30U

/** @} */

/** @name Special register values */
/** @{ */

/** Bus Watchdog (98) value while a Bus Watchdog error is active (-1). */
#define X430_BUS_WATCHDOG_ERROR 0xFFU
/** Bus Watchdog (98) time unit in milliseconds. */
#define X430_BUS_WATCHDOG_UNIT_MS 20U
/** Secondary ID (12) values from 253 disable the Secondary ID. */
#define X430_SECONDARY_ID_DISABLED 253U
/** Goal Position range outside Position Control Mode. */
#define X430_EXTENDED_POSITION_LIMIT 1048575
/** Realtime Tick (120) wraps at this value, in ms. */
#define X430_REALTIME_TICK_PERIOD 32768U
/** Return Delay Time (9) unit in microseconds. */
#define X430_RETURN_DELAY_UNIT_US 2U

/** @} */

/** @name Instruction options */
/** @{ */

/** FACTORY_RESET option: reset everything (refused on the broadcast ID). */
#define X430_FACTORY_RESET_ALL 0xFFU
/** FACTORY_RESET option: keep the ID. */
#define X430_FACTORY_RESET_EXCEPT_ID 0x01U
/** FACTORY_RESET option: keep the ID and the Baud Rate. */
#define X430_FACTORY_RESET_EXCEPT_ID_BAUD 0x02U
/** CLEAR option: clear the multi-turn information of Present Position. */
#define X430_CLEAR_MULTI_TURN 0x01U
/** BACKUP option: store the Control Table. */
#define X430_BACKUP_STORE 0x01U
/** BACKUP option: restore the Control Table. */
#define X430_BACKUP_RESTORE 0x02U

/** @} */

#endif /* X430_DEFS_H */

/** @} */
