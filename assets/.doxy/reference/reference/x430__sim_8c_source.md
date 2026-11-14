

# File x430\_sim.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**x430\_sim.c**](x430__sim_8c.md)

[Go to the documentation of this file](x430__sim_8c.md)


```C++
/*******************************************************************************
 * DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
 *
 * Project:     DYNAMIXEL-X430-Control-Table-Emulator
 * Description: Simulator: a kinematic model of an X430 behind the Control Table
 *              (profiles, operating modes, live registers).
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

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include "x430.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "x430_internal.h"

/* ==============================================================================
 * PRIVATE TYPE DEFINITIONS
 * =============================================================================*/

/* Alignment probes (see x430_device.c) */
struct x430_align_sim_ctx
{
    char c;
    struct x430_sim_ctx x;
};

struct x430_align_sim
{
    char c;
    struct x430_sim x;
};

_Static_assert(sizeof(struct x430_sim_ctx) <= sizeof(struct x430_sim),
               "X430_SIM_BYTES in x430_types.h is too small for this ABI");
_Static_assert((sizeof(struct x430_align_sim_ctx) - sizeof(struct x430_sim_ctx)) <=
                   (sizeof(struct x430_align_sim) - sizeof(struct x430_sim)),
               "struct x430_sim is not aligned for struct x430_sim_ctx");

#if X430_FEATURE_SIMULATOR

/* ==============================================================================
 * DEFINES & MACROS
 * =============================================================================*/

#define X430_SIM_VELOCITY_UNIT 15633
#define X430_SIM_ACCELERATION_UNIT 244140
#define X430_SIM_MILLI 1000
#define X430_SIM_PWM_FULL 885
#define X430_SIM_LOAD_FULL 1000
#define X430_SIM_TURN 4096
#define X430_SIM_DEFAULT_VOLTAGE 120U
#define X430_SIM_DEFAULT_TEMPERATURE 30U
#define X430_SIM_DISTANCE_MAX INT64_C(1000000000)

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS: ARITHMETIC
 * =============================================================================*/

static int64_t x430_sim_abs(int64_t value)
{
    int64_t magnitude = value;

    if (value < 0)
    {
        magnitude = -value;
    }
    return magnitude;
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

static int64_t x430_sim_min(int64_t a, int64_t b)
{
    int64_t result = a;

    if (b < a)
    {
        result = b;
    }
    return result;
}

static uint64_t x430_sim_isqrt(uint64_t value)
{
    uint64_t rest = value;
    uint64_t result = 0U;
    uint64_t bit = (uint64_t)1U << 62;

    while (bit > rest)
    {
        bit >>= 2;
    }
    while (bit != 0U)
    {
        if (rest >= (result + bit))
        {
            rest -= result + bit;
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

static int64_t x430_sim_div(int64_t numerator, int64_t denominator)
{
    const int64_t half = denominator / 2;
    int64_t result = (numerator + half) / denominator;

    if (numerator < 0)
    {
        result = (numerator - half) / denominator;
    }
    return result;
}

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS: MODEL
 * =============================================================================*/

static int32_t x430_sim_value(const struct x430_sim_ctx* s, uint16_t address)
{
    return x430_device_value(s->device, address);
}

static uint8_t x430_sim_byte(const struct x430_sim_ctx* s, uint16_t address)
{
    return x430_device_byte(s->device, address);
}

static int64_t x430_sim_direction(const struct x430_sim_ctx* s)
{
    int64_t direction = 1;

    if ((x430_sim_byte(s, X430_ADDR_DRIVE_MODE) & X430_DRIVE_MODE_REVERSE) != 0U)
    {
        direction = -1;
    }
    return direction;
}

static int32_t x430_sim_present(const struct x430_sim_ctx* s)
{
    const int64_t shaft = x430_sim_div(s->shaft_mpulse, X430_SIM_MILLI);

    return (int32_t)((shaft * x430_sim_direction(s)) + x430_sim_value(s, X430_ADDR_HOMING_OFFSET));
}

static int64_t x430_sim_goal_shaft(const struct x430_sim_ctx* s, int32_t goal)
{
    const int64_t offset = x430_sim_value(s, X430_ADDR_HOMING_OFFSET);
    const int64_t shaft = ((int64_t)goal - offset) * x430_sim_direction(s);

    return shaft * X430_SIM_MILLI;
}

static bool x430_sim_position_mode(uint8_t mode)
{
    return (mode == X430_MODE_POSITION) || (mode == X430_MODE_EXTENDED_POSITION) ||
           (mode == X430_MODE_CURRENT_BASED_POSITION);
}

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

static int64_t x430_sim_tick_position(struct x430_sim_ctx* s, int64_t limit, int64_t acceleration)
{
    const int64_t profile = x430_sim_value(s, X430_ADDR_PROFILE_VELOCITY);
    const int64_t goal = x430_sim_goal_shaft(s, x430_sim_value(s, X430_ADDR_GOAL_POSITION));
    const int64_t error = goal - s->shaft_mpulse;
    const int64_t distance = x430_sim_abs(error);
    int64_t vmax = limit;
    int64_t reach = distance * X430_SIM_MILLI; /* speed that arrives within 1 ms */
    int64_t desired = 0;
    int64_t move = 0;

    if (profile != 0)
    {
        vmax = profile * X430_SIM_VELOCITY_UNIT;
    }
    if (acceleration != 0)
    {
        /* Fastest speed that can still brake to 0 at the goal: sqrt(2 a d). */
        const uint64_t span = (uint64_t)x430_sim_clamp(distance, X430_SIM_DISTANCE_MAX);
        const uint64_t braking = x430_sim_isqrt(2U * (uint64_t)acceleration * span);

        reach = x430_sim_min(reach, (int64_t)braking);
    }
    reach = x430_sim_min(reach, vmax);
    desired = reach;
    if (error < 0)
    {
        desired = -reach;
    }
    s->velocity_mpulse = x430_sim_ramp(s->velocity_mpulse, desired, acceleration / X430_SIM_MILLI);
    move = s->velocity_mpulse / X430_SIM_MILLI;

    if ((distance <= x430_sim_abs(move)) || (distance < (X430_SIM_MILLI / 2)))
    {
        /* Arrives within this millisecond (or is closer than half a pulse). */
        s->shaft_mpulse += error;
        s->velocity_mpulse = 0;
    }
    else
    {
        s->shaft_mpulse += move;
    }
    return desired;
}

static int64_t x430_sim_goal_velocity(const struct x430_sim_ctx* s, uint8_t mode, int64_t limit)
{
    int64_t desired = 0;

    if (mode == X430_MODE_VELOCITY)
    {
        desired = (int64_t)x430_sim_value(s, X430_ADDR_GOAL_VELOCITY) * X430_SIM_VELOCITY_UNIT;
    }
    else if (mode == X430_MODE_PWM)
    {
        const int64_t full = x430_sim_value(s, X430_ADDR_PWM_LIMIT);

        if (full != 0)
        {
            desired = ((int64_t)x430_sim_value(s, X430_ADDR_GOAL_PWM) * limit) / full;
        }
    }
    else
    {
#if X430_MODEL_HAS_CURRENT
        /* Current mode (0), the only one left in the model's Operating Mode check. */
        const int64_t full = x430_sim_value(s, X430_ADDR_CURRENT_LIMIT);

        if (full != 0)
        {
            desired = ((int64_t)x430_sim_value(s, X430_ADDR_GOAL_CURRENT) * limit) / full;
        }
#endif
    }
    return x430_sim_clamp(desired, limit) * x430_sim_direction(s);
}

static int64_t x430_sim_tick(struct x430_sim_ctx* s)
{
    const uint8_t mode = x430_sim_byte(s, X430_ADDR_OPERATING_MODE);
    const int64_t limit =
        (int64_t)x430_sim_value(s, X430_ADDR_VELOCITY_LIMIT) * X430_SIM_VELOCITY_UNIT;
    const int64_t acceleration =
        (int64_t)x430_sim_value(s, X430_ADDR_PROFILE_ACCELERATION) * X430_SIM_ACCELERATION_UNIT;
    int64_t desired = 0;

    if (x430_sim_byte(s, X430_ADDR_TORQUE_ENABLE) == 0U)
    {
        s->velocity_mpulse = 0;
    }
    else if (x430_sim_position_mode(mode))
    {
        desired = x430_sim_tick_position(s, limit, acceleration);
    }
    else
    {
        desired = x430_sim_goal_velocity(s, mode, limit);
        s->velocity_mpulse =
            x430_sim_ramp(s->velocity_mpulse, desired, acceleration / X430_SIM_MILLI);
        s->shaft_mpulse += s->velocity_mpulse / X430_SIM_MILLI;
    }
    return desired;
}

static uint8_t x430_sim_moving_status(const struct x430_sim_ctx* s, bool arrived, bool profile)
{
    const bool accelerated = x430_sim_value(s, X430_ADDR_PROFILE_ACCELERATION) != 0;
    const bool limited = x430_sim_value(s, X430_ADDR_PROFILE_VELOCITY) != 0;
    uint8_t status = 0U;

    if (arrived)
    {
        status |= X430_MOVING_STATUS_IN_POSITION;
    }
    if (profile)
    {
        status |= X430_MOVING_STATUS_PROFILE_ONGOING;
    }
    if (accelerated && limited)
    {
        status |= X430_MOVING_STATUS_PROFILE_TRAPEZOIDAL;
    }
    else if (accelerated)
    {
        status |= X430_MOVING_STATUS_PROFILE_TRIANGULAR;
    }
    else if (limited)
    {
        status |= X430_MOVING_STATUS_PROFILE_RECTANGULAR;
    }
    else
    {
        /* Step profile: bits 4-5 stay 0. */
    }
    return status;
}

static void x430_sim_publish(struct x430_sim_ctx* s, int64_t desired, int64_t change)
{
    struct x430_device* device = s->device;
    const int64_t direction = x430_sim_direction(s);
    const uint8_t mode = x430_sim_byte(s, X430_ADDR_OPERATING_MODE);
    const int32_t velocity =
        (int32_t)x430_sim_div(s->velocity_mpulse * direction, X430_SIM_VELOCITY_UNIT);
    const int64_t limit = x430_sim_value(s, X430_ADDR_VELOCITY_LIMIT);
    const int32_t present = x430_sim_present(s);
    const bool torque = x430_sim_byte(s, X430_ADDR_TORQUE_ENABLE) != 0U;
    const bool position_mode = x430_sim_position_mode(mode);
    const int32_t goal = x430_sim_value(s, X430_ADDR_GOAL_POSITION);
    const bool arrived = position_mode && (present == goal);
    /* Moving and Profile Ongoing: the shaft turns, or a Goal Position move is not done.
       (The model turns the shaft only with the torque on, so the Moving Threshold of a
       real X430 adds nothing here.) */
    const bool moving = torque && ((s->velocity_mpulse != 0) || (position_mode && !arrived));
    int32_t trajectory = present;
    int32_t pwm = 0;
    int32_t voltage = (int32_t)X430_SIM_DEFAULT_VOLTAGE;
    int32_t temperature = (int32_t)X430_SIM_DEFAULT_TEMPERATURE;
    int32_t moving_value = 0;
    uint8_t status = 0U;
    int64_t effort = 0;

    if (position_mode)
    {
        trajectory = goal;
    }
    if (limit != 0)
    {
        pwm = (int32_t)(((int64_t)velocity * X430_SIM_PWM_FULL) / limit);
    }
    if (s->config.input_voltage != 0U)
    {
        voltage = (int32_t)s->config.input_voltage;
    }
    if (s->config.temperature != 0U)
    {
        temperature = (int32_t)s->config.temperature;
    }
    if (moving)
    {
        moving_value = 1;
    }

    x430_device_store(device, X430_ADDR_PRESENT_POSITION, present);
    x430_device_store(device, X430_ADDR_PRESENT_VELOCITY, velocity);
    x430_device_store(device, X430_ADDR_VELOCITY_TRAJECTORY,
                      (int32_t)x430_sim_div(desired * direction, X430_SIM_VELOCITY_UNIT));
    x430_device_store(device, X430_ADDR_POSITION_TRAJECTORY, trajectory);
    x430_device_store(device, X430_ADDR_PRESENT_PWM, pwm);
    x430_device_store(device, X430_ADDR_MOVING, moving_value);
    x430_device_store(device, X430_ADDR_PRESENT_INPUT_VOLTAGE, voltage);
    x430_device_store(device, X430_ADDR_PRESENT_TEMPERATURE, temperature);

    if (torque)
    {
        /* Effort follows the velocity change: a rough stand-in for current or load. */
        effort = x430_sim_div(change * direction, X430_SIM_VELOCITY_UNIT);
#if X430_MODEL_HAS_CURRENT
        effort = x430_sim_clamp(effort, x430_sim_value(s, X430_ADDR_CURRENT_LIMIT));
#else
        effort = x430_sim_clamp(effort, X430_SIM_LOAD_FULL);
#endif
        status = x430_sim_moving_status(s, arrived, moving);
    }
    x430_device_store(device, X430_ADDR_PRESENT_CURRENT, (int32_t)effort);
    x430_device_store(device, X430_ADDR_MOVING_STATUS, (int32_t)status);
}

static struct x430_sim_ctx* x430_sim_of(void* user_data)
{
    /* cppcheck-suppress misra-c2012-11.5 ; DEV-02 callback context */
    struct x430_sim_ctx* s = x430_sim_ctx_of((struct x430_sim*)user_data);

    if ((s != NULL) && (s->ready != X430_READY))
    {
        s = NULL;
    }
    return s;
}

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS: HOOKS
 * =============================================================================*/

static uint8_t x430_sim_write(struct x430_device* device, uint16_t address, uint8_t size,
                              int32_t value, void* user_data)
{
    struct x430_sim_ctx* s = x430_sim_of(user_data);

    (void)device;
    (void)size;
    (void)value;
    if ((s != NULL) &&
        ((address == X430_ADDR_TORQUE_ENABLE) || (address == X430_ADDR_OPERATING_MODE) ||
         (address == X430_ADDR_HOMING_OFFSET) || (address == X430_ADDR_DRIVE_MODE)))
    {
        /* Mode, scale or power changes start from standstill. */
        s->velocity_mpulse = 0;
    }
    return DXL2_ERR_NONE;
}

static uint8_t x430_sim_event(struct x430_device* device, enum x430_event event, void* user_data)
{
    struct x430_sim_ctx* s = x430_sim_of(user_data);

    (void)device;
    if (s != NULL)
    {
        switch (event)
        {
            case X430_EVENT_REBOOT:
            case X430_EVENT_FACTORY_RESET:
            case X430_EVENT_TORQUE_SHUTDOWN:
                s->velocity_mpulse = 0;
                x430_sim_publish(s, 0, 0);
                break;
            case X430_EVENT_BUS_WATCHDOG:
                /* Stop: hold the present position, zero velocity goal. */
                s->velocity_mpulse = 0;
                x430_sim_publish(s, 0, 0);
                x430_device_store(s->device, X430_ADDR_GOAL_POSITION,
                                  x430_sim_value(s, X430_ADDR_PRESENT_POSITION));
                x430_device_store(s->device, X430_ADDR_GOAL_VELOCITY, 0);
                break;
            case X430_EVENT_CLEAR_MULTI_TURN: {
                int32_t present = x430_sim_present(s) % X430_SIM_TURN;

                if (present < 0)
                {
                    present += X430_SIM_TURN;
                }
                s->shaft_mpulse = x430_sim_goal_shaft(s, present);
                x430_sim_publish(s, 0, 0);
                x430_device_store(s->device, X430_ADDR_GOAL_POSITION, present);
                break;
            }
            default:
                /* Baud Rate, backup: nothing to simulate. */
                break;
        }
    }
    return DXL2_ERR_NONE;
}

/* ==============================================================================
 * PUBLIC VARIABLES
 * =============================================================================*/

/* Live registers are refreshed by x430_sim_step(): the simulator needs no read hook. */
const struct x430_hooks x430_sim_hooks = {
    .write = x430_sim_write,
    .read = NULL,
    .event = x430_sim_event,
    .nvm_read = NULL,
    .nvm_write = NULL,
};

/* ==============================================================================
 * PUBLIC FUNCTION IMPLEMENTATIONS
 * =============================================================================*/

int x430_sim_init(struct x430_sim* sim, struct x430_device* device,
                  const struct x430_sim_config* config)
{
    struct x430_sim_ctx* s = x430_sim_ctx_of(sim);
    int result = X430_OK;

    if ((s == NULL) || (device == NULL))
    {
        result = X430_ERR_PARAM;
    }
    else
    {
        *s = (struct x430_sim_ctx){0};
        s->device = device;
        if (config != NULL)
        {
            s->config = *config;
        }
        s->ready = X430_READY;
    }
    return result;
}

int x430_sim_step(struct x430_sim* sim, uint32_t elapsed_ms)
{
    struct x430_sim_ctx* s = x430_sim_ctx_of(sim);
    int result = X430_OK;

    if (s == NULL)
    {
        result = X430_ERR_PARAM;
    }
    else if ((s->ready != X430_READY) || !x430_device_ready(s->device))
    {
        result = X430_ERR_STATE;
    }
    else
    {
        struct x430_bus* bus = x430_device_bus(s->device);
        int64_t desired = 0;
        int64_t change = 0;

        (void)x430_bus_lock(bus); /* X430_ERR_PARAM without a bus: nothing to lock */
        for (uint32_t tick = 0U; tick < elapsed_ms; tick++)
        {
            const int64_t before = s->velocity_mpulse;

            desired = x430_sim_tick(s);
            change = s->velocity_mpulse - before;
        }
        x430_sim_publish(s, desired, change);
        (void)x430_bus_unlock(bus);
    }
    return result;
}

#else /* X430_FEATURE_SIMULATOR */

const struct x430_hooks x430_sim_hooks = {
    .write = NULL,
    .read = NULL,
    .event = NULL,
    .nvm_read = NULL,
    .nvm_write = NULL,
};

int x430_sim_init(struct x430_sim* sim, struct x430_device* device,
                  const struct x430_sim_config* config)
{
    (void)sim;
    (void)device;
    (void)config;
    return X430_ERR_UNSUPPORTED;
}

int x430_sim_step(struct x430_sim* sim, uint32_t elapsed_ms)
{
    (void)sim;
    (void)elapsed_ms;
    return X430_ERR_UNSUPPORTED;
}

#endif /* X430_FEATURE_SIMULATOR */
```


