/**
 * @file x430_bus.c
 * @brief Protocol 2.0 instruction decoding for the emulated X430 devices.
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

#include "x430_bus.h"

#include <string.h>

#include "x430_internal.h"

/** @} */

/** @name Private definitions */
/** @{ */

/** Parameters of READ and of the SYNC_READ header: address and length. */
#define X430_READ_PARAMS 4U
/** Parameters of WRITE before the data: address. */
#define X430_WRITE_HEADER 2U
/** Parameters of CLEAR and BACKUP: option and four magic bytes. */
#define X430_OPTION_PARAMS 5U
/** One BULK_READ entry: ID, address, length. */
#define X430_BULK_READ_ENTRY 5U
/** Fast read: bytes around the data of one device (error, ID, CRC). */
#define X430_FAST_SEGMENT_OVERHEAD 4U
/** Fast read: header, reserved, ID, length, instruction. */
#define X430_FAST_HEADER 8U

/** Origin of an instruction for one device. */
enum x430_target
{
    X430_TARGET_PRIMARY = 0, /**< Addressed by its ID: may answer. */
    X430_TARGET_SECONDARY,   /**< Addressed by its Secondary ID: never answers. */
    X430_TARGET_BROADCAST    /**< Broadcast: answers PING only. */
};

/** @} */

/** @name Private helpers */
/** @{ */

static uint32_t x430_millis(const struct x430_bus* bus)
{
    const struct x430_port* port = bus->config.port;
    uint32_t now = 0U;

    if ((port != NULL) && (port->millis != NULL))
    {
        now = port->millis(port->context);
    }
    return now;
}

static void x430_delay_status(const struct x430_bus* bus, const struct x430_device* device)
{
    const struct x430_port* port = bus->config.port;
    const uint32_t delay =
        (uint32_t) device->table[X430_ADDR_RETURN_DELAY_TIME] * X430_RETURN_DELAY_UNIT_US;

    if ((delay != 0U) && (port != NULL) && (port->delay_us != NULL))
    {
        port->delay_us(port->context, delay);
    }
}

/** Status Return Level rule: PING always, reads from level 1, the rest at level 2. */
static bool x430_answers(const struct x430_device* device, uint8_t instruction)
{
    const uint8_t level = device->table[X430_ADDR_STATUS_RETURN_LEVEL];
    bool answer = false;

    if (instruction == DXL2_INST_PING)
    {
        answer = true;
    }
    else if ((instruction == DXL2_INST_READ) || (instruction == DXL2_INST_SYNC_READ)
             || (instruction == DXL2_INST_BULK_READ) || (instruction == DXL2_INST_FAST_SYNC_READ)
             || (instruction == DXL2_INST_FAST_BULK_READ))
    {
        answer = level >= 1U;
    }
    else
    {
        answer = level >= 2U;
    }
    return answer;
}

static void x430_send_status(struct x430_bus* bus, const struct x430_device* device, uint8_t id,
                             uint8_t error, const uint8_t* data, uint16_t length)
{
    x430_delay_status(bus, device);
    (void) dxl2_link_send_status(&bus->link, id, (uint8_t) (error | x430_device_alert(device)),
                                 (error == DXL2_ERR_NONE) ? data : NULL,
                                 (error == DXL2_ERR_NONE) ? length : 0U);
}

static bool x430_magic_ok(const uint8_t* parameters, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
    return (parameters[1] == b1) && (parameters[2] == b2) && (parameters[3] == b3)
           && (parameters[4] == b4);
}

/** Read for a group instruction; sends the status of one device. */
static void x430_group_read(struct x430_bus* bus, uint8_t id, uint16_t address, uint16_t length,
                            uint8_t instruction)
{
    struct x430_device* device = x430_bus_find(bus, id);

    if ((device != NULL) && x430_answers(device, instruction))
    {
        const uint8_t error = x430_device_read(device, address, length, bus->data);

        x430_send_status(bus, device, id, error, bus->data, length);
    }
}

