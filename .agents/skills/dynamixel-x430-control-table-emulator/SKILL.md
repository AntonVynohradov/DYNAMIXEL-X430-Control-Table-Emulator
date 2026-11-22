---
name: dynamixel-x430-control-table-emulator
description: "Use when working on DYNAMIXEL-X430-Control-Table-Emulator (x430), the C11 device-side emulator of DYNAMIXEL X430 actuators over Protocol 2.0: the bus (instruction decoding, group and Fast reads, Bus Watchdog), the devices (Control Table rules, hooks, persistence), the generated model tables, the simulator, the opaque instances, the unit, model-data and DynamixelSDK tests, MISRA C:2012 compliance, the MkDocs/PDF documentation, releases, or integrating the library into MCU firmware."
---

# DYNAMIXEL-X430-Control-Table-Emulator (x430): DYNAMIXEL X430 Control Table Emulator

## Scope

A portable C11 static library that answers on a DYNAMIXEL bus like a ROBOTIS X430
actuator: it decodes Protocol 2.0 instruction packets (DXL2-C 2.0.0), keeps the Control
Table of the model selected at build time (generated from `models/*.yaml`), enforces the
rules of the real device and calls application hooks for the motor. Version 1.2.0 (since 1.1.0 several
buses share one wire; 1.2.0 needs less RAM per bus with DXL2-C 2.0.0); the public API in `x430.h` and `inc/` is stable within major version 1.

Not in the library: UART and flash drivers, motion and current control, threads and
interrupts, Protocol 1.0. They belong to the firmware that uses it (verified downstream in
the Doppelstep stepper joint firmware, Zephyr).

| Part | Files | Rule |
| --- | --- | --- |
| Public API | `x430.h` | Every public function declaration, `x430_sim_hooks`, version fallbacks 0.0.0 |
| Public types | `inc/x430_types.h`, `inc/x430_defs.h`, `inc/x430_config.h`, `inc/x430_model.h` | Port, hooks, configurations, opaque instances and their size macros; status codes, addresses; build options; model descriptor |
| Bus | `src/x430_bus.c` | Addressing, single and group instructions, Fast read packet, Status Return Level, watchdog timing |
| Device | `src/x430_device.c` | Control Table, write checks (two passes), Indirect Address, REG_WRITE/ACTION, lifecycle, persistence |
| Simulator | `src/x430_sim.c` | Kinematic model; feature `X430_FEATURE_SIMULATOR` |
| Internal | `src/x430_internal.h` | `struct x430_bus_ctx`, `x430_device_ctx`, `x430_sim_ctx`, the `*_ctx_of()` casts, internal functions |
| Model data | `models/`, `tools/x430_gen_model.py`, `tests/models/` | YAML checked against the e-Manual snapshot; the generator has a built-in reader of the YAML subset |
| Build | `CMakeLists.txt`, `x430.cmake`, `cmake/`, `CMakePresets.json`, `Kconfig*` | `cmake/` is not on `CMAKE_MODULE_PATH` (DXL2-C includes its own `version.cmake` by name) |
| Tests | `tests/unit/gtest_x430_*.cpp`, `tests/compile/x430_api_check.c`, `tests/sdk/` | GoogleTest; compile check; DynamixelSDK end to end |

## Execution model

Everything runs in one bus context (RX thread or main loop), never in an interrupt:
`x430_bus_receive()` with the received bytes, `x430_bus_poll()` every 5–20 ms. Hooks run
inside it with the bus lock (`x430_port::lock`) held and may call only the lock-free device
functions. The motor loop updates the live registers between `x430_bus_lock()` and
`x430_bus_unlock()`; `x430_sim_step()` takes the lock itself.

## Hard rules

- Library sources include only `<stddef.h>`, `<stdint.h>`, `<stdbool.h>`, the x430 headers
  and `dxl2.h`. No `<string.h>` (`x430_copy()` / `x430_fill()`), no heap, no RTOS, no
  global or static mutable data (`.data` and `.bss` stay at 0 B), no recursion.
- **Encapsulation.** `struct x430_bus`, `x430_device`, `x430_sim` are opaque
  (`uint64_t opaque[X430_*_WORDS]`); the state is reached only through the `*_ctx_of()`
  functions (MISRA DEV-01). Every public function checks NULL (`X430_ERR_PARAM`) and
  `ready != X430_READY` (`X430_ERR_STATE`) before any other access and calls no hook then.
  The *_init() functions check all arguments before they change the instance and must not
  trust its previous content (a stack instance holds garbage).
- **Instance size.** `X430_BUS_BYTES`, `X430_DEVICE_BYTES`, `X430_SIM_BYTES` and
  `X430_DXL2_LINK_BYTES` in `inc/x430_types.h` are plain arithmetic of the configuration
  macros (no `sizeof`); the `_Static_assert`s fail when a private struct grows, and
  `InstancesHoldTheirStateWithoutWaste` fails when a size wastes a word. DXL2-C's link
  layout is mirrored exactly; a new DXL2-C version must be re-checked.
