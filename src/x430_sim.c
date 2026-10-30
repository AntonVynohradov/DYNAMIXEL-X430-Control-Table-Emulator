/**
 * @file x430_sim.c
 * @brief Kinematic actuator model for the X430 emulator (integer arithmetic).
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

/** @name Includes */
/** @{ */

#include "x430_sim.h"

#include <stddef.h>
#include <string.h>

#include "dxl2.h"
#include "x430_bus.h"

/** @} */

#if X430_FEATURE_SIMULATOR

/** @name Private definitions */
/** @{ */

/** 0.229 rev/min in 1/1000 pulse per second (0.229 * 4096 / 60 * 1000). */
#define X430_SIM_VELOCITY_UNIT 15633
/** 214.577 rev/min^2 in 1/1000 pulse per second^2 (214.577 * 4096 / 3600 * 1000). */
#define X430_SIM_ACCELERATION_UNIT 244140
/** Sub-units per pulse. */
#define X430_SIM_MILLI 1000
/** PWM full scale (100 %). */
#define X430_SIM_PWM_FULL 885
/** Present Load full scale (100 %). */
#define X430_SIM_LOAD_FULL 1000
/** Pulses per turn. */
#define X430_SIM_TURN 4096
#define X430_SIM_DEFAULT_VOLTAGE 120U
#define X430_SIM_DEFAULT_TEMPERATURE 30U

/** @} */

/** @name Private helpers */
/** @{ */

static int64_t x430_sim_abs(int64_t value)
{
    return (value < 0) ? -value : value;
}

static int64_t x430_sim_clamp(int64_t value, int64_t limit)
{
    int64_t result = value;

    if (value > limit)
    {
        result = limit;
    }
    else if (value < -limit)
    {
        result = -limit;
    }
    else
    {
        /* Inside the limit. */
    }
    return result;
}

/** Integer square root (floor). */
static uint64_t x430_sim_isqrt(uint64_t value)
{
    uint64_t result = 0U;
    uint64_t bit = (uint64_t) 1U << 62;

    while (bit > value)
    {
        bit >>= 2;
    }
    while (bit != 0U)
    {
        if (value >= (result + bit))
        {
            value -= result + bit;
            result = (result >> 1) + bit;
        }
        else
        {
            result >>= 1;
        }
        bit >>= 2;
    }
    return result;
}

/** Division rounded to the nearest integer, halves away from zero. */
static int64_t x430_sim_div(int64_t numerator, int64_t denominator)
{
    const int64_t half = denominator / 2;

    return (numerator >= 0) ? ((numerator + half) / denominator) :
                              ((numerator - half) / denominator);
}

static int64_t x430_sim_direction(const struct x430_device* device)
{
    return ((device->table[X430_ADDR_DRIVE_MODE] & X430_DRIVE_MODE_REVERSE) != 0U) ? -1 : 1;
}

/** Shaft position (pulses) to Present Position. */
static int32_t x430_sim_present(const struct x430_sim* sim)
{
    const int64_t shaft = x430_sim_div(sim->shaft_mpulse, X430_SIM_MILLI);

    return (int32_t) ((shaft * x430_sim_direction(sim->device))
                      + x430_device_get(sim->device, X430_ADDR_HOMING_OFFSET));
}

/** Goal Position to shaft position in 1/1000 pulse. */
static int64_t x430_sim_goal_shaft(const struct x430_sim* sim, int32_t goal)
{
    const int64_t shaft = ((int64_t) goal - x430_device_get(sim->device, X430_ADDR_HOMING_OFFSET))
                          * x430_sim_direction(sim->device);

    return shaft * X430_SIM_MILLI;
}

static bool x430_sim_position_mode(uint8_t mode)
{
    return (mode == X430_MODE_POSITION) || (mode == X430_MODE_EXTENDED_POSITION)
           || (mode == X430_MODE_CURRENT_BASED_POSITION);
}

/** Moves velocity towards target, at most step (0 = jump). */
static int64_t x430_sim_ramp(int64_t velocity, int64_t target, int64_t step)
{
    int64_t result = target;

    if (step != 0)
    {
        if (target > (velocity + step))
        {
            result = velocity + step;
        }
        else if (target < (velocity - step))
        {
            result = velocity - step;
        }
        else
        {
            /* Within one step: reach the target. */
        }
    }
    return result;
}

