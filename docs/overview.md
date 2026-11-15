# Introduction

DYNAMIXEL-X430-Control-Table-Emulator (the *X430 emulator*) is a C11 library that makes a
microcontroller answer on a DYNAMIXEL bus like a ROBOTIS X430 actuator. It decodes
Protocol 2.0 instruction packets (with [DXL2-C](https://github.com/AntonVynohradov/DXL2-C)),
keeps the Control Table of the selected X430 model, enforces the rules a real actuator
enforces and answers with status packets. The motor behind it can be anything — a stepper,
a BLDC, a simulation: the library reaches it only through hooks, so the host tools of the
DYNAMIXEL ecosystem work unchanged. DynamixelSDK (end to end) and DYNAMIXEL Wizard 2.0
(on hardware) are verified; ROS 2 `dynamixel_hardware_interface` uses the same
instructions ([points to check](integration.md#ros-2-dynamixel_hardware_interface)).

## What the library does

- **The X430 Control Table.** One of 17 models per build, generated from YAML data
  that the test suite compares with the ROBOTIS e-Manual tables: addresses, sizes, access,
  initial values and ranges, limit registers, the EEPROM lock while the torque is on.
- **Every instruction a host uses.** PING, READ, WRITE, REG_WRITE, ACTION,
  FACTORY_RESET, REBOOT, CLEAR, BACKUP, SYNC_READ/WRITE, BULK_READ/WRITE, FAST_SYNC_READ,
  FAST_BULK_READ, with Status Return Level, Return Delay Time, Secondary ID and broadcast.
- **The rules around the motor.** Indirect Address/Data, Bus Watchdog, hardware error
  shutdown and the alert bit, Startup Configuration, Drive Mode "torque on by goal update",
  Goal Position := Present Position on torque on, persistence of the EEPROM area and of the
  Control Table backup with a header and a CRC.
- **A simulator** (optional) that moves the shaft with the X430 profiles, for tests and for
  setting up ROS 2 or the Wizard before the hardware exists.

## What the library does not do

| Concern | Why it is not in the library |
| --- | --- |
| UART, DMA and baud rate drivers | Hardware-specific; the application moves the received bytes in and writes the status packets out in a callback. |
| Motion and current control | The motor is the application's; the write hook receives each accepted goal, limit and mode. |
| Flash or EEPROM drivers | The library reads and writes opaque blocks in parts through `nvm_read` / `nvm_write`; where they live is up to the application. |
| Threads, queues, interrupts | The library defines the contexts and takes an optional lock; the application decides where they run. See the [integration guide](integration.md). |
| Protocol 1.0, other DYNAMIXEL series | Only Protocol 2.0 and the X430 family are emulated. Another family needs its model data. |

## Who it is for

- **Firmware of custom actuators** (stepper or BLDC joints, grippers) that should appear as
  DYNAMIXEL servos to existing host software.
- **Test benches and CI** that need X430 devices without hardware (the simulator and the
  POSIX host example serve them on a pseudo-terminal).
- **Projects that must be reviewed or certified:** MISRA C:2012, 100 % coverage, no heap.

## Key properties

| Property | Value |
| --- | --- |
| Language | C11, no warnings with `-Wall -Wextra -Wpedantic -Wconversion` |
| Dependencies | `<stddef.h>`, `<stdint.h>`, `<stdbool.h>`; DXL2-C 1.0.0 (fetched by CMake) |
| Memory | No heap, no global or static mutable data (`.data` and `.bss` are empty) |
| RAM | 384 B per device, 2,424 B per bus, 32 B per simulator (Cortex-M4, default options; [details](configuration.md#memory-footprint)) |
| Flash | 10,191 B with the model table, plus 850 B of DXL2-C (Cortex-M4, `-Os`, all features) |
| Stack | 424 B worst case, `x430_bus_receive()`, with any packet size (Cortex-M4 `-Os`, [details](configuration.md#stack-usage)) |
| Coding standard | MISRA C:2012, 0 Cppcheck 2.13 addon violations, [2 deviations](misra.md) |
| Tests | 100 % line and branch coverage of `src/` in both unit test presets (131 and 100 GoogleTest cases); model data against the e-Manual; 52 checks end to end with the official DynamixelSDK |
| Thread safety | One bus context per line, an optional lock for the live register updates; see [Threading](architecture.md#threading-and-reentrancy) |
| License | [Apache License 2.0](LICENSE.md) |

The library was verified on hardware in a stepper joint firmware (an STM32G431 board,
Zephyr) with DYNAMIXEL Wizard 2.0; the screenshots in the
[integration guide](integration.md#dynamixel-wizard-20) come from it.

## Terminology

Host
:   The program that sends instruction packets: DynamixelSDK, DYNAMIXEL Wizard, ROS 2.

Bus
:   One serial line (TTL or RS-485) and the devices this firmware hosts on it (`struct x430_bus`).

Device
:   One emulated actuator with one DYNAMIXEL ID (`struct x430_device`). A dual-axis
    2XL430/2XC430 is two devices.

Control Table
:   The register image of a device: EEPROM area (addresses 0–63, persistent, locked
    while the torque is on) and RAM area (64–661).

Hook
:   An application function the device calls: write, read, event, nvm_read, nvm_write.

Live register
:   A read-only RAM register the application keeps current: Present Position, Present
    Velocity, Moving and the others.

## Conventions in this documentation

- Registers are written with their e-Manual name and address: *Goal Position (116)*.
- Protocol errors are the `DXL2_ERR_*` codes of DXL2-C (Result Fail, Instruction, CRC,
  Data Range, Data Length, Data Limit, Access); library status codes are `X430_OK` and
  `X430_ERR_*`.
- The example model is the XM430-W350, the default of the build.