/** @} */

/** @name Private helpers: single-device instructions */
/** @{ */

static void x430_execute(struct x430_bus* bus, struct x430_device* device,
                         const struct dxl2_packet* packet, enum x430_target target)
{
    const uint8_t id = x430_device_id(device);
    const uint8_t options = (target == X430_TARGET_SECONDARY) ? X430_WRITE_SECONDARY : 0U;
    const uint16_t count = packet->parameter_length;
    const uint8_t* parameters = packet->parameters;
    uint8_t error = DXL2_ERR_NONE;
    uint16_t length = 0U;
    bool answer_first = false; /* REBOOT/FACTORY_RESET answer before they act. */

    switch (packet->instruction)
    {
        case DXL2_INST_PING:
            dxl2_put_u16(&bus->data[0], (uint16_t) x430_device_get(device, X430_ADDR_MODEL_NUMBER));
            bus->data[2] = device->table[X430_ADDR_FIRMWARE_VERSION];
            length = 3U;
            break;
        case DXL2_INST_READ:
            if (count != X430_READ_PARAMS)
            {
                error = DXL2_ERR_DATA_LENGTH;
            }
            else
            {
                length = dxl2_get_u16(&parameters[2]);
                error = (length > (DXL2_MAX_PARAMETER_SIZE - 1U)) ?
                            DXL2_ERR_DATA_LENGTH :
                            x430_device_read(device, dxl2_get_u16(parameters), length, bus->data);
            }
            break;
        case DXL2_INST_WRITE:
        case DXL2_INST_REG_WRITE:
            if (count <= X430_WRITE_HEADER)
            {
                error = DXL2_ERR_DATA_LENGTH;
            }
            else if (packet->instruction == DXL2_INST_WRITE)
            {
                error = x430_device_write(device, dxl2_get_u16(parameters),
                                          (uint16_t) (count - X430_WRITE_HEADER),
                                          &parameters[X430_WRITE_HEADER], options);
            }
            else
            {
                error = x430_device_reg_write(device, dxl2_get_u16(parameters),
                                              (uint16_t) (count - X430_WRITE_HEADER),
                                              &parameters[X430_WRITE_HEADER], options);
            }
            break;
        case DXL2_INST_ACTION:
            error = (count == 0U) ? x430_device_action(device) : DXL2_ERR_DATA_LENGTH;
            break;
        case DXL2_INST_REBOOT:
            error = (count == 0U) ? DXL2_ERR_NONE : DXL2_ERR_DATA_LENGTH;
            answer_first = true;
            break;
        case DXL2_INST_FACTORY_RESET:
            if (count != 1U)
            {
                error = DXL2_ERR_DATA_LENGTH;
            }
            else if ((parameters[0] != X430_FACTORY_RESET_ALL)
                     && (parameters[0] != X430_FACTORY_RESET_EXCEPT_ID)
                     && (parameters[0] != X430_FACTORY_RESET_EXCEPT_ID_BAUD))
            {
                error = DXL2_ERR_DATA_RANGE;
            }
            else if (((target == X430_TARGET_BROADCAST)
                      && (parameters[0] == X430_FACTORY_RESET_ALL))
                     || (target == X430_TARGET_SECONDARY))
            {
                /* Reset All is refused on the broadcast ID; the Secondary ID cannot touch EEPROM.
                 */
                error = DXL2_ERR_ACCESS;
            }
            else
            {
                answer_first = true;
            }
            break;
        case DXL2_INST_CLEAR:
            error = ((count == X430_OPTION_PARAMS)
                     && x430_magic_ok(parameters, 0x44U, 0x58U, 0x4CU, 0x22U)) ?
                        x430_device_clear(device, parameters[0]) :
                        DXL2_ERR_DATA_RANGE;
            break;
#if X430_FEATURE_BACKUP
        case DXL2_INST_BACKUP:
            error = ((count == X430_OPTION_PARAMS)
                     && x430_magic_ok(parameters, (uint8_t) 'C', (uint8_t) 'T', (uint8_t) 'R',
                                      (uint8_t) 'L')) ?
                        x430_device_backup(device, parameters[0]) :
                        DXL2_ERR_DATA_RANGE;
            break;
#endif
        default:
            error = DXL2_ERR_INSTRUCTION;
            break;
    }

    if ((target == X430_TARGET_PRIMARY) && x430_answers(device, packet->instruction))
    {
        x430_send_status(bus, device, id, error, bus->data, length);
    }
    else if ((target == X430_TARGET_BROADCAST) && (packet->instruction == DXL2_INST_PING))
    {
        x430_send_status(bus, device, id, error, bus->data, length);
    }
    else
    {
        /* No status: Secondary ID, broadcast, or Status Return Level. */
    }

    if (answer_first && (error == DXL2_ERR_NONE))
    {
        if (packet->instruction == DXL2_INST_REBOOT)
        {
            (void) x430_device_reboot(device);
        }
        else
        {
            (void) x430_device_factory_reset(device, parameters[0]);
        }
    }
    x430_device_flush_events(device);
}

