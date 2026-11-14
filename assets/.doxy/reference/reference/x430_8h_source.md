

# File x430.h

[**File List**](files.md) **>** [**x430.h**](x430_8h.md)

[Go to the documentation of this file](x430_8h.md)


```C++
/*******************************************************************************
 * DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
 *
 * Project:     DYNAMIXEL-X430-Control-Table-Emulator
 * Description: Public API: library version, public types and all public functions.
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

#ifndef X430_H
#define X430_H

/* ==============================================================================
 * VERSION
 * =============================================================================*/

/* Set by cmake/version.cmake from the latest vX.Y.Z Git tag and stamped into this
 * file in the release package. 0.0.0 means unknown: a header taken from the
 * repository and built without CMake. Do not change these values for a release. */

#ifndef X430_VERSION_MAJOR
#define X430_VERSION_MAJOR 0
#endif

#ifndef X430_VERSION_MINOR
#define X430_VERSION_MINOR 0
#endif

#ifndef X430_VERSION_PATCH
#define X430_VERSION_PATCH 0
#endif

#ifndef X430_VERSION
#define X430_VERSION "0.0.0"
#endif

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <stddef.h>
#include <stdint.h>

#include "x430_config.h" /* first: sets DXL2_MAX_PACKET_SIZE for dxl2.h */
#include "dxl2.h"
#include "x430_defs.h"
#include "x430_model.h"
#include "x430_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==============================================================================
 * PUBLIC VARIABLES
 * =============================================================================*/

extern const struct x430_hooks x430_sim_hooks;

/* ==============================================================================
 * BUS
 * =============================================================================*/

int x430_bus_init(struct x430_bus* bus, const struct x430_bus_config* config);

int x430_bus_attach(struct x430_bus* bus, struct x430_device* device);

int x430_bus_receive(struct x430_bus* bus, const uint8_t* data, size_t length);

int x430_bus_handle_packet(struct x430_bus* bus, const struct dxl2_packet* packet);

int x430_bus_poll(struct x430_bus* bus);

int x430_bus_lock(struct x430_bus* bus);

int x430_bus_unlock(struct x430_bus* bus);

int x430_bus_find(const struct x430_bus* bus, uint8_t id, struct x430_device** device);

/* ==============================================================================
 * DEVICE: LIFECYCLE AND LOCK-FREE ACCESS
 * =============================================================================*/

int x430_device_init(struct x430_device* device, const struct x430_device_config* config);

int x430_device_get(const struct x430_device* device, uint16_t address, int32_t* value);

int x430_device_set(struct x430_device* device, uint16_t address, int32_t value);

int x430_device_id(const struct x430_device* device, uint8_t* id);

int x430_device_raise_hardware_error(struct x430_device* device, uint8_t bits);

int x430_device_flush_events(struct x430_device* device);

/* ==============================================================================
 * DEVICE: INSTRUCTION SEMANTICS
 * =============================================================================*/

int x430_device_read(struct x430_device* device, uint16_t address, uint16_t length, uint8_t* data,
                     uint8_t* error);

int x430_device_write(struct x430_device* device, uint16_t address, uint16_t length,
                      const uint8_t* data, uint8_t options, uint8_t* error);

int x430_device_reg_write(struct x430_device* device, uint16_t address, uint16_t length,
                          const uint8_t* data, uint8_t options, uint8_t* error);

int x430_device_action(struct x430_device* device, uint8_t* error);

int x430_device_reboot(struct x430_device* device);

int x430_device_factory_reset(struct x430_device* device, uint8_t option, uint8_t* error);

int x430_device_clear(struct x430_device* device, uint8_t option, uint8_t* error);

int x430_device_backup(struct x430_device* device, uint8_t option, uint8_t* error);

/* ==============================================================================
 * SIMULATOR
 * =============================================================================*/

int x430_sim_init(struct x430_sim* sim, struct x430_device* device,
                  const struct x430_sim_config* config);

int x430_sim_step(struct x430_sim* sim, uint32_t elapsed_ms);

#ifdef __cplusplus
}
#endif

#endif /* X430_H */
```


