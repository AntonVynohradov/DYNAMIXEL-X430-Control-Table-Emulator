# API Reference

This chapter documents every public function, type and macro of the X430 emulator. Include
one header; it declares every function and includes the public type headers:

```c
#include "x430.h"
```

| Header | Contents |
| --- | --- |
| `x430.h` | All public functions, `x430_sim_hooks`; library version; includes the headers below |
| `x430_config.h` | Build-time configuration (Kconfig or compiler definitions) |
| `x430_defs.h` | Status codes, write options, events, persistent slots, Control Table addresses and values |
| `x430_types.h` | Port, hooks, configuration structures, opaque instances and their sizes |
| `x430_model.h` | Types of the generated model descriptor, `x430_model_active` |
| `x430_model_info.h` | Generated: name, Model Number and table size of the selected model |
| `dxl2.h` | DXL2-C: `DXL2_ERR_*` protocol errors, `dxl2_write_fn`, `struct dxl2_packet` |

Everything else (`src/`) is private and not installed with the library.

## Version — `x430.h`

| Macro | Value | Meaning |
| --- | --- | --- |
| `X430_VERSION_MAJOR`, `…_MINOR`, `…_PATCH` | `0`, `0`, `0` | Version numbers; set by CMake from the latest `vX.Y.Z` tag and stamped into the release package; `0.0.0` means unknown (a header from the repository built without CMake) |
| `X430_VERSION` | `"0.0.0"` | Version string |

## Definitions — `x430_defs.h`

**Status codes** (return value of every function):

| Macro | Value | Meaning |
| --- | --- | --- |
| `X430_OK` | `0` | Success |
| `X430_ERR_PARAM` | `-1` | `NULL` pointer or a value outside its documented range |
| `X430_ERR_STATE` | `-2` | Instance not initialized; device already attached |
| `X430_ERR_CONFIG` | `-3` | Bus full, or the ID is already used on the bus |
| `X430_ERR_HAL` | `-4` | The byte writer of the bus returned an error |
| `X430_ERR_UNSUPPORTED` | `-5` | The feature is not compiled in |

**Write options** of `x430_device_write()` and `x430_device_reg_write()`:
`X430_WRITE_SECONDARY` (0x01, packet addressed by the Secondary ID: the EEPROM area is
locked), `X430_WRITE_VALIDATE_ONLY` (0x02, check only, store nothing, call no hook).

**Events** (`enum x430_event`, argument of the event hook):

| Value | When | Return of the hook |
| --- | --- | --- |
| `X430_EVENT_REBOOT` | After REBOOT: RAM area reset, errors cleared | Ignored |
| `X430_EVENT_FACTORY_RESET` | After FACTORY_RESET (then like REBOOT) | Ignored |
| `X430_EVENT_CLEAR_MULTI_TURN` | CLEAR while the device is not moving | `DXL2_ERR_NONE` after bringing Present Position into one turn, or an error for the host |
| `X430_EVENT_BUS_WATCHDOG` | Bus Watchdog expired (Bus Watchdog now reads −1) | Ignored |
| `X430_EVENT_BAUD_RATE` | Baud Rate (8) changed; after the status packet | Ignored |
| `X430_EVENT_TORQUE_SHUTDOWN` | A hardware error in Shutdown (63) switched the torque off | Ignored |
| `X430_EVENT_BACKUP_RESTORED` | BACKUP restore copied the backup into the table | Ignored |

**Persistent slots** (`enum x430_nvm_slot`): `X430_NVM_EEPROM` (0, `X430_NVM_EEPROM_SIZE`
= 76 bytes), `X430_NVM_BACKUP` (1, `X430_NVM_BACKUP_SIZE` = 328 bytes);
`X430_NVM_HEADER_SIZE` = 10.

