/**
 * @file x430_config.h
 * @brief Build-time configuration of the X430 emulator.
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
 * @defgroup x430_config Configuration
 * @ingroup x430
 * @brief Build-time configuration: Kconfig symbols or plain compiler definitions.
 *
 * @details The values come from three places, highest priority first:
 *          @li a compiler definition of the X430_* macro itself
 *              (for example `-DX430_MAX_DEVICES=8U`);
 *          @li the Kconfig header x430_autoconf.h (CONFIG_X430_* symbols),
 *              included when the build defines X430_HAS_AUTOCONF;
 *          @li the defaults below.
 *
 *          The emulated model is not configured here: CMake generates its
 *          Control Table (x430_model_table.c) and x430_model_info.h for the
 *          model selected with Kconfig or with the X430_MODEL cache variable.
 * @{
 */

#ifndef X430_CONFIG_H
#define X430_CONFIG_H

/** @name Includes */
/** @{ */

#include <stdint.h>

#if defined(X430_HAS_AUTOCONF)
#include "x430_autoconf.h"
#endif

#include "x430_model_info.h"

/** @} */

/** @name Device and bus sizing */
/** @{ */

/**
 * @def DXL2_MAX_PACKET_SIZE
 * @brief Largest packet of the DXL2-C link, in bytes (default 768: a READ of the whole
 *        Control Table, as DYNAMIXEL Wizard does it).
 * @details Defined here, before dxl2.h is included, so that every translation unit that
 *          includes x430.h sees the size the library was built with (the instance size
 *          of a bus depends on it). DXL2-C itself must be compiled with the same value;
 *          x430.cmake passes it to DXL2-C.
 */
#ifndef DXL2_MAX_PACKET_SIZE
#if defined(CONFIG_X430_DXL2_MAX_PACKET_SIZE)
#define DXL2_MAX_PACKET_SIZE (CONFIG_X430_DXL2_MAX_PACKET_SIZE)
#else
#define DXL2_MAX_PACKET_SIZE 768U
#endif
#endif

/**
 * @def X430_MAX_DEVICES
 * @brief Devices (DYNAMIXEL IDs) one bus can host.
 */
#ifndef X430_MAX_DEVICES
#if defined(CONFIG_X430_MAX_DEVICES)
#define X430_MAX_DEVICES (CONFIG_X430_MAX_DEVICES)
#else
#define X430_MAX_DEVICES (4U)
#endif
#endif

/**
 * @def X430_REG_WRITE_SIZE
 * @brief Largest data block a REG_WRITE can register, in bytes.
 */
#ifndef X430_REG_WRITE_SIZE
#if defined(CONFIG_X430_REG_WRITE_SIZE)
#define X430_REG_WRITE_SIZE (CONFIG_X430_REG_WRITE_SIZE)
#else
#define X430_REG_WRITE_SIZE (32U)
#endif
#endif

/** @} */

/** @name Shared bus */
/** @{ */

/**
 * @def X430_TURN_TIMEOUT_MS
 * @brief Longest wait, in milliseconds, for the status packet of the device before a
 *        hosted device in a SYNC_READ or BULK_READ (default 100).
 * @details On a bus shared with devices of other buses (other MCUs, real servos), a
 *          hosted device answers a SYNC_READ or BULK_READ only after it has received
 *          the status packet of the ID listed just before it, like an X series
 *          actuator. When that status does not arrive within this time after the last
 *          progress of the read, the remaining hosted devices of the read stay silent.
 *          A new instruction packet ends the wait as well. The time comes from the
 *          millisecond clock of ::x430_port; without a clock the wait ends only with
 *          the next instruction packet. Keep it longer than one status packet at the
 *          lowest baud rate in use, plus the Return Delay Time.
 */
#ifndef X430_TURN_TIMEOUT_MS
#if defined(CONFIG_X430_TURN_TIMEOUT_MS)
#define X430_TURN_TIMEOUT_MS (CONFIG_X430_TURN_TIMEOUT_MS)
#else
#define X430_TURN_TIMEOUT_MS (100U)
#endif
#endif

/** @} */

/** @name Identity */
/** @{ */

/**
 * @def X430_FIRMWARE_VERSION
 * @brief Firmware Version (address 6) used when the device configuration gives 0.
 */
#ifndef X430_FIRMWARE_VERSION
#if defined(CONFIG_X430_FIRMWARE_VERSION)
#define X430_FIRMWARE_VERSION (CONFIG_X430_FIRMWARE_VERSION)
#else
#define X430_FIRMWARE_VERSION (52U)
#endif
#endif

