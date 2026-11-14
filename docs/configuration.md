# Configuration

Everything is decided at build time. There are two ways to configure the library;
both produce the same library.

## With Kconfig (default)

`x430.cmake` runs [Kconfiglib](https://github.com/ulfalizer/Kconfiglib) during the
CMake configure step:

1. The fragments in `X430_CONF_FILES` (default `configs/defconfig`) are merged in
   order into `<build>/x430/.config`.
2. `<build>/x430/include/x430_autoconf.h` (C macros `CONFIG_X430_*`) and
   `<build>/x430/x430_config.cmake` (CMake variables) are written.
3. The model chosen by `CONFIG_X430_MODEL_*` is generated into
   `<build>/x430/generated/x430_model_table.c` and `x430_model_info.h`.

`.config` is a configure dependency. As long as the fragments do not change, the
existing `.config` is kept, so values chosen in menuconfig survive; when a fragment
changes, `.config` is rebuilt from the fragments (the fragments are the project's
configuration, like `prj.conf` in Zephyr).

```bash
cmake --preset "X430[Debug]"
cmake --build "build/X430[Debug]" --target menuconfig      # terminal UI
cmake --build "build/X430[Debug]" --target guiconfig       # Tk window
cmake --build "build/X430[Debug]"                          # re-runs CMake if .config changed
cmake --build "build/X430[Debug]" --target savedefconfig   # minimal fragment -> build/defconfig
```

Your own fragment, applied after the default:

```bash
cmake -S . -B build -DX430_CONF_FILES="$PWD/configs/defconfig;$PWD/my_robot.conf"
```

```ini
# my_robot.conf
CONFIG_X430_MODEL_XW430_T333=y
CONFIG_X430_MAX_DEVICES=6
CONFIG_X430_FAST_READ=y
```

Unknown symbols and values Kconfig rejects are reported as warnings during the
configure step.

### Project with its own Kconfig tree

Point `X430_KCONFIG_ROOT` at your top-level Kconfig and source the library's symbols:

```kconfig
mainmenu "My robot"
rsource "third_party/sfs-dynamixel-x430/Kconfig.x430"
config ROBOT_AXES
	int "Axes"
	default 3
```

## Without Kconfig

```bash
cmake -S . -B build -DX430_KCONFIG=OFF -DX430_MODEL=xl430_w250
```

Only PyYAML is needed then. Options are plain compiler definitions:

```cmake
include(path/to/sfs-dynamixel-x430/x430.cmake)
target_compile_definitions(x430 PUBLIC X430_MAX_DEVICES=8U X430_FEATURE_BACKUP=0)
```

Without Kconfig every optional feature is on unless you define it to 0.

## Symbols

| Kconfig symbol | Macro (`x430_config.h`) | Default | Meaning |
|---|---|---|---|
| `X430_MODEL_*` (choice) | `X430_MODEL_NAME`, `X430_MODEL_IS_<KEY>` | XM430-W350 | Emulated model (one per build) |
| `X430_MODEL_NUMBER_OVERRIDE` | `X430_MODEL_NUMBER` | 0 = documented | Model Number reported by PING / address 0 |
| `X430_MODEL_OVERRIDES` | — (CMake `X430_MODEL_OVERRIDES` without Kconfig) | empty | YAML file of application overrides, applied after `models/` when the Control Table is generated (see below) |
| `X430_FIRMWARE_VERSION` | `X430_FIRMWARE_VERSION` | 52 | Firmware Version (6) when the device config gives 0 |
| `X430_MAX_DEVICES` | `X430_MAX_DEVICES` | 4 | Devices (IDs) per bus |
| `X430_REG_WRITE_SIZE` | `X430_REG_WRITE_SIZE` | 32 | REG_WRITE data per device |
| `X430_BAUD_RATE_CODE_MAX` | `X430_BAUD_RATE_CODE_MAX` | 7 | Highest Baud Rate (8) code accepted |
| `X430_DXL2_MAX_PACKET_SIZE` | `DXL2_MAX_PACKET_SIZE` (DXL2-C) | 256 | Largest packet; limits READ size and Fast read answers |
| `X430_BULK` | `X430_FEATURE_BULK` | y | BULK_READ / BULK_WRITE |
| `X430_FAST_READ` | `X430_FEATURE_FAST_READ` | y | FAST_SYNC_READ / FAST_BULK_READ |
| `X430_BACKUP` | `X430_FEATURE_BACKUP` | y | BACKUP, Backup Ready, Startup RAM Restore |
| `X430_SIMULATOR` | `X430_FEATURE_SIMULATOR` | y | `x430_sim` |

## CMake variables

| Variable | Default | Meaning |
|---|---|---|
| `X430_KCONFIG` | `ON` | Use Kconfig |
| `X430_CONF_FILES` | `configs/defconfig` | Kconfig fragments |
| `X430_KCONFIG_ROOT` | `Kconfig` | Top-level Kconfig file |
| `X430_MODEL` | `xm430_w350` | Model without Kconfig |
| `X430_DXL2_MAX_PACKET_SIZE` | 256 | Packet size without Kconfig |
| `X430_DXL2_GIT_TAG` | `v1.0.0` | DXL2-C release (FetchContent) |
| `FETCHCONTENT_SOURCE_DIR_DXL2` | – | Local DXL2-C checkout (offline builds) |
| `X430_UNIT_TESTS`, `X430_EXAMPLES` | `OFF` | Tests, host example |
| `X430_COVERAGE_MIN` | 95 | Line coverage required by `coverage` |

## Memory

Per device: the Control Table image (662 bytes) plus `X430_REG_WRITE_SIZE` and a few
words. Per bus: one DXL2-C link (about 2 × `DXL2_MAX_PACKET_SIZE`), a status buffer
and, with Fast reads, one more packet buffer. One static buffer of
`X430_NVM_BACKUP_SIZE` (674 bytes) is shared for persistence. A typical Cortex-M
build of the library takes about 15-25 KiB of flash depending on the features.

## Application overrides of the model

A product built on the emulator may need another range or initial value than the ROBOTIS
model, for example a supply above the 16 V of an XM430. Instead of editing `models/`, give a
YAML file with the same `overrides:` mapping as the model files:

```yaml
overrides:
  Max Voltage Limit: {max: 300}
  Min Voltage Limit: {max: 300}
```

Set its absolute path in `CONFIG_X430_MODEL_OVERRIDES` (Kconfig) or in the CMake cache variable
`X430_MODEL_OVERRIDES`. `tools/x430_gen_model.py --overrides <file>` applies it after the model
file; an unknown register name or an initial value outside the resulting range stops the build.
`--dump <model> --overrides <file>` prints the resolved table.
