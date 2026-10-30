/**
 * @file gtest_x430_protocol.cpp
 * @brief Instruction handling: PING, READ, WRITE, status rules, addressing.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "x430_test_support.h"

TEST_F(X430Test, PingAnswersModelNumberAndFirmware)
{
    auto status = transact(1, DXL2_INST_PING, {});
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].id, 1);
    EXPECT_EQ(status[0].error, 0);
    ASSERT_EQ(status[0].data.size(), 3U);
    EXPECT_EQ(status[0].data[0] | (status[0].data[1] << 8), X430_MODEL_NUMBER);
    EXPECT_EQ(status[0].data[2], X430_FIRMWARE_VERSION);
}

TEST_F(X430Test, UnknownIdIsSilent)
{
    EXPECT_TRUE(transact(42, DXL2_INST_PING, {}).empty());
    EXPECT_TRUE(transact(42, DXL2_INST_READ, cat(le16(0), le16(2))).empty());
}

TEST_F(X430Test, BroadcastPingAnswersInAscendingIdOrder)
{
    add_device(1, 9U);
    add_device(2, 4U);
    auto status = transact(DXL2_BROADCAST_ID, DXL2_INST_PING, {});
    ASSERT_EQ(status.size(), 3U);
    EXPECT_EQ(status[0].id, 1);
    EXPECT_EQ(status[1].id, 4);
    EXPECT_EQ(status[2].id, 9);
}

TEST_F(X430Test, StatusPacketsFromOtherDevicesAreIgnored)
{
    EXPECT_TRUE(transact(1, DXL2_INST_STATUS, {0, 1, 2}).empty());
}

TEST_F(X430Test, ReadReturnsTableBytes)
{
    auto status = read(1, X430_ADDR_MODEL_NUMBER, 8);
    EXPECT_EQ(status.error, 0);
    ASSERT_EQ(status.data.size(), 8U);
    EXPECT_EQ(status.data[0] | (status.data[1] << 8), X430_MODEL_NUMBER);
    EXPECT_EQ(get_i32(status.data, 2), 0x12345678); /* Model Information from the config */
    EXPECT_EQ(status.data[7], 1);                   /* ID */
}

TEST_F(X430Test, ReadWholeTableInChunks)
{
    for (uint16_t address = 0; address < X430_TABLE_SIZE; address += 200)
    {
        const uint16_t length = (uint16_t) std::min<int>(200, X430_TABLE_SIZE - address);
        auto status = read(1, address, length);
        EXPECT_EQ(status.error, 0);
        EXPECT_EQ(status.data.size(), length);
    }
}

TEST_F(X430Test, ReadErrors)
{
    EXPECT_EQ(read(1, 0, 0).error, DXL2_ERR_DATA_LENGTH);
    EXPECT_EQ(read(1, X430_TABLE_SIZE - 1, 2).error, DXL2_ERR_ACCESS);
    EXPECT_EQ(read(1, 0, DXL2_MAX_PARAMETER_SIZE).error, DXL2_ERR_DATA_LENGTH);
    auto status = transact(1, DXL2_INST_READ, {0, 0, 1});
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].error, DXL2_ERR_DATA_LENGTH);
    EXPECT_TRUE(status[0].data.empty());
}

TEST_F(X430Test, ReadHookSeesRequestedRange)
{
    env.reads.clear();
    (void) read(1, X430_ADDR_PRESENT_POSITION, 4);
    ASSERT_EQ(env.reads.size(), 1U);
    EXPECT_EQ(env.reads[0].first, X430_ADDR_PRESENT_POSITION);
    EXPECT_EQ(env.reads[0].second, 4);
}

TEST_F(X430Test, RealtimeTickFollowsPortClock)
{
    env.now = 40000;
    auto status = read(1, X430_ADDR_REALTIME_TICK, 2);
    EXPECT_EQ(status.data[0] | (status.data[1] << 8), 40000 % 32768);
}

TEST_F(X430Test, WriteAndReadBack)
{
    EXPECT_EQ(write32(1, X430_ADDR_PROFILE_VELOCITY, 123), 0);
    EXPECT_EQ(get_i32(read(1, X430_ADDR_PROFILE_VELOCITY, 4).data, 0), 123);
    ASSERT_FALSE(env.writes.empty());
    EXPECT_EQ(env.writes.back().address, X430_ADDR_PROFILE_VELOCITY);
    EXPECT_EQ(env.writes.back().size, 4);
    EXPECT_EQ(env.writes.back().value, 123);
}

