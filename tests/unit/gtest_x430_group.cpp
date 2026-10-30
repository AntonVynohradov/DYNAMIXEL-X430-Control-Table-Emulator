/**
 * @file gtest_x430_group.cpp
 * @brief Indirect addressing, REG_WRITE/ACTION and group instructions.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "x430_test_support.h"

namespace
{
constexpr uint16_t kIndirectAddress1 = 168;
constexpr uint16_t kIndirectData1 = 224;
constexpr uint16_t kIndirectAddress29 = 578;
constexpr uint16_t kIndirectData29 = 634;

/** Maps @p count consecutive addresses starting at @p target to Indirect slot @p slot. */
std::vector<uint8_t> indirect_map(uint16_t target, uint16_t count)
{
    std::vector<uint8_t> data;
    for (uint16_t i = 0; i < count; i++)
    {
        auto bytes = le16(target + i);
        data.insert(data.end(), bytes.begin(), bytes.end());
    }
    return data;
}
}  // namespace

TEST_F(X430Test, IndirectWriteReachesTargetRegister)
{
    EXPECT_EQ(write(1, kIndirectAddress1, indirect_map(X430_ADDR_PROFILE_VELOCITY, 4)), 0);
    env.writes.clear();
    EXPECT_EQ(write32(1, kIndirectData1, 345), 0);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 345);
    ASSERT_EQ(env.writes.size(), 1U);
    EXPECT_EQ(env.writes[0].address, X430_ADDR_PROFILE_VELOCITY);
    EXPECT_EQ(env.writes[0].value, 345);
    EXPECT_EQ(get_i32(read(1, kIndirectData1, 4).data, 0), 345);
}

TEST_F(X430Test, IndirectInheritsAccessAndRange)
{
    EXPECT_EQ(write(1, kIndirectAddress1, indirect_map(X430_ADDR_PRESENT_POSITION, 4)), 0);
    EXPECT_EQ(write32(1, kIndirectData1, 1), DXL2_ERR_ACCESS);
    EXPECT_EQ(write(1, kIndirectAddress1, indirect_map(X430_ADDR_STATUS_RETURN_LEVEL, 1)), 0);
    EXPECT_EQ(write8(1, kIndirectData1, 3), DXL2_ERR_DATA_RANGE);
}

TEST_F(X430Test, IndirectAddressMustBeRam)
{
    EXPECT_EQ(write16(1, kIndirectAddress1, X430_ADDR_ID), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(write16(1, kIndirectAddress1, X430_TABLE_SIZE), DXL2_ERR_DATA_RANGE);
    EXPECT_EQ(write16(1, kIndirectAddress1, X430_RAM_START), 0);
}

TEST_F(X430Test, IndirectReadOfLiveRegistersLikeRos2)
{
    /* dynamixel_hardware_interface maps Present Position and Velocity into block 2. */
    auto map = cat(indirect_map(X430_ADDR_PRESENT_POSITION, 4),
                   indirect_map(X430_ADDR_PRESENT_VELOCITY, 4));
    EXPECT_EQ(write(1, kIndirectAddress29, map), 0);
    ASSERT_EQ(x430_device_set(&dev[0], X430_ADDR_PRESENT_POSITION, -1234), X430_OK);
    ASSERT_EQ(x430_device_set(&dev[0], X430_ADDR_PRESENT_VELOCITY, 56), X430_OK);
    env.reads.clear();
    auto status = read(1, kIndirectData29, 8);
    ASSERT_EQ(status.error, 0);
    EXPECT_EQ(get_i32(status.data, 0), -1234);
    EXPECT_EQ(get_i32(status.data, 4), 56);
    /* The read hook sees the real addresses: 132..135 and 128..131. */
    ASSERT_EQ(env.reads.size(), 2U);
    EXPECT_EQ(env.reads[0].first, 132);
    EXPECT_EQ(env.reads[0].second, 4);
    EXPECT_EQ(env.reads[1].first, 128);
    EXPECT_EQ(env.reads[1].second, 4);
}

TEST_F(X430Test, IndirectDataPointingAtIndirectDataStoresPlainBytes)
{
    EXPECT_EQ(write16(1, kIndirectAddress1, kIndirectData29), 0);
    EXPECT_EQ(write8(1, kIndirectData1, 0x5A), 0);
    EXPECT_EQ(read(1, kIndirectData1, 1).data[0], 0x5A);
}

TEST_F(X430Test, RegWriteThenAction)
{
    auto status = transact(1, DXL2_INST_REG_WRITE, cat(le16(X430_ADDR_PROFILE_VELOCITY), le32(50)));
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].error, 0);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 0);
    EXPECT_EQ(read(1, X430_ADDR_REGISTERED_INSTRUCTION, 1).data[0], 1);
    status = transact(1, DXL2_INST_ACTION, {});
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].error, 0);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 50);
    EXPECT_EQ(read(1, X430_ADDR_REGISTERED_INSTRUCTION, 1).data[0], 0);
    /* ACTION without REG_WRITE is an Instruction Error. */
    EXPECT_EQ(transact(1, DXL2_INST_ACTION, {})[0].error, DXL2_ERR_INSTRUCTION);
    EXPECT_EQ(transact(1, DXL2_INST_ACTION, {1})[0].error, DXL2_ERR_DATA_LENGTH);
}

