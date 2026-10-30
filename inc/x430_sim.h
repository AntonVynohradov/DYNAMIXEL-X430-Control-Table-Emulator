/**
 * @file x430_sim.h
 * @brief Simulated actuator: makes an emulated device move without hardware.
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
 * @defgroup x430_sim Simulator
 * @ingroup x430
 * @brief Kinematic model of an X430 behind the Control Table.
 *
 * @details The simulator replaces the motor: pass @ref x430_sim_hooks and the
 *          simulator object as hooks and user data of a device, then call
 *          @ref x430_sim_step periodically. It is meant for tests, for
 *          configuring a whole system (ROS 2, DYNAMIXEL Wizard, DynamixelSDK)
 *          before hardware exists, and as a reference for real backends.
 *
 *          Model (per step, integer units of the Control Table):
 *          @li Torque off: velocity 0, the shaft keeps its position.
 *          @li Velocity mode (1): Present Velocity ramps to Goal Velocity with
 *              Profile Acceleration (0 = at once).
 *          @li Position modes (3, 4, 5): trapezoidal move to Goal Position with
 *              Profile Velocity (0 = Velocity Limit) and Profile Acceleration
 *              (0 = at once). Drive Mode bit 2 (time-based profile) is treated
 *              like the velocity-based profile.
 *          @li PWM (16) and current (0) modes: velocity proportional to the
 *              goal relative to its limit, scaled by Velocity Limit.
 *          @li Drive Mode bit 0 (reverse) and Homing Offset (20) apply to the
 *              reported position: Present = shaft * direction + offset.
 *          @li Moving (122) follows Moving Threshold (24); Moving Status (123)
 *              reports In-Position and Profile Ongoing.
 *          @li Present Input Voltage and Present Temperature are constants
 *              from the configuration; Present Current/Load follows the
 *              acceleration.
 *
 *          The simulator is built when X430_FEATURE_SIMULATOR is 1
 *          (CONFIG_X430_SIMULATOR).
 * @{
 */

#ifndef X430_SIM_H
#define X430_SIM_H

/** @name Includes */
/** @{ */

#include <stdint.h>

#include "x430_device.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** @} */

/** @name Type definitions */
/** @{ */

/** @brief Simulator configuration. Zero values select the defaults. */
struct x430_sim_config
{
    uint16_t input_voltage; /**< Present Input Voltage, 0.1 V (default 120). */
    uint8_t temperature;    /**< Present Temperature, deg C (default 30). */
};

/** @brief Simulator state. Treat the members as private. */
struct x430_sim
{
    struct x430_device* device;    /**< Simulated device. */
    struct x430_sim_config config; /**< Copy of the configuration. */
    int64_t shaft_mpulse;          /**< Shaft position, 1/1000 pulse. */
    int64_t velocity_mpulse;       /**< Shaft velocity, 1/1000 pulse per second. */
    uint32_t remainder_ms;         /**< Time not simulated yet. */
};

/** @} */

/** @name Public variables */
/** @{ */

/**
 * @brief Hooks implementing the simulated actuator.
 * @details Use with x430_device_config::user_data pointing at the x430_sim.
 */
extern const struct x430_hooks x430_sim_hooks;

/** @} */

/** @name Public function declarations */
/** @{ */

/**
 * @brief Prepare a simulator for @p device. Call before @ref x430_device_init.
 *
 * @code
 * static struct x430_device axis;
 * static struct x430_sim axis_sim;
 *
 * x430_sim_init(&axis_sim, &axis, NULL);
 * const struct x430_device_config cfg = {.id = 1U, .hooks = &x430_sim_hooks, .user_data =
 * &axis_sim}; x430_device_init(&axis, &cfg);
 * @endcode
 *
 * @param sim Simulator to initialize.
 * @param device Device it simulates (not initialized yet).
 * @param config Configuration, NULL for defaults.
 * @retval X430_OK Ready.
 * @retval X430_EINVAL @p sim or @p device is NULL.
 */
int x430_sim_init(struct x430_sim* sim, struct x430_device* device,
                  const struct x430_sim_config* config);

/**
 * @brief Advance the simulation and refresh the live registers.
 * @details Takes the bus lock of the device when it is attached to a bus.
 * @param sim Simulator.
 * @param elapsed_ms Time since the previous step.
 */
void x430_sim_step(struct x430_sim* sim, uint32_t elapsed_ms);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* X430_SIM_H */

/** @} */