TEST_F(X430Test, WriteLengthAndAccessErrors)
{
    auto status = transact(1, DXL2_INST_WRITE, {X430_ADDR_LED, 0});
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].error, DXL2_ERR_DATA_LENGTH);
    EXPECT_EQ(write8(1, X430_TABLE_SIZE, 0), DXL2_ERR_ACCESS);
    EXPECT_EQ(write16(1, X430_ADDR_MODEL_NUMBER, 5), DXL2_ERR_ACCESS); /* read-only */
    EXPECT_EQ(write8(1, X430_ADDR_HARDWARE_ERROR_STATUS, 1), DXL2_ERR_ACCESS);
}

TEST_F(X430Test, WriteRangeChecks)
{
    EXPECT_EQ(write8(1, X430_ADDR_ID, 253), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(write8(1, X430_ADDR_STATUS_RETURN_LEVEL, 3), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(write8(1, X430_ADDR_OPERATING_MODE, 2), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(write8(1, X430_ADDR_OPERATING_MODE, X430_MODE_EXTENDED_POSITION), 0);
#if X430_MODEL_HAS_CURRENT
    EXPECT_EQ(write8(1, X430_ADDR_OPERATING_MODE, X430_MODE_CURRENT), 0);
#else
    EXPECT_EQ(write8(1, X430_ADDR_OPERATING_MODE, X430_MODE_CURRENT), DXL2_ERR_DATA_RANGE);
#endif
    EXPECT_EQ(write8(1, X430_ADDR_DRIVE_MODE, 0x02), DXL2_ERR_DATA_RANGE); /* bit 1 not defined */
    EXPECT_EQ(write8(1, X430_ADDR_DRIVE_MODE, 0x0D), 0);
    EXPECT_EQ(write8(1, X430_ADDR_PROTOCOL_TYPE, 1), DXL2_ERR_DATA_RANGE); /* Protocol 2.0 only */
    EXPECT_EQ(write8(1, X430_ADDR_BAUD_RATE, 8), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(write32(1, X430_ADDR_HOMING_OFFSET, -1044480), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(write32(1, X430_ADDR_HOMING_OFFSET, -1044479), 0);
}

TEST_F(X430Test, SymmetricLimitFollowsLimitRegister)
{
    const int32_t limit = x430_device_get(&dev[0], X430_ADDR_VELOCITY_LIMIT);
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_VELOCITY, limit + 1), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_VELOCITY, -limit), 0);
    EXPECT_EQ(write32(1, X430_ADDR_VELOCITY_LIMIT, 50), 0);
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_VELOCITY, 51), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(write16(1, X430_ADDR_GOAL_PWM, -885), 0);
    EXPECT_EQ(write16(1, X430_ADDR_GOAL_PWM, 886), DXL2_ERR_DATA_RANGE);
}

TEST_F(X430Test, GoalPositionRangeDependsOnOperatingMode)
{
    EXPECT_EQ(write32(1, X430_ADDR_MAX_POSITION_LIMIT, 3000), 0);
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_POSITION, 3001), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_POSITION, 3000), 0);
    EXPECT_EQ(write8(1, X430_ADDR_OPERATING_MODE, X430_MODE_EXTENDED_POSITION), 0);
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_POSITION, -1048575), 0);
    EXPECT_EQ(write32(1, X430_ADDR_GOAL_POSITION, 1048576), DXL2_ERR_DATA_RANGE);
}

TEST_F(X430Test, EepromLockedWhileTorqueOn)
{
    EXPECT_EQ(write8(1, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 0), DXL2_ERR_ACCESS);
    EXPECT_EQ(write8(1, X430_ADDR_OPERATING_MODE, X430_MODE_VELOCITY), DXL2_ERR_ACCESS);
    EXPECT_EQ(write32(1, X430_ADDR_PROFILE_VELOCITY, 10), 0); /* RAM stays open */
    EXPECT_EQ(write8(1, X430_ADDR_TORQUE_ENABLE, 0), 0);
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 0), 0);
}

