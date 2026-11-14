/*******************************************************************************
 * DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
 *
 * Project:     DYNAMIXEL-X430-Control-Table-Emulator
 * Description: Unit tests of the device rules and bus decoding at their edges:
 *              hooks that are absent, malformed group instructions, persistent
 *              blocks that do not belong to this device, Indirect Address corner
 *              cases.
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

#include "x430_test_support.h"

extern "C" {
#include "x430_internal.h"
}

namespace
{

constexpr uint16_t kIndirectAddress1 = 168U; /* Indirect Address 1 */
constexpr uint16_t kIndirectData1 = 224U;    /* Indirect Data 1 */

/** Hooks with a write hook only: no read, event or storage. */
uint8_t write_only(struct x430_device*, uint16_t, uint8_t, int32_t, void*)
{
    return DXL2_ERR_NONE;
}
const struct x430_hooks kWriteOnlyHooks = {write_only, nullptr, nullptr, nullptr, nullptr};
const struct x430_hooks kNoHooks = {nullptr, nullptr, nullptr, nullptr, nullptr};

}  // namespace

/* ------------------------------------------------------------------ port */

TEST(X430Port, EmptyPortMeansNoServices)
{
    TestEnv env;
    g_env = &env;
    const struct x430_port port = {nullptr, nullptr, nullptr, nullptr, nullptr};
    const struct x430_bus_config bus_config = {test_write, &env, &port};
    struct x430_bus bus;
    struct x430_device device;
    const struct x430_device_config config = {1U, 0U, 0U, &test_hooks, &env};
    ASSERT_EQ(x430_bus_init(&bus, &bus_config), X430_OK);
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    ASSERT_EQ(x430_bus_attach(&bus, &device), X430_OK);
    EXPECT_EQ(x430_bus_lock(&bus), X430_OK);
    EXPECT_EQ(x430_bus_unlock(&bus), X430_OK);

    uint8_t raw[DXL2_MAX_PACKET_SIZE];
    size_t length = 0;
    const uint8_t params[] = {X430_ADDR_REALTIME_TICK, 0U, 2U, 0U};
    ASSERT_EQ(dxl2_encode(1U, DXL2_INST_READ, params, 4U, raw, sizeof(raw), &length), 0);
    EXPECT_EQ(x430_bus_receive(&bus, raw, length), X430_OK); /* Return Delay Time 250: no delay */
    auto status = take_status(env);
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].data, std::vector<uint8_t>({0, 0})); /* no clock: tick 0 */
    EXPECT_EQ(x430_bus_poll(&bus), X430_OK);
    g_env = nullptr;
}

/* ---------------------------------------------------- single instructions */

TEST_F(X430Test, ClearNeedsItsParametersAndMagic)
{
    const std::vector<std::vector<uint8_t>> bad = {
        {X430_CLEAR_MULTI_TURN, 0x44, 0x58, 0x4C},       /* 4 parameters */
        {X430_CLEAR_MULTI_TURN, 0x00, 0x58, 0x4C, 0x22}, /* each magic byte wrong */
        {X430_CLEAR_MULTI_TURN, 0x44, 0x00, 0x4C, 0x22},
        {X430_CLEAR_MULTI_TURN, 0x44, 0x58, 0x00, 0x22},
        {X430_CLEAR_MULTI_TURN, 0x44, 0x58, 0x4C, 0x00},
    };
    for (const auto& params : bad)
    {
        auto status = transact(1, DXL2_INST_CLEAR, params);
        ASSERT_EQ(status.size(), 1U);
        EXPECT_EQ(status[0].error, DXL2_ERR_DATA_RANGE);
    }
}

#if X430_FEATURE_BACKUP
TEST_F(X430Test, BackupNeedsItsParameters)
{
    auto status = transact(1, DXL2_INST_BACKUP, {X430_BACKUP_STORE, 'C', 'T', 'R'});
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].error, DXL2_ERR_DATA_RANGE);
}

