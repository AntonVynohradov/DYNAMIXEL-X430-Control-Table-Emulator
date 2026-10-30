/**
 * @file x430_bus.h
 * @brief Protocol 2.0 bus: decodes instructions for the emulated devices.
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
 * @defgroup x430_bus Bus
 * @ingroup x430
 * @brief One serial line with up to X430_MAX_DEVICES emulated devices.
 *
 * @details The bus owns a DXL2-C link (parser + encoder). The application
 *          moves received bytes in with @ref x430_bus_receive and provides the
 *          byte writer. The bus answers like the devices of a real DYNAMIXEL
 *          bus:
 *
 *          | Instruction      | Unicast | Broadcast | Notes                                   |
 *          |------------------|:-------:|:---------:|-----------------------------------------|
 *          | PING (0x01)      | yes     | yes       | broadcast: answers in ascending ID order |
 *          | READ (0x02)      | yes     | -         | Status Return Level >= 1                 |
 *          | WRITE (0x03)     | yes     | yes       | Secondary ID: RAM only, no status        |
 *          | REG_WRITE (0x04) | yes     | yes       |                                         |
 *          | ACTION (0x05)    | yes     | yes       |                                         |
 *          | FACTORY_RESET    | yes     | yes       | 0xFF refused on broadcast                |
 *          | REBOOT (0x08)    | yes     | yes       |                                         |
 *          | CLEAR (0x10)     | yes     | yes       | multi-turn clear                         |
 *          | BACKUP (0x20)    | yes     | yes       | X430_FEATURE_BACKUP                      |
 *          | SYNC_READ/WRITE  | -       | yes       |                                         |
 *          | BULK_READ/WRITE  | -       | yes       | X430_FEATURE_BULK                        |
 *          | FAST_SYNC_READ   | -       | yes       | X430_FEATURE_FAST_READ, one packet       |
 *          | FAST_BULK_READ   | -       | yes       | X430_FEATURE_FAST_READ, one packet       |
 *
 *          Group reads answer for the IDs hosted on this bus, in the order of
 *          the request; IDs that are not hosted are skipped. A Fast read
 *          therefore expects every requested ID to be on this bus.
 * @{
 */

#ifndef X430_BUS_H
#define X430_BUS_H

/** @name Includes */
/** @{ */

#include <stddef.h>
#include <stdint.h>

#include "dxl2.h"
#include "x430_config.h"
#include "x430_device.h"
#include "x430_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** @} */

/** @name Type definitions */
/** @{ */

/** @brief Bus configuration passed to @ref x430_bus_init. */
struct x430_bus_config
{
    dxl2_write_fn write;          /**< Writes a complete packet to the line (required). */
    void* write_context;          /**< Context for write. */
    const struct x430_port* port; /**< Platform services, NULL = none. */
};

/** @brief Bus state. Treat the members as private; use the functions below. */
struct x430_bus
{
    struct dxl2_link link;                         /**< DXL2-C parser/encoder. */
    struct x430_bus_config config;                 /**< Copy of the configuration. */
    struct x430_device* devices[X430_MAX_DEVICES]; /**< Attached devices. */
    uint8_t device_count;                          /**< Entries in devices. */
    uint32_t last_rx_ms;                           /**< Time of the last instruction packet. */
    uint8_t data[DXL2_MAX_PARAMETER_SIZE];         /**< Status data scratch buffer. */
#if X430_FEATURE_FAST_READ
    uint8_t raw[DXL2_MAX_PACKET_SIZE]; /**< Fast read response packet. */
#endif
};

/** @} */

/** @name Public function declarations */
/** @{ */

/**
 * @brief Initialize a bus.
 * @retval X430_OK Bus ready.
 * @retval X430_EINVAL NULL argument or no writer.
 */
int x430_bus_init(struct x430_bus* bus, const struct x430_bus_config* config);

/**
 * @brief Attach an initialized device.
 * @retval X430_OK Attached.
 * @retval X430_EINVAL Bus full, device not initialized, attached elsewhere, or ID in use.
 */
int x430_bus_attach(struct x430_bus* bus, struct x430_device* device);

/**
 * @brief Feed received bytes; answers complete instruction packets.
 * @details Takes the bus lock. Call from one context (an RX thread or the main
 *          loop), not from an interrupt handler.
 */
void x430_bus_receive(struct x430_bus* bus, const uint8_t* data, size_t length);

/**
 * @brief Handle one decoded packet; same signature as dxl2_packet_handler_t.
 * @details For applications that run their own DXL2-C link: pass this function
 *          as the link handler with the bus as context. Takes the bus lock.
 */
void x430_bus_handle_packet(const struct dxl2_packet* packet, void* bus);

/** @brief Periodic service (Bus Watchdog). Call every few milliseconds. Takes the lock. */
void x430_bus_poll(struct x430_bus* bus);

/** @brief Take the bus lock (no-op without a port lock). */
void x430_bus_lock(struct x430_bus* bus);

/** @brief Release the bus lock. */
void x430_bus_unlock(struct x430_bus* bus);

/** @brief Attached device with primary ID @p id, NULL if none (lock-free). */
struct x430_device* x430_bus_find(const struct x430_bus* bus, uint8_t id);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* X430_BUS_H */

/** @} */