TEST_F(X430Test, MultiRegisterWriteIsValidatedBeforeAnythingIsStored)
{
    /* Profile Acceleration (108) ok, Profile Velocity (112) out of range. */
    auto data = cat(le32(10), le32(40000));
    EXPECT_EQ(write(1, X430_ADDR_PROFILE_ACCELERATION, data), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_ACCELERATION), 0);
    data = cat(le32(10), le32(20));
    env.writes.clear();
    EXPECT_EQ(write(1, X430_ADDR_PROFILE_ACCELERATION, data), 0);
    ASSERT_EQ(env.writes.size(), 2U);
    EXPECT_EQ(env.writes[0].address, X430_ADDR_PROFILE_ACCELERATION);
    EXPECT_EQ(env.writes[1].address, X430_ADDR_PROFILE_VELOCITY);
}

TEST_F(X430Test, PartialRegisterWriteMergesBytes)
{
    EXPECT_EQ(write32(1, X430_ADDR_PROFILE_VELOCITY, 0x0102), 0);
    EXPECT_EQ(write8(1, X430_ADDR_PROFILE_VELOCITY + 1, 0x03), 0);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 0x0302);
}

TEST_F(X430Test, ReservedAddressesReadZeroAndIgnoreWrites)
{
    EXPECT_EQ(write(1, 14, {1, 2, 3}), 0);
    auto status = read(1, 14, 3);
    EXPECT_EQ(status.data, (std::vector<uint8_t>{0, 0, 0}));
}

TEST_F(X430Test, WriteHookCanRefuse)
{
    env.refuse_address = X430_ADDR_TORQUE_ENABLE;
    env.refuse_address_error = DXL2_ERR_RESULT_FAIL;
    EXPECT_EQ(write8(1, X430_ADDR_TORQUE_ENABLE, 1), DXL2_ERR_RESULT_FAIL);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_TORQUE_ENABLE), 0);
}

TEST_F(X430Test, IdChangeAnswersWithOldIdAndChecksUniqueness)
{
    add_device(1, 2U);
    EXPECT_EQ(write8(1, X430_ADDR_ID, 2), DXL2_ERR_DATA_RANGE);
    auto status = transact(1, DXL2_INST_WRITE, cat(le16(X430_ADDR_ID), {7}));
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].id, 1);
    EXPECT_EQ(status[0].error, 0);
    EXPECT_EQ(transact(7, DXL2_INST_PING, {}).size(), 1U);
    EXPECT_TRUE(transact(1, DXL2_INST_PING, {}).empty());
}

TEST_F(X430Test, BaudRateEventComesAfterTheStatus)
{
    EXPECT_EQ(write8(1, X430_ADDR_BAUD_RATE, 3), 0);
    ASSERT_EQ(env.events.size(), 1U);
    EXPECT_EQ(env.events[0], X430_EVENT_BAUD_RATE);
    env.events.clear();
    EXPECT_EQ(write8(1, X430_ADDR_BAUD_RATE, 3), 0); /* same value: no event */
    EXPECT_TRUE(env.events.empty());
}

TEST_F(X430Test, StatusReturnLevel)
{
    /* The new level already applies to the status of the write that sets it. */
    EXPECT_EQ(write8(1, X430_ADDR_STATUS_RETURN_LEVEL, 1), 0xFF);
    EXPECT_EQ(write32(1, X430_ADDR_PROFILE_VELOCITY, 1), 0xFF); /* no status */
    EXPECT_EQ(read(1, X430_ADDR_ID, 1).error, 0);
    send(1, DXL2_INST_WRITE, cat(le16(X430_ADDR_STATUS_RETURN_LEVEL), {0}));
    EXPECT_TRUE(take_status(env).empty());
    EXPECT_TRUE(transact(1, DXL2_INST_READ, cat(le16(7), le16(1))).empty());
    EXPECT_EQ(transact(1, DXL2_INST_PING, {}).size(), 1U); /* PING always */
}

TEST_F(X430Test, ReturnDelayTimeUsesPortDelay)
{
    env.delay_total = 0;
    (void) transact(1, DXL2_INST_PING, {});
    EXPECT_EQ(env.delay_total, 250U * 2U); /* default 250 x 2 us */
    EXPECT_EQ(write8(1, X430_ADDR_RETURN_DELAY_TIME, 0), 0);
    env.delay_total = 0;
    (void) transact(1, DXL2_INST_PING, {});
    EXPECT_EQ(env.delay_total, 0U);
}

TEST_F(X430Test, UnknownInstructionIsReported)
{
    auto status = transact(1, 0x7F, {});
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].error, DXL2_ERR_INSTRUCTION);
    EXPECT_EQ(transact(1, DXL2_INST_SYNC_READ, cat(le16(0), le16(1)))[0].error,
              DXL2_ERR_INSTRUCTION);
}

