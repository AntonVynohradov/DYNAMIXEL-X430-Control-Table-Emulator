/*******************************************************************************
 * DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
 *
 * Project:     DYNAMIXEL-X430-Control-Table-Emulator
 * Description: Protocol 2.0 bus: decodes the instructions of one serial line
 *              (DXL2-C) and answers for the attached devices.
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
/** Largest data length of a status packet (the error byte comes first). */
#define X430_STATUS_DATA_MAX (DXL2_MAX_PARAMETER_SIZE - 1U)
/** Status packet in the TX buffer: offset of the instruction, the first byte stuffing covers. */
#define X430_TX_BODY 7U
/** Status packet in the TX buffer: offset of the error byte. */
#define X430_TX_ERROR 8U
/** Status packet in the TX buffer: offset of the status data. */
#define X430_TX_DATA 9U
/** Length field of a packet: offset in the packet. */
#define X430_TX_LENGTH 5U
/** Byte stuffing: the byte inserted after FF FF FD inside the body. */
#define X430_STUFFING 0xFDU

/* ==============================================================================
 * PRIVATE TYPE DEFINITIONS
 * =============================================================================*/

/** Origin of an instruction for one device. */
enum x430_target
{
    X430_TARGET_PRIMARY = 0, /**< Addressed by its ID: may answer. */
    X430_TARGET_SECONDARY,   /**< Addressed by its Secondary ID: never answers. */
    X430_TARGET_BROADCAST    /**< Broadcast: answers PING only. */
};

/* Alignment probes (see x430_device.c) */
struct x430_align_bus_ctx
{
    char c;
    struct x430_bus_ctx x;
};

struct x430_align_bus
{
    char c;
    struct x430_bus x;
};

_Static_assert(sizeof(struct dxl2_link) == X430_DXL2_LINK_BYTES,
               "X430_DXL2_LINK_BYTES in x430_types.h does not match this DXL2-C version");
_Static_assert(sizeof(struct x430_bus_ctx) <= sizeof(struct x430_bus),
               "X430_BUS_BYTES in x430_types.h is too small for this ABI");
_Static_assert((sizeof(struct x430_align_bus_ctx) - sizeof(struct x430_bus_ctx)) <=
                   (sizeof(struct x430_align_bus) - sizeof(struct x430_bus)),
               "struct x430_bus is not aligned for struct x430_bus_ctx");

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS: HELPERS
 * =============================================================================*/

static uint32_t x430_millis(const struct x430_bus_ctx* m)
{
    const struct x430_port* port = m->config.port;
    uint32_t now = 0U;

    if ((port != NULL) && (port->millis != NULL))
    {
        now = port->millis(port->context);
    }
    return now;
}

static void x430_lock(const struct x430_bus_ctx* m)
{
    const struct x430_port* port = m->config.port;

    if ((port != NULL) && (port->lock != NULL))
    {
        port->lock(port->context);
    }
}

static void x430_unlock(const struct x430_bus_ctx* m)
{
    const struct x430_port* port = m->config.port;

    if ((port != NULL) && (port->unlock != NULL))
    {
        port->unlock(port->context);
    }
}

/**
 * True when the body byte at @p index ends the pattern FF FF FD (Protocol 2.0 byte
 * stuffing). @p index is at least ::X430_TX_DATA: the pattern lies inside the body.
 */
static bool x430_stuffed_at(const uint8_t* tx, uint32_t index)
{
    return (tx[index] == 0xFDU) && (tx[index - 1U] == 0xFFU) && (tx[index - 2U] == 0xFFU);
}

/** Writes @p length bytes of the TX buffer with the application writer; records a failure. */
static void x430_write_tx(struct x430_bus_ctx* m, size_t length)
{
    if (m->config.write(m->config.write_context, m->tx, length) != 0)
    {
        m->write_failed = true;
    }
}

static void x430_delay_status(const struct x430_bus_ctx* m, const struct x430_device* device)
{
    const struct x430_port* port = m->config.port;
    const uint32_t delay =
        (uint32_t)x430_device_byte(device, X430_ADDR_RETURN_DELAY_TIME) * X430_RETURN_DELAY_UNIT_US;

    if ((delay != 0U) && (port != NULL) && (port->delay_us != NULL))
    {
        port->delay_us(port->context, delay);
    }
}

/** Status Return Level needed for an answer: PING always, READs from level 1, the rest at 2. */
#define X430_LEVEL_PING 0U
#define X430_LEVEL_READ 1U
#define X430_LEVEL_ALL 2U

