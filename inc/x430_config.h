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

#if defined(X430_HAS_AUTOCONF)
#include "x430_autoconf.h"
#endif

#include "x430_model_info.h"

/** @} */

/** @name Device and bus sizing */
/** @{ */

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

/** @name Consistency checks */
/** @{ */

#if (X430_MAX_DEVICES < 1U) || (X430_MAX_DEVICES > 253U)
#error "X430_MAX_DEVICES must be in the range 1..253"
#endif

#if (X430_REG_WRITE_SIZE < 4U) || (X430_REG_WRITE_SIZE > X430_TABLE_SIZE)
#error "X430_REG_WRITE_SIZE must be in the range 4..X430_TABLE_SIZE"
#endif

/** @} */

#endif /* X430_CONFIG_H */

/** @} */
