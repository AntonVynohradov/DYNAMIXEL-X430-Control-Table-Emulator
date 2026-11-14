/*******************************************************************************
 * DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
 *
 * Project:     DYNAMIXEL-X430-Control-Table-Emulator
 * Description: Unit tests of the API contract: argument checks, calls before
 *              initialization, failed re-initialization, writer errors,
 *              version and instance sizes.
 *
 * Author:      Anton Vynohradov
 * Email:       avynohradov@systemfromscratch.com
 *
 * License:     Apache License 2.0
 *
 * Copyright (c) 2026 Anton Vynohradov
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

#include <cstdio>
#include <string>

#include "x430_test_support.h"

extern "C" {
#include "x430_internal.h"
}

TEST(X430Api, VersionMacrosMatchTheString)
{
    const std::string expected = std::to_string(X430_VERSION_MAJOR) + "." +
                                 std::to_string(X430_VERSION_MINOR) + "." +
                                 std::to_string(X430_VERSION_PATCH);
    EXPECT_EQ(expected, std::string(X430_VERSION));
}

TEST(X430Api, InstancesHoldTheirStateWithoutWaste)
{
    /* The size macros of x430_types.h are exact: less than one 64-bit word of slack. */
    EXPECT_LE(sizeof(struct x430_bus_ctx), sizeof(struct x430_bus));
    EXPECT_LT(sizeof(struct x430_bus) - sizeof(struct x430_bus_ctx), 8U);
    EXPECT_LE(sizeof(struct x430_device_ctx), sizeof(struct x430_device));
    EXPECT_LT(sizeof(struct x430_device) - sizeof(struct x430_device_ctx), 8U);
    EXPECT_LE(sizeof(struct x430_sim_ctx), sizeof(struct x430_sim));
    EXPECT_LT(sizeof(struct x430_sim) - sizeof(struct x430_sim_ctx), 8U);
    std::printf("[ measured ] host sizeof: bus %zu B, device %zu B, sim %zu B\n",
                sizeof(struct x430_bus), sizeof(struct x430_device), sizeof(struct x430_sim));
}

TEST(X430Api, DeviceInitChecksItsArguments)
{
    struct x430_device device = {};
    struct x430_device_config config = {};
    EXPECT_EQ(x430_device_init(nullptr, &config), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_init(&device, nullptr), X430_ERR_PARAM);
    config.id = 253;
    EXPECT_EQ(x430_device_init(&device, &config), X430_ERR_PARAM);
    config.id = 252;
    EXPECT_EQ(x430_device_init(&device, &config), X430_OK);
    config.id = 0;
    EXPECT_EQ(x430_device_init(&device, &config), X430_OK); /* re-init, not attached */
    EXPECT_EQ(id_of(&device), 1);                           /* model default */
    EXPECT_EQ(reg(&device, X430_ADDR_FIRMWARE_VERSION), (int32_t)X430_FIRMWARE_VERSION);
}

TEST(X430Api, DeviceFunctionsRefuseNullAndUninitializedInstances)
{
    struct x430_device zeroed = {};
    uint8_t data[4] = {0, 0, 0, 0};
    uint8_t error = 0;
    int32_t value = 0;
    uint8_t id = 0;

    for (struct x430_device* device : {static_cast<struct x430_device*>(nullptr), &zeroed})
    {
        const int expected = (device == nullptr) ? X430_ERR_PARAM : X430_ERR_STATE;
        EXPECT_EQ(x430_device_get(device, X430_ADDR_ID, &value), expected);
        EXPECT_EQ(x430_device_set(device, X430_ADDR_ID, 1), expected);
        EXPECT_EQ(x430_device_id(device, &id), expected);
        EXPECT_EQ(x430_device_raise_hardware_error(device, 1U), expected);
        EXPECT_EQ(x430_device_flush_events(device), expected);
        EXPECT_EQ(x430_device_read(device, 0U, 1U, data, &error), expected);
        EXPECT_EQ(x430_device_write(device, 0U, 1U, data, 0U, &error), expected);
        EXPECT_EQ(x430_device_reg_write(device, 0U, 1U, data, 0U, &error), expected);
        EXPECT_EQ(x430_device_action(device, &error), expected);
        EXPECT_EQ(x430_device_reboot(device), expected);
        EXPECT_EQ(x430_device_factory_reset(device, X430_FACTORY_RESET_ALL, &error), expected);
        EXPECT_EQ(x430_device_clear(device, X430_CLEAR_MULTI_TURN, &error), expected);
        EXPECT_EQ(x430_device_backup(device, X430_BACKUP_STORE, &error), expected);
    }
}

