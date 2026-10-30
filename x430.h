/**
 * @file x430.h
 * @brief Umbrella header of the DYNAMIXEL X430 Control Table emulator.
 *
 * Project: sfs-dynamixel-x430 - DYNAMIXEL X430 Control Table Emulator
 * Author: Anton Vynohradov
 * Email: avynohradov@systemfromscratch.com
 *
 * @copyright Copyright (c) 2026 Anton Vynohradov
 * @license Apache License 2.0
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
 */

/**
 * @addtogroup x430 X430 API
 * @brief Device-side emulation of DYNAMIXEL X430 actuators (Protocol 2.0).
 *
 * @details Include this single header to use the library:
 * @li x430_config.h  build-time configuration (Kconfig or compiler definitions)
 * @li x430_defs.h    Control Table addresses and register values
 * @li x430_model.h   generated Control Table of the selected model
 * @li x430_port.h    optional platform services (lock, time, delay)
 * @li x430_device.h  one emulated actuator and its application hooks
 * @li x430_bus.h     instruction decoding for the devices on one line
 * @li x430_sim.h     simulated actuator (optional)
 *
 * The Protocol 2.0 wire format comes from DXL2-C (dxl2.h).
 * @{
 */

#ifndef X430_H
#define X430_H

/** @name Includes */
/** @{ */

#include "x430_config.h"
#include "x430_defs.h"
#include "x430_model.h"
#include "x430_port.h"
#include "x430_device.h"
#include "x430_bus.h"
#include "x430_sim.h"

/** @} */

/**
 * @name Library version
 * @details Injected by cmake/x430_version.cmake from the Git tag; the values
 *          below apply to builds that do not use CMake.
 */
/** @{ */

#ifndef X430_VERSION_MAJOR
#define X430_VERSION_MAJOR 0
#endif

#ifndef X430_VERSION_MINOR
#define X430_VERSION_MINOR 1
#endif

#ifndef X430_VERSION_PATCH
#define X430_VERSION_PATCH 0
#endif

/** Version string "MAJOR.MINOR.PATCH". */
#ifndef X430_VERSION
#define X430_VERSION "0.1.0"
#endif

/** @} */

#endif /* X430_H */

/** @} */