/** @} */

/** @name Private helpers: group instructions */
/** @{ */

static void x430_sync_read(struct x430_bus* bus, const struct dxl2_packet* packet)
{
    if (packet->parameter_length > X430_READ_PARAMS)
    {
        const uint16_t address = dxl2_get_u16(packet->parameters);
        const uint16_t length = dxl2_get_u16(&packet->parameters[2]);

        if ((length != 0U) && (length <= (DXL2_MAX_PARAMETER_SIZE - 1U)))
        {
            for (uint16_t index = X430_READ_PARAMS; index < packet->parameter_length; index++)
            {
                x430_group_read(bus, packet->parameters[index], address, length,
                                DXL2_INST_SYNC_READ);
            }
        }
    }
}

static void x430_sync_write(struct x430_bus* bus, const struct dxl2_packet* packet)
{
    if (packet->parameter_length > X430_READ_PARAMS)
    {
        const uint16_t address = dxl2_get_u16(packet->parameters);
        const uint16_t length = dxl2_get_u16(&packet->parameters[2]);
        const uint16_t entry = (uint16_t) (length + 1U);

        if ((length != 0U) && (((packet->parameter_length - X430_READ_PARAMS) % entry) == 0U))
        {
            for (uint16_t index = X430_READ_PARAMS; index < packet->parameter_length;
                 index = (uint16_t) (index + entry))
            {
                struct x430_device* device = x430_bus_find(bus, packet->parameters[index]);

                if (device != NULL)
                {
                    (void) x430_device_write(device, address, length,
                                             &packet->parameters[index + 1U], 0U);
                    x430_device_flush_events(device);
                }
            }
        }
    }
}

#if X430_FEATURE_BULK

static void x430_bulk_read(struct x430_bus* bus, const struct dxl2_packet* packet)
{
    if ((packet->parameter_length != 0U)
        && ((packet->parameter_length % X430_BULK_READ_ENTRY) == 0U))
    {
        for (uint16_t index = 0U; index < packet->parameter_length;
             index = (uint16_t) (index + X430_BULK_READ_ENTRY))
        {
            const uint16_t length = dxl2_get_u16(&packet->parameters[index + 3U]);

            if ((length != 0U) && (length <= (DXL2_MAX_PARAMETER_SIZE - 1U)))
            {
                x430_group_read(bus, packet->parameters[index],
                                dxl2_get_u16(&packet->parameters[index + 1U]), length,
                                DXL2_INST_BULK_READ);
            }
        }
    }
}