/** Status Return Level rule: true when the device answers at @p needed. */
static bool x430_answers(const struct x430_device* device, uint8_t needed)
{
    return x430_device_byte(device, X430_ADDR_STATUS_RETURN_LEVEL) >= needed;
}

/** Sends the status packet whose @p length data bytes are at ::X430_TX_DATA (none on error). */
static void x430_send_status(struct x430_bus_ctx* m, const struct x430_device* device, uint8_t id,
                             uint8_t error, uint16_t length)
{
    uint16_t count = 0U;
    size_t total = 0U;

    if (error == DXL2_ERR_NONE)
    {
        count = length;
    }
    m->tx[X430_TX_ERROR] = (uint8_t)(error | x430_device_alert(device));
    total = x430_frame_status(m->tx, sizeof(m->tx), id, count);
    x430_delay_status(m, device);
    x430_write_tx(m, total);
}

static bool x430_magic_ok(const uint8_t* parameters, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
    return (parameters[1] == b1) && (parameters[2] == b2) && (parameters[3] == b3) &&
           (parameters[4] == b4);
}

static uint8_t x430_id_of(const struct x430_device* device)
{
    return x430_device_byte(device, X430_ADDR_ID);
}

/** Attached device with primary ID @p id, NULL if none. */
static struct x430_device* x430_find(const struct x430_bus_ctx* m, uint8_t id)
{
    struct x430_device* found = NULL;

    for (uint8_t index = 0U; index < m->device_count; index++)
    {
        if (x430_id_of(m->devices[index]) == id)
        {
            found = m->devices[index];
        }
    }
    return found;
}

/** Read for a group instruction; sends the status of one device. True when it answered. */
static bool x430_group_read(struct x430_bus_ctx* m, struct x430_device* device, uint16_t address,
                            uint16_t length)
{
    const bool answered = x430_answers(device, X430_LEVEL_READ);

    if (answered)
    {
        uint8_t error = DXL2_ERR_NONE;

        (void)x430_device_read(device, address, length, &m->tx[X430_TX_DATA], &error);
        x430_send_status(m, device, x430_id_of(device), error, length);
    }
    return answered;
}

/* ------------------------------------------------------------------------------
 * Turn-taking of SYNC_READ and BULK_READ on a shared bus. On one wire a device
 * answers only after the status packet of the ID listed before it in the request
 * (X series behaviour); devices of this bus answer one after the other, and a device
 * after an ID of another bus waits until that status packet has been received.
 * ------------------------------------------------------------------------------*/

/** Ends the pending read: its remaining hosted devices stay silent. */
static void x430_turn_cancel(struct x430_bus_ctx* m)
{
    m->turn_count = 0U;
    m->turn_next = 0U;
}

/**
 * Adds one entry of a SYNC_READ / BULK_READ request in request order. @p after holds
 * the turn of this entry (::X430_TURN_NOW or the ID before it) and is updated for the
 * next entry. An entry of another bus, or one this bus cannot take, makes the next
 * entry wait for its status packet.
 */
static void x430_turn_add(struct x430_bus_ctx* m, uint8_t id, uint16_t address, uint16_t length,
                          uint8_t* after)
{
    struct x430_device* device = x430_find(m, id);

    if ((device != NULL) && (m->turn_count < X430_MAX_DEVICES_U))
    {
        struct x430_turn* turn = &m->turns[m->turn_count];

        turn->device = device;
        turn->address = address;
        turn->length = length;
        turn->after = *after;
        m->turn_count++;
        *after = X430_TURN_NOW;
    }
    else
    {
        *after = id;
    }
}

/** Answers the pending entries whose turn has come, up to one that waits for another bus. */
static void x430_turn_run(struct x430_bus_ctx* m)
{
    bool waiting = false;

    while (!waiting && (m->turn_next < m->turn_count))
    {
        const uint8_t next = m->turn_next;

        if (m->turns[next].after != X430_TURN_NOW)
        {
            waiting = true;
        }
        else if (x430_group_read(m, m->turns[next].device, m->turns[next].address,
                                 m->turns[next].length))
        {
            m->turn_next++;
        }
        else
        {
            /* Silent device (Status Return Level 0): no device after it hears its turn. */
            x430_turn_cancel(m);
        }
    }
    m->turn_ms = x430_millis(m);
}

