/**
 * @file x430_test_support.h
 * @brief Test fixture: a bus with captured output, a fake port and fake storage.
 *
 * Project: sfs-dynamixel-x430 - DYNAMIXEL X430 Control Table Emulator
 * Author: Anton Vynohradov
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef X430_TEST_SUPPORT_H
#define X430_TEST_SUPPORT_H

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <map>
#include <vector>

extern "C" {
#include "x430.h"
}

/** One status packet received from the bus. */
struct Status
{
    uint8_t id;
    uint8_t error;
    std::vector<uint8_t> data;
};

/** Write hook record. */
struct WriteCall
{
    uint16_t address;
    uint8_t size;
    int32_t value;
};

/** Everything the hooks and the port see, plus their scripted answers. */
struct TestEnv
{
    std::vector<uint8_t> wire;                        /* raw bytes written by the bus */
    std::vector<WriteCall> writes;                    /* write hook calls */
    std::vector<std::pair<uint16_t, uint16_t>> reads; /* read hook calls */
    std::vector<x430_event> events;                   /* event hook calls */
    std::map<int, std::vector<uint8_t>> nvm;          /* stored blocks per slot */
    int nvm_stores = 0;                               /* blocks started (nvm_write at offset 0) */
    int nvm_reads = 0;                                /* nvm_read calls */
    int nvm_writes = 0;                               /* nvm_write calls */
    int nvm_read_fail_call = 0;       /* nvm_read call number (1, 2, ...) that fails, 0 = none */
    int nvm_write_fail_call = 0;      /* nvm_write call number (1, 2, ...) that fails, 0 = none */
    uint8_t refuse_address_error = 0; /* write hook refuses this address... */
    uint16_t refuse_address = 0xFFFF; /* ...with this error */
    uint8_t event_result = DXL2_ERR_NONE;
    bool nvm_store_fails = false;
    bool write_fails = false; /* the bus writer returns -1 */
    uint32_t now = 1000;
    uint32_t delay_total = 0;
    int locks = 0;
    int unlocks = 0;
};

extern TestEnv* g_env;

int test_write(void* context, const uint8_t* data, size_t length);
extern const struct x430_hooks test_hooks;
extern const struct x430_port test_port;

/** Parses every packet in TestEnv::wire into status packets (consumes the wire). */
std::vector<Status> take_status(TestEnv& env);

/** Fixture with one bus and up to three devices (IDs 1, 2, 3 unless changed). */
class X430Test : public ::testing::Test {
  protected:
    TestEnv env;
    struct x430_bus bus;
    struct x430_device dev[3];

    void SetUp() override;
    void TearDown() override;

    /** (Re)initializes device @p index with @p id and attaches it. */
    void add_device(int index, uint8_t id, const struct x430_hooks* hooks = &test_hooks);

    /** Sends one instruction packet to the bus (encoded with DXL2-C). */
    void send(uint8_t id, uint8_t instruction, const std::vector<uint8_t>& params);

    /** Sends and returns the status packets produced. */
    std::vector<Status> transact(uint8_t id, uint8_t instruction,
                                 const std::vector<uint8_t>& params);

    /** READ helper: error and data of one device. */
    Status read(uint8_t id, uint16_t address, uint16_t length);

    /** WRITE helper returning the status error (0xFF when no status came back). */
    uint8_t write(uint8_t id, uint16_t address, const std::vector<uint8_t>& data);
    uint8_t write8(uint8_t id, uint16_t address, uint8_t value);
    uint8_t write16(uint8_t id, uint16_t address, int32_t value);
    uint8_t write32(uint8_t id, uint16_t address, int32_t value);
};

/** Register value through x430_device_get(); fails the test on an error status. */
int32_t reg(const struct x430_device* device, uint16_t address);

/** ID through x430_device_id(); fails the test on an error status. */
uint8_t id_of(const struct x430_device* device);

std::vector<uint8_t> le16(uint32_t value);
std::vector<uint8_t> le32(uint32_t value);
std::vector<uint8_t> cat(std::vector<uint8_t> a, const std::vector<uint8_t>& b);
int32_t get_i32(const std::vector<uint8_t>& data, size_t offset);

#endif /* X430_TEST_SUPPORT_H */
