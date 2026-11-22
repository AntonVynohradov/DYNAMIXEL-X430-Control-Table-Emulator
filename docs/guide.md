# Getting Started

This chapter shows how to get the X430 emulator, add it to your project, and build and
test it.

## Get the library

```bash
git clone https://github.com/AntonVynohradov/DYNAMIXEL-X430-Control-Table-Emulator.git
```

Each [release](https://github.com/AntonVynohradov/DYNAMIXEL-X430-Control-Table-Emulator/releases)
provides the source code, a prebuilt static library for Linux x86-64 (default options:
XM430-W350, every feature) with its headers, the MISRA report and the PDF user manual. For
an MCU, and for any other model, build the library from source: the Control Table is
generated for the model chosen at build time.

## Add it to your project

**CMake.**

```cmake
set(X430_MODEL xm430_w350)        # X430_KCONFIG ON for Kconfig fragments, see Configuration
include(path/to/DYNAMIXEL-X430-Control-Table-Emulator/x430.cmake)
target_link_libraries(app PRIVATE x430)
```

`x430.cmake` generates the Control Table of the model into the build directory, fetches
DXL2-C 2.0.0 with FetchContent (or takes a local checkout given in
`FETCHCONTENT_SOURCE_DIR_DXL2`) and defines the static library `x430`, which links `dxl2`.

**Any other build system.** Run
`python3 tools/x430_gen_model.py --models-dir models --model xm430_w350 --out-dir gen`,
compile `src/*.c`, `gen/x430_model_table.c` and the two sources of DXL2-C, and add the
repository root, `inc/`, `gen/` and the DXL2-C folders to the include path. Pass the same
`X430_*` definitions to the library and the application, and define
`DXL2_MAX_PACKET_SIZE=768U` for the library, DXL2-C and the application alike
(`x430_config.h` has no default and stops the build without it).

**As a submodule with tests.** A project that sets `UNIT_TESTS=ON` also builds the unit
tests of this library (`x430_gtest`, CTest label `x430`); `COMPILE_DEFINITIONS` and
`COMPILE_OPTIONS` of the project reach the library as well.

## Prerequisites for development

- C11 compiler, CMake 3.20+, Python 3 (the model generator needs only the standard
  library; PyYAML is used when installed)
- Optional: GoogleTest and gcovr, Cppcheck 2.13, clang-format 18, Kconfiglib, Python 3 with
  `requirements-dev.txt` (MkDocs, WeasyPrint, ziamath), Doxygen, Pango,
  `arm-none-eabi-gcc` for the measurements

## Build with presets

| Preset | Purpose |
| --- | --- |
| `X430[Debug]` | Static library with debug information (XM430-W350, without Kconfig) |
| `X430[Release]` | Optimized static library |
| `X430[UnitTests]` | GoogleTest suite with coverage, every feature; target `coverage` |
| `X430[UnitTestsMinimal]` | The same with every optional feature left out |
| `X430[Kconfig]` | Configuration with Kconfiglib and `configs/defconfig`; target `menuconfig` |
| `X430[Examples]` | `x430_host_sim`, the POSIX host simulator |

```bash
cmake --preset "X430[Debug]"
cmake --build "build/X430[Debug]"
```

Another model: `cmake --preset "X430[Debug]" -DX430_MODEL=xl430_w250`.

## Unit tests and coverage

```bash
cmake --preset "X430[UnitTests]"
cmake --build "build/X430[UnitTests]" --target coverage
```

The `coverage` target runs the tests and fails below `X430_COVERAGE_MIN` (100 %) line or
branch coverage of `src/`. Both unit test presets reach 100 %. The tests are in
`tests/unit/` (one file per area: API contract, protocol, group instructions, lifecycle,
device rules, simulator); `tests/compile/x430_api_check.c` compiles the public API alone,
as an application sees it. CTest also runs `tests/models`, which compares the model data
with the e-Manual tables.

**End to end with the official DynamixelSDK** (Linux):

```bash
cmake --preset "X430[Examples]" && cmake --build "build/X430[Examples]"
tests/sdk/run_sdk_check.sh "build/X430[Examples]"
```

The script clones the ROBOTIS DynamixelSDK, serves three simulated devices on a
pseudo-terminal and runs 52 checks: PING, READ/WRITE, Sync, Bulk and Fast reads, Indirect
Address, REBOOT, FACTORY_RESET, CLEAR.

## Static analysis

| Target | Tool |
| --- | --- |
| `cppcheck-analyze` | Cppcheck, all checks |
| `misra-analyze` | Cppcheck MISRA C:2012 addon; fails on any violation |
| `codeql-analyze` | CodeQL `cpp-queries` |

Check MISRA with the Cppcheck version of CI (2.13, Ubuntu 24.04): newer versions and
Windows hosts report other findings.

### Formatting

```bash
clang-format -i x430.h inc/*.h src/*.c src/*.h tests/unit/*.cpp tests/unit/*.h \
    tests/compile/*.c examples/host_sim/*.c
```

CI checks every C/C++ file with `clang-format` 18.

## Versioning and releases

The X430 emulator follows [Semantic Versioning](https://semver.org): the public API
(`x430.h` and `inc/`) is stable within a major version. The version comes only from the
latest `vX.Y.Z` Git tag: `cmake/version.cmake` sets it in CMake builds and the release
workflow stamps it into the packaged `x430.h`. A header taken from the repository and built
without CMake reports `0.0.0` (unknown). Every change is recorded under `[Unreleased]` in
`CHANGELOG.md`.

A release tag runs `release.yml`: changelog check, version stamp, unit tests with the
coverage gate in every unit test preset, MISRA gate, release build, and a GitHub Release
with the static library, the headers, the MISRA report and the PDF manual.

## Measured figures

`tools/x430_measure.sh <build-dir> [dxl2-dir]` prints every figure of the
[Configuration](configuration.md#memory-footprint) chapter: the instance sizes on
Cortex-M4 and on the host, the flash of each module and the worst-case stack of every
public function.

## Documentation

| Task | Command | VS Code task |
| --- | --- | --- |
| Live preview | `mkdocs serve` | *Docs: Serve on localhost* |
| Build the website | `mkdocs build` | *Docs: Build site* |
| Build the PDF manual | `python tools/docs/build_manual.py` | *Docs: Build PDF manual* |
| Regenerate diagrams | `python tools/docs/diagrams.py` | *Docs: Generate diagrams* |
| Compile the C snippets | `python tools/docs/check_snippets.py` (after configuring `X430[Debug]`) | *Docs: Check C snippets* |