/** Status packet of device @p id received from the bus (another bus, or an echo). */
static void x430_turn_heard(struct x430_bus_ctx* m, uint8_t id)
{
    if (m->turn_next < m->turn_count)
    {
        struct x430_turn* turn = &m->turns[m->turn_next];

        if ((x430_millis(m) - m->turn_ms) > X430_TURN_TIMEOUT_MS_U)
        {
            /* The read stalled (a device before is missing): stay silent. */
            x430_turn_cancel(m);
        }
        else if (turn->after == id)
        {
            turn->after = X430_TURN_NOW;
            x430_turn_run(m);
        }
        else
        {
            /* An earlier device of the read answered: the read makes progress. */
            m->turn_ms = x430_millis(m);
        }
    }
}

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS: SINGLE-DEVICE INSTRUCTIONS
 * =============================================================================*/

/** FACTORY_RESET parameter check; true when the reset may run after the answer. */
static bool x430_factory_reset_check(const struct dxl2_packet* packet, enum x430_target target,
                                     uint8_t* error)
{
    const uint8_t option = packet->parameters[0];
    bool run = false;

    if (packet->parameter_length != 1U)
    {
        *error = DXL2_ERR_DATA_LENGTH;
    }
    else if ((option != X430_FACTORY_RESET_ALL) && (option != X430_FACTORY_RESET_EXCEPT_ID) &&
             (option != X430_FACTORY_RESET_EXCEPT_ID_BAUD))
    {
        *error = DXL2_ERR_DATA_RANGE;
    }
    else if (((target == X430_TARGET_BROADCAST) && (option == X430_FACTORY_RESET_ALL)) ||
             (target == X430_TARGET_SECONDARY))
    {
        /* Reset All is refused on the broadcast ID; the Secondary ID cannot touch EEPROM. */
        *error = DXL2_ERR_ACCESS;
    }
    else
    {
        run = true;
    }
    return run;
}

/** Executes an instruction for one device; returns the status data length. */
static uint16_t x430_instruction(struct x430_bus_ctx* m, struct x430_device* device,
                                 const struct dxl2_packet* packet, enum x430_target target,
                                 uint8_t* error, bool* answer_first)
{
    const uint16_t count = packet->parameter_length;
    const uint8_t* parameters = packet->parameters;
    uint8_t options = 0U;
    uint16_t length = 0U;

    if (target == X430_TARGET_SECONDARY)
    {
        options = X430_WRITE_SECONDARY;
    }

    switch (packet->instruction)
    {
        case DXL2_INST_PING:
            dxl2_put_u16(&m->tx[X430_TX_DATA],
                         (uint16_t)x430_device_value(device, X430_ADDR_MODEL_NUMBER));
            m->tx[X430_TX_DATA + 2U] = x430_device_byte(device, X430_ADDR_FIRMWARE_VERSION);
            length = 3U;
            break;
        case DXL2_INST_READ:
            if (count != X430_READ_PARAMS)
            {
                *error = DXL2_ERR_DATA_LENGTH;
            }
            else
            {
                length = dxl2_get_u16(&parameters[2]);
                if (length > X430_STATUS_DATA_MAX)
                {
                    *error = DXL2_ERR_DATA_LENGTH;
                }
                else
                {
                    (void)x430_device_read(device, dxl2_get_u16(parameters), length,
                                           &m->tx[X430_TX_DATA], error);
                }
            }
            break;
        case DXL2_INST_WRITE:
            if (count <= X430_WRITE_HEADER)
            {
                *error = DXL2_ERR_DATA_LENGTH;
            }
            else
            {
                (void)x430_device_write(device, dxl2_get_u16(parameters),
                                        (uint16_t)(count - X430_WRITE_HEADER),
                                        &parameters[X430_WRITE_HEADER], options, error);
            }
            break;
        case DXL2_INST_REG_WRITE:
            if (count <= X430_WRITE_HEADER)
            {
                *error = DXL2_ERR_DATA_LENGTH;
            }
            else
            {
                (void)x430_device_reg_write(device, dxl2_get_u16(parameters),
                                            (uint16_t)(count - X430_WRITE_HEADER),
                                            &parameters[X430_WRITE_HEADER], options, error);
            }
            break;
        case DXL2_INST_ACTION:
            if (count == 0U)
            {
                (void)x430_device_action(device, error);
            }
            else
            {
                *error = DXL2_ERR_DATA_LENGTH;
            }
            break;
        case DXL2_INST_REBOOT:
            if (count == 0U)
            {
                *answer_first = true;
            }
            else
            {
                *error = DXL2_ERR_DATA_LENGTH;
            }
            break;
        case DXL2_INST_FACTORY_RESET:
            *answer_first = x430_factory_reset_check(packet, target, error);
            break;
        case DXL2_INST_CLEAR:
            if ((count == X430_OPTION_PARAMS) &&
                x430_magic_ok(parameters, 0x44U, 0x58U, 0x4CU, 0x22U))
            {
                (void)x430_device_clear(device, parameters[0], error);
            }
            else
            {
                *error = DXL2_ERR_DATA_RANGE;
            }
            break;
#if X430_FEATURE_BACKUP
        case DXL2_INST_BACKUP:
            if ((count == X430_OPTION_PARAMS) &&
                x430_magic_ok(parameters, (uint8_t)'C', (uint8_t)'T', (uint8_t)'R', (uint8_t)'L'))
            {
                (void)x430_device_backup(device, parameters[0], error);
            }
            else
            {
                *error = DXL2_ERR_DATA_RANGE;
            }
            break;
#endif
        default:
            *error = DXL2_ERR_INSTRUCTION;
            break;
    }
    return length;
}