static void x430_bulk_write(struct x430_bus* bus, const struct dxl2_packet* packet)
{
    uint16_t index = 0U;
    bool valid = true;

    /* Validate the whole packet first: a malformed entry voids the instruction. */
    while (valid && (index < packet->parameter_length))
    {
        if (((uint32_t) index + X430_BULK_READ_ENTRY) > packet->parameter_length)
        {
            valid = false;
        }
        else
        {
            const uint16_t length = dxl2_get_u16(&packet->parameters[index + 3U]);

            valid =
                (length != 0U)
                && (((uint32_t) index + X430_BULK_READ_ENTRY + length) <= packet->parameter_length);
            index = (uint16_t) (index + X430_BULK_READ_ENTRY + length);
        }
    }

    index = 0U;
    while (valid && (index < packet->parameter_length))
    {
        struct x430_device* device = x430_bus_find(bus, packet->parameters[index]);
        const uint16_t length = dxl2_get_u16(&packet->parameters[index + 3U]);

        if (device != NULL)
        {
            (void) x430_device_write(device, dxl2_get_u16(&packet->parameters[index + 1U]), length,
                                     &packet->parameters[index + X430_BULK_READ_ENTRY], 0U);
            x430_device_flush_events(device);
        }
        index = (uint16_t) (index + X430_BULK_READ_ENTRY + length);
    }
}

#endif /* X430_FEATURE_BULK */

#if X430_FEATURE_FAST_READ

/**
 * Fast Sync/Bulk Read: one status packet with ID 0xFE carrying, for every
 * device, [error, ID, data, CRC]. Each CRC covers the packet from the header up
 * to that point; the last one is the packet CRC. The packet is not byte
 * stuffed (DynamixelSDK parses it raw).
 */
static void x430_fast_read(struct x430_bus* bus, const struct dxl2_packet* packet, bool bulk)
{
    uint16_t ends[X430_MAX_DEVICES];
    const struct x430_device* first = NULL;
    uint16_t position = X430_FAST_HEADER;
    uint8_t segments = 0U;
    const uint16_t step = bulk ? X430_BULK_READ_ENTRY : 1U;
    const uint16_t start = bulk ? 0U : X430_READ_PARAMS;
    bool valid = bulk ? ((packet->parameter_length != 0U)
                         && ((packet->parameter_length % X430_BULK_READ_ENTRY) == 0U)) :
                        (packet->parameter_length > X430_READ_PARAMS);

    for (uint16_t index = start; valid && (index < packet->parameter_length);
         index = (uint16_t) (index + step))
    {
        const uint8_t id = packet->parameters[index];
        const uint16_t address =
            bulk ? dxl2_get_u16(&packet->parameters[index + 1U]) : dxl2_get_u16(packet->parameters);
        const uint16_t length = bulk ? dxl2_get_u16(&packet->parameters[index + 3U]) :
                                       dxl2_get_u16(&packet->parameters[2]);
        struct x430_device* device = x430_bus_find(bus, id);

        if ((length == 0U)
            || (((uint32_t) position + length + X430_FAST_SEGMENT_OVERHEAD) > sizeof(bus->raw)))
        {
            valid = false;
        }
        else if ((device != NULL) && (segments < X430_MAX_DEVICES)
                 && x430_answers(device, packet->instruction))
        {
            uint8_t error = x430_device_read(device, address, length, &bus->raw[position + 2U]);

            if (error != DXL2_ERR_NONE)
            {
                (void) memset(&bus->raw[position + 2U], 0, length);
            }
            error = (uint8_t) (error | x430_device_alert(device));
            bus->raw[position] = error;
            bus->raw[position + 1U] = id;
            ends[segments] = (uint16_t) (position + 2U + length);
            position = (uint16_t) (ends[segments] + 2U);
            segments++;
            first = (first == NULL) ? device : first;
        }
        else
        {
            /* ID not hosted here (or silent): skipped. */
        }
    }

    if (valid && (segments != 0U) && (first != NULL))
    {
        bus->raw[0] = 0xFFU;
        bus->raw[1] = 0xFFU;
        bus->raw[2] = 0xFDU;
        bus->raw[3] = 0x00U;
        bus->raw[4] = DXL2_BROADCAST_ID;
        dxl2_put_u16(&bus->raw[5], (uint16_t) (position - 7U));
        bus->raw[7] = DXL2_INST_STATUS;
        for (uint8_t segment = 0U; segment < segments; segment++)
        {
            dxl2_put_u16(&bus->raw[ends[segment]], dxl2_crc16(bus->raw, ends[segment]));
        }
        x430_delay_status(bus, first);
        (void) bus->config.write(bus->config.write_context, bus->raw, position);
    }
}

