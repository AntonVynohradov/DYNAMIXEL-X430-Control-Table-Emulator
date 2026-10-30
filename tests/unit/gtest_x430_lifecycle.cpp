/**
 * @file gtest_x430_lifecycle.cpp
 * @brief Init, persistence, reboot, factory reset, backup, errors, watchdog.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "x430_test_support.h"

TEST(X430Init, RejectsBadArguments)
{
    struct x430_device device;
    struct x430_device_config config = {};
    EXPECT_EQ(x430_device_init(nullptr, &config), X430_EINVAL);
    EXPECT_EQ(x430_device_init(&device, nullptr), X430_EINVAL);
    config.id = 253;
    EXPECT_EQ(x430_device_init(&device, &config), X430_EINVAL);
    config.id = 0;
    EXPECT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(x430_device_id(&device), 1); /* model default */
    EXPECT_EQ(x430_device_get(&device, X430_ADDR_FIRMWARE_VERSION), X430_FIRMWARE_VERSION);
    EXPECT_EQ(x430_device_set(&device, 15, 1), X430_EINVAL); /* reserved */
    EXPECT_EQ(x430_device_get(&device, 15), 0);
    EXPECT_EQ(x430_device_get(&device, X430_ADDR_MODEL_NUMBER + 1), 0); /* not a register start */

    struct x430_bus bus;
    struct x430_bus_config bus_config = {};
    EXPECT_EQ(x430_bus_init(nullptr, &bus_config), X430_EINVAL);
    EXPECT_EQ(x430_bus_init(&bus, nullptr), X430_EINVAL);
    EXPECT_EQ(x430_bus_init(&bus, &bus_config), X430_EINVAL); /* no writer */
    bus_config.write = test_write;
    EXPECT_EQ(x430_bus_init(&bus, &bus_config), X430_OK);
    EXPECT_EQ(x430_bus_attach(&bus, nullptr), X430_EINVAL);
    EXPECT_EQ(x430_bus_attach(nullptr, &device), X430_EINVAL);
    EXPECT_EQ(x430_bus_attach(&bus, &device), X430_OK);
    EXPECT_EQ(x430_bus_attach(&bus, &device), X430_EINVAL); /* already attached */
    struct x430_device other;
    struct x430_device_config other_config = {};
    other_config.id = 1;
    ASSERT_EQ(x430_device_init(&other, &other_config), X430_OK);
    EXPECT_EQ(x430_bus_attach(&bus, &other), X430_EINVAL); /* ID 1 taken */
    std::memset(&other, 0, sizeof(other));
    EXPECT_EQ(x430_bus_attach(&bus, &other), X430_EINVAL); /* not initialized */
    x430_bus_receive(nullptr, nullptr, 0);
    x430_bus_poll(nullptr);
    EXPECT_EQ(x430_bus_find(&bus, 1), &device);
}

TEST(X430Init, BusIsFullAtMaxDevices)
{
    static struct x430_device devices[X430_MAX_DEVICES + 1];
    struct x430_bus bus;
    struct x430_bus_config bus_config = {};
    bus_config.write = test_write;
    ASSERT_EQ(x430_bus_init(&bus, &bus_config), X430_OK);
    for (unsigned i = 0; i <= X430_MAX_DEVICES; i++)
    {
        struct x430_device_config config = {};
        config.id = (uint8_t) (i + 1U);
        ASSERT_EQ(x430_device_init(&devices[i], &config), X430_OK);
        EXPECT_EQ(x430_bus_attach(&bus, &devices[i]),
                  (i < X430_MAX_DEVICES) ? X430_OK : X430_EINVAL);
    }
}