static void x430_execute(struct x430_bus_ctx* m, struct x430_device* device,
                         const struct dxl2_packet* packet, enum x430_target target)
{
    const uint8_t id = x430_id_of(device);
    uint8_t error = DXL2_ERR_NONE;
    bool answer_first = false; /* REBOOT/FACTORY_RESET answer before they act. */
    const uint16_t length = x430_instruction(m, device, packet, target, &error, &answer_first);
    bool answer = false;

    if (target == X430_TARGET_PRIMARY)
    {
        uint8_t needed = X430_LEVEL_ALL;

        if (packet->instruction == DXL2_INST_PING)
        {
            needed = X430_LEVEL_PING;
        }
        else if (packet->instruction == DXL2_INST_READ)
        {
            needed = X430_LEVEL_READ;
        }
        else
        {
            /* Every other instruction answers at level 2 only. */
        }
        answer = x430_answers(device, needed);
    }
    else if (target == X430_TARGET_BROADCAST)
    {
        answer = packet->instruction == DXL2_INST_PING;
    }
    else
    {
        /* Secondary ID: never answers. */
    }
    if (answer)
    {
        x430_send_status(m, device, id, error, length);
    }

    /* x430_instruction() sets answer_first only for an instruction without error */
    if (answer_first)
    {
        if (packet->instruction == DXL2_INST_REBOOT)
        {
            (void)x430_device_reboot(device);
        }
        else
        {
            (void)x430_device_factory_reset(device, packet->parameters[0], &error);
        }
    }
    (void)x430_device_flush_events(device);
}

/* ==============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS: GROUP INSTRUCTIONS
 * =============================================================================*/

static void x430_sync_read(struct x430_bus_ctx* m, const struct dxl2_packet* packet)
{
    if (packet->parameter_length > X430_READ_PARAMS)
    {
        const uint16_t address = dxl2_get_u16(packet->parameters);
        const uint16_t length = dxl2_get_u16(&packet->parameters[2]);

        if ((length != 0U) && (length <= X430_STATUS_DATA_MAX))
        {
            uint8_t after = X430_TURN_NOW;

            for (uint16_t index = X430_READ_PARAMS; index < packet->parameter_length; index++)
            {
                x430_turn_add(m, packet->parameters[index], address, length, &after);
            }
            x430_turn_run(m);
        }
    }
}

static void x430_sync_write(struct x430_bus_ctx* m, const struct dxl2_packet* packet)
{
    if (packet->parameter_length > X430_READ_PARAMS)
    {
        const uint16_t address = dxl2_get_u16(packet->parameters);
        const uint16_t length = dxl2_get_u16(&packet->parameters[2]);
        const uint32_t entry = (uint32_t)length + 1U;
        const uint32_t body = (uint32_t)packet->parameter_length - X430_READ_PARAMS;

        if ((length != 0U) && ((body % entry) == 0U))
        {
            for (uint32_t index = X430_READ_PARAMS; index < packet->parameter_length;
                 index += entry)
            {
                struct x430_device* device = x430_find(m, packet->parameters[index]);

                if (device != NULL)
                {
                    uint8_t error = DXL2_ERR_NONE;

                    (void)x430_device_write(device, address, length,
                                            &packet->parameters[index + 1U], 0U, &error);
                    (void)x430_device_flush_events(device);
                }
            }
        }
    }
}

