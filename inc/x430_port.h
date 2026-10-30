/**
 * @file x430_port.h
 * @brief Operating-system port: lock, time and delay, all optional.
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
 * @defgroup x430_port Port
 * @ingroup x430
 * @brief The only contact points between the library and the platform.
 *
 * @details The library has no RTOS or driver dependency. Every member may be
 *          NULL; the library then works without that service:
 *
 *          | Member   | Used for                                   | NULL means                   |
 *          |----------|--------------------------------------------|------------------------------|
 *          | lock     | mutual exclusion of the Control Tables     | single context (bare metal)  |
 *          | unlock   | (pair of lock)                             |                              |
 *          | millis   | Realtime Tick (120), Bus Watchdog (98)     | tick 0, no watchdog timeout  |
 *          | delay_us | Return Delay Time (9) before a status      | status sent at once          |
 *
 *          lock/unlock wrap every bus entry point (packet handling, polling)
 *          and @ref x430_bus_lock. Hooks run with the lock held, so a hook must
 *          not take it again: use a recursive mutex or call only the
 *          lock-free device functions (x430_device_get / x430_device_set) from
 *          hooks. Typical bindings:
 *
 *          @li Zephyr: k_mutex_lock/k_mutex_unlock (recursive), k_uptime_get_32,
 *              k_busy_wait.
 *          @li FreeRTOS: xSemaphoreTakeRecursive/GiveRecursive, xTaskGetTickCount
 *              converted to ms.
 *          @li POSIX: pthread_mutex with PTHREAD_MUTEX_RECURSIVE, clock_gettime.
 *
 *          Queues are not needed by the library. To move work out of the bus
 *          context, post from a hook (x430_hooks::event, x430_hooks::write) to
 *          an application queue and let another thread act on it; that thread
 *          updates live values between @ref x430_bus_lock and
 *          @ref x430_bus_unlock.
 * @{
 */

#ifndef X430_PORT_H
#define X430_PORT_H

/** @name Includes */
/** @{ */

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** @} */

/** @name Type definitions */
/** @{ */

/** @brief Platform services. Copy semantics: the bus keeps a pointer, keep it alive. */
struct x430_port
{
    void* context;                                /**< Passed to every function. */
    void (*lock)(void* context);                  /**< Take the bus lock. */
    void (*unlock)(void* context);                /**< Release the bus lock. */
    uint32_t (*millis)(void* context);            /**< Free-running millisecond counter. */
    void (*delay_us)(void* context, uint32_t us); /**< Busy or sleeping delay. */
};

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* X430_PORT_H */

/** @} */