**Control Table** constants: `X430_ADDR_*` (addresses of the registers used by the library
and by applications, for example `X430_ADDR_GOAL_POSITION` = 116, `X430_RAM_START` = 64),
`X430_MODE_*` (operating modes), `X430_DRIVE_MODE_*`, `X430_STARTUP_*`, `X430_HW_ERROR_*`,
`X430_MOVING_STATUS_*`, the instruction options `X430_FACTORY_RESET_*`,
`X430_CLEAR_MULTI_TURN`, `X430_BACKUP_*`, and `X430_BUS_WATCHDOG_ERROR`,
`X430_BUS_WATCHDOG_UNIT_MS` (20), `X430_SECONDARY_ID_DISABLED` (253),
`X430_EXTENDED_POSITION_LIMIT` (1,048,575), `X430_REALTIME_TICK_PERIOD` (32,768),
`X430_RETURN_DELAY_UNIT_US` (2).

## Types — `x430_types.h`

| Type | Members | Meaning |
| --- | --- | --- |
| `struct x430_port` | `context`, `lock`, `unlock`, `millis`, `delay_us` | Platform services ([Configuration](configuration.md#struct-x430_port)) |
| `struct x430_hooks` | `write`, `read`, `event`, `nvm_read`, `nvm_write` | Application hooks of a device ([Using the Library](usage.md#reacting-to-the-host-hooks)) |
| `struct x430_bus_config` | `write`, `write_context`, `port` | Bus configuration |
| `struct x430_device_config` | `id`, `firmware_version`, `model_information`, `hooks`, `user_data` | Device configuration |
| `struct x430_sim_config` | `input_voltage`, `temperature` | Simulator configuration |

Callback types:

| Type | Signature | Return |
| --- | --- | --- |
| `x430_lock_fn` | `void (*)(void* context)` | – |
| `x430_millis_fn` | `uint32_t (*)(void* context)` | Milliseconds, wrapping at 2^32 |
| `x430_delay_fn` | `void (*)(void* context, uint32_t us)` | – |
| `x430_write_hook` | `uint8_t (*)(struct x430_device* device, uint16_t address, uint8_t size, int32_t value, void* user_data)` | `DXL2_ERR_NONE` to accept, a `DXL2_ERR_*` code to refuse |
| `x430_read_hook` | `void (*)(struct x430_device* device, uint16_t address, uint16_t length, void* user_data)` | – |
| `x430_event_hook` | `uint8_t (*)(struct x430_device* device, enum x430_event event, void* user_data)` | See the events above |
| `x430_nvm_read_hook` | `int (*)(struct x430_device* device, enum x430_nvm_slot slot, uint16_t offset, uint8_t* data, uint16_t length, void* user_data)` | 0 when `length` bytes from `offset` were read, negative without data |
| `x430_nvm_write_hook` | `int (*)(struct x430_device* device, enum x430_nvm_slot slot, uint16_t offset, const uint8_t* data, uint16_t length, void* user_data)` | 0, or negative (the remaining parts are not written, the host gets Result Fail) |

**Instances.** `struct x430_bus`, `struct x430_device` and `struct x430_sim` are opaque
arrays of `X430_BUS_WORDS`, `X430_DEVICE_WORDS` and `X430_SIM_WORDS` 64-bit words (2,368 B,
384 B and 32 B on Cortex-M4 with the default options). Allocate them, pass their
addresses, never read or write their content.

## Variables — `x430.h`

```c
extern const struct x430_hooks x430_sim_hooks;
```

The hooks of the simulated actuator: pass them with `x430_device_config::user_data`
pointing at an initialized `struct x430_sim`. All members are `NULL` when
`X430_FEATURE_SIMULATOR` is 0.

## Functions — `x430.h`

Every function except `x430_bus_init()`, `x430_device_init()` and `x430_sim_init()`
returns `X430_ERR_STATE` for an instance without a successful initialization and then
calls no hook. Functions marked *lock-free* do not take the bus lock: call them from hooks
or between `x430_bus_lock()` and `x430_bus_unlock()`.

### Bus


#### x430_bus_init

```c
int x430_bus_init(struct x430_bus* bus, const struct x430_bus_config* config);
```

Initialize a bus. Calls no callback except x430_port::millis.

| Parameter | Description |
| --- | --- |
| `bus` | Bus. |
| `config` | Configuration; copied (the port it points at is not). |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Bus ready. |
| `X430_ERR_PARAM` | `NULL` pointer or no writer. |

#### x430_bus_attach

```c
int x430_bus_attach(struct x430_bus* bus, struct x430_device* device);
```

Attach an initialized device to an initialized bus. Takes the bus lock.

| Parameter | Description |
| --- | --- |
| `bus` | Bus. |
| `device` | Device, not attached to any bus. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Attached. |
| `X430_ERR_PARAM` | `NULL` pointer. |
| `X430_ERR_STATE` | Bus or device not initialized, or the device is attached. |
| `X430_ERR_CONFIG` | Bus full (`X430_MAX_DEVICES`) or the ID is used on the bus. |

#### x430_bus_receive

```c
int x430_bus_receive(struct x430_bus* bus, const uint8_t* data, size_t length);
```

Feed received bytes in any chunking; answers every complete instruction packet. Takes the bus lock; runs the hooks.

| Parameter | Description |
| --- | --- |
| `bus` | Bus. |
| `data` | Received bytes (may be `NULL` when `length` is 0). |
| `length` | Number of bytes. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Bytes processed. |
| `X430_ERR_PARAM` | `NULL` bus, or `NULL` data with a non-zero length. |
| `X430_ERR_STATE` | Bus not initialized. |
| `X430_ERR_HAL` | The writer failed for at least one status packet. |

#### x430_bus_handle_packet

```c
int x430_bus_handle_packet(struct x430_bus* bus, const struct dxl2_packet* packet);
```

Handle one decoded instruction packet, for applications that run their own DXL2-C link. Takes the bus lock; runs the hooks.

| Parameter | Description |
| --- | --- |
| `bus` | Bus. |
| `packet` | Packet decoded by DXL2-C. Status packets are ignored. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` pointer |
| `X430_ERR_STATE` | Bus not initialized |
| `X430_ERR_HAL` | The writer failed |

#### x430_bus_poll

```c
int x430_bus_poll(struct x430_bus* bus);
```

Periodic service: Bus Watchdog of every device. Call every few milliseconds (the watchdog unit is 20 ms). Takes the bus lock.

| Parameter | Description |
| --- | --- |
| `bus` | Bus. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

#### x430_bus_lock

```c
int x430_bus_lock(struct x430_bus* bus);
```

Take the bus lock (x430_port::lock; nothing without it). Use it around the live register updates of the application.

| Parameter | Description |
| --- | --- |
| `bus` | Bus. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

#### x430_bus_unlock

```c
int x430_bus_unlock(struct x430_bus* bus);
```

Release the bus lock (x430_port::unlock).

| Parameter | Description |
| --- | --- |
| `bus` | Bus. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

#### x430_bus_find

```c
int x430_bus_find(const struct x430_bus* bus, uint8_t id, struct x430_device** device);
```

Attached device with the primary ID `id` (lock-free).

| Parameter | Description |
| --- | --- |
| `bus` | Bus. |
| `id` | DYNAMIXEL ID. |
| `device` | Set to the device, or to `NULL` when no device has that ID. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Found. |
| `X430_ERR_PARAM` | `NULL` pointer, or no attached device has the ID. |
| `X430_ERR_STATE` | Bus not initialized. |

### Device: lifecycle and lock-free access


#### x430_device_init

```c
int x430_device_init(struct x430_device* device, const struct x430_device_config* config);
```

Initialize a device: factory values, stored EEPROM area (nvm_read), the RAM area and the Startup Configuration (which may call the write hook). Initialize a device before x430_bus_attach(), never while a bus serves it: the content of the instance before the call is not trusted (it may be the garbage of a stack instance), so an attachment cannot be detected.

| Parameter | Description |
| --- | --- |
| `device` | Device, not attached to a bus. |
| `config` | Configuration; copied (the hooks it points at are not). |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Device ready. |
| `X430_ERR_PARAM` | `NULL` pointer or ID above 252. |

#### x430_device_get

```c
int x430_device_get(const struct x430_device* device, uint16_t address, int32_t* value);
```

Value of the register that starts at `address` (lock-free).

| Parameter | Description |
| --- | --- |
| `device` | Device. |
| `address` | First address of a register. |
| `value` | Sign-extended value. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Value read. |
| `X430_ERR_PARAM` | `NULL` pointer, or no register starts at `address`. |
| `X430_ERR_STATE` | Device not initialized. |

#### x430_device_set

```c
int x430_device_set(struct x430_device* device, uint16_t address, int32_t value);
```

Store a value into the register at `address`, ignoring access rights (lock-free). For live registers (Present Position, Moving, ...): no hook is called, no range is checked.

| Parameter | Description |
| --- | --- |
| `device` | Device. |
| `address` | First address of a register. |
| `value` | Value; truncated to the register size. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Value stored. |
| `X430_ERR_PARAM` | `NULL` pointer, or no register starts at `address`. |
| `X430_ERR_STATE` | Device not initialized. |

#### x430_device_id

```c
int x430_device_id(const struct x430_device* device, uint8_t* id);
```

Current ID (7) of the device (lock-free).

| Parameter | Description |
| --- | --- |
| `device` | Device. |
| `id` | ID. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

#### x430_device_raise_hardware_error

```c
int x430_device_raise_hardware_error(struct x430_device* device, uint8_t bits);
```

Report hardware errors (Hardware Error Status bits, lock-free). The bits stay set until REBOOT. Bits enabled in Shutdown (63) switch the torque off (event `X430_EVENT_TORQUE_SHUTDOWN`) and keep it off until REBOOT.

| Parameter | Description |
| --- | --- |
| `device` | Device. |
| `bits` | `X430_HW_ERROR_`* bits. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

#### x430_device_flush_events

```c
int x430_device_flush_events(struct x430_device* device);
```

Deliver deferred events (`X430_EVENT_BAUD_RATE`) after the status packet. The bus calls it; call it after x430_device_write() of your own.

| Parameter | Description |
| --- | --- |
| `device` | Device. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

### Device: instruction semantics


#### x430_device_read

```c
int x430_device_read(struct x430_device* device, uint16_t address, uint16_t length, uint8_t* data,
                     uint8_t* error);
```

READ semantics: read hook, Indirect Data, Realtime Tick.

| Parameter | Description |
| --- | --- |
| `device` | Device. |
| `address` | First address. |
| `length` | Bytes to read. |
| `data` | Destination of `length` bytes. |
| `error` | Protocol result: `DXL2_ERR_NONE`, `DXL2_ERR_DATA_LENGTH` (length 0) or `DXL2_ERR_ACCESS` (beyond the table). |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

#### x430_device_write

```c
int x430_device_write(struct x430_device* device, uint16_t address, uint16_t length,
                      const uint8_t* data, uint8_t options, uint8_t* error);
```

WRITE semantics: checks every touched register, then calls the write hook and stores in address order; stores the EEPROM area when it changed.

| Parameter | Description |
| --- | --- |
| `device` | Device. |
| `address` | First address. |
| `length` | Bytes to write. |
| `data` | `length` bytes. |
| `options` | `X430_WRITE_SECONDARY`, `X430_WRITE_VALIDATE_ONLY`. |
| `error` | Protocol result: `DXL2_ERR_NONE` or the `DXL2_ERR_`* of the first failure. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

#### x430_device_reg_write

```c
int x430_device_reg_write(struct x430_device* device, uint16_t address, uint16_t length,
                          const uint8_t* data, uint8_t options, uint8_t* error);
```

REG_WRITE: validate a write and register it for ACTION.

| Parameter | Description |
| --- | --- |
| `device` | Device. |
| `address` | First address. |
| `length` | Bytes, at most `X430_REG_WRITE_SIZE`. |
| `data` | `length` bytes. |
| `options` | `X430_WRITE_SECONDARY`. |
| `error` | Protocol result. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

#### x430_device_action

```c
int x430_device_action(struct x430_device* device, uint8_t* error);
```

ACTION: execute the registered write.

| Parameter | Description |
| --- | --- |
| `device` | Device. |
| `error` | Protocol result; `DXL2_ERR_INSTRUCTION` when nothing is registered. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

#### x430_device_reboot

```c
int x430_device_reboot(struct x430_device* device);
```

REBOOT: RAM area reset, errors cleared, Startup Configuration applied, event `X430_EVENT_REBOOT`.

| Parameter | Description |
| --- | --- |
| `device` | Device. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

#### x430_device_factory_reset

```c
int x430_device_factory_reset(struct x430_device* device, uint8_t option, uint8_t* error);
```

FACTORY_RESET, then like REBOOT; event `X430_EVENT_FACTORY_RESET`.

| Parameter | Description |
| --- | --- |
| `device` | Device. |
| `option` | `X430_FACTORY_RESET_ALL`, `X430_FACTORY_RESET_EXCEPT_ID` or `X430_FACTORY_RESET_EXCEPT_ID_BAUD`. |
| `error` | Protocol result; `DXL2_ERR_DATA_RANGE` for another option. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

#### x430_device_clear

```c
int x430_device_clear(struct x430_device* device, uint8_t option, uint8_t* error);
```

CLEAR with option `X430_CLEAR_MULTI_TURN` (event `X430_EVENT_CLEAR_MULTI_TURN`).

| Parameter | Description |
| --- | --- |
| `device` | Device. |
| `option` | `X430_CLEAR_MULTI_TURN`. |
| `error` | Protocol result: `DXL2_ERR_RESULT_FAIL` while moving, `DXL2_ERR_DATA_RANGE` for another option. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |

#### x430_device_backup

```c
int x430_device_backup(struct x430_device* device, uint8_t option, uint8_t* error);
```

Control Table BACKUP: store, or restore with the torque off.

| Parameter | Description |
| --- | --- |
| `device` | Device. |
| `option` | `X430_BACKUP_STORE` or `X430_BACKUP_RESTORE`. |
| `error` | Protocol result. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Not initialized |
| `X430_ERR_UNSUPPORTED` | `X430_FEATURE_BACKUP` is 0 |

### Simulator


#### x430_sim_init

```c
int x430_sim_init(struct x430_sim* sim, struct x430_device* device,
                  const struct x430_sim_config* config);
```

Prepare a simulator for `device`. Call it before x430_device_init() of a device whose hooks are x430_sim_hooks and whose user data is `sim`.

| Parameter | Description |
| --- | --- |
| `sim` | Simulator. |
| `device` | Device it simulates (may be initialized later). |
| `config` | Configuration, `NULL` for the defaults. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` sim or device |
| `X430_ERR_UNSUPPORTED` | `X430_FEATURE_SIMULATOR` is 0 |

#### x430_sim_step

```c
int x430_sim_step(struct x430_sim* sim, uint32_t elapsed_ms);
```

Advance the simulation by `elapsed_ms` one-millisecond steps and refresh the live registers. Takes the bus lock of the device when it is attached. The time is proportional to `elapsed_ms`: call it periodically.

| Parameter | Description |
| --- | --- |
| `sim` | Simulator. |
| `elapsed_ms` | Time since the previous call [ms]. |

| Return | Meaning |
| --- | --- |
| `X430_OK` | Success |
| `X430_ERR_PARAM` | `NULL` |
| `X430_ERR_STATE` | Simulator or device not initialized |
| `X430_ERR_UNSUPPORTED` | `X430_FEATURE_SIMULATOR` is 0 |

## Return codes summary

| Function | `OK` | `ERR_PARAM` | `ERR_STATE` | `ERR_CONFIG` | `ERR_HAL` | `ERR_UNSUPPORTED` |
| --- | :-: | :-: | :-: | :-: | :-: | :-: |
| `x430_bus_init()`, `x430_device_init()` | ✓ | ✓ | | | | |
| `x430_bus_attach()` | ✓ | ✓ | ✓ | ✓ | | |
| `x430_bus_receive()`, `x430_bus_handle_packet()` | ✓ | ✓ | ✓ | | ✓ | |
| `x430_device_backup()` | ✓ | ✓ | ✓ | | | ✓ |
| `x430_sim_init()` | ✓ | ✓ | | | | ✓ |
| `x430_sim_step()` | ✓ | ✓ | ✓ | | | ✓ |
| Every other function | ✓ | ✓ | ✓ | | | |