TEST_F(X430Test, EepromIsStoredAndLoadedAtInit)
{
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 7), 0);
    EXPECT_EQ(env.nvm_stores, 1);
    ASSERT_EQ(env.nvm[X430_NVM_EEPROM].size(), X430_NVM_EEPROM_SIZE);
    EXPECT_EQ(write32(1, X430_ADDR_PROFILE_VELOCITY, 3), 0); /* RAM: no store */
    EXPECT_EQ(env.nvm_stores, 1);

    struct x430_device again;
    const struct x430_device_config config = {1U, 0U, 0U, &test_hooks, &env};
    ASSERT_EQ(x430_device_init(&again, &config), X430_OK);
    EXPECT_EQ(x430_device_get(&again, X430_ADDR_RETURN_DELAY_TIME), 7);

    /* A corrupted block is ignored. */
    env.nvm[X430_NVM_EEPROM][20] ^= 0xFF;
    ASSERT_EQ(x430_device_init(&again, &config), X430_OK);
    EXPECT_EQ(x430_device_get(&again, X430_ADDR_RETURN_DELAY_TIME), 250);
}

TEST_F(X430Test, StoredIdWinsOverConfiguredId)
{
    EXPECT_EQ(write8(1, X430_ADDR_ID, 5), 0);
    struct x430_device again;
    const struct x430_device_config config = {1U, 0U, 0U, &test_hooks, &env};
    ASSERT_EQ(x430_device_init(&again, &config), X430_OK);
    EXPECT_EQ(x430_device_id(&again), 5);
}

TEST_F(X430Test, StoreFailureIsResultFail)
{
    env.nvm_store_fails = true;
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 7), DXL2_ERR_RESULT_FAIL);
}

TEST_F(X430Test, RebootResetsRamAndAnswersFirst)
{
    EXPECT_EQ(write32(1, X430_ADDR_PROFILE_VELOCITY, 99), 0);
    x430_device_raise_hardware_error(&dev[0], X430_HW_ERROR_INPUT_VOLTAGE);
    env.events.clear();
    auto status = transact(1, DXL2_INST_REBOOT, {});
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].error, DXL2_ERR_ALERT_BIT); /* status before the reboot */
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 0);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_HARDWARE_ERROR_STATUS), 0);
    ASSERT_EQ(env.events.size(), 1U);
    EXPECT_EQ(env.events[0], X430_EVENT_REBOOT);
    EXPECT_EQ(transact(1, DXL2_INST_REBOOT, {1})[0].error, DXL2_ERR_DATA_LENGTH);
}

TEST_F(X430Test, FactoryResetOptions)
{
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 7), 0);
    EXPECT_EQ(write8(1, X430_ADDR_BAUD_RATE, 3), 0);
    EXPECT_EQ(write8(1, X430_ADDR_ID, 5), 0);

    EXPECT_EQ(transact(5, DXL2_INST_FACTORY_RESET, {X430_FACTORY_RESET_EXCEPT_ID_BAUD})[0].error,
              0);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_RETURN_DELAY_TIME), 250);
    EXPECT_EQ(x430_device_id(&dev[0]), 5);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_BAUD_RATE), 3);

    env.events.clear();
    EXPECT_EQ(transact(5, DXL2_INST_FACTORY_RESET, {X430_FACTORY_RESET_EXCEPT_ID})[0].error, 0);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_BAUD_RATE), 1);
    ASSERT_EQ(env.events.size(), 2U);
    EXPECT_EQ(env.events[0], X430_EVENT_FACTORY_RESET);
    EXPECT_EQ(env.events[1], X430_EVENT_BAUD_RATE);

    /* Reset All returns to the configured factory ID (1). */
    auto status = transact(5, DXL2_INST_FACTORY_RESET, {X430_FACTORY_RESET_ALL});
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].id, 5);
    EXPECT_EQ(x430_device_id(&dev[0]), 1);

    EXPECT_EQ(transact(1, DXL2_INST_FACTORY_RESET, {0x03})[0].error, DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(transact(1, DXL2_INST_FACTORY_RESET, {})[0].error, DXL2_ERR_DATA_LENGTH);
    EXPECT_EQ(x430_device_factory_reset(&dev[0], 0x03), DXL2_ERR_DATA_RANGE);
}

