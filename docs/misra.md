# MISRA C:2012

The library follows the conventions of DXL2-C (C11, no heap, no recursion, fixed-size
types, explicit casts, single exit per function, every `if`/`switch` complete). The
`misra-analyze` target runs the Cppcheck MISRA addon on `src/` the same way as DXL2-C.

Status: the code has not been through the MISRA addon yet; run
`cmake --build <build> --target misra-analyze` and record the deviations here.

Known candidates for documented deviations:

| Rule | Where | Reason |
|---|---|---|
| 11.5 | `x430_sim.c`, hooks | `void*` user data converted to `struct x430_sim*` (callback context) |
| 8.9 | `x430_device.c` | file-scope persistence buffer shared by all devices (bus lock held) |
| 21.6 | `examples/` only | standard I/O in the host example (not part of the library) |