TEST(X430Api, DeviceFunctionsRefuseNullOutputs)
{
    struct x430_device device;
    const struct x430_device_config config = {};
    uint8_t data[4] = {0, 0, 0, 0};
    uint8_t error = 0;
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);

    EXPECT_EQ(x430_device_get(&device, X430_ADDR_ID, nullptr), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_id(&device, nullptr), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_read(&device, 0U, 1U, nullptr, &error), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_read(&device, 0U, 1U, data, nullptr), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_write(&device, 0U, 1U, nullptr, 0U, &error), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_write(&device, 0U, 1U, data, 0U, nullptr), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_reg_write(&device, 0U, 1U, nullptr, 0U, &error), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_action(&device, nullptr), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_factory_reset(&device, X430_FACTORY_RESET_ALL, nullptr), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_clear(&device, X430_CLEAR_MULTI_TURN, nullptr), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_backup(&device, X430_BACKUP_STORE, nullptr), X430_ERR_PARAM);
}

TEST(X430Api, RegisterAccessNeedsTheStartOfARegister)
{
    struct x430_device device;
    const struct x430_device_config config = {};
    int32_t value = 99;
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);

    EXPECT_EQ(x430_device_set(&device, 15U, 1), X430_ERR_PARAM); /* reserved */
    EXPECT_EQ(x430_device_get(&device, 15U, &value), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_get(&device, X430_ADDR_MODEL_NUMBER + 1U, &value), X430_ERR_PARAM);
    EXPECT_EQ(x430_device_get(&device, 0xFFFFU, &value), X430_ERR_PARAM); /* beyond the table */
    EXPECT_EQ(value, 99);
    EXPECT_EQ(x430_device_set(&device, X430_ADDR_PRESENT_POSITION, -7), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_PRESENT_POSITION), -7);
}

TEST(X430Api, BusInitChecksItsArguments)
{
    struct x430_bus bus = {};
    struct x430_bus_config config = {};
    EXPECT_EQ(x430_bus_init(nullptr, &config), X430_ERR_PARAM);
    EXPECT_EQ(x430_bus_init(&bus, nullptr), X430_ERR_PARAM);
    EXPECT_EQ(x430_bus_init(&bus, &config), X430_ERR_PARAM); /* no writer */
    config.write = test_write;
    EXPECT_EQ(x430_bus_init(&bus, &config), X430_OK);
}

TEST(X430Api, BusFunctionsRefuseNullAndUninitializedInstances)
{
    struct x430_bus zeroed = {};
    struct x430_device* found = nullptr;
    const uint8_t byte = 0xFFU;
    static struct dxl2_packet packet;

    for (struct x430_bus* bus : {static_cast<struct x430_bus*>(nullptr), &zeroed})
    {
        const int expected = (bus == nullptr) ? X430_ERR_PARAM : X430_ERR_STATE;
        struct x430_device device = {};
        EXPECT_EQ(x430_bus_attach(bus, &device), expected);
        EXPECT_EQ(x430_bus_receive(bus, &byte, 1U), expected);
        EXPECT_EQ(x430_bus_handle_packet(bus, &packet), expected);
        EXPECT_EQ(x430_bus_poll(bus), expected);
        EXPECT_EQ(x430_bus_lock(bus), expected);
        EXPECT_EQ(x430_bus_unlock(bus), expected);
        found = &device;
        EXPECT_EQ(x430_bus_find(bus, 1U, &found), expected);
        EXPECT_EQ(found, nullptr);
    }
}

