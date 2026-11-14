/*******************************************************************************
 * DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
 *
 * Project:     DYNAMIXEL-X430-Control-Table-Emulator
 * Description: Compile check: the public API alone, as an application sees it.
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

#include "x430.h"

/* The public API must not expose the library internals */
#if defined(X430_INTERNAL_H)
#error "x430.h includes a private header"
#endif

static int uart_write(void* context, const uint8_t* data, size_t length)
{
    (void)context;
    (void)data;
    (void)length;
    return 0;
}

static uint8_t on_write(struct x430_device* device, uint16_t address, uint8_t size, int32_t value,
                        void* user_data)
{
    (void)device;
    (void)address;
    (void)size;
    (void)value;
    (void)user_data;
    return DXL2_ERR_NONE;
}

int x430_api_check(void);
int x430_api_check(void)
{
    static struct x430_bus bus;
    static struct x430_device joint;
    static const struct x430_hooks hooks = {.write = on_write};
    const struct x430_bus_config bus_config = {.write = uart_write};
    const struct x430_device_config joint_config = {.id = 1U, .hooks = &hooks};
    const uint8_t ping[] = {0xFFU, 0xFFU, 0xFDU, 0x00U, 0x01U, 0x03U, 0x00U, 0x01U, 0x19U, 0x4EU};
    int32_t position = 0;
    int r = x430_bus_init(&bus, &bus_config);

    r |= x430_device_init(&joint, &joint_config);
    r |= x430_bus_attach(&bus, &joint);
    r |= x430_bus_receive(&bus, ping, sizeof(ping));
    r |= x430_bus_lock(&bus);
    r |= x430_device_set(&joint, X430_ADDR_PRESENT_POSITION, 2048);
    r |= x430_device_get(&joint, X430_ADDR_GOAL_POSITION, &position);
    r |= x430_bus_unlock(&bus);
    r |= x430_bus_poll(&bus);
    return r;
}