TEST_F(X430Test, RegWriteIsValidatedWhenRegistered)
{
    EXPECT_EQ(
        transact(1, DXL2_INST_REG_WRITE, cat(le16(X430_ADDR_STATUS_RETURN_LEVEL), {9}))[0].error,
        DXL2_ERR_DATA_RANGE);
    std::vector<uint8_t> big(X430_REG_WRITE_SIZE + 1U, 0);
    EXPECT_EQ(transact(1, DXL2_INST_REG_WRITE, cat(le16(X430_ADDR_GOAL_PWM), big))[0].error,
              DXL2_ERR_DATA_LENGTH);
    EXPECT_EQ(transact(1, DXL2_INST_REG_WRITE, {1, 0})[0].error, DXL2_ERR_DATA_LENGTH);
    EXPECT_EQ(read(1, X430_ADDR_REGISTERED_INSTRUCTION, 1).data[0], 0);
}

TEST_F(X430Test, BroadcastActionExecutesAllDevices)
{
    add_device(1, 2U);
    (void) transact(1, DXL2_INST_REG_WRITE, cat(le16(X430_ADDR_PROFILE_VELOCITY), le32(11)));
    (void) transact(2, DXL2_INST_REG_WRITE, cat(le16(X430_ADDR_PROFILE_VELOCITY), le32(22)));
    EXPECT_TRUE(transact(DXL2_BROADCAST_ID, DXL2_INST_ACTION, {}).empty());
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 11);
    EXPECT_EQ(x430_device_get(&dev[1], X430_ADDR_PROFILE_VELOCITY), 22);
}

TEST_F(X430Test, SyncReadAnswersInRequestOrder)
{
    add_device(1, 2U);
    add_device(2, 3U);
    (void) x430_device_set(&dev[2], X430_ADDR_PRESENT_POSITION, 300);
    (void) x430_device_set(&dev[0], X430_ADDR_PRESENT_POSITION, 100);
    auto status = transact(DXL2_BROADCAST_ID, DXL2_INST_SYNC_READ,
                           cat(cat(le16(X430_ADDR_PRESENT_POSITION), le16(4)), {3, 9, 1}));
    ASSERT_EQ(status.size(), 2U); /* ID 9 is not hosted: skipped */
    EXPECT_EQ(status[0].id, 3);
    EXPECT_EQ(get_i32(status[0].data, 0), 300);
    EXPECT_EQ(status[1].id, 1);
    EXPECT_EQ(get_i32(status[1].data, 0), 100);
    /* Malformed requests are ignored. */
    EXPECT_TRUE(transact(DXL2_BROADCAST_ID, DXL2_INST_SYNC_READ, cat(le16(0), le16(0))).empty());
    EXPECT_TRUE(
        transact(DXL2_BROADCAST_ID, DXL2_INST_SYNC_READ, cat(cat(le16(0), le16(0)), {1})).empty());
    /* A read error is reported per device without data. */
    status = transact(DXL2_BROADCAST_ID, DXL2_INST_SYNC_READ,
                      cat(cat(le16(X430_TABLE_SIZE - 1), le16(4)), {1}));
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].error, DXL2_ERR_ACCESS);
}

