

# File x430\_device.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**x430\_device.c**](x430__device_8c.md)

[Go to the source code of this file](x430__device_8c_source.md)



* `#include "x430.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include "x430_internal.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**x430\_align\_device**](structx430__align__device.md) <br> |
| struct | [**x430\_align\_device\_ctx**](structx430__align__device__ctx.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**x430\_copy**](#function-x430_copy) (uint8\_t \* destination, const uint8\_t \* source, uint16\_t length) <br>_Copies_ `length` _bytes (no &lt;string.h&gt;)._ |
|  uint8\_t | [**x430\_device\_alert**](#function-x430_device_alert) (const struct [**x430\_device**](structx430__device.md) \* device) <br>_Status error alert: DXL2\_ERR\_ALERT\_BIT when Hardware Error Status is not 0._  |
|  void | [**x430\_device\_attach**](#function-x430_device_attach) (struct [**x430\_device**](structx430__device.md) \* device, struct [**x430\_bus**](structx430__bus.md) \* bus) <br>_Records the bus of a device on_ [_**x430\_bus\_attach()**_](group__x430.md#function-x430_bus_attach) _._ |
|  struct [**x430\_bus**](structx430__bus.md) \* | [**x430\_device\_bus**](#function-x430_device_bus) (const struct [**x430\_device**](structx430__device.md) \* device) <br>_Bus of the device, NULL when it is not attached._  |
|  uint8\_t | [**x430\_device\_byte**](#function-x430_device_byte) (const struct [**x430\_device**](structx430__device.md) \* device, uint16\_t address) <br>_One byte of the Control Table image._  |
|  bool | [**x430\_device\_ready**](#function-x430_device_ready) (const struct [**x430\_device**](structx430__device.md) \* device) <br>_True for a device that completed_ [_**x430\_device\_init()**_](group__x430.md#function-x430_device_init) _(device may be NULL)._ |
|  void | [**x430\_device\_store**](#function-x430_device_store) (struct [**x430\_device**](structx430__device.md) \* device, uint16\_t address, int32\_t value) <br>_Store_ `value` _into the register that starts at_`address` _(none: no effect)._ |
|  int32\_t | [**x430\_device\_value**](#function-x430_device_value) (const struct [**x430\_device**](structx430__device.md) \* device, uint16\_t address) <br>_Value of the register that starts at_ `address` _, 0 when none starts there._ |
|  void | [**x430\_device\_watchdog**](#function-x430_device_watchdog) (struct [**x430\_device**](structx430__device.md) \* device, uint32\_t idle\_ms) <br>_Bus Watchdog check;_ `idle_ms` _is the time since the last instruction packet._ |
|  void | [**x430\_fill**](#function-x430_fill) (uint8\_t \* destination, uint8\_t value, uint16\_t length) <br>_Sets_ `length` _bytes to_`value` _(no &lt;string.h&gt;)._ |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int32\_t | [**x430\_abs**](#function-x430_abs) (int32\_t value) <br> |
|  void | [**x430\_apply\_eeprom**](#function-x430_apply_eeprom) (struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* m) <br>_Copies the writable EEPROM registers of a valid EEPROM block (nvm buffer) into the table._  |
|  int | [**x430\_check**](#function-x430_check) (const struct [**x430\_device**](structx430__device.md) \* device) <br>_X430\_OK for an initialized device, else the status code to return._  |
|  int | [**x430\_check2**](#function-x430_check2) (const struct [**x430\_device**](structx430__device.md) \* device, const void \* a, const void \* b) <br>_Like_ [_**x430\_check()**_](group__x430__model.md#enum-x430_check) _, and_`a` _and_`b` _must not be NULL._ |
|  bool | [**x430\_check\_value**](#function-x430_check_value) (const struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* m, const struct [**x430\_reg**](structx430__reg.md) \* reg, int32\_t value) <br> |
|  uint8\_t | [**x430\_check\_write**](#function-x430_check_write) (const struct [**x430\_device**](structx430__device.md) \* device, const struct [**x430\_reg**](structx430__reg.md) \* reg, int32\_t value, uint8\_t options) <br>_Error code for writing_ `value` _to_`reg` _, DXL2\_ERR\_NONE when allowed._ |
|  uint8\_t | [**x430\_enable\_torque**](#function-x430_enable_torque) (struct [**x430\_device**](structx430__device.md) \* device, bool sync\_goal) <br>_Torque Enable = 1 through the write hook (startup, Drive Mode bit 3)._  |
|  uint8\_t | [**x430\_event**](#function-x430_event) (struct [**x430\_device**](structx430__device.md) \* device, enum [**x430\_event**](group__x430__defs.md#enum-x430_event) event) <br> |
|  int32\_t | [**x430\_get**](#function-x430_get) (const struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* m, uint16\_t address) <br> |
|  bool | [**x430\_is\_touched**](#function-x430_is_touched) (const uint8\_t \* touched, uint16\_t index) <br>_True when register_ `index` _is marked in_`touched` _._ |
|  void | [**x430\_load\_factory**](#function-x430_load_factory) (struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* m, uint16\_t first, uint16\_t last) <br> |
|  void | [**x430\_load\_identity**](#function-x430_load_identity) (struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* m) <br> |
|  void | [**x430\_mark\_touched**](#function-x430_mark_touched) (const struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* m, uint16\_t address, uint16\_t length, uint8\_t \* touched, bool indirect) <br>_Marks every register touched by the write (bit = register index)._  |
|  uint32\_t | [**x430\_merge**](#function-x430_merge) (const struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* m, const struct [**x430\_reg**](structx430__reg.md) \* reg, uint16\_t address, uint16\_t length, const uint8\_t \* data, bool indirect) <br>_New raw value of_ `reg` _after writing_`data` _at_`address:` _the bytes of the write that land in the register replace the current bytes._ |
|  bool | [**x430\_nvm\_load**](#function-x430_nvm_load) (struct [**x430\_device**](structx430__device.md) \* device, enum [**x430\_nvm\_slot**](group__x430__defs.md#enum-x430_nvm_slot) slot, uint16\_t size) <br>_Loads a block into the nvm buffer; true when it is valid for this model._  |
|  uint16\_t | [**x430\_nvm\_pack**](#function-x430_nvm_pack) (struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* m, enum [**x430\_nvm\_slot**](group__x430__defs.md#enum-x430_nvm_slot) slot, uint16\_t size) <br>_Builds a block from_ `size` _bytes of the table in the nvm buffer; returns its size._ |
|  uint8\_t | [**x430\_nvm\_store**](#function-x430_nvm_store) (struct [**x430\_device**](structx430__device.md) \* device, enum [**x430\_nvm\_slot**](group__x430__defs.md#enum-x430_nvm_slot) slot, uint16\_t size) <br> |
|  void | [**x430\_put**](#function-x430_put) (struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* m, uint16\_t address, int32\_t value) <br> |
|  uint32\_t | [**x430\_raw\_get**](#function-x430_raw_get) (const uint8\_t \* table, uint16\_t address, uint8\_t size) <br> |
|  void | [**x430\_raw\_put**](#function-x430_raw_put) (uint8\_t \* table, uint16\_t address, uint8\_t size, uint32\_t value) <br> |
|  const struct [**x430\_reg**](structx430__reg.md) \* | [**x430\_reg\_at**](#function-x430_reg_at) (uint16\_t address) <br>_Register that contains_ `address` _, NULL for a reserved address._ |
|  const struct [**x430\_reg**](structx430__reg.md) \* | [**x430\_reg\_start**](#function-x430_reg_start) (uint16\_t address) <br>_Register that starts at_ `address` _, NULL when none does._ |
|  uint16\_t | [**x430\_resolve**](#function-x430_resolve) (const struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* m, uint16\_t address) <br>_Address that_ `address` _really accesses (Indirect Data resolved)._ |
|  void | [**x430\_startup**](#function-x430_startup) (struct [**x430\_device**](structx430__device.md) \* device) <br>_RAM area to its startup state, then the Startup Configuration._  |
|  void | [**x430\_store\_raw\_bytes**](#function-x430_store_raw_bytes) (struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* m, uint16\_t address, uint16\_t length, const uint8\_t \* data) <br>_Bytes of a write through Indirect Data whose Indirect Address points at Indirect Data storage itself: they are kept as plain bytes._  |
|  void | [**x430\_sync\_goal**](#function-x430_sync_goal) (struct [**x430\_device**](structx430__device.md) \* device) <br>_Torque on: Goal Position takes the Present Position, so the actuator holds where it is instead of jumping to an old goal (behaviour of the X series)._  |
|  int32\_t | [**x430\_to\_value**](#function-x430_to_value) (const struct [**x430\_reg**](structx430__reg.md) \* reg, uint32\_t raw) <br>_Sign-extends a raw register value._  |
|  bool | [**x430\_torque\_on**](#function-x430_torque_on) (const struct [**x430\_device\_ctx**](structx430__device__ctx.md) \* m) <br> |
|  bool | [**x430\_touches\_indirect**](#function-x430_touches_indirect) (uint16\_t address, uint16\_t length) <br>_True when [address, address + length) overlaps an Indirect Data block._  |
|  uint8\_t | [**x430\_write**](#function-x430_write) (struct [**x430\_device**](structx430__device.md) \* device, uint16\_t address, uint16\_t length, const uint8\_t \* data, uint8\_t options) <br>_WRITE without the argument checks (_ [_**x430\_device\_write()**_](group__x430.md#function-x430_device_write) _, REG\_WRITE, ACTION)._ |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**X430\_ID\_MAX**](x430__device_8c.md#define-x430_id_max)  `252U`<br>_Highest DYNAMIXEL ID a device may have (253 is reserved, 254 is broadcast)._  |
| define  | [**X430\_NVM\_FORMAT**](x430__device_8c.md#define-x430_nvm_format)  `1U`<br>_Persistent block format version._  |
| define  | [**X430\_PULSES\_PER\_TURN**](x430__device_8c.md#define-x430_pulses_per_turn)  `4096`<br>_Turn size of Present Position in pulses._  |
| define  | [**X430\_TOUCHED\_BYTES**](x430__device_8c.md#define-x430_touched_bytes)  `32U`<br>_Bytes of the register-touched bit set (one bit per register index, max 255)._  |

## Public Functions Documentation




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

_One byte of the Control Table image._ 
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
## Public Static Functions Documentation




### function x430\_abs 

```C++
static int32_t x430_abs (
    int32_t value
) 
```




<hr>



### function x430\_apply\_eeprom 

_Copies the writable EEPROM registers of a valid EEPROM block (nvm buffer) into the table._ 
```C++
static void x430_apply_eeprom (
    struct x430_device_ctx * m
) 
```




<hr>



### function x430\_check 

_X430\_OK for an initialized device, else the status code to return._ 
```C++
static int x430_check (
    const struct x430_device * device
) 
```




<hr>



### function x430\_check2 

_Like_ [_**x430\_check()**_](group__x430__model.md#enum-x430_check) _, and_`a` _and_`b` _must not be NULL._
```C++
static int x430_check2 (
    const struct x430_device * device,
    const void * a,
    const void * b
) 
```




<hr>



### function x430\_check\_value 

```C++
static bool x430_check_value (
    const struct x430_device_ctx * m,
    const struct x430_reg * reg,
    int32_t value
) 
```




<hr>



### function x430\_check\_write 

_Error code for writing_ `value` _to_`reg` _, DXL2\_ERR\_NONE when allowed._
```C++
static uint8_t x430_check_write (
    const struct x430_device * device,
    const struct x430_reg * reg,
    int32_t value,
    uint8_t options
) 
```




<hr>



### function x430\_enable\_torque 

_Torque Enable = 1 through the write hook (startup, Drive Mode bit 3)._ 
```C++
static uint8_t x430_enable_torque (
    struct x430_device * device,
    bool sync_goal
) 
```




<hr>



### function x430\_event 

```C++
static uint8_t x430_event (
    struct x430_device * device,
    enum x430_event event
) 
```




<hr>



### function x430\_get 

```C++
static int32_t x430_get (
    const struct x430_device_ctx * m,
    uint16_t address
) 
```




<hr>



### function x430\_is\_touched 

_True when register_ `index` _is marked in_`touched` _._
```C++
static bool x430_is_touched (
    const uint8_t * touched,
    uint16_t index
) 
```




<hr>



### function x430\_load\_factory 

```C++
static void x430_load_factory (
    struct x430_device_ctx * m,
    uint16_t first,
    uint16_t last
) 
```




<hr>



### function x430\_load\_identity 

```C++
static void x430_load_identity (
    struct x430_device_ctx * m
) 
```




<hr>



### function x430\_mark\_touched 

_Marks every register touched by the write (bit = register index)._ 
```C++
static void x430_mark_touched (
    const struct x430_device_ctx * m,
    uint16_t address,
    uint16_t length,
    uint8_t * touched,
    bool indirect
) 
```




<hr>



### function x430\_merge 

_New raw value of_ `reg` _after writing_`data` _at_`address:` _the bytes of the write that land in the register replace the current bytes._
```C++
static uint32_t x430_merge (
    const struct x430_device_ctx * m,
    const struct x430_reg * reg,
    uint16_t address,
    uint16_t length,
    const uint8_t * data,
    bool indirect
) 
```




<hr>



### function x430\_nvm\_load 

_Loads a block into the nvm buffer; true when it is valid for this model._ 
```C++
static bool x430_nvm_load (
    struct x430_device * device,
    enum x430_nvm_slot slot,
    uint16_t size
) 
```




<hr>



### function x430\_nvm\_pack 

_Builds a block from_ `size` _bytes of the table in the nvm buffer; returns its size._
```C++
static uint16_t x430_nvm_pack (
    struct x430_device_ctx * m,
    enum x430_nvm_slot slot,
    uint16_t size
) 
```




<hr>



### function x430\_nvm\_store 

```C++
static uint8_t x430_nvm_store (
    struct x430_device * device,
    enum x430_nvm_slot slot,
    uint16_t size
) 
```




<hr>



### function x430\_put 

```C++
static void x430_put (
    struct x430_device_ctx * m,
    uint16_t address,
    int32_t value
) 
```




<hr>



### function x430\_raw\_get 

```C++
static uint32_t x430_raw_get (
    const uint8_t * table,
    uint16_t address,
    uint8_t size
) 
```




<hr>



### function x430\_raw\_put 

```C++
static void x430_raw_put (
    uint8_t * table,
    uint16_t address,
    uint8_t size,
    uint32_t value
) 
```




<hr>



### function x430\_reg\_at 

_Register that contains_ `address` _, NULL for a reserved address._
```C++
static const struct x430_reg * x430_reg_at (
    uint16_t address
) 
```




<hr>



### function x430\_reg\_start 

_Register that starts at_ `address` _, NULL when none does._
```C++
static const struct x430_reg * x430_reg_start (
    uint16_t address
) 
```




<hr>



### function x430\_resolve 

_Address that_ `address` _really accesses (Indirect Data resolved)._
```C++
static uint16_t x430_resolve (
    const struct x430_device_ctx * m,
    uint16_t address
) 
```




<hr>



### function x430\_startup 

_RAM area to its startup state, then the Startup Configuration._ 
```C++
static void x430_startup (
    struct x430_device * device
) 
```




<hr>



### function x430\_store\_raw\_bytes 

_Bytes of a write through Indirect Data whose Indirect Address points at Indirect Data storage itself: they are kept as plain bytes._ 
```C++
static void x430_store_raw_bytes (
    struct x430_device_ctx * m,
    uint16_t address,
    uint16_t length,
    const uint8_t * data
) 
```




<hr>



### function x430\_sync\_goal 

_Torque on: Goal Position takes the Present Position, so the actuator holds where it is instead of jumping to an old goal (behaviour of the X series)._ 
```C++
static void x430_sync_goal (
    struct x430_device * device
) 
```




<hr>



### function x430\_to\_value 

_Sign-extends a raw register value._ 
```C++
static int32_t x430_to_value (
    const struct x430_reg * reg,
    uint32_t raw
) 
```




<hr>



### function x430\_torque\_on 

```C++
static bool x430_torque_on (
    const struct x430_device_ctx * m
) 
```




<hr>



### function x430\_touches\_indirect 

_True when [address, address + length) overlaps an Indirect Data block._ 
```C++
static bool x430_touches_indirect (
    uint16_t address,
    uint16_t length
) 
```




<hr>



### function x430\_write 

_WRITE without the argument checks (_ [_**x430\_device\_write()**_](group__x430.md#function-x430_device_write) _, REG\_WRITE, ACTION)._
```C++
static uint8_t x430_write (
    struct x430_device * device,
    uint16_t address,
    uint16_t length,
    const uint8_t * data,
    uint8_t options
) 
```




<hr>
## Macro Definition Documentation





### define X430\_ID\_MAX 

_Highest DYNAMIXEL ID a device may have (253 is reserved, 254 is broadcast)._ 
```C++
#define X430_ID_MAX `252U`
```




<hr>



### define X430\_NVM\_FORMAT 

_Persistent block format version._ 
```C++
#define X430_NVM_FORMAT `1U`
```




<hr>



### define X430\_PULSES\_PER\_TURN 

_Turn size of Present Position in pulses._ 
```C++
#define X430_PULSES_PER_TURN `4096`
```




<hr>



### define X430\_TOUCHED\_BYTES 

_Bytes of the register-touched bit set (one bit per register index, max 255)._ 
```C++
#define X430_TOUCHED_BYTES `32U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/x430_device.c`

