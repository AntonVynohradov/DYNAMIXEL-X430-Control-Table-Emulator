/**
 * @file x430_device.c
 * @brief Control Table engine of one emulated DYNAMIXEL X430.
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

#include "x430_device.h"

#include <stddef.h>
#include <string.h>

#include "dxl2.h"
#include "x430_internal.h"

/** @} */

/** @name Private definitions */
/** @{ */

/** Persistent block format version. */
#define X430_NVM_FORMAT 1U
/** Bytes of the register-touched bit set (one bit per register index, max 254). */
#define X430_TOUCHED_BYTES 32U
/** Turn size of Present Position in pulses. */
#define X430_PULSES_PER_TURN 4096

/** Scratch buffer for persistent blocks; used with the bus lock held. */
static uint8_t x430_nvm_buffer[X430_NVM_BACKUP_SIZE];

/** @} */

/** @name Private helpers: raw access */
/** @{ */

static uint32_t x430_raw_get(const uint8_t* table, uint16_t address, uint8_t size)
{
    uint32_t value = 0U;
    uint8_t index = size;

    while (index > 0U)
    {
        index--;
        value = (value << 8) | (uint32_t) table[address + index];
    }
    return value;
}

static void x430_raw_put(uint8_t* table, uint16_t address, uint8_t size, uint32_t value)
{
    uint32_t remaining = value;

    for (uint8_t index = 0U; index < size; index++)
    {
        table[address + index] = (uint8_t) (remaining & 0xFFU);
        remaining >>= 8;
    }
}

/** Sign-extends a raw register value. */
static int32_t x430_to_value(const struct x430_reg* reg, uint32_t raw)
{
    uint32_t extended = raw;

    if (((reg->flags & X430_REG_SIGNED) != 0U) && (reg->size < 4U))
    {
        const uint32_t sign = (uint32_t) 1U << ((reg->size * 8U) - 1U);

        if ((raw & sign) != 0U)
        {
            extended = raw | ~((sign << 1) - 1U);
        }
    }
    return (int32_t) extended;
}

