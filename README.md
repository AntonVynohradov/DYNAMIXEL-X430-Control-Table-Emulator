# sfs-dynamixel-x430: DYNAMIXEL X430 Control Table Emulator

Device-side emulation of ROBOTIS DYNAMIXEL **X430** actuators over **Protocol 2.0**,
in portable C11. Your firmware drives any motor (a stepper, a BLDC, a simulation) and
the host sees a genuine X430: DynamixelSDK, DYNAMIXEL Wizard and ROS 2
`dynamixel_hardware_interface` work unchanged.

The Protocol 2.0 wire format comes from [DXL2-C](https://github.com/AntonVynohradov/DXL2-C)
(fetched with CMake FetchContent). This library adds what sits above it: the
Control Table of the selected model and the rules a real device enforces.

> DYNAMIXEL and ROBOTIS are trademarks of ROBOTIS Co., Ltd. This is an independent
> implementation and is not affiliated with or endorsed by ROBOTIS.

## Highlights

- **All 17 X430 models**, one model per build, chosen with Kconfig or a CMake variable.
  Control Tables are data (`models/*.yaml`), generated into C and checked against the
  e-Manual tables by the test suite.
- **Table and protocol only.** Motion is yours: the library calls your hooks for every
  accepted write and lets you update live registers (Present Position, ...).
- **Every instruction a host uses**: PING, READ, WRITE, REG_WRITE, ACTION,
  FACTORY_RESET, REBOOT, CLEAR, BACKUP, SYNC_READ/WRITE, BULK_READ/WRITE,
  FAST_SYNC_READ, FAST_BULK_READ.
- **Device rules**: access rights, value ranges and limit registers, EEPROM lock while
  torque is on, Indirect Address/Data, Secondary ID, Status Return Level, Return Delay
  Time, Bus Watchdog, hardware error shutdown and alert bit, Startup Configuration,
  Control Table backup, Goal Position := Present Position on torque on.
- **No heap, no RTOS, no drivers.** Optional port for a mutex, a millisecond clock and a
  delay. Persistent storage through two hooks; the library adds header and CRC.
- **Simulator** (`x430_sim`) that makes the devices move without hardware, and a POSIX
  host example that serves them on a pseudo-terminal or a serial port.
- Verified end to end with the official ROBOTIS **DynamixelSDK** (C++), including Fast
  Sync/Bulk Read and Indirect Address as used by ROS 2.

## Supported models

| Model | Kconfig / `X430_MODEL` | Model Number | Operating modes | Current Limit (38) | Velocity Limit (44) | Notes |
|---|---|---:|---|---:|---:|---|
| XM430-W350 | `xm430_w350` | 1020 | 0, 1, 3, 4, 5, 16 | 1193 × 2.69 mA | 200 |  |
| XM430-W210 | `xm430_w210` | 1030 | 0, 1, 3, 4, 5, 16 | 1193 × 2.69 mA | 330 |  |
| XH430-W350 | `xh430_w350` | 1000 | 0, 1, 3, 4, 5, 16 | 648 × 2.69 mA | 130 |  |
| XH430-W210 | `xh430_w210` | 1010 | 0, 1, 3, 4, 5, 16 | 648 × 2.69 mA | 210 |  |
| XH430-V350 | `xh430_v350` | 1040 | 0, 1, 3, 4, 5, 16 | 689 × 1.34 mA | 135 |  |
| XH430-V210 | `xh430_v210` | 1050 | 0, 1, 3, 4, 5, 16 | 689 × 1.34 mA | 230 |  |
| XD430-T350 | `xd430_t350` | 1001 | 0, 1, 3, 4, 5, 16 | 648 × 2.69 mA | 130 |  |
| XD430-T210 | `xd430_t210` | 1011 | 0, 1, 3, 4, 5, 16 | 648 × 2.69 mA | 210 |  |
| XW430-T333 | `xw430_t333` | 1270 | 0, 1, 3, 4, 5, 16 | 648 × 2.69 mA | 139 | no LED |
| XW430-T200 | `xw430_t200` | 1280 | 0, 1, 3, 4, 5, 16 | 648 × 2.69 mA | 235 | no LED; see [models](docs/models.md) |
| XL430-W250 | `xl430_w250` | 1060 | 1, 3, 4, 16 | – (Present Load) | 265 |  |
| 2XL430-W250 | `2xl430_w250` | 1090 | 1, 3, 4, 16 | – (Present Load) | 250 | dual axis: 2 devices |
| XC430-W150 | `xc430_w150` | 1070 | 1, 3, 4, 16 | – (Present Load) | 460 |  |
| XC430-W240 | `xc430_w240` | 1080 | 1, 3, 4, 16 | – (Present Load) | 306 |  |
| XC430-T150BB | `xc430_t150bb` | 1070 | 1, 3, 4, 16 | – (Present Load) | 460 | same number as XC430-W150 |
| XC430-T240BB | `xc430_t240bb` | 1080 | 1, 3, 4, 16 | – (Present Load) | 306 | same number as XC430-W240 |
| 2XC430-W250 | `2xc430_w250` | 1160 | 1, 3, 4, 16 | – (Present Load) | 275 | dual axis: 2 devices |

Operating modes: 0 current, 1 velocity, 3 position, 4 extended position,
5 current-based position, 16 PWM.

## Architecture

```
 host (ROS 2, DynamixelSDK, Wizard)
        │  Protocol 2.0 bytes
 ┌──────▼──────────────────────────────────────────────────────────┐
 │ x430_bus      DXL2-C link (parser/encoder) + instruction decoding │
 │               PING READ WRITE REG_WRITE ACTION REBOOT CLEAR ...    │
 │               SYNC/BULK/FAST group instructions, Bus Watchdog       │
 ├───────────────┬──────────────────┬──────────────────────────────┤
 │ x430_device   │ x430_device      │ ... up to X430_MAX_DEVICES     │
 │ Control Table image + rules of the generated model (x430_model)    │
 └──────┬────────┴──────────────────┴──────────────────────────────┘
        │ hooks: write / read / event / nvm_load / nvm_store
 ┌──────▼─────────────────────────┐   ┌──────────────────────────────┐
 │ your motor backend             │ or│ x430_sim (simulated actuator) │
 └────────────────────────────────┘   └──────────────────────────────┘
        optional x430_port: lock/unlock, millis, delay_us
```

## Quick start

### Build

```bash
python3 -m pip install -r requirements.txt     # PyYAML, Kconfiglib
cmake --preset "X430[Debug]"
cmake --build "build/X430[Debug]"
cmake --build "build/X430[Debug]" --target menuconfig   # choose the model, features
```

| Preset             | Purpose                                                        |
|--------------------|----------------------------------------------------------------|
| `X430[Debug]`      | Static library, Kconfig with `configs/defconfig`               |
| `X430[Release]`    | Optimized static library                                       |
| `X430[UnitTests]`  | GoogleTest suite, model checks; `--target coverage`            |
| `X430[Examples]`   | `x430_host_sim` (POSIX)                                        |
| `X430[NoKconfig]`  | No Kconfiglib needed; `-DX430_MODEL=<model>`                   |

### Use it in your project

```cmake
set(X430_CONF_FILES "${CMAKE_SOURCE_DIR}/x430.conf")   # or -DX430_KCONFIG=OFF -DX430_MODEL=xl430_w250
include(path/to/sfs-dynamixel-x430/x430.cmake)
target_link_libraries(app PRIVATE x430)
```

```c
#include "x430.h"

static struct x430_bus bus;
static struct x430_device joint;

static int uart_write(void* context, const uint8_t* data, size_t length)
{
    (void) context;
    return board_uart_write(data, length);             /* 0 on success */
}

static uint8_t on_write(struct x430_device* dev, uint16_t address, uint8_t size, int32_t value,
                        void* user)
{
    if (address == X430_ADDR_GOAL_POSITION)
    {
        motor_move_to(value);                           /* your motion control */
    }
    else if (address == X430_ADDR_TORQUE_ENABLE)
    {
        return motor_enable(value != 0) ? DXL2_ERR_NONE : DXL2_ERR_RESULT_FAIL;
    }
    return DXL2_ERR_NONE;
}

static const struct x430_hooks hooks = {.write = on_write};

int main(void)
{
    const struct x430_bus_config bus_config = {.write = uart_write};
    const struct x430_device_config joint_config = {.id = 1U, .hooks = &hooks};

    x430_bus_init(&bus, &bus_config);
    x430_device_init(&joint, &joint_config);
    x430_bus_attach(&bus, &joint);

    for (;;)
    {
        uint8_t bytes[64];
        size_t count = board_uart_read(bytes, sizeof(bytes));

        x430_bus_receive(&bus, bytes, count);

        x430_bus_lock(&bus);                            /* live values from the motor */
        x430_device_set(&joint, X430_ADDR_PRESENT_POSITION, motor_position());
        x430_device_set(&joint, X430_ADDR_MOVING, motor_moving());
        x430_bus_unlock(&bus);

        x430_bus_poll(&bus);                            /* Bus Watchdog */
    }
}
```

### Simulate instead of a real motor

```c
static struct x430_sim joint_sim;

x430_sim_init(&joint_sim, &joint, NULL);
const struct x430_device_config cfg = {.id = 1U, .hooks = &x430_sim_hooks, .user_data = &joint_sim};
x430_device_init(&joint, &cfg);
/* every millisecond: */ x430_sim_step(&joint_sim, 1U);
```

On a PC (Linux, macOS, WSL):

```bash
cmake --preset "X430[Examples]" && cmake --build "build/X430[Examples]"
build/X430[Examples]/examples/x430_host_sim --pty --link /tmp/ttyDXL --ids 1,2,3
# point DynamixelSDK / DYNAMIXEL Wizard / ROS 2 at /tmp/ttyDXL
```

## Documentation

- [Configuration](docs/configuration.md): Kconfig symbols, fragments, menuconfig, building without Kconfig
- [Integration](docs/integration.md): hooks, live registers, persistent storage, RTOS port, ROS 2
- [Models](docs/models.md): model data, adding a model, documented deviations
- [Behaviour](docs/behaviour.md): what the emulator enforces, instruction by instruction
- [Migration](docs/migration.md): from the SmartManipulator-X `dynamixel_x430.c`

## Testing

```bash
cmake --preset "X430[UnitTests]" && cmake --build "build/X430[UnitTests]"
ctest --test-dir "build/X430[UnitTests]" --output-on-failure
tests/sdk/run_sdk_check.sh "build/X430[Examples]"      # ROBOTIS DynamixelSDK end to end
```

CI runs the unit tests for every model, the coverage target, a minimal-feature build,
the DynamixelSDK check and clang-format.

## Project structure

```
sfs-dynamixel-x430/
├── x430.h, x430.cmake        umbrella header, library target (include() it)
├── Kconfig, Kconfig.x430     Kconfig tree (Kconfig.x430 can be sourced by a project)
├── configs/defconfig         default configuration fragment
├── models/                   Control Tables: x430_family.yaml + x430/<model>.yaml
├── inc/                      public headers
├── src/                      x430_device.c, x430_bus.c, x430_sim.c
├── tools/                    x430_gen_model.py (YAML -> C), x430_kconfig.py (Kconfiglib)
├── cmake/                    version, Kconfig, unit tests, MISRA helpers
├── examples/host_sim/        POSIX serial/pty simulator
├── tests/unit/               GoogleTest suite
├── tests/models/             model data vs. e-Manual snapshot
├── tests/sdk/                DynamixelSDK end-to-end check
└── docs/                     documentation (MkDocs)
```

## License

Copyright © 2026 Anton Vynohradov. Licensed under the Apache License 2.0. See [LICENSE](LICENSE).