TEST_F(X430Test, SyncWrite)
{
    add_device(1, 2U);
    auto params = cat(cat(le16(X430_ADDR_PROFILE_VELOCITY), le16(4)),
                      cat(cat({1}, le32(5)), cat({2}, le32(6))));
    params = cat(params, cat({8}, le32(7))); /* ID 8 is not hosted */
    EXPECT_TRUE(transact(DXL2_BROADCAST_ID, DXL2_INST_SYNC_WRITE, params).empty());
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 5);
    EXPECT_EQ(x430_device_get(&dev[1], X430_ADDR_PROFILE_VELOCITY), 6);
    /* Length that does not divide the entries: ignored. */
    params = cat(cat(le16(X430_ADDR_PROFILE_VELOCITY), le16(4)), cat({1}, le16(9)));
    (void) transact(DXL2_BROADCAST_ID, DXL2_INST_SYNC_WRITE, params);
    EXPECT_EQ(x430_device_get(&dev[0], X430_ADDR_PROFILE_VELOCITY), 5);
    (void) transact(DXL2_BROADCAST_ID, DXL2_INST_SYNC_WRITE, le16(0));
}

#if X430_FEATURE_BULK
TEST_F(X430Test, BulkReadAndWrite)
{
    add_device(1, 2U);
    auto params = cat(cat({2}, le16(X430_ADDR_PROFILE_VELOCITY)), le16(4));
    params = cat(params, cat(cat({1}, le16(X430_ADDR_STATUS_RETURN_LEVEL)), le16(1)));
    params = cat(params, cat(cat({1}, le16(0)), le16(0))); /* zero length: skipped */
    auto bulk_write = cat(cat(cat({2}, le16(X430_ADDR_PROFILE_VELOCITY)), le16(4)), le32(66));
    bulk_write = cat(bulk_write, cat(cat(cat({7}, le16(X430_ADDR_LED)), le16(1)), {1}));
    EXPECT_TRUE(transact(DXL2_BROADCAST_ID, DXL2_INST_BULK_WRITE, bulk_write).empty());
    EXPECT_EQ(x430_device_get(&dev[1], X430_ADDR_PROFILE_VELOCITY), 66);

    auto status = transact(DXL2_BROADCAST_ID, DXL2_INST_BULK_READ, params);
    ASSERT_EQ(status.size(), 2U);
    EXPECT_EQ(status[0].id, 2);
    EXPECT_EQ(get_i32(status[0].data, 0), 66);
    EXPECT_EQ(status[1].id, 1);
    EXPECT_EQ(status[1].data, (std::vector<uint8_t>{2}));

    /* Truncated bulk write: nothing is written. */
    auto broken = cat(cat(cat({2}, le16(X430_ADDR_PROFILE_VELOCITY)), le16(4)), le32(1));
    broken = cat(broken, {1, 0});
    (void) transact(DXL2_BROADCAST_ID, DXL2_INST_BULK_WRITE, broken);
    EXPECT_EQ(x430_device_get(&dev[1], X430_ADDR_PROFILE_VELOCITY), 66);
    broken = cat(cat({2}, le16(X430_ADDR_PROFILE_VELOCITY)), le16(0));
    (void) transact(DXL2_BROADCAST_ID, DXL2_INST_BULK_WRITE, broken);
    EXPECT_TRUE(transact(DXL2_BROADCAST_ID, DXL2_INST_BULK_READ, {1, 2}).empty());
}
#endif

#if X430_FEATURE_FAST_READ