TEST_F(X430Test, FactoryResetAllIsRefusedOnBroadcast)
{
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 7), 0);
    EXPECT_TRUE(
        transact(DXL2_BROADCAST_ID, DXL2_INST_FACTORY_RESET, {X430_FACTORY_RESET_ALL}).empty());
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_RETURN_DELAY_TIME), 7);
    EXPECT_TRUE(transact(DXL2_BROADCAST_ID, DXL2_INST_FACTORY_RESET, {X430_FACTORY_RESET_EXCEPT_ID})
                    .empty());
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_RETURN_DELAY_TIME), 250);
}

TEST_F(X430Test, FactoryResetAllKeepsIdWhenFactoryIdIsTaken)
{
    add_device(1, 2U);
    EXPECT_EQ(write8(2, X430_ADDR_ID, 6), 0);
    EXPECT_EQ(write8(1, X430_ADDR_ID, 2), 0); /* device 0 now uses device 1's factory ID */
    (void) transact(6, DXL2_INST_FACTORY_RESET, {X430_FACTORY_RESET_ALL});
    EXPECT_EQ(x430_device_id(&dev[1]), 1); /* falls back to the model default */
}

TEST_F(X430Test, SecondaryIdCannotFactoryReset)
{
    EXPECT_EQ(write8(1, X430_ADDR_SECONDARY_ID, 30), 0);
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 7), 0);
    (void) transact(30, DXL2_INST_FACTORY_RESET, {X430_FACTORY_RESET_EXCEPT_ID});
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_RETURN_DELAY_TIME), 7);
}

TEST_F(X430Test, StartupTorqueOn)
{
    EXPECT_EQ(write8(1, X430_ADDR_STARTUP_CONFIGURATION, X430_STARTUP_TORQUE_ON), 0);
    (void) x430_device_set(&dev[0], X430_ADDR_PRESENT_POSITION, 1500);
    (void) transact(1, DXL2_INST_REBOOT, {});
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_TORQUE_ENABLE), 1);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_GOAL_POSITION), 0); /* present reset by reboot */
}

TEST_F(X430Test, TorqueOnCopiesPresentPositionIntoGoal)
{
    (void) x430_device_set(&dev[0], X430_ADDR_PRESENT_POSITION, 2048);
    EXPECT_EQ(write8(1, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_GOAL_POSITION), 2048);
}

TEST_F(X430Test, TorqueOnByGoalUpdate)
{
    EXPECT_EQ(write8(1, X430_ADDR_DRIVE_MODE, X430_DRIVE_MODE_TORQUE_ON_BY_GOAL), 0);
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_POSITION, 1000), 0);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_TORQUE_ENABLE), 1);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_GOAL_POSITION), 1000);
}

TEST_F(X430Test, HardwareErrorShutdownKeepsTorqueOffUntilReboot)
{
    EXPECT_EQ(write8(1, X430_ADDR_TORQUE_ENABLE, 1), 0);
    env.events.clear();
    x430_device_raise_hardware_error(&dev[0], X430_HW_ERROR_INPUT_VOLTAGE); /* not in Shutdown 52 */
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_TORQUE_ENABLE), 1);
    EXPECT_EQ(transact(1, DXL2_INST_PING, {})[0].error, DXL2_ERR_ALERT_BIT);
    x430_device_raise_hardware_error(&dev[0], X430_HW_ERROR_OVERLOAD);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_TORQUE_ENABLE), 0);
    ASSERT_EQ(env.events.size(), 1U);
    EXPECT_EQ(env.events[0], X430_EVENT_TORQUE_SHUTDOWN);
    EXPECT_EQ(write8(1, X430_ADDR_TORQUE_ENABLE, 1), DXL2_ERR_RESULT_FAIL | DXL2_ERR_ALERT_BIT);
    (void) transact(1, DXL2_INST_REBOOT, {});
    EXPECT_EQ(write8(1, X430_ADDR_TORQUE_ENABLE, 1), 0);
}

