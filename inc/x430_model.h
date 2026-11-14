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
/** Shift of the write check (enum x430_check) in the upper four bits of x430_reg::flags. */
#define X430_REG_CHECK_SHIFT 4U
/** Write check @p check as x430_reg::flags bits. */
#define X430_REG_CHECK(check) ((uint32_t)(check) << X430_REG_CHECK_SHIFT)

/** @} */

/** @name Type definitions */
/** @{ */

/** @brief Write check of a register (upper four bits of x430_reg::flags). */
enum x430_check
{
    X430_CHECK_NONE = 0,      /**< Read-only register, no check. */
    X430_CHECK_RANGE,         /**< min <= value <= max. */
    X430_CHECK_MASK,          /**< Only the bits of max may be set. */
    X430_CHECK_OPMODE,        /**< One of x430_model::operating_modes. */
    X430_CHECK_SYMMETRIC,     /**< -limit <= value <= limit; max = limit register address. */
    X430_CHECK_GOAL_POSITION, /**< Position limits in mode 3, +-1,048,575 otherwise. */
    X430_CHECK_INDIRECT       /**< Indirect Address: RAM address min..max. */
};

/** @brief One Control Table item (16 bytes of flash). */
struct x430_reg
{
    uint16_t address; /**< First byte. */
    uint8_t size;     /**< 1, 2 or 4 bytes. */
    uint8_t flags;    /**< X430_REG_* bits, X430_REG_CHECK() of the write check. */
    int32_t initial;  /**< Value after a factory reset. */
    int32_t min;      /**< RANGE/INDIRECT lower bound. */
    int32_t max; /**< RANGE/INDIRECT upper bound, MASK allowed bits, SYMMETRIC limit address. */
};

/** @brief Indirect Address block: count addresses of 2 bytes and count data bytes. */
struct x430_indirect_block
{
    uint16_t address; /**< Indirect Address 1 of the block. */
    uint16_t data;    /**< Indirect Data 1 of the block. */
    uint16_t count;   /**< Number of entries. */
};

/**
 * @brief Segment of the compact Control Table image of a device: Control Table
 *        addresses address .. address + length - 1 are image bytes offset .. offset +
 *        length - 1. Addresses outside every segment are reserved and read 0.
 */
struct x430_segment
{
    uint16_t address; /**< First Control Table address. */
    uint16_t length;  /**< Bytes. */
    uint16_t offset;  /**< Index of the first byte in the image. */
};

/**
 * @brief Control Table and capabilities of one model.
 *
 * The Indirect Address registers are not in @ref x430_model::registers: the library
 * generates them from the blocks (2 bytes, RW, initial value Indirect Data n, range
 * 64 .. table_size - 1) and numbers them after the table registers, which all lie
 * below the first block.
 */
struct x430_model
{
    const char* name;                           /**< e.g. "XM430-W350". */
    const struct x430_reg* registers;           /**< Sorted by address, no Indirect Address. */
    const struct x430_indirect_block* indirect; /**< Indirect blocks, sorted by address. */
    const struct x430_segment* segments;        /**< Image segments; the first starts at 0. */
    uint32_t operating_modes;                   /**< Bit n set: Operating Mode n allowed. */
    uint16_t model_number;                      /**< Number from the documentation. */
    uint16_t table_size;                        /**< Addresses 0 .. table_size - 1. */
    uint16_t register_count;                    /**< Entries in registers. */
    uint16_t current_unit_ua;                   /**< Current unit in uA, 0 = no current. */
    uint8_t indirect_count;                     /**< Entries in indirect. */
    uint8_t segment_count;                      /**< Entries in segments. */
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
