# Extending the Library

Most extensions are built on top of the API: a motor behind the hooks, other devices on the
same line, a product-specific range of a register. This chapter shows where each belongs
and the rules for changes to the library itself.

## Where to extend

| You want to… | Do it in | Library change needed? |
| --- | --- | --- |
| Run on another MCU, UART or RTOS | The byte writer, the port and the RX code | No |
| Drive another kind of motor | The hooks of the device | No |
| Change a range or an initial value of the model (for example a supply above 16 V) | An application overrides file ([Configuration](configuration.md#application-overrides-of-the-model)) | No |
| Report your firmware version to the host | `x430_device_config::model_information` (Model Information, 2) | No |
| Accept more Baud Rate codes | `X430_BAUD_RATE_CODE_MAX` (in `COMPILE_DEFINITIONS` or Kconfig) and a check in the write hook | No |
| Host other, non-X430 devices on the same line | Your own DXL2-C link and `x430_bus_handle_packet()` (below) | No |
| Answer for registers beyond the X430 table | Your own router before the bus (below) | No |
| Add an X430 model | `models/x430/<model>.yaml`, `Kconfig.x430`, the e-Manual snapshot | Model data only |
| Emulate another DYNAMIXEL family | `models/<family>_family.yaml` and its models | Model data; the addresses of `x430_defs.h` must match |
| A new instruction | `src/x430_bus.c` | Yes |

## Several kinds of devices on one line

An application that already runs its own DXL2-C link — for example to host a controller
device with its own Control Table next to the joints — routes the packets itself and hands
the rest to the bus:

```c
#define CONTROLLER_ID 200U

void controller_handle(const struct dxl2_packet* packet);

static void on_packet(const struct dxl2_packet* packet, void* context)
{
    (void)context;
    if (packet->id == CONTROLLER_ID)
    {
        controller_handle(packet);
    }
    else
    {
        (void)x430_bus_handle_packet(&bus, packet); /* answers through the bus writer */
    }
}
```

The same router can answer READs of an address window beyond the 662-byte X430 table (a
driver register window, for example) before the bus sees them; the bus would answer them
with an Access Error.

## Adding a model

1. Write `models/x430/<model>.yaml` with the differences from `models/x430_family.yaml`
   (copy a similar model): Model Number, variant (`current` or `load`), operating modes,
   removed registers, initial values and ranges.
2. Add the choice entry and the `X430_MODEL_NAME` default to `Kconfig.x430`
   (`tests/models` fails if it is missing).
3. Add the e-Manual rows to `tests/models/emanual_x430.json`.
4. Run the unit tests with `-DX430_MODEL=<model>`.

`python3 tools/x430_gen_model.py --models-dir models --dump <model>` prints the resolved
table.

## Contributing to the library

1. **Code.** C11, no new dependencies, no heap, no global state; the rules of
   `misra-analyze`; internal types stay in `src/`, `x430.h` declares the public API only.
2. **Tests.** 100 % line and branch coverage of `src/` in every unit test preset
   (`X430[UnitTests]`, `X430[UnitTestsMinimal]`); the test binary also passes in one
   process.
3. **Instance size.** If a private state grows, update the arithmetic of
   `X430_BUS_BYTES`, `X430_DEVICE_BYTES` or `X430_SIM_BYTES` in `inc/x430_types.h`; the
   build fails until it fits, and a unit test fails if it wastes a word.
4. **Analysis.** `misra-analyze` 0 violations with Cppcheck 2.13, or a recorded deviation
   in [MISRA C:2012](misra.md).
5. **Format.** `clang-format` 18 reports no changes.
6. **Documentation.** The affected pages, the measured figures
   (`tools/x430_measure.sh`), the diagrams and `CHANGELOG.md` `[Unreleased]`.
7. **Compatibility.** The public API is stable within a major version.