namespace
{
/** Checks a raw fast-read answer like DynamixelSDK and returns the per-device segments. */
std::vector<Status> parse_fast(const std::vector<uint8_t>& raw,
                               const std::vector<uint16_t>& lengths)
{
    std::vector<Status> result;
    EXPECT_GE(raw.size(), 10U);
    EXPECT_EQ(raw[0], 0xFF);
    EXPECT_EQ(raw[1], 0xFF);
    EXPECT_EQ(raw[2], 0xFD);
    EXPECT_EQ(raw[4], DXL2_BROADCAST_ID);
    EXPECT_EQ(raw[7], DXL2_INST_STATUS);
    EXPECT_EQ((size_t) (raw[5] | (raw[6] << 8)) + 7U, raw.size());
    size_t index = 8;
    for (uint16_t length : lengths)
    {
        Status status;
        status.error = raw[index];
        status.id = raw[index + 1];
        status.data.assign(raw.begin() + (long) index + 2, raw.begin() + (long) index + 2 + length);
        const size_t end = index + 2 + length;
        const uint16_t crc = (uint16_t) (raw[end] | (raw[end + 1] << 8));
        EXPECT_EQ(crc, dxl2_crc16(raw.data(), end)) << "segment CRC";
        result.push_back(status);
        index += length + 4U;
    }
    EXPECT_EQ(index, raw.size());
    return result;
}
}  // namespace

TEST_F(X430Test, FastSyncReadIsOnePacket)
{
    add_device(1, 2U);
    (void) x430_device_set(&dev[0], X430_ADDR_PRESENT_POSITION, 11);
    (void) x430_device_set(&dev[1], X430_ADDR_PRESENT_POSITION, 22);
    env.wire.clear();
    send(DXL2_BROADCAST_ID, DXL2_INST_FAST_SYNC_READ,
         cat(cat(le16(X430_ADDR_PRESENT_POSITION), le16(4)), {2, 1}));
    auto segments = parse_fast(env.wire, {4, 4});
    ASSERT_EQ(segments.size(), 2U);
    EXPECT_EQ(segments[0].id, 2);
    EXPECT_EQ(get_i32(segments[0].data, 0), 22);
    EXPECT_EQ(segments[1].id, 1);
    EXPECT_EQ(get_i32(segments[1].data, 0), 11);
    /* The whole packet still parses as a valid Protocol 2.0 packet. */
    auto status = take_status(env);
    ASSERT_EQ(status.size(), 1U);
    EXPECT_EQ(status[0].id, DXL2_BROADCAST_ID);
}

TEST_F(X430Test, FastBulkReadWithErrorsAndSkippedIds)
{
    add_device(1, 2U);
    x430_device_raise_hardware_error(&dev[1], X430_HW_ERROR_OVERHEATING);
    auto params = cat(cat({1}, le16(X430_ADDR_ID)), le16(1));
    params = cat(params, cat(cat({5}, le16(X430_ADDR_ID)), le16(1)));        /* not hosted */
    params = cat(params, cat(cat({2}, le16(X430_TABLE_SIZE - 1)), le16(2))); /* access error */
    env.wire.clear();
    send(DXL2_BROADCAST_ID, DXL2_INST_FAST_BULK_READ, params);
    auto segments = parse_fast(env.wire, {1, 2});
    ASSERT_EQ(segments.size(), 2U);
    EXPECT_EQ(segments[0].data, (std::vector<uint8_t>{1}));
    EXPECT_EQ(segments[0].error, 0);
    EXPECT_EQ(segments[1].error, DXL2_ERR_ACCESS | DXL2_ERR_ALERT_BIT);
    EXPECT_EQ(segments[1].data, (std::vector<uint8_t>{0, 0}));
    env.wire.clear();
}

TEST_F(X430Test, FastReadIgnoresMalformedRequests)
{
    env.wire.clear();
    send(DXL2_BROADCAST_ID, DXL2_INST_FAST_SYNC_READ, cat(cat(le16(0), le16(0)), {1}));
    send(DXL2_BROADCAST_ID, DXL2_INST_FAST_SYNC_READ, cat(cat(le16(0), le16(2)), {9}));
    send(DXL2_BROADCAST_ID, DXL2_INST_FAST_BULK_READ, {1, 0, 0});
    send(DXL2_BROADCAST_ID, DXL2_INST_FAST_SYNC_READ,
         cat(cat(le16(0), le16(DXL2_MAX_PACKET_SIZE)), {1}));
    EXPECT_TRUE(env.wire.empty());
}
#endif