/**
 * @def X430_MODEL_NUMBER
 * @brief Model Number reported by PING and address 0.
 * @details CONFIG_X430_MODEL_NUMBER_OVERRIDE (non-zero) replaces the number of
 *          the ROBOTIS documentation, for hosts that only know another number.
 */
#ifndef X430_MODEL_NUMBER
#if defined(CONFIG_X430_MODEL_NUMBER_OVERRIDE) && (CONFIG_X430_MODEL_NUMBER_OVERRIDE != 0)
#define X430_MODEL_NUMBER (CONFIG_X430_MODEL_NUMBER_OVERRIDE)
#else
#define X430_MODEL_NUMBER (X430_MODEL_NUMBER_DEFAULT)
#endif
#endif

/**
 * @def X430_BAUD_RATE_CODE_MAX
 * @brief Highest Baud Rate (address 8) code accepted by a write.
 * @details The X430 table defines 0..7. A larger value lets an application use
 *          its own codes; the application validates them in the write hook.
 */
#ifndef X430_BAUD_RATE_CODE_MAX
#if defined(CONFIG_X430_BAUD_RATE_CODE_MAX)
#define X430_BAUD_RATE_CODE_MAX (CONFIG_X430_BAUD_RATE_CODE_MAX)
#else
#define X430_BAUD_RATE_CODE_MAX (7U)
#endif
#endif

/** @} */

/**
 * @name Optional features
 * @details 1 = compiled in, 0 = left out. With Kconfig, a feature is off when
 *          its symbol is not set; without Kconfig every feature is on.
 */
/** @{ */

/** @brief BULK_READ and BULK_WRITE instructions. */
#ifndef X430_FEATURE_BULK
#if defined(CONFIG_X430_BULK) || !defined(X430_HAS_AUTOCONF)
#define X430_FEATURE_BULK 1
#else
#define X430_FEATURE_BULK 0
#endif
#endif

/** @brief FAST_SYNC_READ and FAST_BULK_READ instructions (used by ROS 2 by default). */
#ifndef X430_FEATURE_FAST_READ
#if defined(CONFIG_X430_FAST_READ) || !defined(X430_HAS_AUTOCONF)
#define X430_FEATURE_FAST_READ 1
#else
#define X430_FEATURE_FAST_READ 0
#endif
#endif

/** @brief Control Table BACKUP instruction, Backup Ready and Startup RAM Restore. */
#ifndef X430_FEATURE_BACKUP
#if defined(CONFIG_X430_BACKUP) || !defined(X430_HAS_AUTOCONF)
#define X430_FEATURE_BACKUP 1
#else
#define X430_FEATURE_BACKUP 0
#endif
#endif

/** @brief Built-in simulator backend (x430_sim.h). */
#ifndef X430_FEATURE_SIMULATOR
#if defined(CONFIG_X430_SIMULATOR) || !defined(X430_HAS_AUTOCONF)
#define X430_FEATURE_SIMULATOR 1
#else
#define X430_FEATURE_SIMULATOR 0
#endif
#endif

/** @} */

/**
 * @name Options as unsigned constants
 * @details A definition on the compiler command line may be a plain (signed)
 *          literal; the library uses these copies in expressions (MISRA C:2012
 *          Rule 10.4).
 */
/** @{ */

#define X430_MAX_DEVICES_U ((uint32_t)(X430_MAX_DEVICES))
#define X430_REG_WRITE_SIZE_U ((uint32_t)(X430_REG_WRITE_SIZE))
#define X430_BAUD_RATE_CODE_MAX_U ((uint32_t)(X430_BAUD_RATE_CODE_MAX))
#define X430_TURN_TIMEOUT_MS_U ((uint32_t)(X430_TURN_TIMEOUT_MS))

/** @} */

/** @name Consistency checks */
/** @{ */

#if (X430_MAX_DEVICES < 1U) || (X430_MAX_DEVICES > 253U)
#error "X430_MAX_DEVICES must be in the range 1..253"
#endif

#if (X430_REG_WRITE_SIZE < 4U) || (X430_REG_WRITE_SIZE > X430_TABLE_SIZE)
#error "X430_REG_WRITE_SIZE must be in the range 4..X430_TABLE_SIZE"
#endif

#if (X430_TURN_TIMEOUT_MS < 1U) || (X430_TURN_TIMEOUT_MS > 60000U)
#error "X430_TURN_TIMEOUT_MS must be in the range 1..60000"
#endif

/** @} */

#endif /* X430_CONFIG_H */

/** @} */