TEST_F(X430Test, BackupStoreFailureKeepsBackupReadyOff)
{
    env.nvm_store_fails = true;
    auto status = transact(1, DXL2_INST_BACKUP, {X430_BACKUP_STORE, 'C', 'T', 'R', 'L'});
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].error, DXL2_ERR_RESULT_FAIL);
    EXPECT_EQ(reg(&dev[0], X430_ADDR_BACKUP_READY), 0);
}

TEST_F(X430Test, ValidBackupWithoutRamRestoreOnlySetsBackupReady)
{
    EXPECT_EQ(write32(1, X430_ADDR_PROFILE_VELOCITY, 33), 0);
    ASSERT_EQ(transact(1, DXL2_INST_BACKUP, {X430_BACKUP_STORE, 'C', 'T', 'R', 'L'})[0].error, 0);
    EXPECT_EQ(x430_device_reboot(&dev[0]), X430_OK); /* Startup Configuration 0 */
    EXPECT_EQ(reg(&dev[0], X430_ADDR_BACKUP_READY), 1);
    EXPECT_EQ(reg(&dev[0], X430_ADDR_PROFILE_VELOCITY), 0);
}
#endif

TEST_F(X430Test, OperatingModeOutsideTheBitSetIsARangeError)
{
    EXPECT_EQ(write8(1, X430_ADDR_OPERATING_MODE, 40), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(write8(1, X430_ADDR_OPERATING_MODE, 2), DXL2_ERR_DATA_RANGE); /* not an X430 mode */
}

TEST_F(X430Test, GoalPositionBelowMinPositionLimit)
{
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_POSITION, -1), DXL2_ERR_DATA_RANGE); /* mode 3 */
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_POSITION, 0), 0);
}

TEST_F(X430Test, TorqueEnableWrittenTwiceKeepsTheGoal)
{
    EXPECT_EQ(write8(1, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_POSITION, 1234), 0);
    EXPECT_EQ(write8(1, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(reg(&dev[0], X430_ADDR_GOAL_POSITION), 1234);
}

TEST_F(X430Test, RefusedRegisterAfterAnEepromRegisterStillStoresTheEeprom)
{
    env.refuse_address = X430_ADDR_DRIVE_MODE;
    env.refuse_address_error = DXL2_ERR_RESULT_FAIL;
    /* Return Delay Time (9) and Drive Mode (10) in one write */
    EXPECT_EQ(write(1, X430_ADDR_RETURN_DELAY_TIME, {7, 0}), DXL2_ERR_RESULT_FAIL);
    EXPECT_EQ(reg(&dev[0], X430_ADDR_RETURN_DELAY_TIME), 7);
    EXPECT_EQ(env.nvm_stores, 1);
}

TEST_F(X430Test, RefusedRegisterAfterAGoalDoesNotEnableTheTorque)
{
    EXPECT_EQ(write8(1, X430_ADDR_DRIVE_MODE, X430_DRIVE_MODE_TORQUE_ON_BY_GOAL), 0);
    EXPECT_EQ(write8(1, X430_ADDR_OPERATING_MODE, X430_MODE_VELOCITY), 0);
    env.refuse_address = X430_ADDR_PROFILE_ACCELERATION;
    env.refuse_address_error = DXL2_ERR_RESULT_FAIL;
    /* Goal Velocity (104) and Profile Acceleration (108) in one write */
    EXPECT_EQ(write(1, X430_ADDR_GOAL_VELOCITY, cat(le32(10), le32(5))), DXL2_ERR_RESULT_FAIL);
    EXPECT_EQ(reg(&dev[0], X430_ADDR_TORQUE_ENABLE), 0);
}

TEST_F(X430Test, StartupTorqueOnCanBeRefusedByTheWriteHook)
{
    (void)x430_device_set(&dev[0], X430_ADDR_STARTUP_CONFIGURATION, X430_STARTUP_TORQUE_ON);
    env.refuse_address = X430_ADDR_TORQUE_ENABLE;
    env.refuse_address_error = DXL2_ERR_RESULT_FAIL;
    EXPECT_EQ(x430_device_reboot(&dev[0]), X430_OK);
    EXPECT_EQ(reg(&dev[0], X430_ADDR_TORQUE_ENABLE), 0);
}

TEST_F(X430Test, ConfiguredFirmwareVersion)
{
    struct x430_device device;
    const struct x430_device_config config = {9U, 60U, 0U, &test_hooks, &env};
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_FIRMWARE_VERSION), 60);
}