/** Position modes: trapezoidal approach of the goal for one millisecond. */
static int64_t x430_sim_tick_position(struct x430_sim* sim, int64_t limit, int64_t acceleration)
{
    const struct x430_device* device = sim->device;
    const int32_t profile = x430_device_get(device, X430_ADDR_PROFILE_VELOCITY);
    const int64_t vmax = (profile == 0) ? limit : ((int64_t) profile * X430_SIM_VELOCITY_UNIT);
    const int64_t error = x430_sim_goal_shaft(sim, x430_device_get(device, X430_ADDR_GOAL_POSITION))
                          - sim->shaft_mpulse;
    const int64_t distance = x430_sim_abs(error);
    int64_t reach = distance * X430_SIM_MILLI; /* speed that arrives within 1 ms */
    int64_t desired = 0;
    int64_t move = 0;

    if (acceleration != 0)
    {
        /* Fastest speed that can still brake to 0 at the goal: sqrt(2 a d). */
        const uint64_t braking =
            x430_sim_isqrt(2U * (uint64_t) acceleration
                           * (uint64_t) x430_sim_clamp(distance, INT64_C(1000000000)));

        reach = (reach < (int64_t) braking) ? reach : (int64_t) braking;
    }
    reach = (reach < vmax) ? reach : vmax;
    desired = (error >= 0) ? reach : -reach;
    sim->velocity_mpulse =
        x430_sim_ramp(sim->velocity_mpulse, desired, acceleration / X430_SIM_MILLI);
    move = sim->velocity_mpulse / X430_SIM_MILLI;

    if ((distance <= x430_sim_abs(move)) || (distance < (X430_SIM_MILLI / 2)))
    {
        /* Arrives within this millisecond (or is closer than half a pulse). */
        sim->shaft_mpulse += error;
        sim->velocity_mpulse = 0;
    }
    else
    {
        sim->shaft_mpulse += move;
    }
    return desired;
}

/** One millisecond of motion; returns the velocity the profile wants. */
static int64_t x430_sim_tick(struct x430_sim* sim)
{
    const struct x430_device* device = sim->device;
    const uint8_t mode = device->table[X430_ADDR_OPERATING_MODE];
    const int64_t limit =
        (int64_t) x430_device_get(device, X430_ADDR_VELOCITY_LIMIT) * X430_SIM_VELOCITY_UNIT;
    const int64_t acceleration = (int64_t) x430_device_get(device, X430_ADDR_PROFILE_ACCELERATION)
                                 * X430_SIM_ACCELERATION_UNIT;
    int64_t desired = 0;

    if (device->table[X430_ADDR_TORQUE_ENABLE] == 0U)
    {
        sim->velocity_mpulse = 0;
    }
    else if (x430_sim_position_mode(mode))
    {
        desired = x430_sim_tick_position(sim, limit, acceleration);
    }
    else
    {
        if (mode == X430_MODE_VELOCITY)
        {
            desired =
                (int64_t) x430_device_get(device, X430_ADDR_GOAL_VELOCITY) * X430_SIM_VELOCITY_UNIT;
        }
        else if (mode == X430_MODE_PWM)
        {
            const int32_t full = x430_device_get(device, X430_ADDR_PWM_LIMIT);

            desired = (full == 0) ?
                          0 :
                          (((int64_t) x430_device_get(device, X430_ADDR_GOAL_PWM) * limit) / full);
        }
        else
        {
#if X430_MODEL_HAS_CURRENT
            const int32_t full = x430_device_get(device, X430_ADDR_CURRENT_LIMIT);

            desired =
                (full == 0) ?
                    0 :
                    (((int64_t) x430_device_get(device, X430_ADDR_GOAL_CURRENT) * limit) / full);
#endif
        }
        desired = x430_sim_clamp(desired, limit) * x430_sim_direction(device);
        sim->velocity_mpulse =
            x430_sim_ramp(sim->velocity_mpulse, desired, acceleration / X430_SIM_MILLI);
        sim->shaft_mpulse += sim->velocity_mpulse / X430_SIM_MILLI;
    }
    return desired;
}

