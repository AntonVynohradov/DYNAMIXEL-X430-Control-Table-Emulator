

# File x430\_bus.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**x430\_bus.c**](x430__bus_8c.md)

[Go to the source code of this file](x430__bus_8c_source.md)



* `#include "x430.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include "x430_internal.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**x430\_align\_bus**](structx430__align__bus.md) <br> |
| struct | [**x430\_align\_bus\_ctx**](structx430__align__bus__ctx.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**x430\_target**](#enum-x430_target)  <br>_Origin of an instruction for one device._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**x430\_bus\_id\_taken**](#function-x430_bus_id_taken) (const struct [**x430\_bus**](structx430__bus.md) \* bus, const struct [**x430\_device**](structx430__device.md) \* except, uint8\_t id) <br>_True when another device on_ `bus` _uses_`id` _(_`bus` _may be NULL)._ |
|  uint32\_t | [**x430\_bus\_millis**](#function-x430_bus_millis) (const struct [**x430\_bus**](structx430__bus.md) \* bus) <br>_Milliseconds from the port of_ `bus` _; 0 without bus or clock._ |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  bool | [**x430\_answers**](#function-x430_answers) (const struct [**x430\_device**](structx430__device.md) \* device, uint8\_t needed) <br>_Status Return Level rule: true when the device answers at_ `needed` _._ |
|  void | [**x430\_broadcast**](#function-x430_broadcast) (struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m, const struct dxl2\_packet \* packet) <br> |
|  void | [**x430\_broadcast\_ping**](#function-x430_broadcast_ping) (struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m, const struct dxl2\_packet \* packet) <br>_Broadcast PING: every device answers, lowest ID first._  |
|  int | [**x430\_check**](#function-x430_check) (const struct [**x430\_bus**](structx430__bus.md) \* bus) <br>_X430\_OK for an initialized bus, else the status code to return._  |
|  void | [**x430\_delay\_status**](#function-x430_delay_status) (const struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m, const struct [**x430\_device**](structx430__device.md) \* device) <br> |
|  void | [**x430\_dispatch**](#function-x430_dispatch) (struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m, const struct dxl2\_packet \* packet) <br>_Packet dispatcher, called with the bus lock held._  |
|  void | [**x430\_execute**](#function-x430_execute) (struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m, struct [**x430\_device**](structx430__device.md) \* device, const struct dxl2\_packet \* packet, enum x430\_target target) <br> |
|  bool | [**x430\_factory\_reset\_check**](#function-x430_factory_reset_check) (const struct dxl2\_packet \* packet, enum x430\_target target, uint8\_t \* error) <br>_FACTORY\_RESET parameter check; true when the reset may run after the answer._  |
|  struct [**x430\_device**](structx430__device.md) \* | [**x430\_find**](#function-x430_find) (const struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m, uint8\_t id) <br>_Attached device with primary ID_ `id` _, NULL if none._ |
|  void | [**x430\_group\_read**](#function-x430_group_read) (struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m, uint8\_t id, uint16\_t address, uint16\_t length) <br>_Read for a group instruction; sends the status of one device._  |
|  void | [**x430\_handler**](#function-x430_handler) (const struct dxl2\_packet \* packet, void \* context) <br>_Packet handler given to DXL2-C._  |
|  uint8\_t | [**x430\_id\_of**](#function-x430_id_of) (const struct [**x430\_device**](structx430__device.md) \* device) <br> |
|  uint16\_t | [**x430\_instruction**](#function-x430_instruction) (struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m, struct [**x430\_device**](structx430__device.md) \* device, const struct dxl2\_packet \* packet, enum x430\_target target, uint8\_t \* error, bool \* answer\_first) <br>_Executes an instruction for one device; returns the status data length._  |
|  void | [**x430\_lock**](#function-x430_lock) (const struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m) <br> |
|  bool | [**x430\_magic\_ok**](#function-x430_magic_ok) (const uint8\_t \* parameters, uint8\_t b1, uint8\_t b2, uint8\_t b3, uint8\_t b4) <br> |
|  uint32\_t | [**x430\_millis**](#function-x430_millis) (const struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m) <br> |
|  void | [**x430\_send\_status**](#function-x430_send_status) (struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m, const struct [**x430\_device**](structx430__device.md) \* device, uint8\_t id, uint8\_t error, uint16\_t length) <br> |
|  void | [**x430\_sync\_read**](#function-x430_sync_read) (struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m, const struct dxl2\_packet \* packet) <br> |
|  void | [**x430\_sync\_write**](#function-x430_sync_write) (struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m, const struct dxl2\_packet \* packet) <br> |
|  int | [**x430\_take\_write\_result**](#function-x430_take_write_result) (struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m) <br>_Status of an entry point that may have sent status packets._  |
|  void | [**x430\_unlock**](#function-x430_unlock) (const struct [**x430\_bus\_ctx**](structx430__bus__ctx.md) \* m) <br> |
|  int | [**x430\_writer**](#function-x430_writer) (void \* context, const uint8\_t \* data, size\_t length) <br>_Writer given to DXL2-C: calls the application writer and records a failure._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**X430\_BULK\_READ\_ENTRY**](x430__bus_8c.md#define-x430_bulk_read_entry)  `5U`<br>_One BULK\_READ entry: ID, address, length._  |
| define  | [**X430\_FAST\_HEADER**](x430__bus_8c.md#define-x430_fast_header)  `8U`<br>_Fast read: header, reserved, ID, length, instruction._  |
| define  | [**X430\_FAST\_SEGMENT\_OVERHEAD**](x430__bus_8c.md#define-x430_fast_segment_overhead)  `4U`<br>_Fast read: bytes around the data of one device (error, ID, CRC)._  |
| define  | [**X430\_LEVEL\_ALL**](x430__bus_8c.md#define-x430_level_all)  `2U`<br> |
| define  | [**X430\_LEVEL\_PING**](x430__bus_8c.md#define-x430_level_ping)  `0U`<br>_Status Return Level needed for an answer: PING always, READs from level 1, the rest at 2._  |
| define  | [**X430\_LEVEL\_READ**](x430__bus_8c.md#define-x430_level_read)  `1U`<br> |
| define  | [**X430\_OPTION\_PARAMS**](x430__bus_8c.md#define-x430_option_params)  `5U`<br>_Parameters of CLEAR and BACKUP: option and four magic bytes._  |
| define  | [**X430\_READ\_PARAMS**](x430__bus_8c.md#define-x430_read_params)  `4U`<br>_Parameters of READ and of the SYNC\_READ header: address and length._  |
| define  | [**X430\_STATUS\_DATA\_MAX**](x430__bus_8c.md#define-x430_status_data_max)  `(DXL2\_MAX\_PARAMETER\_SIZE - 1U)`<br>_Largest data length of a status packet (the error byte comes first)._  |
| define  | [**X430\_WRITE\_HEADER**](x430__bus_8c.md#define-x430_write_header)  `2U`<br>_Parameters of WRITE before the data: address._  |

## Public Types Documentation




### enum x430\_target 

_Origin of an instruction for one device._ 
```C++
enum x430_target {
    X430_TARGET_PRIMARY = 0,
    X430_TARGET_SECONDARY,
    X430_TARGET_BROADCAST
};
```




<hr>
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
## Public Static Functions Documentation




### function x430\_answers 

_Status Return Level rule: true when the device answers at_ `needed` _._
```C++
static bool x430_answers (
    const struct x430_device * device,
    uint8_t needed
) 
```




<hr>



### function x430\_broadcast 

```C++
static void x430_broadcast (
    struct x430_bus_ctx * m,
    const struct dxl2_packet * packet
) 
```




<hr>



### function x430\_broadcast\_ping 

_Broadcast PING: every device answers, lowest ID first._ 
```C++
static void x430_broadcast_ping (
    struct x430_bus_ctx * m,
    const struct dxl2_packet * packet
) 
```




<hr>



### function x430\_check 

_X430\_OK for an initialized bus, else the status code to return._ 
```C++
static int x430_check (
    const struct x430_bus * bus
) 
```




<hr>



### function x430\_delay\_status 

```C++
static void x430_delay_status (
    const struct x430_bus_ctx * m,
    const struct x430_device * device
) 
```




<hr>



### function x430\_dispatch 

_Packet dispatcher, called with the bus lock held._ 
```C++
static void x430_dispatch (
    struct x430_bus_ctx * m,
    const struct dxl2_packet * packet
) 
```




<hr>



### function x430\_execute 

```C++
static void x430_execute (
    struct x430_bus_ctx * m,
    struct x430_device * device,
    const struct dxl2_packet * packet,
    enum x430_target target
) 
```




<hr>



### function x430\_factory\_reset\_check 

_FACTORY\_RESET parameter check; true when the reset may run after the answer._ 
```C++
static bool x430_factory_reset_check (
    const struct dxl2_packet * packet,
    enum x430_target target,
    uint8_t * error
) 
```




<hr>



### function x430\_find 

_Attached device with primary ID_ `id` _, NULL if none._
```C++
static struct x430_device * x430_find (
    const struct x430_bus_ctx * m,
    uint8_t id
) 
```




<hr>



### function x430\_group\_read 

_Read for a group instruction; sends the status of one device._ 
```C++
static void x430_group_read (
    struct x430_bus_ctx * m,
    uint8_t id,
    uint16_t address,
    uint16_t length
) 
```




<hr>



### function x430\_handler 

_Packet handler given to DXL2-C._ 
```C++
static void x430_handler (
    const struct dxl2_packet * packet,
    void * context
) 
```




<hr>



### function x430\_id\_of 

```C++
static uint8_t x430_id_of (
    const struct x430_device * device
) 
```




<hr>



### function x430\_instruction 

_Executes an instruction for one device; returns the status data length._ 
```C++
static uint16_t x430_instruction (
    struct x430_bus_ctx * m,
    struct x430_device * device,
    const struct dxl2_packet * packet,
    enum x430_target target,
    uint8_t * error,
    bool * answer_first
) 
```




<hr>



### function x430\_lock 

```C++
static void x430_lock (
    const struct x430_bus_ctx * m
) 
```




<hr>



### function x430\_magic\_ok 

```C++
static bool x430_magic_ok (
    const uint8_t * parameters,
    uint8_t b1,
    uint8_t b2,
    uint8_t b3,
    uint8_t b4
) 
```




<hr>



### function x430\_millis 

```C++
static uint32_t x430_millis (
    const struct x430_bus_ctx * m
) 
```




<hr>



### function x430\_send\_status 

```C++
static void x430_send_status (
    struct x430_bus_ctx * m,
    const struct x430_device * device,
    uint8_t id,
    uint8_t error,
    uint16_t length
) 
```




<hr>



### function x430\_sync\_read 

```C++
static void x430_sync_read (
    struct x430_bus_ctx * m,
    const struct dxl2_packet * packet
) 
```




<hr>



### function x430\_sync\_write 

```C++
static void x430_sync_write (
    struct x430_bus_ctx * m,
    const struct dxl2_packet * packet
) 
```




<hr>



### function x430\_take\_write\_result 

_Status of an entry point that may have sent status packets._ 
```C++
static int x430_take_write_result (
    struct x430_bus_ctx * m
) 
```




<hr>



### function x430\_unlock 

```C++
static void x430_unlock (
    const struct x430_bus_ctx * m
) 
```




<hr>



### function x430\_writer 

_Writer given to DXL2-C: calls the application writer and records a failure._ 
```C++
static int x430_writer (
    void * context,
    const uint8_t * data,
    size_t length
) 
```




<hr>
## Macro Definition Documentation





### define X430\_BULK\_READ\_ENTRY 

_One BULK\_READ entry: ID, address, length._ 
```C++
#define X430_BULK_READ_ENTRY `5U`
```




<hr>



### define X430\_FAST\_HEADER 

_Fast read: header, reserved, ID, length, instruction._ 
```C++
#define X430_FAST_HEADER `8U`
```




<hr>



### define X430\_FAST\_SEGMENT\_OVERHEAD 

_Fast read: bytes around the data of one device (error, ID, CRC)._ 
```C++
#define X430_FAST_SEGMENT_OVERHEAD `4U`
```




<hr>



### define X430\_LEVEL\_ALL 

```C++
#define X430_LEVEL_ALL `2U`
```




<hr>



### define X430\_LEVEL\_PING 

_Status Return Level needed for an answer: PING always, READs from level 1, the rest at 2._ 
```C++
#define X430_LEVEL_PING `0U`
```




<hr>



### define X430\_LEVEL\_READ 

```C++
#define X430_LEVEL_READ `1U`
```




<hr>



### define X430\_OPTION\_PARAMS 

_Parameters of CLEAR and BACKUP: option and four magic bytes._ 
```C++
#define X430_OPTION_PARAMS `5U`
```




<hr>



### define X430\_READ\_PARAMS 

_Parameters of READ and of the SYNC\_READ header: address and length._ 
```C++
#define X430_READ_PARAMS `4U`
```




<hr>



### define X430\_STATUS\_DATA\_MAX 

_Largest data length of a status packet (the error byte comes first)._ 
```C++
#define X430_STATUS_DATA_MAX `(DXL2_MAX_PARAMETER_SIZE - 1U)`
```




<hr>



### define X430\_WRITE\_HEADER 

_Parameters of WRITE before the data: address._ 
```C++
#define X430_WRITE_HEADER `2U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/x430_bus.c`

