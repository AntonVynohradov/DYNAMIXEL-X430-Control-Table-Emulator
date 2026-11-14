---
title: DYNAMIXEL-X430-Control-Table-Emulator — any motor as a DYNAMIXEL X430
description: Device-side emulation of DYNAMIXEL X430 actuators over Protocol 2.0 in portable C11, with no heap, no RTOS, MISRA C:2012 and 100 % test coverage.
template: home.html
hide:
  - navigation
  - toc
---

<span class="sfs-eyebrow">Quick start</span>

## One joint on one serial line

The application gives the bus a byte writer and feeds it the received bytes; the device
calls a hook for every accepted register write, and the motor loop keeps the live
registers current. Everything runs in the main loop (or one RX thread).

```c
#include "x430.h"

static struct x430_bus bus;
static struct x430_device joint;

static int uart_write(void* context, const uint8_t* data, size_t length)
{
    (void)context;
    return board_uart_write(data, length); /* 0 on success */
}

static uint8_t on_write(struct x430_device* device, uint16_t address, uint8_t size,
                        int32_t value, void* user_data)
{
    uint8_t result = DXL2_ERR_NONE;

    (void)device;
    (void)size;
    (void)user_data;
    if (address == X430_ADDR_TORQUE_ENABLE)
    {
        if (!motor_enable(value != 0))
        {
            result = DXL2_ERR_RESULT_FAIL; /* the host sees Result Fail */
        }
    }
    else if (address == X430_ADDR_GOAL_POSITION)
    {
        motor_move_to(value);
    }
    else
    {
        /* other registers are only stored */
    }
    return result;
}

static const struct x430_hooks hooks = {.write = on_write};

int main(void)
{
    const struct x430_bus_config bus_config = {.write = uart_write};
    const struct x430_device_config joint_config = {.id = 1U, .hooks = &hooks};

    board_init();
    if ((x430_bus_init(&bus, &bus_config) != X430_OK) ||
        (x430_device_init(&joint, &joint_config) != X430_OK) ||
        (x430_bus_attach(&bus, &joint) != X430_OK))
    {
        for (;;)
        {
        }
    }
    for (;;)
    {
        uint8_t bytes[64];
        const size_t count = board_uart_read(bytes, sizeof(bytes));

        (void)x430_bus_receive(&bus, bytes, count); /* parse, check, hooks, answer */
        if (board_tick_1ms())
        {
            (void)x430_bus_lock(&bus); /* live values from the motor */
            (void)x430_device_set(&joint, X430_ADDR_PRESENT_POSITION, motor_position());
            (void)x430_device_set(&joint, X430_ADDR_MOVING, motor_moving() ? 1 : 0);
            (void)x430_bus_unlock(&bus);
            (void)x430_bus_poll(&bus); /* Bus Watchdog */
        }
    }
}
```

The `board_…` and `motor_…` functions stand for your UART and motor code; the
[Integration Guide](integration.md) shows the RTOS layout, persistent storage and the
port (lock, clock, delay).

## Find your way

| If you want to… | Read |
| --- | --- |
| Understand what the library does and does not do | [Introduction](overview.md) |
| Build it, run the tests, add it to your project | [Getting Started](guide.md) |
| Understand the modules, contexts, states and memory | [How It Works](architecture.md) |
| Set up devices and react to the host | [Using the Library](usage.md) |
| Know exactly what an X430 host sees | [X430 Emulation](theory.md) |
| Choose the model and the build options | [Configuration](configuration.md) |
| Connect it to a UART, an RTOS and flash | [Integration Guide](integration.md) |
| Look up a function | [API Reference](api.md) |
| Read everything offline or print it | [PDF user manual](https://antonvynohradov.github.io/DYNAMIXEL-X430-Control-Table-Emulator/downloads/DYNAMIXEL-X430-Control-Table-Emulator_User_Manual.pdf) |
