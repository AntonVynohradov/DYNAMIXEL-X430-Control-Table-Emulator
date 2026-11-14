// SPDX-License-Identifier: Apache-2.0
// End-to-end check of the emulator with the official ROBOTIS DynamixelSDK (C++):
// PING, broadcast PING, READ/WRITE, Indirect Address, Sync/Fast Sync Read,
// Bulk/Fast Bulk Read, Sync/Bulk Write, REG_WRITE/ACTION, REBOOT, CLEAR,
// FACTORY_RESET. Usage: sdk_check <port> <expected model number>
#include "dynamixel_sdk.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <unistd.h>
using namespace dynamixel;
static int fails = 0;
#define CHECK(c, ...)                                   \
    do                                                  \
    {                                                   \
        if (!(c))                                       \
        {                                               \
            fails++;                                    \
            printf("FAIL %s:%d: ", __FILE__, __LINE__); \
            printf(__VA_ARGS__);                        \
            printf("\n");                               \
        }                                               \
        else                                            \
        {                                               \
            printf("ok   %s\n", #c);                    \
        }                                               \
    } while (0)
int main(int argc, char** argv)
{
    const char* dev = argc > 1 ? argv[1] : "/tmp/ttyDXL";
    PortHandler* port = PortHandler::getPortHandler(dev);
    PacketHandler* ph = PacketHandler::getPacketHandler(2.0);
    if (!port->openPort() || !port->setBaudRate(1000000))
    {
        printf("open failed\n");
        return 2;
    }
    uint8_t err = 0;
    uint16_t model = 0;
    int r = ph->ping(port, 1, &model, &err);
    CHECK(r == COMM_SUCCESS && model == (uint16_t)atoi(argv[2]), "ping r=%d model=%u", r, model);
    std::vector<uint8_t> ids;
    r = ph->broadcastPing(port, ids);
    CHECK(r == COMM_SUCCESS && ids.size() == 3 && ids[0] == 1 && ids[1] == 2 && ids[2] == 3,
          "broadcast r=%d n=%zu", r, ids.size());
    uint8_t fw = 0;
    r = ph->read1ByteTxRx(port, 2, 6, &fw, &err);
    CHECK(r == COMM_SUCCESS && fw == 52, "fw %u", fw);
    // configure like dynamixel_hardware_interface: torque off, indirect read of present pos+vel (block 2)
    for (uint8_t id = 1; id <= 3; id++)
    {
        r = ph->write1ByteTxRx(port, id, 64, 0, &err);
        CHECK(r == COMM_SUCCESS && err == 0, "torque off");
        r = ph->write1ByteTxRx(port, id, 9, 0, &err);
        CHECK(r == COMM_SUCCESS && err == 0, "rdt");
        for (int i = 0; i < 8; i++)
        {
            r = ph->write2ByteTxRx(port, id, 578 + 2 * i, (i < 4 ? 132 : 124) + (i % 4), &err);
        }
        CHECK(r == COMM_SUCCESS && err == 0, "indirect");
        r = ph->write4ByteTxRx(port, id, 112, 200, &err);
        CHECK(err == 0, "profile vel");
        r = ph->write4ByteTxRx(port, id, 108, 50, &err);
        CHECK(err == 0, "profile acc");
        r = ph->write1ByteTxRx(port, id, 64, 1, &err);
        CHECK(r == COMM_SUCCESS && err == 0, "torque on");
    }
    r = ph->write1ByteTxRx(port, 1, 11, 1, &err);
    CHECK(r == COMM_SUCCESS && err == 7, "EEPROM locked err=%u", err);
    // sync write goals
    GroupSyncWrite sw(port, ph, 116, 4);
    int32_t goals[4] = {0, 1000, 2000, 3000};
    for (uint8_t id = 1; id <= 3; id++)
    {
        uint8_t p[4] = {DXL_LOBYTE(DXL_LOWORD(goals[id])), DXL_HIBYTE(DXL_LOWORD(goals[id])),
                        DXL_LOBYTE(DXL_HIWORD(goals[id])), DXL_HIBYTE(DXL_HIWORD(goals[id]))};
        sw.addParam(id, p);
    }
    r = sw.txPacket();
    CHECK(r == COMM_SUCCESS, "sync write");
    usleep(3000000);
    // sync read via indirect
    GroupSyncRead sr(port, ph, 634, 8);
    for (uint8_t id = 1; id <= 3; id++)
        sr.addParam(id);
    r = sr.txRxPacket();
    CHECK(r == COMM_SUCCESS, "sync read r=%d", r);
    for (uint8_t id = 1; id <= 3; id++)
    {
        int32_t pos = sr.getData(id, 634, 4);
        CHECK(pos == goals[id], "sync read id %u pos %d", id, pos);
    }
    // fast sync read
    GroupFastSyncRead fsr(port, ph, 634, 8);
    for (uint8_t id = 1; id <= 3; id++)
        fsr.addParam(id);
    r = fsr.txRxPacket();
    CHECK(r == COMM_SUCCESS, "fast sync read r=%d", r);
    for (uint8_t id = 1; id <= 3; id++)
    {
        CHECK(fsr.isAvailable(id, 634, 4), "avail");
        int32_t pos = fsr.getData(id, 634, 4);
        CHECK(pos == goals[id], "fast sync id %u pos %d", id, pos);
    }
    // bulk read + fast bulk read with different items
    GroupBulkRead br(port, ph);
    br.addParam(1, 132, 4);
    br.addParam(2, 122, 1);
    br.addParam(3, 0, 2);
    r = br.txRxPacket();
    CHECK(r == COMM_SUCCESS, "bulk read r=%d", r);
    CHECK((int32_t)br.getData(1, 132, 4) == 1000, "bulk pos");
    CHECK(br.getData(2, 122, 1) == 0, "bulk moving");
    CHECK(br.getData(3, 0, 2) == model, "bulk model");
    GroupFastBulkRead fbr(port, ph);
    fbr.addParam(1, 132, 4);
    fbr.addParam(2, 122, 1);
    fbr.addParam(3, 0, 2);
    r = fbr.txRxPacket();
    CHECK(r == COMM_SUCCESS, "fast bulk read r=%d", r);
    CHECK((int32_t)fbr.getData(1, 132, 4) == 1000, "fbulk pos %d", (int)fbr.getData(1, 132, 4));
    CHECK(fbr.getData(3, 0, 2) == model, "fbulk model");
    // bulk write
    GroupBulkWrite bw(port, ph);
    uint8_t led[1] = {1};
    uint8_t vel[4] = {100, 0, 0, 0};
    bw.addParam(1, 112, 4, vel);
    bw.addParam(2, 112, 4, vel);
    r = bw.txPacket();
    CHECK(r == COMM_SUCCESS, "bulk write");
    (void)led;
    uint32_t v = 0;
    ph->read4ByteTxRx(port, 2, 112, &v, &err);
    CHECK(v == 100, "bulk write applied %u", v);
    // range error, reg write/action, reboot
    r = ph->write4ByteTxRx(port, 1, 116, 5000, &err);
    CHECK(r == COMM_SUCCESS && err == 4, "range err %u", err);
    r = ph->regWriteTxRx(port, 3, 116, 4, (uint8_t*)"\x00\x00\x00\x00", &err);
    CHECK(r == COMM_SUCCESS && err == 0, "regwrite");
    r = ph->action(port, 3);
    CHECK(r == COMM_SUCCESS, "action");
    usleep(3500000);
    uint32_t p3 = 1;
    ph->read4ByteTxRx(port, 3, 132, &p3, &err);
    CHECK(p3 == 0, "after action pos %u", p3);
    r = ph->reboot(port, 2, &err);
    CHECK(r == COMM_SUCCESS && err == 0, "reboot");
    uint8_t t = 9;
    ph->read1ByteTxRx(port, 2, 64, &t, &err);
    CHECK(t == 0, "torque after reboot %u", t);
    r = ph->clearMultiTurn(port, 1, &err);
    CHECK(r == COMM_SUCCESS && err == 0, "clear multi-turn err=%u", err);
    r = ph->factoryReset(port, 3, 0x01, &err);
    CHECK(r == COMM_SUCCESS && err == 0, "factory reset");
    usleep(100000);
    r = ph->ping(port, 3, &model, &err);
    CHECK(r == COMM_SUCCESS, "ping after reset");
    port->closePort();
    printf("%s (%d failures)\n", fails ? "FAILED" : "ALL OK", fails);
    return fails ? 1 : 0;
}