/* ------------------------------------------------------- devices without hooks */

TEST(X430Hooks, DeviceWithoutHooks)
{
    struct x430_device device;
    const struct x430_device_config config = {1U, 0U, 0U, nullptr, nullptr};
    uint8_t error = 0xFFU;
    uint8_t data[4] = {0, 0, 0, 0};
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);

    /* Startup Torque On without a write hook */
    EXPECT_EQ(x430_device_set(&device, X430_ADDR_STARTUP_CONFIGURATION, X430_STARTUP_TORQUE_ON),
              X430_OK);
    EXPECT_EQ(x430_device_reboot(&device), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_TORQUE_ENABLE), 1);

    /* Shutdown without an event hook */
    EXPECT_EQ(x430_device_raise_hardware_error(&device, X430_HW_ERROR_OVERHEATING), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_TORQUE_ENABLE), 0);

    /* Not attached: no clock, no other device to clash with */
    EXPECT_EQ(x430_device_read(&device, X430_ADDR_REALTIME_TICK, 2U, data, &error), X430_OK);
    EXPECT_EQ(error, DXL2_ERR_NONE);
    EXPECT_EQ(x430_device_write(&device, X430_ADDR_ID, 1U, data, 0U, &error), X430_OK);
    EXPECT_EQ(error, DXL2_ERR_NONE);
    EXPECT_EQ(id_of(&device), 0);

    /* Factory reset (all) with configured ID 1 */
    EXPECT_EQ(x430_device_factory_reset(&device, X430_FACTORY_RESET_ALL, &error), X430_OK);
    EXPECT_EQ(id_of(&device), 1);
}

TEST(X430Hooks, FactoryResetAllWithoutConfiguredIdUsesTheModelDefault)
{
    struct x430_device device;
    const struct x430_device_config config = {0U, 0U, 0U, nullptr, nullptr};
    const uint8_t id = 9U;
    uint8_t error = 0xFFU;
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(x430_device_write(&device, X430_ADDR_ID, 1U, &id, 0U, &error), X430_OK);
    EXPECT_EQ(id_of(&device), 9);
    EXPECT_EQ(x430_device_factory_reset(&device, X430_FACTORY_RESET_ALL, &error), X430_OK);
    EXPECT_EQ(error, DXL2_ERR_NONE);
    EXPECT_EQ(id_of(&device), 1);
}

TEST(X430Hooks, HooksWithoutEventReadAndStorage)
{
    struct x430_device device;
    const struct x430_device_config config = {1U, 0U, 0U, &kWriteOnlyHooks, nullptr};
    uint8_t error = 0xFFU;
    uint8_t data[4] = {0, 0, 0, 0};
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);

    EXPECT_EQ(x430_device_read(&device, X430_ADDR_ID, 1U, data, &error), X430_OK);
    EXPECT_EQ(data[0], 1U);
    (void)x430_device_set(&device, X430_ADDR_PRESENT_POSITION, -5000);
    EXPECT_EQ(x430_device_clear(&device, X430_CLEAR_MULTI_TURN, &error), X430_OK);
    EXPECT_EQ(error, DXL2_ERR_NONE);
    EXPECT_EQ(reg(&device, X430_ADDR_PRESENT_POSITION), -5000 + 2 * 4096);
    EXPECT_EQ(x430_device_set(&device, X430_ADDR_TORQUE_ENABLE, 1), X430_OK);
    EXPECT_EQ(x430_device_raise_hardware_error(&device, X430_HW_ERROR_OVERHEATING), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_TORQUE_ENABLE), 0);
}

