# Integration

## Objects

| Object | One per | Owns |
|---|---|---|
| `struct x430_bus` | serial line | DXL2-C link, list of devices, port |
| `struct x430_device` | DYNAMIXEL ID | Control Table image, hooks |
| `struct x430_sim` | simulated device | kinematic state |

All objects are caller-owned (static or on a long-lived stack); the library has no heap.

```c
x430_bus_init(&bus, &(struct x430_bus_config){.write = uart_write, .port = &port});
x430_device_init(&axis[i], &(struct x430_device_config){.id = 11U + i, .hooks = &hooks, .user_data = &motor[i]});
x430_bus_attach(&bus, &axis[i]);
```

`x430_device_config::id` is the **factory ID** of that device: it is used when no
EEPROM block is stored yet, and again after FACTORY_RESET 0xFF. A stored ID wins at
startup. For the dual-axis 2XL430/2XC430 create two devices (factory IDs 1 and 2).

## Hooks: the motor side

```c
static const struct x430_hooks hooks = {
    .write = on_write,         /* accepted register write, before it is stored */
    .read = on_read,           /* data is about to be read */
    .event = on_event,         /* reboot, factory reset, clear, watchdog, baud, shutdown */
    .nvm_load = flash_load,    /* EEPROM area and backup */
    .nvm_store = flash_store,
};
```

### write

Called for every register a WRITE/SYNC_WRITE/BULK_WRITE/ACTION touches, in address
order, **after** the library validated all of them (access, range, EEPROM lock, Bus
Watchdog) and **before** the value is stored. Return `DXL2_ERR_NONE` to accept or a
`DXL2_ERR_*` code to refuse; the instruction stops at that register and the code goes
into the status packet. Indirect Data writes arrive with the real address.

Typical reactions:

| Register | Reaction |
|---|---|
| Torque Enable (64) | enable/disable the driver; refuse with Result Fail if it fails |
| Goal Position (116), Goal Velocity (104), Goal PWM (100), Goal Current (102) | new set point |
| Profile Velocity/Acceleration (112/108) | new ramp |
| Operating Mode (11), Drive Mode (10), Homing Offset (20) | change scaling (torque is off) |
| Current Limit (38), Velocity Limit (44), PWM Limit (36) | driver limits |

On Torque Enable 0 → 1 the library copies Present Position into Goal Position (as an
X430 does), so refresh Present Position before or let the read hook do it.

### read and live registers

The device does not know the motor state. Keep these registers current with
`x430_device_set()`: Present Position (132), Present Velocity (128), Present
Current/Load (126), Present PWM (124), Moving (122), Moving Status (123),
Velocity/Position Trajectory (136/140), Present Input Voltage (144), Present
Temperature (146). Either

- periodically from the control loop, between `x430_bus_lock()` and `x430_bus_unlock()`, or
- on demand in the `read` hook (called with the real address ranges, also for
  Indirect Data), where the lock is already held.

Report hardware faults with `x430_device_raise_hardware_error()`; bits enabled in
Shutdown (63) switch the torque off until REBOOT and every status carries the alert bit.

### event

| Event | When | Your reaction |
|---|---|---|
| `X430_EVENT_REBOOT` | after REBOOT | reset the motor state (RAM is at defaults, errors cleared) |
| `X430_EVENT_FACTORY_RESET` | after FACTORY_RESET | as reboot; EEPROM is at factory values |
| `X430_EVENT_CLEAR_MULTI_TURN` | CLEAR, not moving | bring Present Position into one turn; return an error to refuse |
| `X430_EVENT_BUS_WATCHDOG` | watchdog expired (torque on) | stop the motor |
| `X430_EVENT_BAUD_RATE` | after the status of a Baud Rate change | switch the UART |
| `X430_EVENT_TORQUE_SHUTDOWN` | hardware error in Shutdown mask | power stage off |
| `X430_EVENT_BACKUP_RESTORED` | BACKUP restore | re-read limits |

### Persistent storage

The library stores opaque blocks: `X430_NVM_EEPROM_SIZE` (76 bytes) for the EEPROM
area and `X430_NVM_BACKUP_SIZE` (674 bytes) for the Control Table backup. Each block
carries a header (magic, model number, format, slot) and a CRC-16; a block that does
not match is ignored and the device starts with factory values. `nvm_store` is called
after the change from the bus context; when your flash is slow, copy the block and
write it from another thread.

## Port: RTOS or bare metal

```c
static const struct x430_port port = {
    .context = &bus_mutex,
    .lock = mutex_lock, .unlock = mutex_unlock,   /* recursive mutex */
    .millis = uptime_ms,                          /* Realtime Tick, Bus Watchdog */
    .delay_us = busy_wait_us,                     /* Return Delay Time */
};
```

Every member may be NULL. The bus takes the lock in `x430_bus_receive`,
`x430_bus_handle_packet`, `x430_bus_poll` and `x430_sim_step`; hooks run with it
held. Use a recursive mutex if a hook calls into code that locks again.

Queues are not needed by the library. Typical RTOS layout:

```
UART ISR ──bytes──▶ queue ──▶ RX thread: x430_bus_receive()      (bus context, lock held inside)
                                  └─ hooks ──commands──▶ queue ──▶ motor thread
motor thread: x430_bus_lock(); x430_device_set(Present...); x430_bus_unlock();
timer: x430_bus_poll() every 5-20 ms
```

Do not call `x430_bus_receive` from an interrupt handler: it parses, answers and calls
hooks.

### Zephyr

```c
K_MUTEX_DEFINE(x430_mutex);
static void lock(void* c) { k_mutex_lock(c, K_FOREVER); }     /* Zephyr mutexes are recursive */
static void unlock(void* c) { k_mutex_unlock(c); }
static uint32_t millis(void* c) { (void) c; return k_uptime_get_32(); }
static void delay(void* c, uint32_t us) { (void) c; k_busy_wait(us); }
static const struct x430_port port = {&x430_mutex, lock, unlock, millis, delay};
```

Without the library's Kconfig inside Zephyr, build with `X430_KCONFIG=OFF` and pass
the options as definitions, or source `Kconfig.x430` from the application Kconfig and
pass Zephyr's `autoconf.h` directory with `X430_HAS_AUTOCONF`.

## Several transports or custom devices on the same line

If the application already runs its own DXL2-C link (for example to host extra,
non-X430 devices), route packets itself and hand the rest to the bus:

```c
static void on_packet(const struct dxl2_packet* packet, void* context)
{
    if (packet->id == MY_CONTROLLER_ID) { my_controller_handle(packet); }
    else { x430_bus_handle_packet(packet, &bus); }
}
```

The bus sends its answers through the writer given in `x430_bus_config`.

## ROS 2 (`dynamixel_hardware_interface`)

The interface pings each ID, loads the model file by Model Number, writes
Torque Enable / Operating Mode, programs Indirect Address blocks and reads them with
Fast Sync Read (falling back to Sync Read). All of this is emulated. Points to check:

- The Model Number must exist in its `param/dxl_model/dynamixel.model`. XW430-T200
  (1280) is not listed there; use `CONFIG_X430_MODEL_NUMBER_OVERRIDE=1270` or add a
  model file.
- Fast reads answer for the IDs hosted on this bus only. Keep all IDs of one ROS 2
  hardware component on the same emulator bus.
- Set `X430_DXL2_MAX_PACKET_SIZE` large enough for the Fast Sync Read answer:
  `10 + devices × (bytes + 4)`.
