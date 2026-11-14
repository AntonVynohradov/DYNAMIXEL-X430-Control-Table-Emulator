# Using the Library

This chapter shows how to set up a bus and its devices, how the application reacts to the
host through the hooks, how it keeps the live registers current and how errors come back.
The UART, RTOS and flash side is in the [Integration Guide](integration.md); the build
options are in [Configuration](configuration.md).

## Setting up a bus and its devices

One `struct x430_bus` serves one serial line; one `struct x430_device` is one DYNAMIXEL ID.
Initialize the bus and every device, then attach the devices:

```c
static struct x430_bus bus;
static struct x430_device joint;

static int uart_write(void* context, const uint8_t* data, size_t length)
{
    (void)context;
    return board_uart_write(data, length);
}

int app_start(void)
{
    const struct x430_bus_config bus_config = {.write = uart_write, .port = &port};
    const struct x430_device_config joint_config = {
        .id = 11U,                  /* factory ID */
        .firmware_version = 0U,     /* 0: X430_FIRMWARE_VERSION (52) */
        .model_information = 0x00010000U,
        .hooks = &hooks,
        .user_data = NULL,
    };
    int result = x430_bus_init(&bus, &bus_config);

    if (result == X430_OK)
    {
        result = x430_device_init(&joint, &joint_config);
    }
    if (result == X430_OK)
    {
        result = x430_bus_attach(&bus, &joint);
    }
    return result;
}
```

`x430_device_config::id` is the **factory ID**: it is used while no EEPROM block is stored
and again after FACTORY_RESET 0xFF; an ID stored by the host wins at startup.
`x430_device_init()` loads the factory values, the stored EEPROM area (`nvm_read`), the RAM
area and applies the Startup Configuration — which may already call the write hook to
switch the torque on. Initialize a device before attaching it, never while a bus serves
it. A dual-axis 2XL430/2XC430 is two devices (factory IDs 1 and 2).

## Reacting to the host: hooks

| Hook | Called when | The application |
| --- | --- | --- |
| `write` | For every register a WRITE, SYNC_WRITE, BULK_WRITE or ACTION touches, after all of them passed the checks, before the value is stored | Applies goals, limits and modes to the motor; returns `DXL2_ERR_NONE` or a `DXL2_ERR_*` code to refuse |
| `read` | Before data is read, once per run of contiguous real addresses | May refresh the live registers it covers |
| `event` | REBOOT, FACTORY_RESET, CLEAR, Bus Watchdog, Baud Rate, shutdown, backup restored | Resets or stops the motor, switches the UART |
| `nvm_read`, `nvm_write` | Initialization, REBOOT, EEPROM writes, FACTORY_RESET, BACKUP | Reads or writes part of an opaque block at an offset |

Every member may be NULL. A write hook for a stepper joint:

```c
static uint8_t joint_write(struct x430_device* device, uint16_t address, uint8_t size,
                           int32_t value, void* user_data)
{
    uint8_t result = DXL2_ERR_NONE;
    int32_t mode = 0;

    (void)size;
    (void)user_data;
    (void)x430_device_get(device, X430_ADDR_OPERATING_MODE, &mode); /* already validated */
    switch (address)
    {
        case X430_ADDR_TORQUE_ENABLE:
            if (!motor_enable(value != 0))
            {
                result = DXL2_ERR_RESULT_FAIL;
            }
            break;
        case X430_ADDR_GOAL_POSITION:
            motor_move_to(value);
            break;
        case X430_ADDR_GOAL_VELOCITY:
            if (mode == (int32_t)X430_MODE_VELOCITY)
            {
                motor_run(value);
            }
            break;
        default:
            break; /* stored only */
    }
    return result;
}
```

The value arrives sign-extended and already checked (range, limit registers, EEPROM lock,
Bus Watchdog). On Torque Enable 0 → 1 the library copies Present Position into Goal
Position after the hook, so keep Present Position current.

## Keeping the live registers current