TEST(X430Hooks, HooksWithoutWrite)
{
    struct x430_device device;
    const struct x430_device_config config = {1U, 0U, 0U, &kNoHooks, nullptr};
    const uint8_t on = 1U;
    uint8_t error = 0xFFU;
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(x430_device_write(&device, X430_ADDR_TORQUE_ENABLE, 1U, &on, 0U, &error), X430_OK);
    EXPECT_EQ(error, DXL2_ERR_NONE);
    EXPECT_EQ(reg(&device, X430_ADDR_TORQUE_ENABLE), 1);
    const uint8_t off = 0U;
    const uint8_t delay = 7U;
    EXPECT_EQ(x430_device_write(&device, X430_ADDR_TORQUE_ENABLE, 1U, &off, 0U, &error), X430_OK);
    /* EEPROM write without a storage hook */
    EXPECT_EQ(x430_device_write(&device, X430_ADDR_RETURN_DELAY_TIME, 1U, &delay, 0U, &error),
              X430_OK);
    EXPECT_EQ(error, DXL2_ERR_NONE);
    /* Shutdown bit with the torque off: nothing to switch off */
    EXPECT_EQ(x430_device_raise_hardware_error(&device, X430_HW_ERROR_OVERHEATING), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_HARDWARE_ERROR_STATUS), X430_HW_ERROR_OVERHEATING);
    EXPECT_EQ(x430_device_reboot(&device), X430_OK); /* clears the error */
    (void)x430_device_set(&device, X430_ADDR_STARTUP_CONFIGURATION, X430_STARTUP_TORQUE_ON);
    EXPECT_EQ(x430_device_reboot(&device), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_TORQUE_ENABLE), 1);
}

TEST(X430Hooks, WriteOfLengthZero)
{
    struct x430_device device;
    const struct x430_device_config config = {};
    const uint8_t byte = 0U;
    uint8_t error = 0U;
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(x430_device_write(&device, X430_ADDR_LED, 0U, &byte, 0U, &error), X430_OK);
    EXPECT_EQ(error, DXL2_ERR_DATA_LENGTH);
}

/* ------------------------------------------------------- persistent blocks */

TEST_F(X430Test, PartlyReadableEepromBlocks)
{
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 7), 0);
    const std::vector<uint8_t> good = env.nvm[X430_NVM_EEPROM];
    const struct x430_device_config config = {9U, 0U, 0U, &test_hooks, &env};
    struct x430_device device;

    /* The CRC cannot be read: no block */
    env.nvm[X430_NVM_EEPROM].resize(good.size() - 1U);
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_RETURN_DELAY_TIME), 250);

    /* Checked in 6 reads (header, 4 payload chunks, CRC), then one read per register in
       address order: ID (7), Baud Rate (8), Return Delay Time (9). A register whose read
       fails keeps its factory value; the others are loaded. */
    env.nvm[X430_NVM_EEPROM] = good;
    env.nvm_reads = 0;
    env.nvm_read_fail_call = 9;
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_RETURN_DELAY_TIME), 250);
    env.nvm_reads = 0;
    env.nvm_read_fail_call = 8;
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_RETURN_DELAY_TIME), 7);
}

TEST_F(X430Test, InterruptedEepromStoreLeavesNoValidBlock)
{
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 7), 0);
    env.nvm_writes = 0;
    env.nvm_write_fail_call = 2; /* the header is written, the payload fails */
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 8), DXL2_ERR_RESULT_FAIL);
    struct x430_device device;
    const struct x430_device_config config = {9U, 0U, 0U, &test_hooks, &env};
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_RETURN_DELAY_TIME), 250);

    env.nvm_writes = 0;
    env.nvm_write_fail_call = 3; /* the CRC fails */
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 9), DXL2_ERR_RESULT_FAIL);
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_RETURN_DELAY_TIME), 250);
}

TEST_F(X430Test, EepromBlockKeepsItsFormat)
{
    /* Header "X430", model number, format 1, slot, payload size; CRC-16 (0x8005) of all */
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 7), 0);
    const std::vector<uint8_t>& block = env.nvm[X430_NVM_EEPROM];
    ASSERT_EQ(block.size(), X430_NVM_EEPROM_SIZE);
    EXPECT_EQ(std::vector<uint8_t>(block.begin(), block.begin() + 4),
              (std::vector<uint8_t>{'X', '4', '3', '0'}));
    EXPECT_EQ(block[4] | (block[5] << 8), x430_model_active.model_number);
    EXPECT_EQ(block[6], 1);
    EXPECT_EQ(block[7], X430_NVM_EEPROM);
    EXPECT_EQ(block[8] | (block[9] << 8), X430_RAM_START);
    EXPECT_EQ(block[X430_NVM_HEADER_SIZE + X430_ADDR_RETURN_DELAY_TIME], 7);
    const size_t crc_at = X430_NVM_EEPROM_SIZE - 2U;
    EXPECT_EQ(block[crc_at] | (block[crc_at + 1U] << 8), dxl2_crc16(block.data(), crc_at));
}