TEST_F(X430Test, BroadcastWriteReachesAllWithoutStatus)
{
    add_device(1, 2U);
    send(DXL2_BROADCAST_ID, DXL2_INST_WRITE, cat(le16(X430_ADDR_PROFILE_VELOCITY), le32(77)));
    EXPECT_TRUE(take_status(env).empty());
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 77);
    EXPECT_EQ(x430_device_get(&dev[1], X430_ADDR_PROFILE_VELOCITY), 77);
    EXPECT_TRUE(transact(DXL2_BROADCAST_ID, DXL2_INST_READ, cat(le16(0), le16(2))).empty());
    EXPECT_TRUE(transact(DXL2_BROADCAST_ID, 0x7F, {}).empty());
}

TEST_F(X430Test, SecondaryIdActsSilentlyAndCannotWriteEeprom)
{
    add_device(1, 2U);
    EXPECT_EQ(write8(1, X430_ADDR_SECONDARY_ID, 20), 0);
    EXPECT_EQ(write8(2, X430_ADDR_SECONDARY_ID, 20), 0);
    send(20, DXL2_INST_WRITE, cat(le16(X430_ADDR_PROFILE_VELOCITY), le32(9)));
    EXPECT_TRUE(take_status(env).empty());
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 9);
    EXPECT_EQ(x430_device_get(&dev[1], X430_ADDR_PROFILE_VELOCITY), 9);
    send(20, DXL2_INST_WRITE, cat(le16(X430_ADDR_RETURN_DELAY_TIME), {3}));
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_RETURN_DELAY_TIME), 250);
    /* Primary ID wins: ID 2 answers, ID 1 (secondary 2) acts silently. */
    EXPECT_EQ(write8(1, X430_ADDR_SECONDARY_ID, 2), 0);
    auto status = transact(2, DXL2_INST_WRITE, cat(le16(X430_ADDR_PROFILE_VELOCITY), le32(5)));
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].id, 2);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 5);
    /* 253..255 disable the Secondary ID. */
    EXPECT_EQ(write8(1, X430_ADDR_SECONDARY_ID, 253), 0);
    send(253, DXL2_INST_WRITE, cat(le16(X430_ADDR_PROFILE_VELOCITY), le32(1)));
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 5);
}

TEST_F(X430Test, ExternalLinkEntryPoint)
{
    static struct dxl2_packet packet;
    packet.id = 1;
    packet.instruction = DXL2_INST_PING;
    packet.parameter_length = 0;
    x430_bus_handle_packet(&packet, &bus);
    EXPECT_EQ(take_status(env).size(), 1U);
    x430_bus_handle_packet(nullptr, &bus);
    x430_bus_handle_packet(&packet, nullptr);
    EXPECT_TRUE(take_status(env).empty());
}

#if X430_MODEL_HAS_LED
TEST_F(X430Test, LedRegister)
{
    EXPECT_EQ(write8(1, X430_ADDR_LED, 1), 0);
    EXPECT_EQ(write8(1, X430_ADDR_LED, 2), DXL2_ERR_DATA_RANGE);
}
#else
TEST_F(X430Test, NoLedOnThisModel)
{
    EXPECT_EQ(write8(1, X430_ADDR_LED, 1), 0); /* reserved address: ignored */
    EXPECT_EQ(read(1, X430_ADDR_LED, 1).data[0], 0);
}
#endif

#if X430_MODEL_HAS_CURRENT
TEST_F(X430Test, GoalCurrentWithinCurrentLimit)
{
    const int32_t limit = x430_device_get(&dev[0], X430_ADDR_CURRENT_LIMIT);
    EXPECT_GT(limit, 0);
    EXPECT_EQ(write16(1, X430_ADDR_GOAL_CURRENT, -limit), 0);
    EXPECT_EQ(write16(1, X430_ADDR_GOAL_CURRENT, limit + 1), DXL2_ERR_DATA_RANGE);
}
#else
TEST_F(X430Test, NoCurrentRegistersOnThisModel)
{
    EXPECT_EQ(write16(1, X430_ADDR_CURRENT_LIMIT, 10), 0); /* reserved: ignored */
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_CURRENT_LIMIT), 0);
    EXPECT_EQ(write16(1, X430_ADDR_PRESENT_LOAD, 1), DXL2_ERR_ACCESS);
}
#endif
