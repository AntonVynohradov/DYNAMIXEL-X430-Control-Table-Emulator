# Models

## Data

`models/x430_family.yaml` holds the Control Table shared by the X430 family, with two
variants:

- **current** (XM, XH, XD, XW): Current Limit (38), Goal Current (102), Present
  Current (126); operating modes 0, 1, 3, 4, 5, 16.
- **load** (XL, XC, 2XL, 2XC): no current control, Present Load (126); operating
  modes 1, 3, 4, 16.

`models/x430/<model>.yaml` lists only what differs: Model Number, variant, removed
registers (LED on XW) and initial values or ranges. `tools/x430_gen_model.py` merges
both, validates the result (sizes, overlaps, EEPROM below 64, read-only registers
without checks, ...) and writes `x430_model_table.c` with a register list and an
address map.

```bash
python3 tools/x430_gen_model.py --models-dir models --list
python3 tools/x430_gen_model.py --models-dir models --dump xh430_v350
```

`tests/models` compares every model with a snapshot of the e-Manual control tables
(`tests/models/emanual_x430.json`, extracted from the ROBOTIS e-Manual sources): size,
access, area, initial value and range of every register. Differences are allowed only
when listed as deviations below.

## Deviations from the documentation

| Item | e-Manual | Emulator | Why |
|---|---|---|---|
| Protocol Type (13) | 1 ~ 2 | 2 only | Only Protocol 2.0 is emulated |
| Protocol Type initial on XW | 1 | 2 | Same; the XW page's description says 2 is the default |
| Secondary ID (12) range | 0 ~ 252 | 0 ~ 255 | 253 ~ 255 disable the function (documented in the description) |
| Bus Watchdog (98) range | 1 ~ 127 | 0 ~ 127 | 0 disables and clears the error (documented in the description) |
| XW430-T200 Model Number | 1,270 | 1,280 | The table repeats the number of XW430-T333; ROBOTIS tools use 1,280 |
| Profile Acceleration, time-based | 0 ~ 32,737 | 0 ~ 32,767 | The time-based limit belongs to the motion backend |
| ID write that collides with another emulated device | accepted | Data Range Error | Two local devices with one ID would answer together |
| Partial write of a multi-byte register | – | bytes are merged | Not specified; checked as the whole new value |

Behaviour that belongs to the motor (profiles, PID gains, current control, time-based
profile rules) is not emulated by the library; the simulator approximates it.

## Adding a model

1. Write `models/<family>/<model>.yaml` (copy a similar model).
2. Add the choice entry and the `X430_MODEL_NAME` default to `Kconfig.x430`
   (`tests/models` fails if it is missing).
3. Add the e-Manual rows to `tests/models/emanual_x430.json`.
4. Run the unit tests with `-DX430_KCONFIG=OFF -DX430_MODEL=<model>`.

Another family (X330, X540) gets its own `<family>_family.yaml`; the generator and
the library are family-independent as long as the addresses in `x430_defs.h` match.
