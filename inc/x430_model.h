/**
 * @file x430_model.h
 * @brief Model descriptor: the Control Table of the emulated model.
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
 * @defgroup x430_model Model descriptor
 * @ingroup x430
 * @brief Types of the generated Control Table (one model per build).
 *
 * @details tools/x430_gen_model.py writes x430_model_table.c with the constant
 *          @ref x430_model_active from the YAML description in models/. The
 *          library reads the descriptor only; it never changes it.
 * @{
 */

#ifndef X430_MODEL_H
#define X430_MODEL_H

/** @name Includes */
/** @{ */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @} */

/** @name Register flags (x430_reg::flags) */
/** @{ */

/** Writable by an instruction (RW); without it the register is read-only. */
#define X430_REG_WRITABLE 0x01U
/** EEPROM area: persistent, locked while Torque Enable is 1. */
#define X430_REG_EEPROM 0x02U
/** Two's complement value. */
#define X430_REG_SIGNED 0x04U
/** Goal value: read-only while a Bus Watchdog error is active. */
#define X430_REG_GOAL 0x08U

/** @} */

/** @name Type definitions */
/** @{ */

/** @brief Write check of a register (x430_reg::check). */
enum x430_check
{
    X430_CHECK_NONE = 0,      /**< Read-only register, no check. */
    X430_CHECK_RANGE,         /**< min <= value <= max. */
    X430_CHECK_MASK,          /**< Only the bits of max may be set. */
    X430_CHECK_OPMODE,        /**< One of x430_model::operating_modes. */
    X430_CHECK_SYMMETRIC,     /**< -ref <= value <= ref; ref = limit register address. */
    X430_CHECK_GOAL_POSITION, /**< Position limits in mode 3, +-1,048,575 otherwise. */
    X430_CHECK_INDIRECT       /**< Indirect Address: RAM address min..max. */
};

/** @brief One Control Table item. */
struct x430_reg
{
    uint16_t address; /**< First byte. */
    uint8_t size;     /**< 1, 2 or 4 bytes. */
    uint8_t flags;    /**< X430_REG_* bits. */
    uint8_t check;    /**< enum x430_check. */
    uint16_t ref;     /**< Limit register address for SYMMETRIC. */
    int32_t initial;  /**< Value after a factory reset. */
    int32_t min;      /**< RANGE/INDIRECT lower bound. */
    int32_t max;      /**< RANGE/INDIRECT upper bound, MASK allowed bits. */
};

/** @brief Indirect Address block: count addresses of 2 bytes and count data bytes. */
struct x430_indirect_block
{
    uint16_t address; /**< Indirect Address 1 of the block. */
    uint16_t data;    /**< Indirect Data 1 of the block. */
    uint16_t count;   /**< Number of entries. */
};

/** @brief Control Table and capabilities of one model. */
struct x430_model
{
    const char* name;                           /**< e.g. "XM430-W350". */
    uint16_t model_number;                      /**< Number from the documentation. */
    uint16_t table_size;                        /**< Addresses 0 .. table_size - 1. */
    const struct x430_reg* registers;           /**< Sorted by address. */
    uint16_t register_count;                    /**< Entries in registers. */
    const uint8_t* register_map;                /**< Index + 1 per address, 0 = reserved. */
    const struct x430_indirect_block* indirect; /**< Indirect blocks. */
    uint8_t indirect_count;                     /**< Entries in indirect. */
    uint32_t operating_modes;                   /**< Bit n set: Operating Mode n allowed. */
    uint16_t current_unit_ua;                   /**< Current unit in uA, 0 = no current. */
    uint8_t axes;                               /**< 2 for 2XL430/2XC430. */
};

/** @} */

/** @name Generated data */
/** @{ */

/** @brief The model selected at build time (x430_model_table.c). */
extern const struct x430_model x430_model_active;

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* X430_MODEL_H */

/** @} */