/** Writes the live registers from the model state. */
static void x430_sim_publish(struct x430_sim* sim, int64_t desired, int64_t change)
{
    struct x430_device* device = sim->device;
    const int64_t direction = x430_sim_direction(device);
    const uint8_t mode = device->table[X430_ADDR_OPERATING_MODE];
    const int32_t velocity =
        (int32_t) x430_sim_div(sim->velocity_mpulse * direction, X430_SIM_VELOCITY_UNIT);
    const int32_t limit = x430_device_get(device, X430_ADDR_VELOCITY_LIMIT);
    const int32_t present = x430_sim_present(sim);
    const bool torque = device->table[X430_ADDR_TORQUE_ENABLE] != 0U;
    const bool position_mode = x430_sim_position_mode(mode);
    const int32_t goal = x430_device_get(device, X430_ADDR_GOAL_POSITION);
    const bool arrived = position_mode && (present == goal);
    const bool profile = torque && ((sim->velocity_mpulse != 0) || (position_mode && !arrived));
    /* Moving: |Present Velocity| > Moving Threshold, or a Goal Position profile in progress. */
    const bool moving = profile || (velocity > x430_device_get(device, X430_ADDR_MOVING_THRESHOLD))
                        || (-velocity > x430_device_get(device, X430_ADDR_MOVING_THRESHOLD));
    uint8_t status = 0U;
    int64_t effort = 0;

    (void) x430_device_set(device, X430_ADDR_PRESENT_POSITION, present);
    (void) x430_device_set(device, X430_ADDR_PRESENT_VELOCITY, velocity);
    (void) x430_device_set(device, X430_ADDR_VELOCITY_TRAJECTORY,
                           (int32_t) x430_sim_div(desired * direction, X430_SIM_VELOCITY_UNIT));
    (void) x430_device_set(device, X430_ADDR_POSITION_TRAJECTORY, position_mode ? goal : present);
    (void) x430_device_set(
        device, X430_ADDR_PRESENT_PWM,
        (limit == 0) ? 0 : (int32_t) (((int64_t) velocity * X430_SIM_PWM_FULL) / limit));
    (void) x430_device_set(device, X430_ADDR_MOVING, moving ? 1 : 0);
    (void) x430_device_set(device, X430_ADDR_PRESENT_INPUT_VOLTAGE,
                           (int32_t) ((sim->config.input_voltage != 0U) ?
                                          sim->config.input_voltage :
                                          X430_SIM_DEFAULT_VOLTAGE));
    (void) x430_device_set(device, X430_ADDR_PRESENT_TEMPERATURE,
                           (int32_t) ((sim->config.temperature != 0U) ?
                                          sim->config.temperature :
                                          X430_SIM_DEFAULT_TEMPERATURE));

    if (torque)
    {
        /* Effort follows the velocity change: a rough stand-in for current or load. */
        effort = x430_sim_div(change * direction, X430_SIM_VELOCITY_UNIT);
#if X430_MODEL_HAS_CURRENT
        effort = x430_sim_clamp(effort, x430_device_get(device, X430_ADDR_CURRENT_LIMIT));
#else
        effort = x430_sim_clamp(effort, X430_SIM_LOAD_FULL);
#endif
        if (arrived)
        {
            status |= X430_MOVING_STATUS_IN_POSITION;
        }
        if (profile)
        {
            status |= X430_MOVING_STATUS_PROFILE_ONGOING;
        }
        if (x430_device_get(device, X430_ADDR_PROFILE_ACCELERATION) != 0)
        {
            status |= (x430_device_get(device, X430_ADDR_PROFILE_VELOCITY) != 0) ?
                          X430_MOVING_STATUS_PROFILE_TRAPEZOIDAL :
                          X430_MOVING_STATUS_PROFILE_TRIANGULAR;
        }
        else if (x430_device_get(device, X430_ADDR_PROFILE_VELOCITY) != 0)
        {
            status |= X430_MOVING_STATUS_PROFILE_RECTANGULAR;
        }
        else
        {
            /* Step profile: bits 4-5 stay 0. */
        }
    }
    (void) x430_device_set(device, X430_ADDR_PRESENT_CURRENT, (int32_t) effort);
    (void) x430_device_set(device, X430_ADDR_MOVING_STATUS, (int32_t) status);
}