#endif /* X430_FEATURE_FAST_READ */

/** Broadcast PING: every device answers, lowest ID first. */
static void x430_broadcast_ping(struct x430_bus* bus, const struct dxl2_packet* packet)
{
    int16_t last = -1;

    for (uint8_t round = 0U; round < bus->device_count; round++)
    {
        struct x430_device* next = NULL;

        for (uint8_t index = 0U; index < bus->device_count; index++)
        {
            struct x430_device* device = bus->devices[index];
            const int16_t id = (int16_t) x430_device_id(device);

            if ((id > last) && ((next == NULL) || (id < (int16_t) x430_device_id(next))))
            {
                next = device;
            }
        }
        if (next != NULL)
        {
            last = (int16_t) x430_device_id(next);
            x430_execute(bus, next, packet, X430_TARGET_BROADCAST);
        }
    }
}

static void x430_broadcast(struct x430_bus* bus, const struct dxl2_packet* packet)
{
    switch (packet->instruction)
    {
        case DXL2_INST_PING:
            x430_broadcast_ping(bus, packet);
            break;
        case DXL2_INST_SYNC_READ:
            x430_sync_read(bus, packet);
            break;
        case DXL2_INST_SYNC_WRITE:
            x430_sync_write(bus, packet);
            break;
#if X430_FEATURE_BULK
        case DXL2_INST_BULK_READ:
            x430_bulk_read(bus, packet);
            break;
        case DXL2_INST_BULK_WRITE:
            x430_bulk_write(bus, packet);
            break;
#endif
#if X430_FEATURE_FAST_READ
        case DXL2_INST_FAST_SYNC_READ:
            x430_fast_read(bus, packet, false);
            break;
        case DXL2_INST_FAST_BULK_READ:
            x430_fast_read(bus, packet, true);
            break;
#endif
        case DXL2_INST_WRITE:
        case DXL2_INST_REG_WRITE:
        case DXL2_INST_ACTION:
        case DXL2_INST_FACTORY_RESET:
        case DXL2_INST_REBOOT:
        case DXL2_INST_CLEAR:
        case DXL2_INST_BACKUP:
            for (uint8_t index = 0U; index < bus->device_count; index++)
            {
                x430_execute(bus, bus->devices[index], packet, X430_TARGET_BROADCAST);
            }
            break;
        default:
            /* READ and unknown instructions on the broadcast ID: no device answers. */
            break;
    }
}

/** Packet dispatcher, called with the bus lock held. */
static void x430_dispatch(const struct dxl2_packet* packet, void* context)
{
    struct x430_bus* bus = (struct x430_bus*) context;

    if ((packet != NULL) && (bus != NULL) && (packet->instruction != DXL2_INST_STATUS))
    {
        const uint32_t now = x430_millis(bus);

        for (uint8_t index = 0U; index < bus->device_count; index++)
        {
            x430_device_watchdog(bus->devices[index], now - bus->last_rx_ms);
        }
        bus->last_rx_ms = now;

        if (packet->id == DXL2_BROADCAST_ID)
        {
            x430_broadcast(bus, packet);
        }
        else
        {
            /* The primary ID answers; devices sharing it as Secondary ID act silently. */
            for (uint8_t index = 0U; index < bus->device_count; index++)
            {
                struct x430_device* device = bus->devices[index];
                const uint8_t secondary = device->table[X430_ADDR_SECONDARY_ID];

                if (x430_device_id(device) == packet->id)
                {
                    x430_execute(bus, device, packet, X430_TARGET_PRIMARY);
                }
                else if ((secondary == packet->id) && (secondary < X430_SECONDARY_ID_DISABLED))
                {
                    x430_execute(bus, device, packet, X430_TARGET_SECONDARY);
                }
                else
                {
                    /* Not addressed. */
                }
            }
        }
    }
}

