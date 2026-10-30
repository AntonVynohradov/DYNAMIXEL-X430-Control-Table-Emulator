# Migration from SmartManipulator-X `dynamixel_x430.c`

The SmartManipulator-X firmware (`apps/SmartManipulator_Controller/src/dynamixel_x430.c`)
mixes three things: the Protocol 2.0 transport (`protocol2.c`, `communication.c`), the
XW430-T333 Control Table, and the stepper motion (`motor_*`). With this library the
first two are replaced by DXL2-C and `x430_bus`/`x430_device`; the motion code stays in
the application and is reached through hooks.

## Mapping

| SmartManipulator-X | sfs-dynamixel-x430 |
|---|---|
| `protocol2.c`, `communication.c` | DXL2-C link inside `x430_bus` (`x430_bus_receive`) |
| `DXL_XW430_T333_R_MODEL_NUMBER` | `CONFIG_X430_MODEL_XW430_T333=y` |
| `x430_read_static_byte()`, `x430_expected_write_width()`, `x430_validate_write()` | generated table + `x430_device_write()` checks |
| `struct x430_axis` RAM fields | the device's Control Table image |
| `x430_write_register()` side effects | `x430_hooks::write` |
| `x430_read_registers()` telemetry | `x430_hooks::read` or periodic `x430_device_set()` |
| REG_WRITE/ACTION, SYNC_READ/WRITE | built in, plus BULK and FAST reads, Indirect Address |
| Hardware Error Status latch (driver reset) | `x430_device_raise_hardware_error(dev, X430_HW_ERROR_INPUT_VOLTAGE)` |
| Model Information (driver, frame, rated current) | `x430_device_config::model_information` |
| Baud codes 23 (230400) and 46 (460800) | `CONFIG_X430_BAUD_RATE_CODE_MAX=46`, check the code in the write hook |
| `motor_sim.c` | `x430_sim` (or keep `motor_sim.c` behind the same hooks) |
| `k_sem` wait in `x430_motor_call()` | your hook decides; the library does not block |

## The write hook of a stepper axis

The unit conversions and ramp logic of `dynamixel_x430.c` move into the application
hook almost unchanged:

```c
static uint8_t axis_write(struct x430_device* dev, uint16_t address, uint8_t size, int32_t value,
                          void* user)
{
    struct axis* axis = user;

    switch (address)
    {
        case X430_ADDR_TORQUE_ENABLE:
            /* was x430_write_torque_enable(): wrap the counter in mode 3, enable/disable */
            return axis_set_torque(axis, dev, value != 0);
        case X430_ADDR_GOAL_POSITION:
            /* was x430_position_to_motor() + x430_apply_ramp() + MOVE_TO */
            return axis_move_to(axis, dev, value);
        case X430_ADDR_GOAL_VELOCITY:
            return axis_run(axis, dev, value);
        case X430_ADDR_PROFILE_VELOCITY:
        case X430_ADDR_PROFILE_ACCELERATION:
            return axis_refresh_ramp(axis, dev, address, value);
        case X430_ADDR_CURRENT_LIMIT:
            /* 2.69 mA units -> TMC5160 IRUN; refuse above the rated current */
            return axis_set_run_current(axis, value);
        default:
            return DXL2_ERR_NONE;
    }
}
```

Read the other registers the hook needs (Drive Mode, Homing Offset, Velocity Limit,
Operating Mode) with `x430_device_get(dev, ...)`; they already hold validated values.

## What stays in the application

- The controller device (model 0x5358) and the gripper servo device (0x5356): route
  their IDs in your own DXL2-C link handler and pass everything else to
  `x430_bus_handle_packet()` (see [Integration](integration.md)).
- The TMC5160 register window at addresses 1024+ (beyond the 662-byte X430 table):
  handle READs of that range in the same router before the bus sees them.
- Driver presence (axes that stay silent without a driver): attach a device to the bus
  only once its driver answers.