/** @} */

/** @name Hooks */
/** @{ */

static uint8_t x430_sim_write(struct x430_device* device, uint16_t address, uint8_t size,
                              int32_t value, void* user_data)
{
    struct x430_sim* sim = (struct x430_sim*) user_data;

    (void) device;
    (void) size;
    (void) value;
    if ((sim != NULL)
        && ((address == X430_ADDR_TORQUE_ENABLE) || (address == X430_ADDR_OPERATING_MODE)
            || (address == X430_ADDR_HOMING_OFFSET) || (address == X430_ADDR_DRIVE_MODE)))
    {
        /* Mode, scale or power changes start from standstill. */
        sim->velocity_mpulse = 0;
    }
    return DXL2_ERR_NONE;
}

static uint8_t x430_sim_event(struct x430_device* device, enum x430_event event, void* user_data)
{
    struct x430_sim* sim = (struct x430_sim*) user_data;
    uint8_t result = DXL2_ERR_NONE;

    (void) device;
    if (sim != NULL)
    {
        switch (event)
        {
            case X430_EVENT_REBOOT:
            case X430_EVENT_FACTORY_RESET:
            case X430_EVENT_TORQUE_SHUTDOWN:
                sim->velocity_mpulse = 0;
                x430_sim_publish(sim, 0, 0);
                break;
            case X430_EVENT_BUS_WATCHDOG:
                /* Stop: hold the present position, zero velocity goal. */
                sim->velocity_mpulse = 0;
                x430_sim_publish(sim, 0, 0);
                (void) x430_device_set(sim->device, X430_ADDR_GOAL_POSITION,
                                       x430_device_get(sim->device, X430_ADDR_PRESENT_POSITION));
                (void) x430_device_set(sim->device, X430_ADDR_GOAL_VELOCITY, 0);
                break;
            case X430_EVENT_CLEAR_MULTI_TURN:
            {
                int32_t present = x430_sim_present(sim) % X430_SIM_TURN;

                present = (present < 0) ? (present + X430_SIM_TURN) : present;
                sim->shaft_mpulse = x430_sim_goal_shaft(sim, present);
                x430_sim_publish(sim, 0, 0);
                (void) x430_device_set(sim->device, X430_ADDR_GOAL_POSITION, present);
                break;
            }
            default:
                /* Baud Rate, backup: nothing to simulate. */
                break;
        }
    }
    return result;
}

static void x430_sim_read(struct x430_device* device, uint16_t address, uint16_t length,
                          void* user_data)
{
    /* Live registers are refreshed by x430_sim_step; nothing to do on a read. */
    (void) device;
    (void) address;
    (void) length;
    (void) user_data;
}

const struct x430_hooks x430_sim_hooks = {
    .write = x430_sim_write,
    .read = x430_sim_read,
    .event = x430_sim_event,
    .nvm_load = NULL,
    .nvm_store = NULL,
};

/** @} */

/** @name Public functions */
/** @{ */

int x430_sim_init(struct x430_sim* sim, struct x430_device* device,
                  const struct x430_sim_config* config)
{
    int result = X430_EINVAL;

    if ((sim != NULL) && (device != NULL))
    {
        (void) memset(sim, 0, sizeof(*sim));
        sim->device = device;
        if (config != NULL)
        {
            sim->config = *config;
        }
        result = X430_OK;
    }
    return result;
}

void x430_sim_step(struct x430_sim* sim, uint32_t elapsed_ms)
{
    if ((sim != NULL) && (sim->device != NULL))
    {
        struct x430_bus* bus = sim->device->bus;
        int64_t desired = 0;
        int64_t change = 0;

        if (bus != NULL)
        {
            x430_bus_lock(bus);
        }
        for (uint32_t tick = 0U; tick < elapsed_ms; tick++)
        {
            const int64_t before = sim->velocity_mpulse;

            desired = x430_sim_tick(sim);
            change = sim->velocity_mpulse - before;
        }
        x430_sim_publish(sim, desired, change);
        if (bus != NULL)
        {
            x430_bus_unlock(bus);
        }
    }
}

/** @} */

#endif /* X430_FEATURE_SIMULATOR */