TEST_F(X430Test, BusWatchdog)
{
    EXPECT_EQ(write8(1, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(write8(1, X430_ADDR_BUS_WATCHDOG, 5), 0); /* 100 ms */
    env.now += 100;
    x430_bus_poll(&bus);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_BUS_WATCHDOG), 5);
    env.events.clear();
    env.now += 1;
    x430_bus_poll(&bus);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_BUS_WATCHDOG), 0xFF);
    ASSERT_EQ(env.events.size(), 1U);
    EXPECT_EQ(env.events[0], X430_EVENT_BUS_WATCHDOG);
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_POSITION, 10), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(write32(1, X430_ADDR_PROFILE_VELOCITY, 10), 0);
    EXPECT_EQ(write8(1, X430_ADDR_BUS_WATCHDOG, 0), 0);
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_POSITION, 10), 0);
}

TEST_F(X430Test, BusWatchdogNeedsTorque)
{
    EXPECT_EQ(write8(1, X430_ADDR_BUS_WATCHDOG, 1), 0);
    env.now += 1000;
    x430_bus_poll(&bus);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_BUS_WATCHDOG), 1);
}

TEST_F(X430Test, ClearMultiTurn)
{
    const std::vector<uint8_t> magic = {X430_CLEAR_MULTI_TURN, 0x44, 0x58, 0x4C, 0x22};
    (void) x430_device_set(&dev[0], X430_ADDR_MOVING, 1);
    EXPECT_EQ(transact(1, DXL2_INST_CLEAR, magic)[0].error, DXL2_ERR_RESULT_FAIL);
    (void) x430_device_set(&dev[0], X430_ADDR_MOVING, 0);
    env.events.clear();
    env.event_result = DXL2_ERR_RESULT_FAIL;
    EXPECT_EQ(transact(1, DXL2_INST_CLEAR, magic)[0].error, DXL2_ERR_RESULT_FAIL);
    ASSERT_EQ(env.events.size(), 1U);
    EXPECT_EQ(env.events[0], X430_EVENT_CLEAR_MULTI_TURN);
    EXPECT_EQ(transact(1, DXL2_INST_CLEAR, {1, 2, 3, 4, 5})[0].error, DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(transact(1, DXL2_INST_CLEAR, {0x02, 0x44, 0x58, 0x4C, 0x22})[0].error,
              DXL2_ERR_DATA_RANGE);
}

TEST_F(X430Test, ClearMultiTurnWithoutEventHookWrapsPresentPosition)
{
    add_device(1, 2U, nullptr);
    const std::vector<uint8_t> magic = {X430_CLEAR_MULTI_TURN, 0x44, 0x58, 0x4C, 0x22};
    (void) x430_device_set(&dev[1], X430_ADDR_PRESENT_POSITION, -5000);
    EXPECT_EQ(transact(2, DXL2_INST_CLEAR, magic)[0].error, 0);
    EXPECT_EQ(x430_device_get(&dev[1], X430_ADDR_PRESENT_POSITION), -5000 + 2 * 4096);
    (void) x430_device_set(&dev[1], X430_ADDR_PRESENT_POSITION, 9000);
    EXPECT_EQ(transact(2, DXL2_INST_CLEAR, magic)[0].error, 0);
    EXPECT_EQ(x430_device_get(&dev[1], X430_ADDR_PRESENT_POSITION), 9000 - 2 * 4096);
}

#if X430_FEATURE_BACKUP
TEST_F(X430Test, BackupStoreAndRestore)
{
    const std::vector<uint8_t> store = {X430_BACKUP_STORE, 'C', 'T', 'R', 'L'};
    const std::vector<uint8_t> restore = {X430_BACKUP_RESTORE, 'C', 'T', 'R', 'L'};
    EXPECT_EQ(read(1, X430_ADDR_BACKUP_READY, 1).data[0], 0);
    EXPECT_EQ(transact(1, DXL2_INST_BACKUP, restore)[0].error,
              DXL2_ERR_RESULT_FAIL); /* nothing yet */
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 7), 0);
    EXPECT_EQ(write32(1, X430_ADDR_PROFILE_VELOCITY, 33), 0);
    EXPECT_EQ(transact(1, DXL2_INST_BACKUP, store)[0].error, 0);
    EXPECT_EQ(read(1, X430_ADDR_BACKUP_READY, 1).data[0], 1);
    ASSERT_EQ(env.nvm[X430_NVM_BACKUP].size(), X430_NVM_BACKUP_SIZE);

    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 9), 0);
    EXPECT_EQ(write32(1, X430_ADDR_PROFILE_VELOCITY, 44), 0);
    EXPECT_EQ(write8(1, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(transact(1, DXL2_INST_BACKUP, restore)[0].error, DXL2_ERR_ACCESS);
    EXPECT_EQ(write8(1, X430_ADDR_TORQUE_ENABLE, 0), 0);
    env.events.clear();
    EXPECT_EQ(transact(1, DXL2_INST_BACKUP, restore)[0].error, 0);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_RETURN_DELAY_TIME), 7);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 33);
    ASSERT_EQ(env.events.size(), 1U);
    EXPECT_EQ(env.events[0], X430_EVENT_BACKUP_RESTORED);

    EXPECT_EQ(transact(1, DXL2_INST_BACKUP, {3, 'C', 'T', 'R', 'L'})[0].error, DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(transact(1, DXL2_INST_BACKUP, {1, 'C', 'T', 'R', 'X'})[0].error, DXL2_ERR_DATA_RANGE);
}

