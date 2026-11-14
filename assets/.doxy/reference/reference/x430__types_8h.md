

# File x430\_types.h



[**FileList**](files.md) **>** [**inc**](dir_bfccd401955b95cf8c75461437045ac0.md) **>** [**x430\_types.h**](x430__types_8h.md)

[Go to the source code of this file](x430__types_8h_source.md)



* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include "x430_config.h"`
* `#include "dxl2.h"`
* `#include "x430_defs.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**x430\_bus**](structx430__bus.md) <br>_One serial line with up to_ [_**X430\_MAX\_DEVICES**_](group__x430__config.md#define-x430_max_devices) _emulated devices._ |
| struct | [**x430\_bus\_config**](structx430__bus__config.md) <br>_Bus configuration, copied by_ [_**x430\_bus\_init()**_](group__x430.md#function-x430_bus_init) _._ |
| struct | [**x430\_device**](structx430__device.md) <br>_One emulated actuator (one DYNAMIXEL ID): Control Table image and rules._  |
| struct | [**x430\_device\_config**](structx430__device__config.md) <br>_Device configuration, copied by_ [_**x430\_device\_init()**_](group__x430.md#function-x430_device_init) _._ |
| struct | [**x430\_hooks**](structx430__hooks.md) <br>_Application hooks of a device._  |
| struct | [**x430\_port**](structx430__port.md) <br>_Platform services._  |
| struct | [**x430\_sim**](structx430__sim.md) <br>_Kinematic model of an X430 behind a device._  |
| struct | [**x430\_sim\_config**](structx430__sim__config.md) <br>_Simulator configuration, copied by_ [_**x430\_sim\_init()**_](group__x430.md#function-x430_sim_init) _._ |



















































------------------------------
The documentation for this class was generated from the following file `inc/x430_types.h`