- **API errors vs. protocol errors.** API misuse returns `X430_ERR_*`; what the host does
  wrong is a `DXL2_ERR_*` in the status packet and in the `error` output, with `X430_OK`.
- **Build interface.** `COMPILE_DEFINITIONS` (public definitions, e.g. `X430_*` options),
  `COMPILE_OPTIONS` (private flags of the library sources) and `UNIT_TESTS` have no
  prefix on purpose: a parent project sets them once for all its libraries. With
  `UNIT_TESTS` the library stays a static library; the tests are a separate executable
  (`X430_gtest` here, `x430_gtest` with CTest label `x430` in a parent project).
  `X430_KCONFIG` is off by default; `DXL2_MAX_PACKET_SIZE` defaults to 768 (a READ of the
  whole Control Table, as DYNAMIXEL Wizard does it), set only in `Kconfig.x430` and
  `x430.cmake`: `x430_config.h` has no default and stops a build without either.
- Options defined on the command line may be signed: use `X430_MAX_DEVICES_U`,
  `X430_REG_WRITE_SIZE_U`, `X430_BAUD_RATE_CODE_MAX_U` in expressions (Rule 10.4).
- MISRA C:2012: `misra-analyze` reports 0 violations with Cppcheck 2.13 (Ubuntu 24.04);
  deviations DEV-01 (11.3) and DEV-02 (11.5) only, in `docs/misra.md`. No conditions that
  are constant in one variant: both unit test presets must reach 100 % branches.
- No warnings with `-std=c11 -Wall -Wextra -Wpedantic -Wconversion` (host and
  `arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb`). Style: `.clang-format`, `clang-format` 18.
- Keep the public API stable within a major version; record every user-visible change
  under `[Unreleased]` in `CHANGELOG.md`. Downstream (Doppelstep `drivers/x430/`) uses
  `x430_device_get/set/write/flush_events`, the bus functions and the hooks.

## Tests (definition of done)

```sh
cmake --preset "X430[UnitTests]" && cmake --build "build/X430[UnitTests]" --target coverage
cmake --preset "X430[UnitTestsMinimal]" && cmake --build "build/X430[UnitTestsMinimal]" --target coverage
"build/X430[UnitTests]/X430_gtest"            # once in one process: no state between tests
cmake --preset "X430[Debug]" && cmake --build "build/X430[Debug]" --target misra-analyze
cmake --preset "X430[Examples]" && cmake --build "build/X430[Examples]" && tests/sdk/run_sdk_check.sh "build/X430[Examples]"
python tools/docs/check_snippets.py
tools/x430_measure.sh "build/X430[Release]"   # figures of docs/configuration.md
```

- `coverage` fails below 100 % line or branch coverage of `src/` in every unit test
  preset. Register new test files in `x430.cmake`.
- Model-based checks: `TrapezoidalMoveTimeMatchesTheClosedForm` (2,920 ms vs 2,940.3 ms)
  and `VelocityRampTimeMatchesTheClosedForm` (1,275 ms vs 1,280.7 ms) print
  `[ measured ]` lines quoted in `docs/theory.md`; `tests/models` checks the model data
  against the e-Manual; the DynamixelSDK check runs 52 checks.
- Run the sanitizers (ASan, UBSan) and MISRA in an Ubuntu 24.04 container before a release.

## Documentation

MkDocs Material website (`mkdocs.yml`, pages in `docs/`) and a PDF manual from the same
pages (`python tools/docs/build_manual.py`), in English. Formulas in LaTeX. Diagrams: edit
and run `tools/docs/diagrams.py`, never edit the SVGs; the motion profile there uses the
integer arithmetic of `x430_sim.c`. Hero: the animated `hero-3d.svg` from `diagrams.py`
(host adapter, board with the emulator chip, stepper motor). The DYNAMIXEL Wizard
screenshots in `docs/assets/images/wizard/` come from the Doppelstep firmware. C snippets
compile with `tools/docs/check_snippets.py` (configure `X430[Debug]` first: it uses
`build/X430[Debug]/x430/analysis-include`); API prototypes must match the headers exactly.
Every number is measured. Change project data only in the shared tooling (`mkdocs.yml`,
`diagrams.py`, `snippets.json`, `snippet_stubs.h`).

## Releases

The Git tag is the only source of the version: do not write it into `x430.h`,
`cmake/version.cmake`, `CITATION.cff` or the documentation.

1. Move the `[Unreleased]` entries in `CHANGELOG.md` to `[X.Y.Z] - date` and add the
   compare links at the end of the file.
2. Merge to `main`, then push the tag `vX.Y.Z`; the Release workflow does the rest.
   `release.yml` differs from the family template on purpose (the only difference of
   `scaffold.py --check`): the package also carries `libdxl2.a`, the DXL2-C headers and
   `x430_model_info.h`, without which `x430.h` does not compile.
3. Move the Doppelstep `west.yml` pin of `dynamixel-x430` to the tag.

Never commit, push a tag or publish a release without the maintainer's explicit request.
