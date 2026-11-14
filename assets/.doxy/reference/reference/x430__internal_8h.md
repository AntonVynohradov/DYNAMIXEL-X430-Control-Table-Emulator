

# File x430\_internal.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**x430\_internal.h**](x430__internal_8h.md)

[Go to the source code of this file](x430__internal_8h_source.md)



* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include "x430.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**x430\_bus\_ctx**](structx430__bus__ctx.md) <br>_State behind the opaque_ [_**x430\_bus**_](structx430__bus.md) _._ |
| struct | [**x430\_device\_ctx**](structx430__device__ctx.md) <br>_State behind the opaque_ [_**x430\_device**_](structx430__device.md) _._ |
| struct | [**x430\_sim\_ctx**](structx430__sim__ctx.md) <br>_State behind the opaque_ [_**x430\_sim**_](structx430__sim.md) _._ |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**x430\_bus\_id\_taken**](#function-x430_bus_id_taken) (const struct [**x430\_bus**](structx430__bus.md) \* bus, const struct [**x430\_device**](structx430__device.md) \* except, uint8\_t id) <br>_True when another device on_ `bus` _uses_`id` _(_`bus` _may be NULL)._ |
|  uint32\_t | [**x430\_bus\_millis**](#function-x430_bus_millis) (const struct [**x430\_bus**](structx430__bus.md) \* bus) <br>_Milliseconds from the port of_ `bus` _; 0 without bus or clock._ |
|  void | [**x430\_copy**](#function-x430_copy) (uint8\_t \* destination, const uint8\_t \* source, uint16\_t length) <br>_Copies_ `length` _bytes (no &lt;string.h&gt;)._ |
|  uint8\_t | [**x430\_device\_alert**](#function-x430_device_alert) (const struct [**x430\_device**](structx430__device.md) \* device) <br>_Status error alert: DXL2\_ERR\_ALERT\_BIT when Hardware Error Status is not 0._  |
|  void | [**x430\_device\_attach**](#function-x430_device_attach) (struct [**x430\_device**](structx430__device.md) \* device, struct [**x430\_bus**](structx430__bus.md) \* bus) <br>_Records the bus of a device on_ [_**x430\_bus\_attach()**_](group__x430.md#function-x430_bus_attach) _._ |
|  struct [**x430\_bus**](structx430__bus.md) \* | [**x430\_device\_bus**](#function-x430_device_bus) (const struct [**x430\_device**](structx430__device.md) \* device) <br>_Bus of the device, NULL when it is not attached._  |
|  uint8\_t | [**x430\_device\_byte**](#function-x430_device_byte) (const struct [**x430\_device**](structx430__device.md) \* device, uint16\_t address) <br>_Control Table byte_ `address` _, below ::X430\_IMAGE\_DIRECT (stored at its own index)._ |
|  bool | [**x430\_device\_ready**](#function-x430_device_ready) (const struct [**x430\_device**](structx430__device.md) \* device) <br>_True for a device that completed_ [_**x430\_device\_init()**_](group__x430.md#function-x430_device_init) _(device may be NULL)._ |
|  void | [**x430\_device\_store**](#function-x430_device_store) (struct [**x430\_device**](structx430__device.md) \* device, uint16\_t address, int32\_t value) <br>_Store_ `value` _into the register that starts at_`address` _(none: no effect)._ |
|  int32\_t | [**x430\_device\_value**](#function-x430_device_value) (const struct [**x430\_device**](structx430__device.md) \* device, uint16\_t address) <br>_Value of the register that starts at_ `address` _, 0 when none starts there._ |
|  void | [**x430\_device\_watchdog**](#function-x430_device_watchdog) (struct [**x430\_device**](structx430__device.md) \* device, uint32\_t idle\_ms) <br>_Bus Watchdog check;_ `idle_ms` _is the time since the last instruction packet._ |
|  void | [**x430\_fill**](#function-x430_fill) (uint8\_t \* destination, uint8\_t value, uint16\_t length) <br>_Sets_ `length` _bytes to_`value` _(no &lt;string.h&gt;)._ |
|  size\_t | [**x430\_frame\_status**](#function-x430_frame_status) (uint8\_t \* tx, size\_t capacity, uint8\_t id, uint16\_t count) <br>_Completes a status packet in place (no second buffer): header,_ `id` _, the byte stuffing of instruction, error and data, the length field and the CRC._ |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* | [**x430\_bus\_ctx\_of**](#function-x430_bus_ctx_of) (struct [**x430\_bus**](structx430__bus.md) \* bus) <br>_State of a bus._  |
|  const struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* | [**x430\_bus\_ctx\_of\_const**](#function-x430_bus_ctx_of_const) (const struct [**x430\_bus**](structx430__bus.md) \* bus) <br>_Read-only variant of x430\_bus\_ctx\_of()._  |
|  struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* | [**x430\_device\_ctx\_of**](#function-x430_device_ctx_of) (struct [**x430\_device**](structx430__device.md) \* device) <br>_State of a device._  |
|  const struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* | [**x430\_device\_ctx\_of\_const**](#function-x430_device_ctx_of_const) (const struct [**x430\_device**](structx430__device.md) \* device) <br>_Read-only variant of x430\_device\_ctx\_of()._  |
|  struct [**x430\_sim\_ctx**](structx430__sim__ctx.md) \* | [**x430\_sim\_ctx\_of**](#function-x430_sim_ctx_of) (struct [**x430\_sim**](structx430__sim.md) \* sim) <br>_State of a simulator._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**X430\_READY**](x430__internal_8h.md#define-x430_ready)  `0x58343330U`<br>_Value of the ready field after a successful initialization._  |

## Public Functions Documentation




### function x430\_bus\_id\_taken 

_True when another device on_ `bus` _uses_`id` _(_`bus` _may be NULL)._
```C++
bool x430_bus_id_taken (
    const struct x430_bus * bus,
    const struct x430_device * except,
    uint8_t id
) 
```




<hr>



### function x430\_bus\_millis 

_Milliseconds from the port of_ `bus` _; 0 without bus or clock._
```C++
uint32_t x430_bus_millis (
    const struct x430_bus * bus
) 
```




<hr>



### function x430\_copy 

_Copies_ `length` _bytes (no &lt;string.h&gt;)._
```C++
void x430_copy (
    uint8_t * destination,
    const uint8_t * source,
    uint16_t length
) 
```




<hr>



### function x430\_device\_alert 

_Status error alert: DXL2\_ERR\_ALERT\_BIT when Hardware Error Status is not 0._ 
```C++
uint8_t x430_device_alert (
    const struct x430_device * device
) 
```




<hr>



### function x430\_device\_attach 

_Records the bus of a device on_ [_**x430\_bus\_attach()**_](group__x430.md#function-x430_bus_attach) _._
```C++
void x430_device_attach (
    struct x430_device * device,
    struct x430_bus * bus
) 
```




<hr>



### function x430\_device\_bus 

_Bus of the device, NULL when it is not attached._ 
```C++
struct x430_bus * x430_device_bus (
    const struct x430_device * device
) 
```




<hr>



### function x430\_device\_byte 

_Control Table byte_ `address` _, below ::X430\_IMAGE\_DIRECT (stored at its own index)._
```C++
uint8_t x430_device_byte (
    const struct x430_device * device,
    uint16_t address
) 
```




<hr>



### function x430\_device\_ready 

_True for a device that completed_ [_**x430\_device\_init()**_](group__x430.md#function-x430_device_init) _(device may be NULL)._
```C++
bool x430_device_ready (
    const struct x430_device * device
) 
```




<hr>



### function x430\_device\_store 

_Store_ `value` _into the register that starts at_`address` _(none: no effect)._
```C++
void x430_device_store (
    struct x430_device * device,
    uint16_t address,
    int32_t value
) 
```




<hr>



### function x430\_device\_value 

_Value of the register that starts at_ `address` _, 0 when none starts there._
```C++
int32_t x430_device_value (
    const struct x430_device * device,
    uint16_t address
) 
```




<hr>



### function x430\_device\_watchdog 

_Bus Watchdog check;_ `idle_ms` _is the time since the last instruction packet._
```C++
void x430_device_watchdog (
    struct x430_device * device,
    uint32_t idle_ms
) 
```




<hr>



### function x430\_fill 

_Sets_ `length` _bytes to_`value` _(no &lt;string.h&gt;)._
```C++
void x430_fill (
    uint8_t * destination,
    uint8_t value,
    uint16_t length
) 
```




<hr>



### function x430\_frame\_status 

_Completes a status packet in place (no second buffer): header,_ `id` _, the byte stuffing of instruction, error and data, the length field and the CRC._
```C++
size_t x430_frame_status (
    uint8_t * tx,
    size_t capacity,
    uint8_t id,
    uint16_t count
) 
```



The result is byte for byte the packet of dxl2\_encode(). A status whose stuffed form does not fit `capacity` becomes a Data Length Error without data (the alert bit is kept). 

**Parameters:**


* `tx` Packet buffer: error byte at offset 8, `count` data bytes from 9. 
* `capacity` Size of `tx`, at least 9 + `count` and at least 11 bytes. 
* `id` ID of the answering device. 
* `count` Status data bytes. 



**Returns:**

Packet length (at least 11). 





        

<hr>
## Public Static Functions Documentation




### function x430\_bus\_ctx\_of 

_State of a bus._ 
```C++
static inline struct x430_bus_ctx * x430_bus_ctx_of (
    struct x430_bus * bus
) 
```





**Note:**

MISRA C:2012 Rule 11.3, deviation DEV-01 (docs/misra.md): [**x430\_bus**](structx430__bus.md) is the opaque storage of [**x430\_bus\_ctx**](structx430__bus__ctx.md); size and alignment are checked in x430\_bus.c. 





        

<hr>



### function x430\_bus\_ctx\_of\_const 

_Read-only variant of x430\_bus\_ctx\_of()._ 
```C++
static inline const struct x430_bus_ctx * x430_bus_ctx_of_const (
    const struct x430_bus * bus
) 
```




<hr>



### function x430\_device\_ctx\_of 

_State of a device._ 
```C++
static inline struct x430_device_ctx * x430_device_ctx_of (
    struct x430_device * device
) 
```





**Note:**

MISRA C:2012 Rule 11.3, deviation DEV-01 (docs/misra.md); checked in x430\_device.c. 





        

<hr>



### function x430\_device\_ctx\_of\_const 

_Read-only variant of x430\_device\_ctx\_of()._ 
```C++
static inline const struct x430_device_ctx * x430_device_ctx_of_const (
    const struct x430_device * device
) 
```




<hr>



### function x430\_sim\_ctx\_of 

_State of a simulator._ 
```C++
static inline struct x430_sim_ctx * x430_sim_ctx_of (
    struct x430_sim * sim
) 
```





**Note:**

MISRA C:2012 Rule 11.3, deviation DEV-01 (docs/misra.md); checked in x430\_sim.c. 





        

<hr>
## Macro Definition Documentation





### define X430\_READY 

_Value of the ready field after a successful initialization._ 
```C++
#define X430_READY `0x58343330U`
```



A zeroed or never initialized instance holds another value: every function but the \*\_init() functions then returns [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/x430_internal.h`