TEST(X430Api, AttachChecksBothInstances)
{
    struct x430_bus bus;
    struct x430_bus_config bus_config = {};
    bus_config.write = test_write;
    ASSERT_EQ(x430_bus_init(&bus, &bus_config), X430_OK);

    struct x430_device device = {};
    EXPECT_EQ(x430_bus_attach(&bus, nullptr), X430_ERR_PARAM);
    EXPECT_EQ(x430_bus_attach(&bus, &device), X430_ERR_STATE); /* not initialized */

    struct x430_device_config config = {};
    config.id = 1;
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(x430_bus_attach(&bus, &device), X430_OK);
    EXPECT_EQ(x430_bus_attach(&bus, &device), X430_ERR_STATE); /* already attached */

    struct x430_device other;
    ASSERT_EQ(x430_device_init(&other, &config), X430_OK);
    EXPECT_EQ(x430_bus_attach(&bus, &other), X430_ERR_CONFIG); /* ID 1 taken */

    struct x430_device* found = nullptr;
    EXPECT_EQ(x430_bus_find(&bus, 1U, &found), X430_OK);
    EXPECT_EQ(found, &device);
    EXPECT_EQ(x430_bus_find(&bus, 2U, &found), X430_ERR_PARAM); /* no such ID */
    EXPECT_EQ(found, nullptr);
    EXPECT_EQ(x430_bus_find(&bus, 1U, nullptr), X430_ERR_PARAM);
}

TEST(X430Api, RefusedReinitKeepsTheDevice)
{
    struct x430_device device;
    struct x430_device_config config = {};
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    config.id = 253;
    /* Arguments are checked before the state is touched: the device stays usable. */
    EXPECT_EQ(x430_device_init(&device, &config), X430_ERR_PARAM);
    EXPECT_EQ(id_of(&device), 1);
}

TEST(X430Api, ReceiveChecksItsData)
{
    struct x430_bus bus;
    struct x430_bus_config config = {};
    config.write = test_write;
    ASSERT_EQ(x430_bus_init(&bus, &config), X430_OK);
    EXPECT_EQ(x430_bus_receive(&bus, nullptr, 1U), X430_ERR_PARAM);
    EXPECT_EQ(x430_bus_receive(&bus, nullptr, 0U), X430_OK);
    EXPECT_EQ(x430_bus_lock(&bus), X430_OK); /* no port: nothing to lock */
    EXPECT_EQ(x430_bus_unlock(&bus), X430_OK);
    EXPECT_EQ(x430_bus_poll(&bus), X430_OK);
}

TEST_F(X430Test, WriterErrorIsReported)
{
    uint8_t raw[DXL2_MAX_PACKET_SIZE];
    size_t length = 0;
    ASSERT_EQ(dxl2_encode(1U, DXL2_INST_PING, nullptr, 0U, raw, sizeof(raw), &length), 0);

    env.write_fails = true;
    EXPECT_EQ(x430_bus_receive(&bus, raw, length), X430_ERR_HAL);
    static struct dxl2_packet packet;
    packet.id = 1U;
    packet.instruction = DXL2_INST_PING;
    EXPECT_EQ(x430_bus_handle_packet(&bus, &packet), X430_ERR_HAL);

    env.write_fails = false; /* the error is reported once */
    EXPECT_EQ(x430_bus_receive(&bus, raw, length), X430_OK);
    EXPECT_EQ(take_status(env).size(), 1U);
}

#if !X430_FEATURE_SIMULATOR
TEST(X430Api, SimulatorLeftOut)
{
    struct x430_sim sim;
    struct x430_device device;
    EXPECT_EQ(x430_sim_init(&sim, &device, nullptr), X430_ERR_UNSUPPORTED);
    EXPECT_EQ(x430_sim_step(&sim, 1U), X430_ERR_UNSUPPORTED);
    EXPECT_EQ(x430_sim_hooks.write, nullptr);
    EXPECT_EQ(x430_sim_hooks.event, nullptr);
}
#endif

#if !X430_FEATURE_BACKUP
TEST(X430Api, BackupLeftOut)
{
    struct x430_device device;
    const struct x430_device_config config = {};
    uint8_t error = 0;
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(x430_device_backup(&device, X430_BACKUP_STORE, &error), X430_ERR_UNSUPPORTED);
}
#endif
