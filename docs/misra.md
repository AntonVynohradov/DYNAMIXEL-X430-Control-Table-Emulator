# MISRA C:2012 compliance

This page records how the library is checked against MISRA C:2012 and every deviation from
it. It covers the library code only: `src/`, `inc/` and `x430.h`. The generated model
table is constant data written by `tools/x430_gen_model.py`; the host example and the tests
are not part of the library.

## Checking

| Check | Where | Gate |
| --- | --- | --- |
| Cppcheck MISRA C:2012 addon | `misra-analyze`, CI *MISRA C:2012 Analysis* | Fails on any violation |
| Cppcheck, all checks | `cppcheck-analyze`, CI *Cppcheck Analysis* | Report |
| CodeQL `cpp-queries` | `codeql-analyze`, CI *CodeQL Analysis* | Report |
| Compiler | `-std=c11 -Wall -Wextra -Wpedantic -Wconversion` (`-Werror` in the unit tests) | No warnings |
| Sanitizers | AddressSanitizer and UndefinedBehaviorSanitizer on both unit test presets | No finding |
| Manual review | Pull request review | Reviewer |

The analysis sees the sources without the Kconfig header, so Cppcheck checks every
configuration of the `#if` blocks; the generated model header and the DXL2-C headers are
given to it as copies in `<build>/x430/analysis-include`.

## Current status

| Item | Result |
| --- | --- |
| Cppcheck MISRA addon | 0 violations (Cppcheck 2.13.0, Ubuntu 24.04) |
| Unit test coverage of `src/` | 100 % lines and branches, `X430[UnitTests]` and `X430[UnitTestsMinimal]` |
| Deviations | 2 (DEV-01, DEV-02) |

## Manual review notes

| Guideline | How the library complies |
| --- | --- |
| Dir 4.1 (run-time failures) | Every table access is bounded by the table size: addresses and lengths of instructions are checked before use, Indirect Address targets outside the table are ignored, packet parameters are read only within their length; sanitizers in CI. |
| Dir 4.11 (validity of arguments) | Every public function checks its pointers and the initialization of the instance before any access. |
| Dir 4.12, Rule 21.3 (dynamic memory) | No heap. |
| Rule 8.9 (objects at block scope) | No file-scope object: the only constant data are the model table and `x430_sim_hooks`, both used from several functions or by the application. |
| Rule 10.4 (essential types) | Build-time options may be signed literals on the command line; the code uses the unsigned copies `X430_MAX_DEVICES_U`, `X430_REG_WRITE_SIZE_U`, `X430_BAUD_RATE_CODE_MAX_U`. |
| Rule 15.5 (single point of exit) | One `return` per function. |
| Rule 17.2 (recursion) | No recursion; `tools/docs/stack_usage.py` fails on a call cycle. |
| Rule 21.6 (standard I/O) | No `<stdio.h>`; no `<string.h>` either (byte copies are explicit loops). |

## Deviation register

### DEV-01: Rule 11.3 (required), opaque instances

- **Location:** `x430_bus_ctx_of()`, `x430_bus_ctx_of_const()`, `x430_device_ctx_of()`,
  `x430_device_ctx_of_const()` and `x430_sim_ctx_of()` in `src/x430_internal.h`, the only
  casts from the public instance types to the private state types. Marked with
  `cppcheck-suppress misra-c2012-11.3`.
- **Rationale:** The application allocates the instances (no heap, no global state), so it
  must know their size, but not their layout. The public `struct x430_bus`,
  `struct x430_device` and `struct x430_sim` are arrays of `uint64_t` of that size; the
  private `struct x430_bus_ctx`, `struct x430_device_ctx` and `struct x430_sim_ctx` hold
  the state. The internal types stay out of the application code and can change without
  breaking the API.
- **Risk control:** `x430_bus.c`, `x430_device.c` and `x430_sim.c` check at compile time
  that each public type is at least as large and as aligned as its private state, and that
  the DXL2-C link has the size the public header assumes; a unit test checks that the
  public size wastes less than one word. Every public function converts the pointer before
  any member access and checks it for NULL and for a successful initialization (`ready`).

### DEV-02: Rule 11.5 (advisory), context of a callback

- **Location:** `x430_handler()` in `src/x430_bus.c` (the packet handler given to DXL2-C)
  and `x430_sim_of()` in `src/x430_sim.c` (the user data of the simulator hooks). Marked with `cppcheck-suppress misra-c2012-11.5`.
- **Rationale:** DXL2-C and the hook interface pass a `void*` context back to the callback,
  the usual C way to give a callback its object without global state. The library converts
  it back to the type it registered.
- **Risk control:** The bus registers its own private state as the DXL2-C context in
  `x430_bus_init()`; nothing else can reach these callbacks. The simulator hooks accept
  only an initialized `struct x430_sim` (the `ready` field is checked; NULL and an
  uninitialized simulator are ignored), which the application passes as
  `x430_device_config::user_data` as documented.