The device does not know the motor. Update Present Position (132), Present Velocity (128),
Present Current/Load (126), Present PWM (124), Moving (122), Moving Status (123),
Velocity/Position Trajectory (136/140), Present Input Voltage (144) and Present
Temperature (146) with `x430_device_set()`, either periodically:

```c
void motor_loop_1ms(void)
{
    (void)x430_bus_lock(&bus);
    (void)x430_device_set(&joint, X430_ADDR_PRESENT_POSITION, motor_position());
    (void)x430_device_set(&joint, X430_ADDR_PRESENT_VELOCITY, motor_velocity());
    (void)x430_device_set(&joint, X430_ADDR_MOVING, motor_moving() ? 1 : 0);
    (void)x430_bus_unlock(&bus);
}
```

or on demand in the `read` hook, where the lock is already held. `x430_device_set()`
checks nothing but that a register starts at the address.

## Faults

Report a fault of the power stage with `x430_device_raise_hardware_error()`:

```c
void on_overtemperature(void)
{
    (void)x430_bus_lock(&bus);
    (void)x430_device_raise_hardware_error(&joint, X430_HW_ERROR_OVERHEATING);
    (void)x430_bus_unlock(&bus);
}
```

The bit stays in Hardware Error Status (70) until REBOOT and every status packet carries
the alert bit. If the bit is enabled in Shutdown (63) and the torque is on, the library
switches Torque Enable off, calls the event hook with `X430_EVENT_TORQUE_SHUTDOWN` and
refuses Torque Enable = 1 with Result Fail until REBOOT.

## Using the simulator

Instead of a motor, a device can run the built-in model:

```c
int sim_start(void)
{
    const struct x430_device_config config = {
        .id = 1U, .hooks = &x430_sim_hooks, .user_data = &joint_sim};
    int result = x430_sim_init(&joint_sim, &joint, NULL); /* before the device */

    if (result == X430_OK)
    {
        result = x430_device_init(&joint, &config);
    }
    return result;
}

void sim_tick_1ms(void)
{
    (void)x430_sim_step(&joint_sim, 1U); /* takes the bus lock when attached */
}
```

## Return values

| Code | Value | Returned when |
| --- | --- | --- |
| `X430_OK` | 0 | Success (also when the host gets a protocol error) |
| `X430_ERR_PARAM` | −1 | `NULL` pointer, a value outside its range, no register at the address, no device with the ID |
| `X430_ERR_STATE` | −2 | An instance without a successful initialization; a device that is already attached |
| `X430_ERR_CONFIG` | −3 | Bus full, or the device ID is already used on the bus |
| `X430_ERR_HAL` | −4 | The byte writer returned non-zero for a status packet |
| `X430_ERR_UNSUPPORTED` | −5 | BACKUP or the simulator is not compiled in |

## Common mistakes

| Symptom | Likely cause |
| --- | --- |
| Every call returns `X430_ERR_STATE` | The *_init() function failed or was not called |
| The host gets no answer | Wrong baud rate or ID; Status Return Level 0/1; the received bytes never reach `x430_bus_receive()` |
| DYNAMIXEL Wizard shows TIME OUT on the Control Table tab | `DXL2_MAX_PACKET_SIZE` was lowered below 768: the Wizard reads the whole table (662 bytes) at once ([Configuration](configuration.md#packet-size)) |
| ROS 2 refuses the device | The Model Number is not in its model files (XW430-T200); use `CONFIG_X430_MODEL_NUMBER_OVERRIDE` |
| The arm jumps when the torque is switched on | Present Position was not current when the host wrote Torque Enable |
| EEPROM writes answer Access Error | The torque is on (as on a real X430), or the packet used the Secondary ID |
| The host sees the old values after a power cycle | No `nvm_write` hook, `nvm_write` does not keep the parts at their offsets, or `nvm_read` fails |
| Deadlock in a hook | The hook called a bus function, or the lock is not recursive |
