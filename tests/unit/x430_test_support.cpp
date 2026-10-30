/**
 * @file x430_test_support.cpp
 * @brief Test fixture implementation.
 *
 * Project: sfs-dynamixel-x430 - DYNAMIXEL X430 Control Table Emulator
 * Author: Anton Vynohradov
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "x430_test_support.h"

TestEnv* g_env = nullptr;

int test_write(void* context, const uint8_t* data, size_t length)
{
    auto* env = static_cast<TestEnv*>(context);
    env->wire.insert(env->wire.end(), data, data + length);
    return 0;
}

static uint8_t hook_write(struct x430_device*, uint16_t address, uint8_t size, int32_t value, void*)
{
    g_env->writes.push_back({address, size, value});
    if (address == g_env->refuse_address)
    {
        return g_env->refuse_address_error;
    }
    return DXL2_ERR_NONE;
}

static void hook_read(struct x430_device*, uint16_t address, uint16_t length, void*)
{
    g_env->reads.emplace_back(address, length);
}

static uint8_t hook_event(struct x430_device*, enum x430_event event, void*)
{
    g_env->events.push_back(event);
    return g_env->event_result;
}

static int hook_nvm_load(struct x430_device*, enum x430_nvm_slot slot, uint8_t* data, uint16_t size,
                         void*)
{
    auto it = g_env->nvm.find(slot);
    if ((it == g_env->nvm.end()) || (it->second.size() != size))
    {
        return -1;
    }
    std::memcpy(data, it->second.data(), size);
    return 0;
}

static int hook_nvm_store(struct x430_device*, enum x430_nvm_slot slot, const uint8_t* data,
                          uint16_t size, void*)
{
    g_env->nvm_stores++;
    if (g_env->nvm_store_fails)
    {
        return -1;
    }
    g_env->nvm[slot] = std::vector<uint8_t>(data, data + size);
    return 0;
}

const struct x430_hooks test_hooks = {
    hook_write, hook_read, hook_event, hook_nvm_load, hook_nvm_store,
};

static void port_lock(void* context)
{
    static_cast<TestEnv*>(context)->locks++;
}

static void port_unlock(void* context)
{
    static_cast<TestEnv*>(context)->unlocks++;
}

static uint32_t port_millis(void* context)
{
    return static_cast<TestEnv*>(context)->now;
}

static void port_delay(void* context, uint32_t us)
{
    static_cast<TestEnv*>(context)->delay_total += us;
}

const struct x430_port test_port = {nullptr, port_lock, port_unlock, port_millis, port_delay};

std::vector<Status> take_status(TestEnv& env)
{
    std::vector<Status> result;
    struct dxl2_parser parser;
    static struct dxl2_packet packet;

    dxl2_parser_init(&parser);
    for (uint8_t byte : env.wire)
    {
        if (dxl2_parser_feed(&parser, byte, &packet) == 1)
        {
            EXPECT_EQ(packet.instruction, DXL2_INST_STATUS);
            Status status;
            status.id = packet.id;
            status.error = (packet.parameter_length > 0U) ? packet.parameters[0] : 0xEEU;
            status.data.assign(packet.parameters + 1, packet.parameters + packet.parameter_length);
            result.push_back(status);
        }
    }
    env.wire.clear();
    return result;
}

void X430Test::SetUp()
{
    static struct x430_port port;

    g_env = &env;
    port = test_port;
    port.context = &env;
    const struct x430_bus_config config = {test_write, &env, &port};
    ASSERT_EQ(x430_bus_init(&bus, &config), X430_OK);
    add_device(0, 1U);
}

void X430Test::TearDown()
{
    EXPECT_EQ(env.locks, env.unlocks);
    g_env = nullptr;
}

void X430Test::add_device(int index, uint8_t id, const struct x430_hooks* hooks)
{
    const struct x430_device_config config = {id, 0U, 0x12345678U, hooks, &env};
    ASSERT_EQ(x430_device_init(&dev[index], &config), X430_OK);
    ASSERT_EQ(x430_bus_attach(&bus, &dev[index]), X430_OK);
}

void X430Test::send(uint8_t id, uint8_t instruction, const std::vector<uint8_t>& params)
{
    uint8_t raw[DXL2_MAX_PACKET_SIZE];
    size_t length = 0;

    ASSERT_EQ(dxl2_encode(id, instruction, params.empty() ? nullptr : params.data(),
                          (uint16_t) params.size(), raw, sizeof(raw), &length),
              0);
    x430_bus_receive(&bus, raw, length);
}

std::vector<Status> X430Test::transact(uint8_t id, uint8_t instruction,
                                       const std::vector<uint8_t>& params)
{
    send(id, instruction, params);
    return take_status(env);
}

Status X430Test::read(uint8_t id, uint16_t address, uint16_t length)
{
    auto status = transact(id, DXL2_INST_READ, cat(le16(address), le16(length)));
    EXPECT_EQ(status.size(), 1U);
    return status.empty() ? Status{0, 0xFF, {}} : status[0];
}

uint8_t X430Test::write(uint8_t id, uint16_t address, const std::vector<uint8_t>& data)
{
    auto status = transact(id, DXL2_INST_WRITE, cat(le16(address), data));
    return status.empty() ? 0xFFU : status[0].error;
}

uint8_t X430Test::write8(uint8_t id, uint16_t address, uint8_t value)
{
    return write(id, address, {value});
}

uint8_t X430Test::write16(uint8_t id, uint16_t address, int32_t value)
{
    return write(id, address, le16((uint32_t) value));
}

uint8_t X430Test::write32(uint8_t id, uint16_t address, int32_t value)
{
    return write(id, address, le32((uint32_t) value));
}

std::vector<uint8_t> le16(uint32_t value)
{
    return {(uint8_t) value, (uint8_t) (value >> 8)};
}

std::vector<uint8_t> le32(uint32_t value)
{
    return {(uint8_t) value, (uint8_t) (value >> 8), (uint8_t) (value >> 16),
            (uint8_t) (value >> 24)};
}

std::vector<uint8_t> cat(std::vector<uint8_t> a, const std::vector<uint8_t>& b)
{
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

int32_t get_i32(const std::vector<uint8_t>& data, size_t offset)
{
    return (int32_t) ((uint32_t) data[offset] | ((uint32_t) data[offset + 1] << 8)
                      | ((uint32_t) data[offset + 2] << 16) | ((uint32_t) data[offset + 3] << 24));
}