TEST_F(X430Test, StartupRamRestore)
{
    const std::vector<uint8_t> store = {X430_BACKUP_STORE, 'C', 'T', 'R', 'L'};
    EXPECT_EQ(write32(1, X430_ADDR_PROFILE_VELOCITY, 33), 0);
    EXPECT_EQ(write8(1, X430_ADDR_STARTUP_CONFIGURATION, X430_STARTUP_RAM_RESTORE), 0);
    EXPECT_EQ(transact(1, DXL2_INST_BACKUP, store)[0].error, 0);
    (void) transact(1, DXL2_INST_REBOOT, {});
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 33);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_BACKUP_READY), 1);
}

TEST_F(X430Test, BackupWithoutStorageHook)
{
    add_device(1, 2U, nullptr);
    EXPECT_EQ(transact(2, DXL2_INST_BACKUP, {X430_BACKUP_STORE, 'C', 'T', 'R', 'L'})[0].error, 0);
    EXPECT_EQ(x430_device_get(&dev[1], X430_ADDR_BACKUP_READY), 0);
}
#endif

TEST_F(X430Test, LockIsTakenAroundEveryEntryPoint)
{
    env.locks = env.unlocks = 0;
    (void) transact(1, DXL2_INST_PING, {});
    x430_bus_poll(&bus);
    EXPECT_EQ(env.locks, 2);
    EXPECT_EQ(env.unlocks, 2);
}

TEST(X430NoPort, WorksWithoutPortServices)
{
    TestEnv env;
    g_env = &env;
    struct x430_bus bus;
    struct x430_device device;
    const struct x430_bus_config bus_config = {test_write, &env, nullptr};
    const struct x430_device_config config = {1U, 0U, 0U, nullptr, nullptr};
    ASSERT_EQ(x430_bus_init(&bus, &bus_config), X430_OK);
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    ASSERT_EQ(x430_bus_attach(&bus, &device), X430_OK);
    uint8_t raw[32];
    size_t length = 0;
    const uint8_t params[] = {X430_ADDR_REALTIME_TICK, 0, 2, 0};
    ASSERT_EQ(dxl2_encode(1, DXL2_INST_READ, params, sizeof(params), raw, sizeof(raw), &length), 0);
    x430_bus_receive(&bus, raw, length);
    auto status = take_status(env);
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].data, (std::vector<uint8_t>{0, 0}));
    const uint8_t torque[] = {X430_ADDR_TORQUE_ENABLE, 0, 1};
    ASSERT_EQ(dxl2_encode(1, DXL2_INST_WRITE, torque, sizeof(torque), raw, sizeof(raw), &length),
              0);
    x430_bus_receive(&bus, raw, length);
    EXPECT_EQ(x430_device_get(&device, X430_ADDR_TORQUE_ENABLE), 1);
    x430_bus_poll(&bus);
    g_env = nullptr;
}