#if X430_FEATURE_BULK

static void x430_bulk_read(struct x430_bus_ctx* m, const struct dxl2_packet* packet)
{
    if ((packet->parameter_length != 0U) &&
        ((packet->parameter_length % X430_BULK_READ_ENTRY) == 0U))
    {
        uint8_t after = X430_TURN_NOW;

        for (uint16_t index = 0U; index < packet->parameter_length;
             index = (uint16_t)(index + X430_BULK_READ_ENTRY))
        {
            const uint8_t id = packet->parameters[index];
            const uint16_t length = dxl2_get_u16(&packet->parameters[index + 3U]);

            if ((length != 0U) && (length <= X430_STATUS_DATA_MAX))
            {
                x430_turn_add(m, id, dxl2_get_u16(&packet->parameters[index + 1U]), length, &after);
            }
            else
            {
                /* Not answered: the entry after it waits for a status that never comes. */
                after = id;
            }
        }
        x430_turn_run(m);
    }
}

static void x430_bulk_write(struct x430_bus_ctx* m, const struct dxl2_packet* packet)
{
    uint32_t index = 0U;
    bool valid = true;

    /* Validate the whole packet first: a malformed entry voids the instruction. */
    while (valid && (index < packet->parameter_length))
    {
        if ((index + X430_BULK_READ_ENTRY) > packet->parameter_length)
        {
            valid = false;
        }
        else
        {
            const uint16_t length = dxl2_get_u16(&packet->parameters[index + 3U]);

            valid = (length != 0U) &&
                    ((index + X430_BULK_READ_ENTRY + length) <= packet->parameter_length);
            index += X430_BULK_READ_ENTRY + (uint32_t)length;
        }
    }

    index = 0U;
    while (valid && (index < packet->parameter_length))
    {
        struct x430_device* device = x430_find(m, packet->parameters[index]);
        const uint16_t length = dxl2_get_u16(&packet->parameters[index + 3U]);

        if (device != NULL)
        {
            uint8_t error = DXL2_ERR_NONE;

            (void)x430_device_write(device, dxl2_get_u16(&packet->parameters[index + 1U]), length,
                                    &packet->parameters[index + X430_BULK_READ_ENTRY], 0U, &error);
            (void)x430_device_flush_events(device);
        }
        index += X430_BULK_READ_ENTRY + (uint32_t)length;
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
static void x430_fast_read(struct x430_bus_ctx* m, const struct dxl2_packet* packet, bool bulk)
{
    uint16_t ends[X430_MAX_DEVICES];
    const struct x430_device* first = NULL;
    uint32_t position = X430_FAST_HEADER;
    uint8_t segments = 0U;
    uint16_t step = 1U;
    uint16_t start = X430_READ_PARAMS;
    bool valid = packet->parameter_length > X430_READ_PARAMS;

    if (bulk)
    {
        step = X430_BULK_READ_ENTRY;
        start = 0U;
        valid = (packet->parameter_length != 0U) &&
                ((packet->parameter_length % X430_BULK_READ_ENTRY) == 0U);
    }

    uint16_t index = start;

    while (valid && (index < packet->parameter_length))
    {
        const uint8_t id = packet->parameters[index];
        uint16_t address = dxl2_get_u16(packet->parameters);
        uint16_t length = dxl2_get_u16(&packet->parameters[2]);
        struct x430_device* device = x430_find(m, id);

        if (bulk)
        {
            address = dxl2_get_u16(&packet->parameters[index + 1U]);
            length = dxl2_get_u16(&packet->parameters[index + 3U]);
        }

        if ((length == 0U) || (device == NULL) ||
            ((position + length + X430_FAST_SEGMENT_OVERHEAD) > DXL2_MAX_PACKET_SIZE))
        {
            /* Malformed, or an ID of another bus: the answer of several buses would have
               to be one chained packet, which this bus cannot build. It stays silent, and
               the host falls back to SYNC_READ / BULK_READ. */
            valid = false;
        }
        else if ((segments < X430_MAX_DEVICES_U) && x430_answers(device, X430_LEVEL_READ))
        {
            uint8_t error = DXL2_ERR_NONE;
            uint8_t* data = &m->tx[position + 2U];

            (void)x430_device_read(device, address, length, data, &error);
            if (error != DXL2_ERR_NONE)
            {
                x430_fill(data, 0U, length);
            }
            m->tx[position] = (uint8_t)(error | x430_device_alert(device));
            m->tx[position + 1U] = id;
            ends[segments] = (uint16_t)(position + 2U + length);
            position = (uint32_t)ends[segments] + 2U;
            segments++;
            if (first == NULL)
            {
                first = device;
            }
        }
        else
        {
            /* Silent device (Status Return Level 0), or listed more than once: skipped. */
        }
        index = (uint16_t)(index + step);
    }

    if (valid && (first != NULL))
    {
        m->tx[0] = 0xFFU;
        m->tx[1] = 0xFFU;
        m->tx[2] = 0xFDU;
        m->tx[3] = 0x00U;
        m->tx[4] = DXL2_BROADCAST_ID;
        dxl2_put_u16(&m->tx[5], (uint16_t)(position - 7U));
        m->tx[7] = DXL2_INST_STATUS;
        for (uint8_t segment = 0U; segment < segments; segment++)
        {
            dxl2_put_u16(&m->tx[ends[segment]], dxl2_crc16(m->tx, ends[segment]));
        }
        x430_delay_status(m, first);
        x430_write_tx(m, (size_t)position);
    }
}

#endif /* X430_FEATURE_FAST_READ */

/** Broadcast PING: every device answers, lowest ID first. */
static void x430_broadcast_ping(struct x430_bus_ctx* m, const struct dxl2_packet* packet)
{
    int16_t last = -1;

    for (uint8_t round = 0U; round < m->device_count; round++)
    {
        struct x430_device* next = NULL;
        int16_t next_id = 256;

        for (uint8_t index = 0U; index < m->device_count; index++)
        {
            struct x430_device* device = m->devices[index];
            const int16_t id = (int16_t)x430_id_of(device);

            if ((id > last) && (id < next_id))
            {
                next = device;
                next_id = id;
            }
        }
        /* FACTORY_RESET (all) may give two devices the same ID: the second one stays silent. */
        if (next != NULL)
        {
            last = next_id;
            x430_execute(m, next, packet, X430_TARGET_BROADCAST);
        }
    }
}

static void x430_broadcast(struct x430_bus_ctx* m, const struct dxl2_packet* packet)
{
    switch (packet->instruction)
    {
        case DXL2_INST_PING:
            x430_broadcast_ping(m, packet);
            break;
        case DXL2_INST_SYNC_READ:
            x430_sync_read(m, packet);
            break;
        case DXL2_INST_SYNC_WRITE:
            x430_sync_write(m, packet);
            break;
#if X430_FEATURE_BULK
        case DXL2_INST_BULK_READ:
            x430_bulk_read(m, packet);
            break;
        case DXL2_INST_BULK_WRITE:
            x430_bulk_write(m, packet);
            break;
#endif
#if X430_FEATURE_FAST_READ
        case DXL2_INST_FAST_SYNC_READ:
            x430_fast_read(m, packet, false);
            break;
        case DXL2_INST_FAST_BULK_READ:
            x430_fast_read(m, packet, true);
            break;
#endif
        case DXL2_INST_WRITE:
        case DXL2_INST_REG_WRITE:
        case DXL2_INST_ACTION:
        case DXL2_INST_FACTORY_RESET:
        case DXL2_INST_REBOOT:
        case DXL2_INST_CLEAR:
        case DXL2_INST_BACKUP:
            for (uint8_t index = 0U; index < m->device_count; index++)
            {
                x430_execute(m, m->devices[index], packet, X430_TARGET_BROADCAST);
            }
            break;
        default:
            /* READ and unknown instructions on the broadcast ID: no device answers. */
            break;
    }
}

/** Packet dispatcher, called with the bus lock held. */
static void x430_dispatch(struct x430_bus_ctx* m, const struct dxl2_packet* packet)
{
    if (packet->instruction != DXL2_INST_STATUS)
    {
        const uint32_t now = x430_millis(m);

        /* A new instruction ends a read that still waits for its turn. */
        x430_turn_cancel(m);
        for (uint8_t index = 0U; index < m->device_count; index++)
        {
            x430_device_watchdog(m->devices[index], now - m->last_rx_ms);
        }
        m->last_rx_ms = now;

        if (packet->id == DXL2_BROADCAST_ID)
        {
            x430_broadcast(m, packet);
        }
        else
        {
            /* The primary ID answers; devices sharing it as Secondary ID act silently. */
            for (uint8_t index = 0U; index < m->device_count; index++)
            {
                struct x430_device* device = m->devices[index];
                const uint8_t secondary = x430_device_byte(device, X430_ADDR_SECONDARY_ID);

                if (x430_id_of(device) == packet->id)
                {
                    x430_execute(m, device, packet, X430_TARGET_PRIMARY);
                }
                else if ((secondary == packet->id) && (secondary < X430_SECONDARY_ID_DISABLED))
                {
                    x430_execute(m, device, packet, X430_TARGET_SECONDARY);
                }
                else
                {
                    /* Not addressed. */
                }
            }
        }
    }
    else
    {
        /* Status packet of another device on the shared bus (or the echo of our own). */
        x430_turn_heard(m, packet->id);
    }
}

/** Packet handler given to DXL2-C. */
static void x430_handler(const struct dxl2_packet* packet, void* context)
{
    /* cppcheck-suppress misra-c2012-11.5 ; DEV-02 callback context */
    x430_dispatch((struct x430_bus_ctx*)context, packet);
}

/** X430_OK for an initialized bus, else the status code to return. */
static int x430_check(const struct x430_bus* bus)
{
    const struct x430_bus_ctx* m = x430_bus_ctx_of_const(bus);
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

/** Status of an entry point that may have sent status packets. */
static int x430_take_write_result(struct x430_bus_ctx* m)
{
    int result = X430_OK;

    if (m->write_failed)
    {
        m->write_failed = false;
        result = X430_ERR_HAL;
    }
    return result;
}

/* ==============================================================================
 * INTERNAL FUNCTIONS (x430_internal.h)
 * =============================================================================*/

size_t x430_frame_status(uint8_t* tx, size_t capacity, uint8_t id, uint16_t count)
{
    uint32_t end = X430_TX_DATA + (uint32_t)count;
    uint32_t stuffing = 0U;
    uint32_t from = 0U;
    uint32_t to = 0U;

    tx[X430_TX_BODY] = DXL2_INST_STATUS;
    for (uint32_t index = X430_TX_DATA; index < end; index++)
    {
        if (x430_stuffed_at(tx, index))
        {
            stuffing++;
        }
    }
    if (((size_t)end + stuffing + 2U) > capacity)
    {
        /* The stuffed data does not fit: Data Length Error without data (alert bit kept). */
        tx[X430_TX_ERROR] =
            (uint8_t)((tx[X430_TX_ERROR] & DXL2_ERR_ALERT_BIT) | DXL2_ERR_DATA_LENGTH);
        end = X430_TX_DATA;
        stuffing = 0U;
    }

    /* Moves each byte up by the stuffing still to insert below it; reads stay below writes. */
    from = end;
    to = end + stuffing;
    while (to > from)
    {
        from--;
        if (x430_stuffed_at(tx, from))
        {
            to--;
            tx[to] = X430_STUFFING;
        }
        to--;
        tx[to] = tx[from];
    }
    tx[0] = 0xFFU;
    tx[1] = 0xFFU;
    tx[2] = 0xFDU;
    tx[3] = 0x00U;
    tx[4] = id;
    dxl2_put_u16(&tx[X430_TX_LENGTH], (uint16_t)((end + stuffing + 2U) - X430_TX_BODY));
    dxl2_put_u16(&tx[end + stuffing], dxl2_crc16(tx, (size_t)end + stuffing));
    return (size_t)end + stuffing + 2U;
}

bool x430_bus_id_taken(const struct x430_bus* bus, const struct x430_device* except, uint8_t id)
{
    bool taken = false;

    if (bus != NULL)
    {
        const struct x430_bus_ctx* m = x430_bus_ctx_of_const(bus);

        for (uint8_t index = 0U; index < m->device_count; index++)
        {
            if ((m->devices[index] != except) && (x430_id_of(m->devices[index]) == id))
            {
                taken = true;
            }
        }
    }
    return taken;
}

uint32_t x430_bus_millis(const struct x430_bus* bus)
{
    uint32_t now = 0U;

    if (bus != NULL)
    {
        now = x430_millis(x430_bus_ctx_of_const(bus));
    }
    return now;
}

/* ==============================================================================
 * PUBLIC FUNCTION IMPLEMENTATIONS
 * =============================================================================*/

int x430_bus_init(struct x430_bus* bus, const struct x430_bus_config* config)
{
    struct x430_bus_ctx* m = x430_bus_ctx_of(bus);
    int result = X430_OK;

    if ((m == NULL) || (config == NULL))
    {
        result = X430_ERR_PARAM;
    }
    else if (config->write == NULL)
    {
        result = X430_ERR_PARAM;
    }
    else
    {
        /* Receive only: the bus frames its status packets in tx[] itself, so that no
           packet-sized buffer is needed on the stack (dxl2_link_send() has one). */
        const struct dxl2_link_config link = {
            .write = NULL,
            .write_context = NULL,
            .handler = x430_handler,
            .handler_context = m,
        };

        /* Field by field (no temporary copy of the context); devices[] is read only below
           device_count, tx[] is written before it is sent. */
        m->config = *config;
        m->device_count = 0U;
        m->write_failed = false;
        x430_turn_cancel(m);
        (void)dxl2_link_init(&m->link, &link);
        m->last_rx_ms = x430_millis(m);
        m->turn_ms = m->last_rx_ms;
        m->ready = X430_READY;
    }
    return result;
}

int x430_bus_attach(struct x430_bus* bus, struct x430_device* device)
{
    int result = x430_check(bus);

    if ((result == X430_OK) && (device == NULL))
    {
        result = X430_ERR_PARAM;
    }
    else if (result == X430_OK)
    {
        struct x430_bus_ctx* m = x430_bus_ctx_of(bus);

        if (!x430_device_ready(device) || (x430_device_bus(device) != NULL))
        {
            result = X430_ERR_STATE;
        }
        else if ((m->device_count >= X430_MAX_DEVICES_U) ||
                 x430_bus_id_taken(bus, NULL, x430_id_of(device)))
        {
            result = X430_ERR_CONFIG;
        }
        else
        {
            x430_lock(m);
            m->devices[m->device_count] = device;
            m->device_count++;
            x430_device_attach(device, bus);
            x430_unlock(m);
        }
    }
    else
    {
        /* Bus not usable: result from x430_check(). */
    }
    return result;
}

int x430_bus_receive(struct x430_bus* bus, const uint8_t* data, size_t length)
{
    int result = x430_check(bus);

    if ((result == X430_OK) && (data == NULL) && (length != 0U))
    {
        result = X430_ERR_PARAM;
    }
    else if (result == X430_OK)
    {
        struct x430_bus_ctx* m = x430_bus_ctx_of(bus);

        x430_lock(m);
        dxl2_link_receive(&m->link, data, length);
        result = x430_take_write_result(m);
        x430_unlock(m);
    }
    else
    {
        /* Bus not usable: result from x430_check(). */
    }
    return result;
}

int x430_bus_handle_packet(struct x430_bus* bus, const struct dxl2_packet* packet)
{
    int result = x430_check(bus);

    if ((result == X430_OK) && (packet == NULL))
    {
        result = X430_ERR_PARAM;
    }
    else if (result == X430_OK)
    {
        struct x430_bus_ctx* m = x430_bus_ctx_of(bus);

        x430_lock(m);
        x430_dispatch(m, packet);
        result = x430_take_write_result(m);
        x430_unlock(m);
    }
    else
    {
        /* Bus not usable: result from x430_check(). */
    }
    return result;
}

int x430_bus_poll(struct x430_bus* bus)
{
    int result = x430_check(bus);

    if (result == X430_OK)
    {
        struct x430_bus_ctx* m = x430_bus_ctx_of(bus);
        uint32_t idle = 0U;

        x430_lock(m);
        idle = x430_millis(m) - m->last_rx_ms;
        for (uint8_t index = 0U; index < m->device_count; index++)
        {
            x430_device_watchdog(m->devices[index], idle);
        }
        x430_unlock(m);
    }
    return result;
}

int x430_bus_lock(struct x430_bus* bus)
{
    int result = x430_check(bus);

    if (result == X430_OK)
    {
        x430_lock(x430_bus_ctx_of_const(bus));
    }
    return result;
}

int x430_bus_unlock(struct x430_bus* bus)
{
    int result = x430_check(bus);

    if (result == X430_OK)
    {
        x430_unlock(x430_bus_ctx_of_const(bus));
    }
    return result;
}

int x430_bus_find(const struct x430_bus* bus, uint8_t id, struct x430_device** device)
{
    int result = x430_check(bus);

    if (device == NULL)
    {
        result = X430_ERR_PARAM;
    }
    else
    {
        *device = NULL;
        if (result == X430_OK)
        {
            *device = x430_find(x430_bus_ctx_of_const(bus), id);
            if (*device == NULL)
            {
                result = X430_ERR_PARAM;
            }
        }
    }
    return result;
}