/** @} */

/** @name Internal functions (x430_internal.h) */
/** @{ */

bool x430_bus_id_taken(const struct x430_bus* bus, const struct x430_device* except, uint8_t id)
{
    bool taken = false;

    if (bus != NULL)
    {
        for (uint8_t index = 0U; index < bus->device_count; index++)
        {
            if ((bus->devices[index] != except) && (x430_device_id(bus->devices[index]) == id))
            {
                taken = true;
            }
        }
    }
    return taken;
}

uint32_t x430_bus_millis(const struct x430_bus* bus)
{
    return (bus != NULL) ? x430_millis(bus) : 0U;
}

/** @} */

/** @name Public functions */
/** @{ */

int x430_bus_init(struct x430_bus* bus, const struct x430_bus_config* config)
{
    int result = X430_EINVAL;

    if ((bus != NULL) && (config != NULL) && (config->write != NULL))
    {
        const struct dxl2_link_config link = {
            .write = config->write,
            .write_context = config->write_context,
            .handler = x430_dispatch,
            .handler_context = bus,
        };

        (void) memset(bus, 0, sizeof(*bus));
        bus->config = *config;
        (void) dxl2_link_init(&bus->link, &link);
        bus->last_rx_ms = x430_millis(bus);
        result = X430_OK;
    }
    return result;
}

int x430_bus_attach(struct x430_bus* bus, struct x430_device* device)
{
    int result = X430_EINVAL;

    if ((bus != NULL) && (device != NULL) && device->initialized && (device->bus == NULL)
        && (bus->device_count < X430_MAX_DEVICES)
        && !x430_bus_id_taken(bus, NULL, x430_device_id(device)))
    {
        x430_bus_lock(bus);
        bus->devices[bus->device_count] = device;
        bus->device_count++;
        device->bus = bus;
        x430_bus_unlock(bus);
        result = X430_OK;
    }
    return result;
}

void x430_bus_receive(struct x430_bus* bus, const uint8_t* data, size_t length)
{
    if (bus != NULL)
    {
        x430_bus_lock(bus);
        dxl2_link_receive(&bus->link, data, length);
        x430_bus_unlock(bus);
    }
}

void x430_bus_handle_packet(const struct dxl2_packet* packet, void* bus)
{
    struct x430_bus* self = (struct x430_bus*) bus;

    if (self != NULL)
    {
        x430_bus_lock(self);
        x430_dispatch(packet, self);
        x430_bus_unlock(self);
    }
}

void x430_bus_poll(struct x430_bus* bus)
{
    if (bus != NULL)
    {
        x430_bus_lock(bus);
        {
            const uint32_t idle = x430_millis(bus) - bus->last_rx_ms;

            for (uint8_t index = 0U; index < bus->device_count; index++)
            {
                x430_device_watchdog(bus->devices[index], idle);
            }
        }
        x430_bus_unlock(bus);
    }
}

void x430_bus_lock(struct x430_bus* bus)
{
    const struct x430_port* port = bus->config.port;

    if ((port != NULL) && (port->lock != NULL))
    {
        port->lock(port->context);
    }
}

void x430_bus_unlock(struct x430_bus* bus)
{
    const struct x430_port* port = bus->config.port;

    if ((port != NULL) && (port->unlock != NULL))
    {
        port->unlock(port->context);
    }
}

struct x430_device* x430_bus_find(const struct x430_bus* bus, uint8_t id)
{
    struct x430_device* found = NULL;

    for (uint8_t index = 0U; index < bus->device_count; index++)
    {
        if (x430_device_id(bus->devices[index]) == id)
        {
            found = bus->devices[index];
        }
    }
    return found;
}

/** @} */