TEST_F(X430Test, ForeignOrDamagedEepromBlocksAreIgnored)
{
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 7), 0);
    const std::vector<uint8_t> good = env.nvm[X430_NVM_EEPROM];
    ASSERT_EQ(good.size(), X430_NVM_EEPROM_SIZE);

    /* magic (0..3), model number (4), format (6), slot (7), size (8), payload (CRC) */
    for (size_t index : {0U, 1U, 2U, 3U, 4U, 6U, 7U, 8U, 20U})
    {
        std::vector<uint8_t> bad = good;
        bad[index] ^= 0x5AU;
        env.nvm[X430_NVM_EEPROM] = bad;
        struct x430_device device;
        const struct x430_device_config config = {9U, 0U, 0U, &test_hooks, &env};
        ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
        EXPECT_EQ(reg(&device, X430_ADDR_RETURN_DELAY_TIME), 250) << "byte " << index;
    }
    env.nvm[X430_NVM_EEPROM] = good;
    struct x430_device device;
    const struct x430_device_config config = {9U, 0U, 0U, &test_hooks, &env};
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    EXPECT_EQ(reg(&device, X430_ADDR_RETURN_DELAY_TIME), 7);
}

/* ------------------------------------------------------- Indirect Address */

TEST_F(X430Test, IndirectWriteToTwoRegisters)
{
    /* Indirect Data 1 -> Status Return Level (68), Indirect Data 2 -> Velocity I Gain (76) */
    EXPECT_EQ(write(1, kIndirectAddress1,
                    cat(le16(X430_ADDR_STATUS_RETURN_LEVEL), le16(X430_ADDR_VELOCITY_I_GAIN))),
              0);
    EXPECT_EQ(write(1, kIndirectData1, {2, 5}), 0); /* level 2: WRITE still answers */
    EXPECT_EQ(reg(&dev[0], X430_ADDR_STATUS_RETURN_LEVEL), 2);
    EXPECT_EQ(reg(&dev[0], X430_ADDR_VELOCITY_I_GAIN) & 0xFF, 5);
}

TEST_F(X430Test, IndirectWriteToAReservedAddressIsDropped)
{
    EXPECT_EQ(write(1, kIndirectAddress1, le16(150U)), 0); /* reserved RAM address */
    EXPECT_EQ(write(1, kIndirectData1, {7}), 0);
    EXPECT_EQ(read(1, 150U, 1U).data, std::vector<uint8_t>({0}));
    EXPECT_EQ(read(1, kIndirectData1, 1U).data, std::vector<uint8_t>({0}));
}

TEST_F(X430Test, IndirectAddressSetOutsideTheTableReadsItself)
{
    /* x430_device_set() does not check: the library must not read beyond the table */
    EXPECT_EQ(x430_device_set(&dev[0], kIndirectAddress1, 9999), X430_OK);
    auto status = read(1, kIndirectData1, 1U);
    EXPECT_EQ(status.error, 0);
    EXPECT_EQ(status.data, std::vector<uint8_t>({0})); /* Indirect Data 1 storage itself */
}

TEST(X430Internal, ValueAndStoreOfANonRegisterAddress)
{
    struct x430_device device;
    const struct x430_device_config config = {};
    ASSERT_EQ(x430_device_init(&device, &config), X430_OK);
    x430_device_store(&device, 15U, 99); /* reserved: no effect */
    EXPECT_EQ(x430_device_value(&device, 15U), 0);
    EXPECT_EQ(x430_device_byte(&device, 15U), 0U);
}

/* ---------------------------------------------------- group instructions */

