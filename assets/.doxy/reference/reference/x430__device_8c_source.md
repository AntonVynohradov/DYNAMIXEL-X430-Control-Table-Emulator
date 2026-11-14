

# File x430\_device.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**x430\_device.c**](x430__device_8c.md)

[Go to the documentation of this file](x430__device_8c.md)


```C++
/*******************************************************************************
 * DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
 *
 * Project:     DYNAMIXEL-X430-Control-Table-Emulator
 * Description: One emulated actuator: Control Table image, the rules of the real
 *              device and the instruction semantics.
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
 * DEFINES & MACROS
 * =============================================================================*/

#define X430_NVM_FORMAT 1U
#define X430_TOUCHED_BYTES 32U
#define X430_PULSES_PER_TURN 4096
#define X430_ID_MAX 252U

/* ==============================================================================
 * PRIVATE TYPE DEFINITIONS
 * =============================================================================*/

/* Alignment probes: sizeof(probe) - sizeof(x) is the alignment of the type of x
   (no _Alignof, Rule 1.4; no offsetof, which the static analysis cannot resolve) */
struct x430_align_device_ctx
{
    char c;
    struct x430_device_ctx x;
};

struct x430_align_device
{
    char c;
    struct x430_device x;
};

/* The opaque public instance must hold the private state */
_Static_assert(sizeof(struct x430_device_ctx) <= sizeof(struct x430_device),
               "X430_DEVICE_BYTES in x430_types.h is too small for this ABI");
_Static_assert((sizeof(struct x430_align_device_ctx) - sizeof(struct x430_device_ctx)) <=
                   (sizeof(struct x430_align_device) - sizeof(struct x430_device)),
               "struct x430_device is not aligned for struct x430_device_ctx");

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS: RAW ACCESS
 * =============================================================================*/

static uint32_t x430_raw_get(const uint8_t* table, uint16_t address, uint8_t size)
{
    uint32_t value = 0U;
    uint8_t index = size;

    while (index > 0U)
    {
        index--;
        value = (value << 8) | (uint32_t)table[address + index];
    }
    return value;
}

static void x430_raw_put(uint8_t* table, uint16_t address, uint8_t size, uint32_t value)
{
    uint32_t remaining = value;

    for (uint8_t index = 0U; index < size; index++)
    {
        table[address + index] = (uint8_t)(remaining & 0xFFU);
        remaining >>= 8;
    }
}

static int32_t x430_to_value(const struct x430_reg* reg, uint32_t raw)
{
    uint32_t extended = raw;

    if (((reg->flags & X430_REG_SIGNED) != 0U) && (reg->size < 4U))
    {
        const uint32_t bits = (uint32_t)reg->size * 8U;
        const uint32_t sign = (uint32_t)1U << (bits - 1U);

        if ((raw & sign) != 0U)
        {
            extended = raw | ~((sign << 1) - 1U);
        }
    }
    return (int32_t)extended;
}

static const struct x430_reg* x430_reg_at(uint16_t address)
{
    const struct x430_model* model = &x430_model_active;
    const struct x430_reg* reg = NULL;

    if (address < model->table_size)
    {
        const uint8_t index = model->register_map[address];

        if (index != 0U)
        {
            reg = &model->registers[index - 1U];
        }
    }
    return reg;
}

static const struct x430_reg* x430_reg_start(uint16_t address)
{
    const struct x430_reg* reg = x430_reg_at(address);
    const struct x430_reg* start = NULL;

    if ((reg != NULL) && (reg->address == address))
    {
        start = reg;
    }
    return start;
}

static int32_t x430_get(const struct x430_device_ctx* m, uint16_t address)
{
    const struct x430_reg* reg = x430_reg_start(address);
    int32_t value = 0;

    if (reg != NULL)
    {
        value = x430_to_value(reg, x430_raw_get(m->table, reg->address, reg->size));
    }
    return value;
}

static void x430_put(struct x430_device_ctx* m, uint16_t address, int32_t value)
{
    const struct x430_reg* reg = x430_reg_start(address);

    if (reg != NULL)
    {
        x430_raw_put(m->table, reg->address, reg->size, (uint32_t)value);
    }
}

static uint16_t x430_resolve(const struct x430_device_ctx* m, uint16_t address)
{
    const struct x430_model* model = &x430_model_active;
    uint16_t resolved = address;

    for (uint8_t block = 0U; block < model->indirect_count; block++)
    {
        const struct x430_indirect_block* entry = &model->indirect[block];
        const uint32_t end = (uint32_t)entry->data + entry->count;

        if ((address >= entry->data) && ((uint32_t)address < end))
        {
            const uint32_t offset = 2U * ((uint32_t)address - entry->data);
            const uint16_t slot = (uint16_t)((uint32_t)entry->address + offset);
            const uint16_t target = (uint16_t)x430_raw_get(m->table, slot, 2U);

            /* Writes are range checked (X430_CHECK_INDIRECT), x430_device_set() is not. */
            if (target < X430_TABLE_SIZE)
            {
                resolved = target;
            }
        }
    }
    return resolved;
}

static bool x430_touches_indirect(uint16_t address, uint16_t length)
{
    const struct x430_model* model = &x430_model_active;
    bool touches = false;
    const uint32_t end = (uint32_t)address + length;

    for (uint8_t block = 0U; block < model->indirect_count; block++)
    {
        const struct x430_indirect_block* entry = &model->indirect[block];
        const uint32_t block_end = (uint32_t)entry->data + entry->count;

        if (((uint32_t)address < block_end) && (end > entry->data))
        {
            touches = true;
        }
    }
    return touches;
}

static uint8_t x430_event(struct x430_device* device, enum x430_event event)
{
    const struct x430_device_ctx* m = x430_device_ctx_of_const(device);
    const struct x430_hooks* hooks = m->config.hooks;
    uint8_t result = DXL2_ERR_NONE;

    if ((hooks != NULL) && (hooks->event != NULL))
    {
        result = hooks->event(device, event, m->config.user_data);
    }
    return result;
}

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS: PERSISTENCE
 * =============================================================================*/

static uint16_t x430_nvm_pack(struct x430_device_ctx* m, enum x430_nvm_slot slot, uint16_t size)
{
    const uint16_t total = (uint16_t)(X430_NVM_HEADER_SIZE + size + 2U);
    const uint16_t crc_at = (uint16_t)(X430_NVM_HEADER_SIZE + size);

    m->nvm[0] = (uint8_t)'X';
    m->nvm[1] = (uint8_t)'4';
    m->nvm[2] = (uint8_t)'3';
    m->nvm[3] = (uint8_t)'0';
    dxl2_put_u16(&m->nvm[4], x430_model_active.model_number);
    m->nvm[6] = (uint8_t)X430_NVM_FORMAT;
    m->nvm[7] = (uint8_t)slot;
    dxl2_put_u16(&m->nvm[8], size);
    x430_copy(&m->nvm[X430_NVM_HEADER_SIZE], m->table, size);
    dxl2_put_u16(&m->nvm[crc_at], dxl2_crc16(m->nvm, (size_t)crc_at));
    return total;
}

static bool x430_nvm_load(struct x430_device* device, enum x430_nvm_slot slot, uint16_t size)
{
    struct x430_device_ctx* m = x430_device_ctx_of(device);
    const struct x430_hooks* hooks = m->config.hooks;
    const uint16_t total = (uint16_t)(X430_NVM_HEADER_SIZE + size + 2U);
    const uint16_t crc_at = (uint16_t)(X430_NVM_HEADER_SIZE + size);
    bool valid = false;

    if ((hooks != NULL) && (hooks->nvm_load != NULL))
    {
        if (hooks->nvm_load(device, slot, m->nvm, total, m->config.user_data) == 0)
        {
            valid = (m->nvm[0] == (uint8_t)'X') && (m->nvm[1] == (uint8_t)'4') &&
                    (m->nvm[2] == (uint8_t)'3') && (m->nvm[3] == (uint8_t)'0') &&
                    (dxl2_get_u16(&m->nvm[4]) == x430_model_active.model_number) &&
                    (m->nvm[6] == (uint8_t)X430_NVM_FORMAT) && (m->nvm[7] == (uint8_t)slot) &&
                    (dxl2_get_u16(&m->nvm[8]) == size) &&
                    (dxl2_get_u16(&m->nvm[crc_at]) == dxl2_crc16(m->nvm, (size_t)crc_at));
        }
    }
    return valid;
}

static uint8_t x430_nvm_store(struct x430_device* device, enum x430_nvm_slot slot, uint16_t size)
{
    struct x430_device_ctx* m = x430_device_ctx_of(device);
    const struct x430_hooks* hooks = m->config.hooks;
    uint8_t result = DXL2_ERR_NONE;

    if ((hooks != NULL) && (hooks->nvm_store != NULL))
    {
        const uint16_t total = x430_nvm_pack(m, slot, size);

        if (hooks->nvm_store(device, slot, m->nvm, total, m->config.user_data) != 0)
        {
            result = DXL2_ERR_RESULT_FAIL;
        }
    }
    return result;
}

static void x430_apply_eeprom(struct x430_device_ctx* m)
{
    const struct x430_model* model = &x430_model_active;

    for (uint16_t index = 0U; index < model->register_count; index++)
    {
        const struct x430_reg* reg = &model->registers[index];

        if (((reg->flags & X430_REG_WRITABLE) != 0U) && ((reg->flags & X430_REG_EEPROM) != 0U))
        {
            x430_copy(&m->table[reg->address], &m->nvm[X430_NVM_HEADER_SIZE + reg->address],
                      reg->size);
        }
    }
}

#if X430_FEATURE_BACKUP
static void x430_apply_backup(struct x430_device_ctx* m, bool ram_only)
{
    const struct x430_model* model = &x430_model_active;

    for (uint16_t index = 0U; index < model->register_count; index++)
    {
        const struct x430_reg* reg = &model->registers[index];
        const bool identity = (reg->address == X430_ADDR_ID) ||
                              (reg->address == X430_ADDR_BAUD_RATE) ||
                              (reg->address == X430_ADDR_PROTOCOL_TYPE);
        const bool runtime =
            (reg->address == X430_ADDR_TORQUE_ENABLE) || (reg->address == X430_ADDR_BUS_WATCHDOG);
        const bool eeprom = (reg->flags & X430_REG_EEPROM) != 0U;

        if (((reg->flags & X430_REG_WRITABLE) != 0U) && !runtime && !identity &&
            (!ram_only || !eeprom))
        {
            x430_copy(&m->table[reg->address], &m->nvm[X430_NVM_HEADER_SIZE + reg->address],
                      reg->size);
        }
    }
}
#endif

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS: CHECKS
 * =============================================================================*/

static bool x430_torque_on(const struct x430_device_ctx* m)
{
    return m->table[X430_ADDR_TORQUE_ENABLE] != 0U;
}

static int32_t x430_abs(int32_t value)
{
    int32_t magnitude = value;

    if (value < 0)
    {
        magnitude = -value;
    }
    return magnitude;
}

static bool x430_check_value(const struct x430_device_ctx* m, const struct x430_reg* reg,
                             int32_t value)
{
    const struct x430_model* model = &x430_model_active;
    bool valid = false;

    /* tools/x430_gen_model.py gives every writable register a check other than NONE */
    switch (reg->check)
    {
        case (uint8_t)X430_CHECK_MASK:
            valid = (((uint32_t)value) & ~((uint32_t)reg->max)) == 0U;
            break;
        case (uint8_t)X430_CHECK_OPMODE:
            /* Operating Mode is one unsigned byte: value is 0..255 */
            if ((uint32_t)value < 32U)
            {
                valid = (model->operating_modes & ((uint32_t)1U << (uint32_t)value)) != 0U;
            }
            break;
        case (uint8_t)X430_CHECK_SYMMETRIC:
            valid = x430_abs(value) <= x430_get(m, reg->ref);
            break;
        case (uint8_t)X430_CHECK_GOAL_POSITION:
            if (m->table[X430_ADDR_OPERATING_MODE] == X430_MODE_POSITION)
            {
                valid = (value >= x430_get(m, X430_ADDR_MIN_POSITION_LIMIT)) &&
                        (value <= x430_get(m, X430_ADDR_MAX_POSITION_LIMIT));
            }
            else
            {
                valid = x430_abs(value) <= X430_EXTENDED_POSITION_LIMIT;
            }
            break;
        default:
            /* X430_CHECK_RANGE and X430_CHECK_INDIRECT */
            valid = (value >= reg->min) && (value <= reg->max);
            break;
    }
    return valid;
}

static uint8_t x430_check_write(const struct x430_device* device, const struct x430_reg* reg,
                                int32_t value, uint8_t options)
{
    const struct x430_device_ctx* m = x430_device_ctx_of_const(device);
    uint8_t result = DXL2_ERR_NONE;

    if ((reg->flags & X430_REG_WRITABLE) == 0U)
    {
        result = DXL2_ERR_ACCESS;
    }
    else if (((reg->flags & X430_REG_EEPROM) != 0U) &&
             (x430_torque_on(m) || ((options & X430_WRITE_SECONDARY) != 0U)))
    {
        result = DXL2_ERR_ACCESS;
    }
    else if (((reg->flags & X430_REG_GOAL) != 0U) &&
             (m->table[X430_ADDR_BUS_WATCHDOG] == X430_BUS_WATCHDOG_ERROR))
    {
        result = DXL2_ERR_DATA_RANGE;
    }
    else if (reg->address == X430_ADDR_BAUD_RATE)
    {
        /* The model table says 0..7; applications may allow their own codes. Baud Rate
           is one unsigned byte: value is 0..255. */
        if ((uint32_t)value > X430_BAUD_RATE_CODE_MAX_U)
        {
            result = DXL2_ERR_DATA_RANGE;
        }
    }
    else if (!x430_check_value(m, reg, value))
    {
        result = DXL2_ERR_DATA_RANGE;
    }
    else if ((reg->address == X430_ADDR_ID) && x430_bus_id_taken(m->bus, device, (uint8_t)value))
    {
        result = DXL2_ERR_DATA_RANGE;
    }
    else if ((reg->address == X430_ADDR_TORQUE_ENABLE) && (value != 0) &&
             ((m->table[X430_ADDR_HARDWARE_ERROR_STATUS] & m->table[X430_ADDR_SHUTDOWN]) != 0U))
    {
        /* After a shutdown only REBOOT re-enables the torque. */
        result = DXL2_ERR_RESULT_FAIL;
    }
    else
    {
        result = DXL2_ERR_NONE;
    }
    return result;
}

static uint32_t x430_merge(const struct x430_device_ctx* m, const struct x430_reg* reg,
                           uint16_t address, uint16_t length, const uint8_t* data, bool indirect)
{
    uint8_t bytes[4] = {0U, 0U, 0U, 0U};
    const uint32_t end = (uint32_t)address + length;
    const uint32_t reg_end = (uint32_t)reg->address + reg->size;

    x430_copy(bytes, &m->table[reg->address], reg->size);
    if (!indirect)
    {
        for (uint8_t offset = 0U; offset < reg->size; offset++)
        {
            const uint32_t target = (uint32_t)reg->address + offset;

            if ((target >= address) && (target < end))
            {
                bytes[offset] = data[target - address];
            }
        }
    }
    else
    {
        for (uint16_t index = 0U; index < length; index++)
        {
            const uint16_t target = x430_resolve(m, (uint16_t)((uint32_t)address + index));

            if ((target >= reg->address) && ((uint32_t)target < reg_end))
            {
                bytes[target - reg->address] = data[index];
            }
        }
    }
    return x430_raw_get(bytes, 0U, reg->size);
}

static void x430_mark_touched(const struct x430_device_ctx* m, uint16_t address, uint16_t length,
                              uint8_t* touched, bool indirect)
{
    for (uint16_t index = 0U; index < length; index++)
    {
        const uint16_t raw = (uint16_t)((uint32_t)address + index);
        uint16_t target = raw;
        uint8_t entry = 0U;

        if (indirect)
        {
            target = x430_resolve(m, raw);
        }
        entry = x430_model_active.register_map[target];
        if (entry != 0U)
        {
            const uint8_t bit = (uint8_t)(entry - 1U);
            const uint8_t byte = bit / 8U;
            const uint8_t shift = bit % 8U;
            const uint8_t mask = (uint8_t)((uint32_t)1U << shift);

            touched[byte] |= mask;
        }
    }
}

static bool x430_is_touched(const uint8_t* touched, uint16_t index)
{
    const uint16_t byte = index / 8U;
    const uint16_t shift = index % 8U;
    const uint8_t mask = (uint8_t)((uint32_t)1U << shift);

    return (touched[byte] & mask) != 0U;
}

static void x430_store_raw_bytes(struct x430_device_ctx* m, uint16_t address, uint16_t length,
                                 const uint8_t* data)
{
    for (uint16_t index = 0U; index < length; index++)
    {
        const uint16_t target = x430_resolve(m, (uint16_t)((uint32_t)address + index));

        if ((x430_model_active.register_map[target] == 0U) && x430_touches_indirect(target, 1U))
        {
            m->table[target] = data[index];
        }
    }
}

static void x430_sync_goal(struct x430_device* device)
{
    struct x430_device_ctx* m = x430_device_ctx_of(device);
    const struct x430_hooks* hooks = m->config.hooks;

    if ((hooks != NULL) && (hooks->read != NULL))
    {
        hooks->read(device, X430_ADDR_PRESENT_POSITION, 4U, m->config.user_data);
    }
    x430_copy(&m->table[X430_ADDR_GOAL_POSITION], &m->table[X430_ADDR_PRESENT_POSITION], 4U);
}

static uint8_t x430_enable_torque(struct x430_device* device, bool sync_goal)
{
    struct x430_device_ctx* m = x430_device_ctx_of(device);
    const struct x430_hooks* hooks = m->config.hooks;
    uint8_t result = DXL2_ERR_NONE;

    if ((hooks != NULL) && (hooks->write != NULL))
    {
        result = hooks->write(device, X430_ADDR_TORQUE_ENABLE, 1U, 1, m->config.user_data);
    }
    if (result == DXL2_ERR_NONE)
    {
        m->table[X430_ADDR_TORQUE_ENABLE] = 1U;
        if (sync_goal)
        {
            x430_sync_goal(device);
        }
    }
    return result;
}

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS: STARTUP
 * =============================================================================*/

static void x430_load_factory(struct x430_device_ctx* m, uint16_t first, uint16_t last)
{
    const struct x430_model* model = &x430_model_active;

    for (uint16_t index = 0U; index < model->register_count; index++)
    {
        const struct x430_reg* reg = &model->registers[index];

        if ((reg->address >= first) && (reg->address <= last))
        {
            x430_raw_put(m->table, reg->address, reg->size, (uint32_t)reg->initial);
        }
    }
}

static void x430_load_identity(struct x430_device_ctx* m)
{
    uint8_t firmware = (uint8_t)X430_FIRMWARE_VERSION;

    if (m->config.firmware_version != 0U)
    {
        firmware = m->config.firmware_version;
    }
    x430_put(m, X430_ADDR_MODEL_NUMBER, (int32_t)X430_MODEL_NUMBER);
    m->table[X430_ADDR_FIRMWARE_VERSION] = firmware;
    x430_raw_put(m->table, X430_ADDR_MODEL_INFORMATION, 4U, m->config.model_information);
}

static void x430_startup(struct x430_device* device)
{
    struct x430_device_ctx* m = x430_device_ctx_of(device);
    const uint8_t startup = m->table[X430_ADDR_STARTUP_CONFIGURATION];
    const uint16_t last = (uint16_t)(X430_TABLE_SIZE - 1U);

    x430_fill(&m->table[X430_RAM_START], 0U, (uint16_t)(X430_TABLE_SIZE - X430_RAM_START));
    x430_load_factory(m, X430_RAM_START, last);
    m->reg_length = 0U;
    m->baud_changed = false;

#if X430_FEATURE_BACKUP
    if (x430_nvm_load(device, X430_NVM_BACKUP, X430_TABLE_SIZE))
    {
        m->table[X430_ADDR_BACKUP_READY] = 1U;
        if ((startup & X430_STARTUP_RAM_RESTORE) != 0U)
        {
            x430_apply_backup(m, true);
        }
    }
#endif

    if ((startup & X430_STARTUP_TORQUE_ON) != 0U)
    {
        (void)x430_enable_torque(device, true);
    }
}

static int x430_check(const struct x430_device* device)
{
    const struct x430_device_ctx* m = x430_device_ctx_of_const(device);
    int result = X430_OK;

    if (m == NULL)
    {
        result = X430_ERR_PARAM;
    }
    else if (m->ready != X430_READY)
    {
        result = X430_ERR_STATE;
    }
    else
    {
        /* Ready. */
    }
    return result;
}

static int x430_check2(const struct x430_device* device, const void* a, const void* b)
{
    int result = X430_ERR_PARAM;

    if ((a != NULL) && (b != NULL))
    {
        result = x430_check(device);
    }
    return result;
}

/* ==============================================================================
 * INTERNAL FUNCTIONS (x430_internal.h)
 * =============================================================================*/

void x430_copy(uint8_t* destination, const uint8_t* source, uint16_t length)
{
    for (uint16_t index = 0U; index < length; index++)
    {
        destination[index] = source[index];
    }
}

void x430_fill(uint8_t* destination, uint8_t value, uint16_t length)
{
    for (uint16_t index = 0U; index < length; index++)
    {
        destination[index] = value;
    }
}

int32_t x430_device_value(const struct x430_device* device, uint16_t address)
{
    return x430_get(x430_device_ctx_of_const(device), address);
}

void x430_device_store(struct x430_device* device, uint16_t address, int32_t value)
{
    x430_put(x430_device_ctx_of(device), address, value);
}

uint8_t x430_device_byte(const struct x430_device* device, uint16_t address)
{
    return x430_device_ctx_of_const(device)->table[address];
}

uint8_t x430_device_alert(const struct x430_device* device)
{
    uint8_t alert = DXL2_ERR_NONE;

    if (x430_device_byte(device, X430_ADDR_HARDWARE_ERROR_STATUS) != 0U)
    {
        alert = DXL2_ERR_ALERT_BIT;
    }
    return alert;
}

void x430_device_watchdog(struct x430_device* device, uint32_t idle_ms)
{
    struct x430_device_ctx* m = x430_device_ctx_of(device);
    const uint8_t timeout = m->table[X430_ADDR_BUS_WATCHDOG];

    if ((timeout != 0U) && (timeout != X430_BUS_WATCHDOG_ERROR) && x430_torque_on(m) &&
        (idle_ms > ((uint32_t)timeout * X430_BUS_WATCHDOG_UNIT_MS)))
    {
        m->table[X430_ADDR_BUS_WATCHDOG] = X430_BUS_WATCHDOG_ERROR;
        (void)x430_event(device, X430_EVENT_BUS_WATCHDOG);
    }
}

struct x430_bus* x430_device_bus(const struct x430_device* device)
{
    return x430_device_ctx_of_const(device)->bus;
}

void x430_device_attach(struct x430_device* device, struct x430_bus* bus)
{
    x430_device_ctx_of(device)->bus = bus;
}

bool x430_device_ready(const struct x430_device* device)
{
    return x430_check(device) == X430_OK;
}

/* ==============================================================================
 * PUBLIC FUNCTION IMPLEMENTATIONS: LIFECYCLE AND LOCK-FREE ACCESS
 * =============================================================================*/

int x430_device_init(struct x430_device* device, const struct x430_device_config* config)
{
    struct x430_device_ctx* m = x430_device_ctx_of(device);
    int result = X430_OK;

    if ((m == NULL) || (config == NULL) || (config->id > X430_ID_MAX))
    {
        result = X430_ERR_PARAM;
    }
    else
    {
        /* Field by field (no temporary copy of the context, no memset() of the C library);
           reg_data and nvm are written before they are read. */
        m->config = *config;
        m->bus = NULL;
        m->reg_address = 0U;
        m->reg_length = 0U;
        m->baud_changed = false;
        x430_fill(m->table, 0U, X430_TABLE_SIZE);
        /* Nothing below can fail; hooks called from here may use the device. */
        m->ready = X430_READY;

        x430_load_factory(m, 0U, X430_RAM_START - 1U);
        x430_load_identity(m);
        if (config->id != 0U)
        {
            m->table[X430_ADDR_ID] = config->id;
        }
        if (x430_nvm_load(device, X430_NVM_EEPROM, X430_RAM_START))
        {
            x430_apply_eeprom(m);
        }
        x430_startup(device);
    }
    return result;
}

int x430_device_get(const struct x430_device* device, uint16_t address, int32_t* value)
{
    int result = x430_check2(device, value, value);

    if (result == X430_OK)
    {
        const struct x430_reg* reg = x430_reg_start(address);

        if (reg == NULL)
        {
            result = X430_ERR_PARAM;
        }
        else
        {
            *value = x430_get(x430_device_ctx_of_const(device), address);
        }
    }
    return result;
}

int x430_device_set(struct x430_device* device, uint16_t address, int32_t value)
{
    int result = x430_check(device);

    if (result == X430_OK)
    {
        if (x430_reg_start(address) == NULL)
        {
            result = X430_ERR_PARAM;
        }
        else
        {
            x430_put(x430_device_ctx_of(device), address, value);
        }
    }
    return result;
}

int x430_device_id(const struct x430_device* device, uint8_t* id)
{
    int result = x430_check2(device, id, id);

    if (result == X430_OK)
    {
        *id = x430_device_byte(device, X430_ADDR_ID);
    }
    return result;
}

int x430_device_raise_hardware_error(struct x430_device* device, uint8_t bits)
{
    int result = x430_check(device);

    if (result == X430_OK)
    {
        struct x430_device_ctx* m = x430_device_ctx_of(device);

        m->table[X430_ADDR_HARDWARE_ERROR_STATUS] |= bits;
        if (((bits & m->table[X430_ADDR_SHUTDOWN]) != 0U) && x430_torque_on(m))
        {
            m->table[X430_ADDR_TORQUE_ENABLE] = 0U;
            (void)x430_event(device, X430_EVENT_TORQUE_SHUTDOWN);
        }
    }
    return result;
}

int x430_device_flush_events(struct x430_device* device)
{
    int result = x430_check(device);

    if (result == X430_OK)
    {
        struct x430_device_ctx* m = x430_device_ctx_of(device);

        if (m->baud_changed)
        {
            m->baud_changed = false;
            (void)x430_event(device, X430_EVENT_BAUD_RATE);
        }
    }
    return result;
}

/* ==============================================================================
 * PUBLIC FUNCTION IMPLEMENTATIONS: INSTRUCTION SEMANTICS
 * =============================================================================*/

int x430_device_read(struct x430_device* device, uint16_t address, uint16_t length, uint8_t* data,
                     uint8_t* error)
{
    int result = x430_check2(device, data, error);

    if (result == X430_OK)
    {
        struct x430_device_ctx* m = x430_device_ctx_of(device);
        const struct x430_hooks* hooks = m->config.hooks;

        *error = DXL2_ERR_NONE;
        if (length == 0U)
        {
            *error = DXL2_ERR_DATA_LENGTH;
        }
        else if (((uint32_t)address + length) > X430_TABLE_SIZE)
        {
            *error = DXL2_ERR_ACCESS;
        }
        else
        {
            const uint32_t tick = x430_bus_millis(m->bus) % X430_REALTIME_TICK_PERIOD;

            x430_raw_put(m->table, X430_ADDR_REALTIME_TICK, 2U, tick);

            if ((hooks != NULL) && (hooks->read != NULL))
            {
                uint16_t run_start = x430_resolve(m, address);
                uint16_t run_length = 1U;

                for (uint16_t index = 1U; index < length; index++)
                {
                    const uint16_t target = x430_resolve(m, (uint16_t)((uint32_t)address + index));

                    if ((uint32_t)target == ((uint32_t)run_start + run_length))
                    {
                        run_length++;
                    }
                    else
                    {
                        hooks->read(device, run_start, run_length, m->config.user_data);
                        run_start = target;
                        run_length = 1U;
                    }
                }
                hooks->read(device, run_start, run_length, m->config.user_data);
            }

            for (uint16_t index = 0U; index < length; index++)
            {
                data[index] = m->table[x430_resolve(m, (uint16_t)((uint32_t)address + index))];
            }
        }
    }
    return result;
}

static uint8_t x430_write(struct x430_device* device, uint16_t address, uint16_t length,
                          const uint8_t* data, uint8_t options)
{
    struct x430_device_ctx* m = x430_device_ctx_of(device);
    const struct x430_model* model = &x430_model_active;
    const struct x430_hooks* hooks = m->config.hooks;
    uint8_t touched[X430_TOUCHED_BYTES];
    uint8_t result = DXL2_ERR_NONE;
    bool indirect = false;
    bool goal_written = false;
    bool eeprom_written = false;

    x430_fill(touched, 0U, X430_TOUCHED_BYTES);

    if (length == 0U)
    {
        result = DXL2_ERR_DATA_LENGTH;
    }
    else if (((uint32_t)address + length) > X430_TABLE_SIZE)
    {
        result = DXL2_ERR_ACCESS;
    }
    else
    {
        indirect = x430_touches_indirect(address, length);
        x430_mark_touched(m, address, length, touched, indirect);

        /* Pass 1: every touched register must accept its new value. */
        for (uint16_t index = 0U; index < model->register_count; index++)
        {
            if ((result == DXL2_ERR_NONE) && x430_is_touched(touched, index))
            {
                const struct x430_reg* reg = &model->registers[index];
                const int32_t value =
                    x430_to_value(reg, x430_merge(m, reg, address, length, data, indirect));

                result = x430_check_write(device, reg, value, options);
            }
        }
    }

    if ((result == DXL2_ERR_NONE) && ((options & X430_WRITE_VALIDATE_ONLY) == 0U))
    {
        /* Pass 2: hooks and storage, in address order. */
        for (uint16_t index = 0U; index < model->register_count; index++)
        {
            if ((result == DXL2_ERR_NONE) && x430_is_touched(touched, index))
            {
                const struct x430_reg* reg = &model->registers[index];
                const uint32_t raw = x430_merge(m, reg, address, length, data, indirect);
                const uint32_t previous = x430_raw_get(m->table, reg->address, reg->size);

                if ((hooks != NULL) && (hooks->write != NULL))
                {
                    result = hooks->write(device, reg->address, reg->size, x430_to_value(reg, raw),
                                          m->config.user_data);
                }
                if (result == DXL2_ERR_NONE)
                {
                    x430_raw_put(m->table, reg->address, reg->size, raw);
                    if ((reg->flags & X430_REG_EEPROM) != 0U)
                    {
                        eeprom_written = true;
                    }
                    if ((reg->flags & X430_REG_GOAL) != 0U)
                    {
                        goal_written = true;
                    }
                    if ((reg->address == X430_ADDR_BAUD_RATE) && (raw != previous))
                    {
                        m->baud_changed = true;
                    }
                    if ((reg->address == X430_ADDR_TORQUE_ENABLE) && (raw != 0U) &&
                        (previous == 0U))
                    {
                        x430_sync_goal(device);
                    }
                }
            }
        }
        if (indirect)
        {
            x430_store_raw_bytes(m, address, length, data);
        }

        if (eeprom_written)
        {
            const uint8_t stored = x430_nvm_store(device, X430_NVM_EEPROM, X430_RAM_START);

            if (result == DXL2_ERR_NONE)
            {
                result = stored;
            }
        }

        /* Drive Mode bit 3: a goal value written with torque off enables the torque. */
        if (goal_written && (result == DXL2_ERR_NONE) && !x430_torque_on(m) &&
            ((m->table[X430_ADDR_DRIVE_MODE] & X430_DRIVE_MODE_TORQUE_ON_BY_GOAL) != 0U))
        {
            result = x430_enable_torque(device, false);
        }
    }
    return result;
}

int x430_device_write(struct x430_device* device, uint16_t address, uint16_t length,
                      const uint8_t* data, uint8_t options, uint8_t* error)
{
    int result = x430_check2(device, data, error);

    if (result == X430_OK)
    {
        *error = x430_write(device, address, length, data, options);
    }
    return result;
}

int x430_device_reg_write(struct x430_device* device, uint16_t address, uint16_t length,
                          const uint8_t* data, uint8_t options, uint8_t* error)
{
    int result = x430_check2(device, data, error);

    if (result == X430_OK)
    {
        struct x430_device_ctx* m = x430_device_ctx_of(device);

        if (length > X430_REG_WRITE_SIZE_U)
        {
            *error = DXL2_ERR_DATA_LENGTH;
        }
        else
        {
            *error = x430_write(device, address, length, data,
                                (uint8_t)(options | X430_WRITE_VALIDATE_ONLY));
        }
        if (*error == DXL2_ERR_NONE)
        {
            x430_copy(m->reg_data, data, length);
            m->reg_address = address;
            m->reg_length = length;
            m->table[X430_ADDR_REGISTERED_INSTRUCTION] = 1U;
        }
    }
    return result;
}

int x430_device_action(struct x430_device* device, uint8_t* error)
{
    int result = x430_check2(device, error, error);

    if (result == X430_OK)
    {
        struct x430_device_ctx* m = x430_device_ctx_of(device);

        *error = DXL2_ERR_INSTRUCTION;
        if (m->reg_length != 0U)
        {
            const uint16_t length = m->reg_length;

            m->reg_length = 0U;
            m->table[X430_ADDR_REGISTERED_INSTRUCTION] = 0U;
            *error = x430_write(device, m->reg_address, length, m->reg_data, 0U);
        }
    }
    return result;
}

int x430_device_reboot(struct x430_device* device)
{
    int result = x430_check(device);

    if (result == X430_OK)
    {
        x430_startup(device);
        (void)x430_event(device, X430_EVENT_REBOOT);
    }
    return result;
}

int x430_device_factory_reset(struct x430_device* device, uint8_t option, uint8_t* error)
{
    int result = x430_check2(device, error, error);

    if (result == X430_OK)
    {
        struct x430_device_ctx* m = x430_device_ctx_of(device);

        *error = DXL2_ERR_DATA_RANGE;
        if ((option == X430_FACTORY_RESET_ALL) || (option == X430_FACTORY_RESET_EXCEPT_ID) ||
            (option == X430_FACTORY_RESET_EXCEPT_ID_BAUD))
        {
            const uint8_t id = m->table[X430_ADDR_ID];
            const uint8_t baud = m->table[X430_ADDR_BAUD_RATE];

            x430_load_factory(m, 0U, X430_RAM_START - 1U);
            x430_load_identity(m);
            if (option != X430_FACTORY_RESET_ALL)
            {
                m->table[X430_ADDR_ID] = id;
            }
            else if ((m->config.id != 0U) && !x430_bus_id_taken(m->bus, device, m->config.id))
            {
                m->table[X430_ADDR_ID] = m->config.id;
            }
            else
            {
                /* Factory ID from the model table (1). */
            }
            if (option == X430_FACTORY_RESET_EXCEPT_ID_BAUD)
            {
                m->table[X430_ADDR_BAUD_RATE] = baud;
            }
            *error = x430_nvm_store(device, X430_NVM_EEPROM, X430_RAM_START);
            x430_startup(device);
            m->baud_changed = m->table[X430_ADDR_BAUD_RATE] != baud;
            (void)x430_event(device, X430_EVENT_FACTORY_RESET);
        }
    }
    return result;
}

int x430_device_clear(struct x430_device* device, uint8_t option, uint8_t* error)
{
    int result = x430_check2(device, error, error);

    if (result == X430_OK)
    {
        struct x430_device_ctx* m = x430_device_ctx_of(device);
        const struct x430_hooks* hooks = m->config.hooks;

        if (option != X430_CLEAR_MULTI_TURN)
        {
            *error = DXL2_ERR_DATA_RANGE;
        }
        else if (m->table[X430_ADDR_MOVING] != 0U)
        {
            *error = DXL2_ERR_RESULT_FAIL;
        }
        else if ((hooks != NULL) && (hooks->event != NULL))
        {
            *error = hooks->event(device, X430_EVENT_CLEAR_MULTI_TURN, m->config.user_data);
        }
        else
        {
            int32_t position = x430_get(m, X430_ADDR_PRESENT_POSITION) % X430_PULSES_PER_TURN;

            if (position < 0)
            {
                position += X430_PULSES_PER_TURN;
            }
            x430_put(m, X430_ADDR_PRESENT_POSITION, position);
            *error = DXL2_ERR_NONE;
        }
    }
    return result;
}

int x430_device_backup(struct x430_device* device, uint8_t option, uint8_t* error)
{
    int result = x430_check2(device, error, error);

#if X430_FEATURE_BACKUP
    if (result == X430_OK)
    {
        struct x430_device_ctx* m = x430_device_ctx_of(device);
        const struct x430_hooks* hooks = m->config.hooks;

        *error = DXL2_ERR_DATA_RANGE;
        if (option == X430_BACKUP_STORE)
        {
            *error = x430_nvm_store(device, X430_NVM_BACKUP, X430_TABLE_SIZE);
            if ((*error == DXL2_ERR_NONE) && (hooks != NULL) && (hooks->nvm_store != NULL))
            {
                m->table[X430_ADDR_BACKUP_READY] = 1U;
            }
        }
        else if (option == X430_BACKUP_RESTORE)
        {
            if (x430_torque_on(m))
            {
                *error = DXL2_ERR_ACCESS;
            }
            else if (!x430_nvm_load(device, X430_NVM_BACKUP, X430_TABLE_SIZE))
            {
                *error = DXL2_ERR_RESULT_FAIL;
            }
            else
            {
                x430_apply_backup(m, false);
                *error = x430_nvm_store(device, X430_NVM_EEPROM, X430_RAM_START);
                (void)x430_event(device, X430_EVENT_BACKUP_RESTORED);
            }
        }
        else
        {
            /* Unknown option: Data Range Error. */
        }
    }
#else
    (void)option;
    if (result == X430_OK)
    {
        result = X430_ERR_UNSUPPORTED;
    }
#endif
    return result;
}
```


