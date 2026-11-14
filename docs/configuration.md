# Configuration

The X430 emulator has two kinds of configuration. **Build-time options** select the
emulated model, the sizes and the optional instructions; they come from Kconfig or from
plain compiler definitions and end up as macros of `inc/x430_config.h`. **Run-time
configuration** is the content of `struct x430_bus_config`, `struct x430_device_config`
and `struct x430_sim_config`, passed to the *_init() functions. This chapter lists both,
then the memory footprint and the measured stack usage.

## Build-time options

| Kconfig symbol | Macro | Default | Range | Meaning |
| --- | --- | --- | --- | --- |
| `X430_MODEL_*` (choice) | `X430_MODEL_NAME`, `X430_MODEL_IS_<KEY>` | XM430-W350 | 17 models | Emulated model (one per build); CMake `X430_MODEL` without Kconfig |
| `X430_MODEL_NUMBER_OVERRIDE` | `X430_MODEL_NUMBER` | 0 = documented number | 0..65535 | Model Number reported by PING and address 0 |
| `X430_MODEL_OVERRIDES` | – | empty | file | YAML file of application overrides of the model (below) |
| `X430_FIRMWARE_VERSION` | `X430_FIRMWARE_VERSION` | 52 | 1..255 | Firmware Version (6) when the device configuration gives 0 |
| `X430_MAX_DEVICES` | `X430_MAX_DEVICES` | 4 | 1..253 | Devices (IDs) per bus |
| `X430_REG_WRITE_SIZE` | `X430_REG_WRITE_SIZE` | 32 | 4..662 | Largest REG_WRITE data per device, bytes |
| `X430_BAUD_RATE_CODE_MAX` | `X430_BAUD_RATE_CODE_MAX` | 7 | 7..255 | Highest Baud Rate (8) code a write accepts |
| `X430_DXL2_MAX_PACKET_SIZE` | `DXL2_MAX_PACKET_SIZE` (DXL2-C) | 768 | 64..4096 | Largest packet; limits READ length and Fast read answers |
| `X430_BULK` | `X430_FEATURE_BULK` | y | y/n | BULK_READ, BULK_WRITE |
| `X430_FAST_READ` | `X430_FEATURE_FAST_READ` | y | y/n | FAST_SYNC_READ, FAST_BULK_READ |
| `X430_BACKUP` | `X430_FEATURE_BACKUP` | y | y/n | BACKUP, Backup Ready, Startup RAM Restore |
| `X430_SIMULATOR` | `X430_FEATURE_SIMULATOR` | y | y/n | `x430_sim` |

The ranges are those of `Kconfig.x430`; without Kconfig, `x430_config.h` checks `X430_MAX_DEVICES` and `X430_REG_WRITE_SIZE` with `#error` and DXL2-C checks its packet size. The application must be compiled with the
same values as the library; with CMake they are public compile definitions of the `x430`
target.

Without Kconfig (the default) the options are compiler definitions, given in the CMake
variable `COMPILE_DEFINITIONS`; compiler options of the library sources (optimization,
CPU flags) go into `COMPILE_OPTIONS`. Both names have no prefix on purpose: a project
sets them once for all the libraries it includes.

### With Kconfig