/** Register that contains @p address, NULL for a reserved address. */
static const struct x430_reg* x430_reg_at(const struct x430_model* model, uint16_t address)
{
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

static int32_t x430_get(const struct x430_device* device, uint16_t address)
{
    const struct x430_reg* reg = x430_reg_at(device->model, address);
    int32_t value = 0;

    if ((reg != NULL) && (reg->address == address))
    {
        value = x430_to_value(reg, x430_raw_get(device->table, reg->address, reg->size));
    }
    return value;
}

static void x430_put(struct x430_device* device, uint16_t address, int32_t value)
{
    const struct x430_reg* reg = x430_reg_at(device->model, address);

    if ((reg != NULL) && (reg->address == address))
    {
        x430_raw_put(device->table, reg->address, reg->size, (uint32_t) value);
    }
}

/** Address that @p address really accesses (Indirect Data resolved). */
static uint16_t x430_resolve(const struct x430_device* device, uint16_t address)
{
    const struct x430_model* model = device->model;
    uint16_t resolved = address;

    for (uint8_t block = 0U; block < model->indirect_count; block++)
    {
        const struct x430_indirect_block* entry = &model->indirect[block];

        if ((address >= entry->data) && (address < (entry->data + entry->count)))
        {
            const uint16_t slot = (uint16_t) (entry->address + (2U * (address - entry->data)));
            const uint16_t target = (uint16_t) x430_raw_get(device->table, slot, 2U);

            /* Indirect Address values are validated on write; keep the guard anyway. */
            if (target < model->table_size)
            {
                resolved = target;
            }
        }
    }
    return resolved;
}

/** True when [address, address + length) overlaps an Indirect Data block. */
static bool x430_touches_indirect(const struct x430_model* model, uint16_t address, uint16_t length)
{
    bool touches = false;
    const uint32_t end = (uint32_t) address + length;

    for (uint8_t block = 0U; block < model->indirect_count; block++)
    {
        const struct x430_indirect_block* entry = &model->indirect[block];

        if ((address < (entry->data + entry->count)) && (end > entry->data))
        {
            touches = true;
        }
    }
    return touches;
}

static const struct x430_hooks* x430_hooks_of(const struct x430_device* device)
{
    return device->config.hooks;
}

static uint8_t x430_event(struct x430_device* device, enum x430_event event)
{
    const struct x430_hooks* hooks = x430_hooks_of(device);
    uint8_t result = DXL2_ERR_NONE;

    if ((hooks != NULL) && (hooks->event != NULL))
    {
        result = hooks->event(device, event, device->config.user_data);
    }
    return result;
}

/** @} */

/** @name Private helpers: persistence */
/** @{ */

static uint16_t x430_nvm_pack(enum x430_nvm_slot slot, const uint8_t* payload, uint16_t size)
{
    const uint16_t total = (uint16_t) (X430_NVM_HEADER_SIZE + size + 2U);

    x430_nvm_buffer[0] = (uint8_t) 'X';
    x430_nvm_buffer[1] = (uint8_t) '4';
    x430_nvm_buffer[2] = (uint8_t) '3';
    x430_nvm_buffer[3] = (uint8_t) '0';
    dxl2_put_u16(&x430_nvm_buffer[4], x430_model_active.model_number);
    x430_nvm_buffer[6] = (uint8_t) X430_NVM_FORMAT;
    x430_nvm_buffer[7] = (uint8_t) slot;
    dxl2_put_u16(&x430_nvm_buffer[8], size);
    (void) memcpy(&x430_nvm_buffer[X430_NVM_HEADER_SIZE], payload, size);
    dxl2_put_u16(&x430_nvm_buffer[X430_NVM_HEADER_SIZE + size],
                 dxl2_crc16(x430_nvm_buffer, (size_t) X430_NVM_HEADER_SIZE + size));
    return total;
}

/** Loads a block into x430_nvm_buffer; true when it is valid for this model. */
static bool x430_nvm_load(struct x430_device* device, enum x430_nvm_slot slot, uint16_t size)
{
    const struct x430_hooks* hooks = x430_hooks_of(device);
    const uint16_t total = (uint16_t) (X430_NVM_HEADER_SIZE + size + 2U);
    bool valid = false;

    if ((hooks != NULL) && (hooks->nvm_load != NULL)
        && (hooks->nvm_load(device, slot, x430_nvm_buffer, total, device->config.user_data) == 0))
    {
        valid = (x430_nvm_buffer[0] == (uint8_t) 'X') && (x430_nvm_buffer[1] == (uint8_t) '4')
                && (x430_nvm_buffer[2] == (uint8_t) '3') && (x430_nvm_buffer[3] == (uint8_t) '0')
                && (dxl2_get_u16(&x430_nvm_buffer[4]) == x430_model_active.model_number)
                && (x430_nvm_buffer[6] == (uint8_t) X430_NVM_FORMAT)
                && (x430_nvm_buffer[7] == (uint8_t) slot)
                && (dxl2_get_u16(&x430_nvm_buffer[8]) == size)
                && (dxl2_get_u16(&x430_nvm_buffer[X430_NVM_HEADER_SIZE + size])
                    == dxl2_crc16(x430_nvm_buffer, (size_t) X430_NVM_HEADER_SIZE + size));
    }
    return valid;
}

static uint8_t x430_nvm_store(struct x430_device* device, enum x430_nvm_slot slot, uint16_t size)
{
    const struct x430_hooks* hooks = x430_hooks_of(device);
    uint8_t result = DXL2_ERR_NONE;

    if ((hooks != NULL) && (hooks->nvm_store != NULL))
    {
        const uint16_t total = x430_nvm_pack(slot, device->table, size);

        if (hooks->nvm_store(device, slot, x430_nvm_buffer, total, device->config.user_data) != 0)
        {
            result = DXL2_ERR_RESULT_FAIL;
        }
    }
    return result;
}

/**
 * Copies writable registers of a valid block (x430_nvm_buffer) into the table.
 * @param first,last Address window to copy.
 * @param keep_identity Leave ID, Baud Rate and Protocol Type untouched.
 */
static void x430_apply_block(struct x430_device* device, uint16_t first, uint16_t last,
                             bool keep_identity)
{
    const struct x430_model* model = device->model;

    for (uint16_t index = 0U; index < model->register_count; index++)
    {
        const struct x430_reg* reg = &model->registers[index];
        const bool identity = (reg->address == X430_ADDR_ID)
                              || (reg->address == X430_ADDR_BAUD_RATE)
                              || (reg->address == X430_ADDR_PROTOCOL_TYPE);
        const bool runtime =
            (reg->address == X430_ADDR_TORQUE_ENABLE) || (reg->address == X430_ADDR_BUS_WATCHDOG);

        if (((reg->flags & X430_REG_WRITABLE) != 0U) && (reg->address >= first)
            && (reg->address <= last) && !runtime && !(keep_identity && identity))
        {
            (void) memcpy(&device->table[reg->address],
                          &x430_nvm_buffer[X430_NVM_HEADER_SIZE + reg->address], reg->size);
        }
    }
}

/** @} */

/** @name Private helpers: checks */
/** @{ */

static bool x430_torque_on(const struct x430_device* device)
{
    return device->table[X430_ADDR_TORQUE_ENABLE] != 0U;
}

static int32_t x430_abs(int32_t value)
{
    return (value < 0) ? -value : value;
}

static bool x430_check_value(const struct x430_device* device, const struct x430_reg* reg,
                             int32_t value)
{
    const struct x430_model* model = device->model;
    bool valid = false;

    switch (reg->check)
    {
        case X430_CHECK_RANGE:
        case X430_CHECK_INDIRECT:
            valid = (value >= reg->min) && (value <= reg->max);
            break;
        case X430_CHECK_MASK:
            valid = (((uint32_t) value) & ~((uint32_t) reg->max)) == 0U;
            break;
        case X430_CHECK_OPMODE:
            valid = (value >= 0) && (value < 32)
                    && ((model->operating_modes & ((uint32_t) 1U << (uint32_t) value)) != 0U);
            break;
        case X430_CHECK_SYMMETRIC:
            valid = x430_abs(value) <= x430_get(device, reg->ref);
            break;
        case X430_CHECK_UPTO:
            valid = (value >= 0) && (value <= x430_get(device, reg->ref));
            break;
        case X430_CHECK_GOAL_POSITION:
            if (device->table[X430_ADDR_OPERATING_MODE] == X430_MODE_POSITION)
            {
                valid = (value >= x430_get(device, X430_ADDR_MIN_POSITION_LIMIT))
                        && (value <= x430_get(device, X430_ADDR_MAX_POSITION_LIMIT));
            }
            else
            {
                valid = x430_abs(value) <= X430_EXTENDED_POSITION_LIMIT;
            }
            break;
        default:
            valid = false;
            break;
    }
    return valid;
}

/** Error code for writing @p value to @p reg, DXL2_ERR_NONE when allowed. */
static uint8_t x430_check_write(const struct x430_device* device, const struct x430_reg* reg,
                                int32_t value, uint8_t options)
{
    uint8_t result = DXL2_ERR_NONE;

    if ((reg->flags & X430_REG_WRITABLE) == 0U)
    {
        result = DXL2_ERR_ACCESS;
    }
    else if (((reg->flags & X430_REG_EEPROM) != 0U)
             && (x430_torque_on(device) || ((options & X430_WRITE_SECONDARY) != 0U)))
    {
        result = DXL2_ERR_ACCESS;
    }
    else if (((reg->flags & X430_REG_GOAL) != 0U)
             && (device->table[X430_ADDR_BUS_WATCHDOG] == X430_BUS_WATCHDOG_ERROR))
    {
        result = DXL2_ERR_DATA_RANGE;
    }
    else if (reg->address == X430_ADDR_BAUD_RATE)
    {
        /* The model table says 0..7; applications may allow their own codes. */
        result = ((value >= 0) && (value <= (int32_t) X430_BAUD_RATE_CODE_MAX)) ?
                     DXL2_ERR_NONE :
                     DXL2_ERR_DATA_RANGE;
    }
    else if (!x430_check_value(device, reg, value))
    {
        result = DXL2_ERR_DATA_RANGE;
    }
    else if ((reg->address == X430_ADDR_ID)
             && x430_bus_id_taken(device->bus, device, (uint8_t) value))
    {
        result = DXL2_ERR_DATA_RANGE;
    }
    else if ((reg->address == X430_ADDR_TORQUE_ENABLE) && (value != 0)
             && ((device->table[X430_ADDR_HARDWARE_ERROR_STATUS]
                  & device->table[X430_ADDR_SHUTDOWN])
                 != 0U))
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

/**
 * New raw value of @p reg after writing @p data at @p address: the bytes of the
 * write that land in the register replace the current bytes.
 */
static uint32_t x430_merge(const struct x430_device* device, const struct x430_reg* reg,
                           uint16_t address, uint16_t length, const uint8_t* data, bool indirect)
{
    uint8_t bytes[4];

    (void) memcpy(bytes, &device->table[reg->address], reg->size);
    if (!indirect)
    {
        for (uint8_t offset = 0U; offset < reg->size; offset++)
        {
            const uint16_t target = (uint16_t) (reg->address + offset);

            if ((target >= address) && (target < (address + length)))
            {
                bytes[offset] = data[target - address];
            }
        }
    }
    else
    {
        for (uint16_t index = 0U; index < length; index++)
        {
            const uint16_t target = x430_resolve(device, (uint16_t) (address + index));

            if ((target >= reg->address) && (target < (reg->address + reg->size)))
            {
                bytes[target - reg->address] = data[index];
            }
        }
    }
    return x430_raw_get(bytes, 0U, reg->size);
}

/** Marks every register touched by the write (bit = register index). */
static void x430_mark_touched(const struct x430_device* device, uint16_t address, uint16_t length,
                              uint8_t* touched, bool indirect)
{
    for (uint16_t index = 0U; index < length; index++)
    {
        const uint16_t raw = (uint16_t) (address + index);
        const uint16_t target = indirect ? x430_resolve(device, raw) : raw;
        const uint8_t entry = device->model->register_map[target];

        if (entry != 0U)
        {
            const uint8_t bit = (uint8_t) (entry - 1U);

            touched[bit / 8U] |= (uint8_t) (1U << (bit % 8U));
        }
    }
}

/**
 * Bytes of a write through Indirect Data whose Indirect Address points at
 * Indirect Data storage itself: they are kept as plain bytes.
 */
static void x430_store_raw_bytes(struct x430_device* device, uint16_t address, uint16_t length,
                                 const uint8_t* data)
{
    for (uint16_t index = 0U; index < length; index++)
    {
        const uint16_t target = x430_resolve(device, (uint16_t) (address + index));

        if ((device->model->register_map[target] == 0U)
            && x430_touches_indirect(device->model, target, 1U))
        {
            device->table[target] = data[index];
        }
    }
}

/**
 * Torque on: Goal Position takes the Present Position, so the actuator holds
 * where it is instead of jumping to an old goal (behaviour of the X series).
 */
static void x430_sync_goal(struct x430_device* device)
{
    const struct x430_hooks* hooks = x430_hooks_of(device);

    if ((hooks != NULL) && (hooks->read != NULL))
    {
        hooks->read(device, X430_ADDR_PRESENT_POSITION, 4U, device->config.user_data);
    }
    (void) memcpy(&device->table[X430_ADDR_GOAL_POSITION],
                  &device->table[X430_ADDR_PRESENT_POSITION], 4U);
}

/** Torque Enable = 1 through the write hook (startup, Drive Mode bit 3). */
static uint8_t x430_enable_torque(struct x430_device* device, bool sync_goal)
{
    const struct x430_hooks* hooks = x430_hooks_of(device);
    uint8_t result = DXL2_ERR_NONE;

    if ((hooks != NULL) && (hooks->write != NULL))
    {
        result = hooks->write(device, X430_ADDR_TORQUE_ENABLE, 1U, 1, device->config.user_data);
    }
    if ((result == DXL2_ERR_NONE) && !x430_torque_on(device))
    {
        device->table[X430_ADDR_TORQUE_ENABLE] = 1U;
        if (sync_goal)
        {
            x430_sync_goal(device);
        }
    }
    return result;
}

/** @} */

/** @name Private helpers: startup */
/** @{ */

static void x430_load_factory(struct x430_device* device, uint16_t first, uint16_t last)
{
    const struct x430_model* model = device->model;

    for (uint16_t index = 0U; index < model->register_count; index++)
    {
        const struct x430_reg* reg = &model->registers[index];

        if ((reg->address >= first) && (reg->address <= last))
        {
            x430_raw_put(device->table, reg->address, reg->size, (uint32_t) reg->initial);
        }
    }
}

static void x430_load_identity(struct x430_device* device)
{
    x430_put(device, X430_ADDR_MODEL_NUMBER, (int32_t) X430_MODEL_NUMBER);
    x430_put(device, X430_ADDR_FIRMWARE_VERSION,
             (int32_t) ((device->config.firmware_version != 0U) ? device->config.firmware_version :
                                                                  X430_FIRMWARE_VERSION));
    x430_raw_put(device->table, X430_ADDR_MODEL_INFORMATION, 4U, device->config.model_information);
}

/** RAM area to its startup state, then the Startup Configuration. */
static void x430_startup(struct x430_device* device)
{
    const uint8_t startup = device->table[X430_ADDR_STARTUP_CONFIGURATION];

    (void) memset(&device->table[X430_RAM_START], 0,
                  (size_t) device->model->table_size - X430_RAM_START);
    x430_load_factory(device, X430_RAM_START, (uint16_t) (device->model->table_size - 1U));
    device->reg_length = 0U;
    device->baud_changed = false;

#if X430_FEATURE_BACKUP
    if (x430_nvm_load(device, X430_NVM_BACKUP, device->model->table_size))
    {
        device->table[X430_ADDR_BACKUP_READY] = 1U;
        if ((startup & X430_STARTUP_RAM_RESTORE) != 0U)
        {
            x430_apply_block(device, X430_RAM_START, (uint16_t) (device->model->table_size - 1U),
                             true);
        }
    }
#endif

    if ((startup & X430_STARTUP_TORQUE_ON) != 0U)
    {
        (void) x430_enable_torque(device, true);
    }
}

/** @} */

/** @name Public functions: lifecycle and lock-free access */
/** @{ */

int x430_device_init(struct x430_device* device, const struct x430_device_config* config)
{
    int result = X430_EINVAL;

    if ((device != NULL) && (config != NULL) && (config->id <= DXL2_MAX_ID))
    {
        (void) memset(device, 0, sizeof(*device));
        device->model = &x430_model_active;
        device->config = *config;

        x430_load_factory(device, 0U, X430_RAM_START - 1U);
        x430_load_identity(device);
        if (config->id != 0U)
        {
            device->table[X430_ADDR_ID] = config->id;
        }
        if (x430_nvm_load(device, X430_NVM_EEPROM, X430_RAM_START))
        {
            x430_apply_block(device, 0U, X430_RAM_START - 1U, false);
        }
        x430_startup(device);
        device->initialized = true;
        result = X430_OK;
    }
    return result;
}

int32_t x430_device_get(const struct x430_device* device, uint16_t address)
{
    return x430_get(device, address);
}

int x430_device_set(struct x430_device* device, uint16_t address, int32_t value)
{
    const struct x430_reg* reg = x430_reg_at(device->model, address);
    int result = X430_EINVAL;

    if ((reg != NULL) && (reg->address == address))
    {
        x430_raw_put(device->table, address, reg->size, (uint32_t) value);
        result = X430_OK;
    }
    return result;
}

uint8_t x430_device_id(const struct x430_device* device)
{
    return device->table[X430_ADDR_ID];
}

void x430_device_raise_hardware_error(struct x430_device* device, uint8_t bits)
{
    device->table[X430_ADDR_HARDWARE_ERROR_STATUS] |= bits;
    if (((bits & device->table[X430_ADDR_SHUTDOWN]) != 0U) && x430_torque_on(device))
    {
        device->table[X430_ADDR_TORQUE_ENABLE] = 0U;
        (void) x430_event(device, X430_EVENT_TORQUE_SHUTDOWN);
    }
}

/** @} */

/** @name Public functions: instruction semantics */
/** @{ */

uint8_t x430_device_read(struct x430_device* device, uint16_t address, uint16_t length,
                         uint8_t* data)
{
    const struct x430_hooks* hooks = x430_hooks_of(device);
    uint8_t result = DXL2_ERR_NONE;

    if (length == 0U)
    {
        result = DXL2_ERR_DATA_LENGTH;
    }
    else if (((uint32_t) address + length) > device->model->table_size)
    {
        result = DXL2_ERR_ACCESS;
    }
    else
    {
        const uint32_t tick = x430_bus_millis(device->bus) % X430_REALTIME_TICK_PERIOD;

        x430_raw_put(device->table, X430_ADDR_REALTIME_TICK, 2U, tick);

        if ((hooks != NULL) && (hooks->read != NULL))
        {
            uint16_t run_start = x430_resolve(device, address);
            uint16_t run_length = 1U;

            for (uint16_t index = 1U; index <= length; index++)
            {
                const uint16_t target =
                    (index < length) ? x430_resolve(device, (uint16_t) (address + index)) : 0U;

                if ((index < length) && (target == (run_start + run_length)))
                {
                    run_length++;
                }
                else
                {
                    hooks->read(device, run_start, run_length, device->config.user_data);
                    run_start = target;
                    run_length = 1U;
                }
            }
        }

        for (uint16_t index = 0U; index < length; index++)
        {
            data[index] = device->table[x430_resolve(device, (uint16_t) (address + index))];
        }
    }
    return result;
}

uint8_t x430_device_write(struct x430_device* device, uint16_t address, uint16_t length,
                          const uint8_t* data, uint8_t options)
{
    const struct x430_model* model = device->model;
    const struct x430_hooks* hooks = x430_hooks_of(device);
    uint8_t touched[X430_TOUCHED_BYTES];
    uint8_t result = DXL2_ERR_NONE;
    bool indirect = false;
    bool goal_written = false;
    bool eeprom_written = false;

    (void) memset(touched, 0, sizeof(touched));

    if (length == 0U)
    {
        result = DXL2_ERR_DATA_LENGTH;
    }
    else if (((uint32_t) address + length) > model->table_size)
    {
        result = DXL2_ERR_ACCESS;
    }
    else
    {
        indirect = x430_touches_indirect(model, address, length);
        x430_mark_touched(device, address, length, touched, indirect);

        /* Pass 1: every touched register must accept its new value. */
        for (uint16_t index = 0U; (index < model->register_count) && (result == DXL2_ERR_NONE);
             index++)
        {
            if ((touched[index / 8U] & (uint8_t) (1U << (index % 8U))) != 0U)
            {
                const struct x430_reg* reg = &model->registers[index];
                const int32_t value =
                    x430_to_value(reg, x430_merge(device, reg, address, length, data, indirect));

                result = x430_check_write(device, reg, value, options);
            }
        }
    }

    if ((result == DXL2_ERR_NONE) && ((options & X430_WRITE_VALIDATE_ONLY) == 0U))
    {
        /* Pass 2: hooks and storage, in address order. */
        for (uint16_t index = 0U; (index < model->register_count) && (result == DXL2_ERR_NONE);
             index++)
        {
            if ((touched[index / 8U] & (uint8_t) (1U << (index % 8U))) != 0U)
            {
                const struct x430_reg* reg = &model->registers[index];
                const uint32_t raw = x430_merge(device, reg, address, length, data, indirect);
                const uint32_t previous = x430_raw_get(device->table, reg->address, reg->size);

                if ((hooks != NULL) && (hooks->write != NULL))
                {
                    result = hooks->write(device, reg->address, reg->size, x430_to_value(reg, raw),
                                          device->config.user_data);
                }
                if (result == DXL2_ERR_NONE)
                {
                    x430_raw_put(device->table, reg->address, reg->size, raw);
                    eeprom_written = eeprom_written || ((reg->flags & X430_REG_EEPROM) != 0U);
                    goal_written = goal_written || ((reg->flags & X430_REG_GOAL) != 0U);
                    if ((reg->address == X430_ADDR_BAUD_RATE) && (raw != previous))
                    {
                        device->baud_changed = true;
                    }
                    if ((reg->address == X430_ADDR_TORQUE_ENABLE) && (raw != 0U)
                        && (previous == 0U))
                    {
                        x430_sync_goal(device);
                    }
                }
            }
        }
        if (indirect)
        {
            x430_store_raw_bytes(device, address, length, data);
        }

        if (eeprom_written)
        {
            const uint8_t stored = x430_nvm_store(device, X430_NVM_EEPROM, X430_RAM_START);

            result = (result == DXL2_ERR_NONE) ? stored : result;
        }

        /* Drive Mode bit 3: a goal value written with torque off enables the torque. */
        if (goal_written && (result == DXL2_ERR_NONE) && !x430_torque_on(device)
            && ((device->table[X430_ADDR_DRIVE_MODE] & X430_DRIVE_MODE_TORQUE_ON_BY_GOAL) != 0U))
        {
            result = x430_enable_torque(device, false);
        }
    }
    return result;
}

uint8_t x430_device_reg_write(struct x430_device* device, uint16_t address, uint16_t length,
                              const uint8_t* data, uint8_t options)
{
    uint8_t result = DXL2_ERR_NONE;

    if (length > X430_REG_WRITE_SIZE)
    {
        result = DXL2_ERR_DATA_LENGTH;
    }
    else
    {
        result = x430_device_write(device, address, length, data,
                                   (uint8_t) (options | X430_WRITE_VALIDATE_ONLY));
    }
    if (result == DXL2_ERR_NONE)
    {
        (void) memcpy(device->reg_data, data, length);
        device->reg_address = address;
        device->reg_length = length;
        device->table[X430_ADDR_REGISTERED_INSTRUCTION] = 1U;
    }
    return result;
}

uint8_t x430_device_action(struct x430_device* device)
{
    uint8_t result = DXL2_ERR_INSTRUCTION;

    if (device->reg_length != 0U)
    {
        const uint16_t length = device->reg_length;

        device->reg_length = 0U;
        device->table[X430_ADDR_REGISTERED_INSTRUCTION] = 0U;
        result = x430_device_write(device, device->reg_address, length, device->reg_data, 0U);
    }
    return result;
}

uint8_t x430_device_reboot(struct x430_device* device)
{
    x430_startup(device);
    (void) x430_event(device, X430_EVENT_REBOOT);
    return DXL2_ERR_NONE;
}

uint8_t x430_device_factory_reset(struct x430_device* device, uint8_t option)
{
    uint8_t result = DXL2_ERR_DATA_RANGE;

    if ((option == X430_FACTORY_RESET_ALL) || (option == X430_FACTORY_RESET_EXCEPT_ID)
        || (option == X430_FACTORY_RESET_EXCEPT_ID_BAUD))
    {
        const uint8_t id = device->table[X430_ADDR_ID];
        const uint8_t baud = device->table[X430_ADDR_BAUD_RATE];

        x430_load_factory(device, 0U, X430_RAM_START - 1U);
        x430_load_identity(device);
        if (option != X430_FACTORY_RESET_ALL)
        {
            device->table[X430_ADDR_ID] = id;
        }
        else if ((device->config.id != 0U)
                 && !x430_bus_id_taken(device->bus, device, device->config.id))
        {
            device->table[X430_ADDR_ID] = device->config.id;
        }
        else
        {
            /* Factory ID from the model table (1). */
        }
        if (option == X430_FACTORY_RESET_EXCEPT_ID_BAUD)
        {
            device->table[X430_ADDR_BAUD_RATE] = baud;
        }
        result = x430_nvm_store(device, X430_NVM_EEPROM, X430_RAM_START);
        x430_startup(device);
        device->baud_changed = device->table[X430_ADDR_BAUD_RATE] != baud;
        (void) x430_event(device, X430_EVENT_FACTORY_RESET);
    }
    return result;
}

uint8_t x430_device_clear(struct x430_device* device, uint8_t option)
{
    uint8_t result = DXL2_ERR_DATA_RANGE;

    if (option == X430_CLEAR_MULTI_TURN)
    {
        const struct x430_hooks* hooks = x430_hooks_of(device);

        if (device->table[X430_ADDR_MOVING] != 0U)
        {
            result = DXL2_ERR_RESULT_FAIL;
        }
        else if ((hooks != NULL) && (hooks->event != NULL))
        {
            result = hooks->event(device, X430_EVENT_CLEAR_MULTI_TURN, device->config.user_data);
        }
        else
        {
            int32_t position = x430_get(device, X430_ADDR_PRESENT_POSITION) % X430_PULSES_PER_TURN;

            if (position < 0)
            {
                position += X430_PULSES_PER_TURN;
            }
            x430_put(device, X430_ADDR_PRESENT_POSITION, position);
            result = DXL2_ERR_NONE;
        }
    }
    return result;
}

uint8_t x430_device_backup(struct x430_device* device, uint8_t option)
{
    uint8_t result = DXL2_ERR_DATA_RANGE;

#if X430_FEATURE_BACKUP
    if (option == X430_BACKUP_STORE)
    {
        result = x430_nvm_store(device, X430_NVM_BACKUP, device->model->table_size);
        if ((result == DXL2_ERR_NONE) && (x430_hooks_of(device) != NULL)
            && (x430_hooks_of(device)->nvm_store != NULL))
        {
            device->table[X430_ADDR_BACKUP_READY] = 1U;
        }
    }
    else if (option == X430_BACKUP_RESTORE)
    {
        if (x430_torque_on(device))
        {
            result = DXL2_ERR_ACCESS;
        }
        else if (!x430_nvm_load(device, X430_NVM_BACKUP, device->model->table_size))
        {
            result = DXL2_ERR_RESULT_FAIL;
        }
        else
        {
            x430_apply_block(device, 0U, (uint16_t) (device->model->table_size - 1U), true);
            result = x430_nvm_store(device, X430_NVM_EEPROM, X430_RAM_START);
            (void) x430_event(device, X430_EVENT_BACKUP_RESTORED);
        }
    }
    else
    {
        /* Unknown option: Data Range Error. */
    }
#else
    (void) device;
    (void) option;
#endif
    return result;
}

uint8_t x430_device_alert(const struct x430_device* device)
{
    return (device->table[X430_ADDR_HARDWARE_ERROR_STATUS] != 0U) ? DXL2_ERR_ALERT_BIT :
                                                                    DXL2_ERR_NONE;
}

void x430_device_watchdog(struct x430_device* device, uint32_t idle_ms)
{
    const uint8_t timeout = device->table[X430_ADDR_BUS_WATCHDOG];

    if ((timeout != 0U) && (timeout != X430_BUS_WATCHDOG_ERROR) && x430_torque_on(device)
        && (idle_ms > ((uint32_t) timeout * X430_BUS_WATCHDOG_UNIT_MS)))
    {
        device->table[X430_ADDR_BUS_WATCHDOG] = X430_BUS_WATCHDOG_ERROR;
        (void) x430_event(device, X430_EVENT_BUS_WATCHDOG);
    }
}

void x430_device_flush_events(struct x430_device* device)
{
    if (device->baud_changed)
    {
        device->baud_changed = false;
        (void) x430_event(device, X430_EVENT_BAUD_RATE);
    }
}

/** @} */