TEST_F(X430Test, GroupReadsSkipSilentDevices)
{
    EXPECT_EQ(write8(1, X430_ADDR_STATUS_RETURN_LEVEL, 0), 0xFF); /* level 0: no status */
    send(DXL2_BROADCAST_ID, DXL2_INST_SYNC_READ, cat(cat(le16(X430_ADDR_ID), le16(1)), {1}));
    EXPECT_TRUE(take_status(env).empty());
#if X430_FEATURE_FAST_READ
    send(DXL2_BROADCAST_ID, DXL2_INST_FAST_SYNC_READ, cat(cat(le16(X430_ADDR_ID), le16(1)), {1}));
    EXPECT_TRUE(env.wire.empty());
#endif
}

TEST_F(X430Test, SyncInstructionsCheckTheirLengths)
{
    const uint16_t too_long = DXL2_MAX_PARAMETER_SIZE;
    send(DXL2_BROADCAST_ID, DXL2_INST_SYNC_READ, cat(cat(le16(X430_ADDR_ID), le16(too_long)), {1}));
    EXPECT_TRUE(take_status(env).empty());
    send(DXL2_BROADCAST_ID, DXL2_INST_SYNC_WRITE,
         cat(cat(le16(X430_ADDR_PROFILE_VELOCITY), le16(0)), {1, 9}));
    EXPECT_EQ(reg(&dev[0], X430_ADDR_PROFILE_VELOCITY), 0);
}

#if X430_FEATURE_BULK
TEST_F(X430Test, BulkInstructionsCheckTheirEntries)
{
    send(DXL2_BROADCAST_ID, DXL2_INST_BULK_READ, {});
    EXPECT_TRUE(take_status(env).empty());
    const uint16_t too_long = DXL2_MAX_PARAMETER_SIZE;
    send(DXL2_BROADCAST_ID, DXL2_INST_BULK_READ, cat(cat({1}, le16(X430_ADDR_ID)), le16(too_long)));
    EXPECT_TRUE(take_status(env).empty());
    /* the entry announces 4 data bytes, the packet carries 2 */
    send(DXL2_BROADCAST_ID, DXL2_INST_BULK_WRITE,
         cat(cat(cat({1}, le16(X430_ADDR_PROFILE_VELOCITY)), le16(4)), {9, 0}));
    EXPECT_EQ(reg(&dev[0], X430_ADDR_PROFILE_VELOCITY), 0);
}
#endif

#if X430_FEATURE_FAST_READ
TEST_F(X430Test, FastReadsCheckTheirEntries)
{
    send(DXL2_BROADCAST_ID, DXL2_INST_FAST_BULK_READ, {});
    EXPECT_TRUE(env.wire.empty());

    /* The same ID more often than a bus has devices: one segment per device slot */
    std::vector<uint8_t> ids(X430_MAX_DEVICES + 1U, 1U);
    send(DXL2_BROADCAST_ID, DXL2_INST_FAST_SYNC_READ, cat(cat(le16(X430_ADDR_ID), le16(1)), ids));
    auto status = take_status(env);
    ASSERT_EQ(status.size(), 1U);
    /* per segment error, ID, 1 data byte, CRC; the error byte of the first segment is the
       status error, the CRC of the last one the packet CRC */
    EXPECT_EQ(status[0].data.size(), (size_t)(X430_MAX_DEVICES * 5U - 3U));
}
#endif

TEST_F(X430Test, BroadcastPingWithTwoDevicesOnTheSameId)
{
    add_device(1, 2U);
    add_device(2, 3U);
    EXPECT_EQ(write8(2, X430_ADDR_ID, 5), 0);
    EXPECT_EQ(write8(3, X430_ADDR_ID, 2), 0);
    /* dev[1]: factory ID 2 is taken by dev[2] -> model default 1, already used by dev[0] */
    EXPECT_EQ(transact(5, DXL2_INST_FACTORY_RESET, {X430_FACTORY_RESET_ALL})[0].error, 0);
    EXPECT_EQ(id_of(&dev[1]), 1);
    auto status = transact(DXL2_BROADCAST_ID, DXL2_INST_PING, {});
    ASSERT_EQ(status.size(), 2U);
    EXPECT_EQ(status[0].id, 1);
    EXPECT_EQ(status[1].id, 2);
}
