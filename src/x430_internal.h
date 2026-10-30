/**
 * @file x430_internal.h
 * @brief Library-internal helpers shared by the device and the bus.
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

#ifndef X430_INTERNAL_H
#define X430_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

#include "x430_bus.h"
#include "x430_device.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** @brief True when another device on @p bus uses @p id (bus may be NULL). */
bool x430_bus_id_taken(const struct x430_bus* bus, const struct x430_device* except, uint8_t id);

/** @brief Milliseconds from the bus port, 0 without bus or clock. */
uint32_t x430_bus_millis(const struct x430_bus* bus);

#ifdef __cplusplus
}
#endif

#endif /* X430_INTERNAL_H */