With `X430_KCONFIG=ON` (off by default; preset `X430[Kconfig]`),
[Kconfiglib](https://github.com/ulfalizer/Kconfiglib) merges the fragments of
`X430_CONF_FILES` (default `configs/defconfig`) into `<build>/x430/.config` and writes
`x430_autoconf.h`. As long as the fragments do not change, the existing `.config` is kept,
so values chosen in menuconfig survive.

```bash
cmake --preset "X430[Kconfig]"
cmake --build "build/X430[Kconfig]" --target menuconfig      # terminal UI
cmake --build "build/X430[Kconfig]" --target savedefconfig   # minimal fragment
cmake -S . -B build -DX430_CONF_FILES="$PWD/configs/defconfig;$PWD/my_robot.conf"
```

```ini
# my_robot.conf
CONFIG_X430_MODEL_XW430_T333=y
CONFIG_X430_MAX_DEVICES=6
CONFIG_X430_BACKUP=n
```

A project with its own Kconfig tree switches `X430_KCONFIG` on, points `X430_KCONFIG_ROOT`
at its top-level Kconfig and sources `Kconfig.x430` from it. An RTOS with its own Kconfig
(Zephyr) keeps `X430_KCONFIG` off: the application Kconfig sources `Kconfig.x430` and the
glue passes the `CONFIG_X430_*` values to the CMake variables.

### Without Kconfig

Without Kconfig only Python 3 is needed. Every optional feature is on unless defined to
0; options are compiler definitions, given in `COMPILE_DEFINITIONS`:

```bash
cmake -S . -B build -DX430_MODEL=xl430_w250 \
    -DCOMPILE_DEFINITIONS="X430_MAX_DEVICES=8U;X430_FEATURE_BACKUP=0" -DCOMPILE_OPTIONS="-Os"
```

### Packet size

`DXL2_MAX_PACKET_SIZE` limits the length of a READ answer to `DXL2_MAX_PACKET_SIZE − 11`
bytes and the size of a Fast read answer to `DXL2_MAX_PACKET_SIZE` bytes
(`10 + devices × (bytes + 4)`). The default is 768: a READ of up to 757 bytes, so a host
can read the whole Control Table (addresses 0–661, 662 bytes) at once, as DYNAMIXEL
Wizard 2.0 does (with 256 it gets a Data Length Error and shows TIME OUT). A product whose
hosts read less may lower it: with 256 the bus needs 832 B instead of 2,368 B (the stack
does not depend on the packet size: the bus builds its answers in its own buffer). `x430_config.h` sets the default
before it includes `dxl2.h`, so every file that includes `x430.h` sees the size the library
was built with. Without CMake, compile DXL2-C with the same value
(`-DDXL2_MAX_PACKET_SIZE=768U`; the default of DXL2-C alone is 256), and include `x430.h`
before `dxl2.h` in the application.

### Application overrides of the model

A product built on the emulator may need another range or initial value than the ROBOTIS
model, for example a supply above the 16 V of an XM430. Instead of editing `models/`, give
a YAML file with the same `overrides:` mapping as the model files:

```yaml
overrides:
  Max Voltage Limit: {min: 95, max: 300}
  Min Voltage Limit: {min: 95, max: 300}
```

Set its path in `CONFIG_X430_MODEL_OVERRIDES` or in the CMake cache variable
`X430_MODEL_OVERRIDES`. `tools/x430_gen_model.py --overrides <file>` applies it after the
model file; an unknown register name or an initial value outside the resulting range stops
the build.

## Run-time configuration

The *_init() functions copy the structures, so they may be temporaries or `const` in flash.
The hooks and the port they point at are not copied: keep them alive.

### `struct x430_bus_config`

| Field | Required | Meaning |
| --- | --- | --- |
| `write` | Yes | `int write(void* context, const uint8_t* data, size_t length)`: writes one complete status packet; returns 0, or non-zero on error (reported as `X430_ERR_HAL`) |
| `write_context` | – | Passed to `write` |
| `port` | – | `struct x430_port`: platform services, NULL for none |

### `struct x430_port`

| Member | Called from | Used for | NULL means |
| --- | --- | --- | --- |
| `context` | – | Passed to every member | – |
| `lock`, `unlock` | Bus functions, `x430_bus_lock()`, `x430_bus_unlock()`, `x430_sim_step()` | Mutual exclusion of the Control Tables | Single context (bare metal) |
| `millis` | Bus functions, `x430_device_read()` | Realtime Tick (120), Bus Watchdog | Tick 0, no watchdog timeout |
| `delay_us` | Before each status packet | Return Delay Time (9) × 2 µs | Status sent at once |

### `struct x430_device_config`

| Field | Unit | Range | Example | Meaning |
| --- | --- | --- | --- | --- |
| `id` | – | 0..252 | 11 | Factory ID (0 = model default 1); a stored ID wins |
| `firmware_version` | – | 0..255 | 0 | Firmware Version (6); 0 = `X430_FIRMWARE_VERSION` |
| `model_information` | – | 32 bit | `0x00010000` | Model Information (2), free for the application |
| `hooks` | – | pointer | `&hooks` | Application hooks, NULL = pure register device |
| `user_data` | – | pointer | `&motor` | Passed to every hook |

### `struct x430_sim_config`

| Field | Unit | Default (0) | Meaning |
| --- | --- | --- | --- |
| `input_voltage` | 0.1 V | 120 (12.0 V) | Present Input Voltage (144) |
| `temperature` | °C | 30 | Present Temperature (146) |

## Memory footprint

Measured with `tools/x430_measure.sh` and `arm-none-eabi-gcc` 13.2.1 for Cortex-M4
(`-mcpu=cortex-m4 -mthumb`), model XM430-W350, `X430_MAX_DEVICES` 4,
`X430_REG_WRITE_SIZE` 32, `DXL2_MAX_PACKET_SIZE` 768:

| Object | All features | No optional feature |
| --- | ---: | ---: |
| `struct x430_device` (RAM per device) | 384 B | 384 B |
| `struct x430_bus` (RAM per bus) | 2,368 B | 2,368 B |
| `struct x430_bus`, `DXL2_MAX_PACKET_SIZE` 256 | 832 B | 832 B |
| `struct x430_sim` | 32 B | – |
| `.data` + `.bss` | 0 B | 0 B |

A device is its compact Control Table image (316 B), the REG_WRITE data
(`X430_REG_WRITE_SIZE`, 32 B) and its state; it keeps no copy of a persistent block. A bus
is the DXL2-C link (receive buffer and decoded packet, 1,560 B with 768-byte packets),
one packet buffer for its answers (`DXL2_MAX_PACKET_SIZE`) and the device list
(`X430_MAX_DEVICES` pointers). The optional features change code, not RAM.

Flash (`.text`, including constants) per module:

| Module | `-Os` | `-O2` | `-Os`, no optional feature |
| --- | ---: | ---: | ---: |
| `x430_device.c` | 4,158 B | 5,916 B | 3,814 B |
| `x430_bus.c` | 2,820 B | 3,824 B | 2,042 B |
| `x430_sim.c` | 2,022 B | 2,364 B | 32 B |
| `x430_model_table.c` (XM430-W350) | 921 B | 921 B | 921 B |
| **Library** | **9,921 B** | **13,025 B** | **6,809 B** |
| DXL2-C (`dxl2_protocol.c`, `dxl2_link.c`) | 850 B | 992 B | 850 B |

The model table holds 55 registers of 16 bytes, the two Indirect Address blocks (the
library generates their 56 registers) and the three segments of the image; a register is
found by binary search. DXL2-C is listed as compiled: the bus uses only its parser and
`dxl2_crc16()`, so a link with `--gc-sections` drops `dxl2_link_send()` and
`dxl2_link_send_status()`.

On the x86-64 host the device needs 392 B and the bus 2,424 B (64-bit pointers).

## Stack usage

Worst-case stack of each public function including the library and DXL2-C functions it
calls, measured with `-fstack-usage -fcallgraph-info=su` and `tools/docs/stack_usage.py`.
The packet handler and the byte writer that DXL2-C calls through function pointers are
included; the application's hooks, port functions and byte writer are not — add their
stack to the function that calls them.

| Function | Cortex-M4 `-Os` | Cortex-M4 `-O0` | `-Os`, packet 256 |
| --- | ---: | ---: | ---: |
| `x430_bus_init()` | 48 | 104 | 48 |
| `x430_bus_attach()` | 40 | 104 | 40 |
| `x430_bus_receive()` | 424 | 656 | 424 |
| `x430_bus_handle_packet()` | 392 | 600 | 392 |
| `x430_bus_poll()` | 32 | 104 | 32 |
| `x430_bus_lock()`, `x430_bus_unlock()` | 8 | 64 | 8 |
| `x430_bus_find()` | 40 | 104 | 40 |
| `x430_device_init()` | 136 | 208 | 136 |
| `x430_device_get()` | 92 | 144 | 92 |
| `x430_device_set()` | 92 | 144 | 92 |
| `x430_device_id()` | 16 | 96 | 16 |
| `x430_device_raise_hardware_error()` | 20 | 72 | 20 |
| `x430_device_flush_events()` | 20 | 72 | 20 |
| `x430_device_read()` | 112 | 136 | 112 |
| `x430_device_write()` | 256 | 384 | 256 |
| `x430_device_reg_write()` | 264 | 384 | 264 |
| `x430_device_action()` | 248 | 384 | 248 |
| `x430_device_reboot()` | 128 | 200 | 128 |
| `x430_device_factory_reset()` | 144 | 216 | 144 |
| `x430_device_clear()` | 108 | 176 | 108 |
| `x430_device_backup()` | 120 | 192 | 120 |
| `x430_sim_init()` | 16 | 80 | 16 |
| `x430_sim_step()` | 284 | 680 | 284 |

The deepest chain of `x430_bus_receive()` is a REG_WRITE: dispatch, the write of the
device (the written bytes merged with Indirect Data resolved). No packet-sized buffer is on
the stack: the bus builds and byte-stuffs each status packet in place in its own buffer,
and the device reads and writes persistent blocks in parts (a 16-byte chunk on the stack
while it checks a block).
Give the bus context this figure plus its hooks.

To re-measure (in a configured build directory):

```bash
tools/x430_measure.sh "build/X430[Release]"
```

## CMake options

| Option | Default | Meaning |
| --- | --- | --- |
| `COMPILE_DEFINITIONS` | empty | Compile definitions of the library (public), e.g. `X430_MAX_DEVICES=8U` |
| `COMPILE_OPTIONS` | empty | Compile options of the library sources, e.g. `-Os` |
| `UNIT_TESTS` | `OFF` | Unit tests: in this repository the GoogleTest suite and the `coverage` target; as a submodule the executable `x430_gtest`, registered with the CTest of the project (label `x430`) |
| `X430_KCONFIG` | `OFF` | Configure with Kconfiglib |
| `X430_CONF_FILES` | `configs/defconfig` | Kconfig fragments |
| `X430_KCONFIG_ROOT` | `Kconfig` | Top-level Kconfig file |
| `X430_MODEL` | `xm430_w350` | Model without Kconfig |
| `X430_MODEL_OVERRIDES` | empty | Application overrides of the model without Kconfig |
| `X430_DXL2_MAX_PACKET_SIZE` | 768 | `DXL2_MAX_PACKET_SIZE` without Kconfig |
| `X430_DXL2_GIT_TAG` | `v1.0.0` | DXL2-C release fetched with FetchContent |
| `FETCHCONTENT_SOURCE_DIR_DXL2` | – | Local DXL2-C checkout (offline builds) |
| `X430_EXAMPLES` | `OFF` | POSIX host example |
| `X430_COVERAGE_MIN` | `100` | Coverage gate (lines and branches, %) |
| `X430_MISRA_FAIL_ON_VIOLATION` | `ON` | `misra-analyze` fails on any violation |
| `X430_MISRA_RULE_TEXTS` | empty | Licensed MISRA rule texts for readable reports |
