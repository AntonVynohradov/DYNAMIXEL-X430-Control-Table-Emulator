

# Group x430\_types



[**Modules**](modules.md) **>** [**x430\_types**](group__x430__types.md)



_Callbacks, configuration structures and the opaque instances._ 
















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


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**x430\_delay\_fn**](#typedef-x430_delay_fn)  <br>_Busy or sleeping delay (_ [_**x430\_port::delay\_us**_](structx430__port.md#variable-delay_us) _)._ |
| typedef uint8\_t(\* | [**x430\_event\_hook**](#typedef-x430_event_hook)  <br>_Device event (_ [_**x430\_hooks::event**_](structx430__hooks.md#variable-event) _)._ |
| typedef void(\* | [**x430\_lock\_fn**](#typedef-x430_lock_fn)  <br>_Take or release the bus lock (_ [_**x430\_port::lock**_](structx430__port.md#variable-lock) _,_[_**x430\_port::unlock**_](structx430__port.md#variable-unlock) _)._ |
| typedef uint32\_t(\* | [**x430\_millis\_fn**](#typedef-x430_millis_fn)  <br>_Free-running millisecond counter (_ [_**x430\_port::millis**_](structx430__port.md#variable-millis) _)._ |
| typedef int(\* | [**x430\_nvm\_load\_hook**](#typedef-x430_nvm_load_hook)  <br>_Load a persistent block (_ [_**x430\_hooks::nvm\_load**_](structx430__hooks.md#variable-nvm_load) _)._ |
| typedef int(\* | [**x430\_nvm\_store\_hook**](#typedef-x430_nvm_store_hook)  <br>_Store a persistent block (_ [_**x430\_hooks::nvm\_store**_](structx430__hooks.md#variable-nvm_store) _), after the change; may be slow._ |
| typedef void(\* | [**x430\_read\_hook**](#typedef-x430_read_hook)  <br>_Data is about to be read (_ [_**x430\_hooks::read**_](structx430__hooks.md#variable-read) _)._ |
| typedef uint8\_t(\* | [**x430\_write\_hook**](#typedef-x430_write_hook)  <br>_A validated register write, before the value is stored (_ [_**x430\_hooks::write**_](structx430__hooks.md#variable-write) _)._ |















































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**X430\_ALIGN\_UP**](group__x430__types.md#define-x430_align_up) (n, a) `(((((n) + (a)) - 1U) / (a)) \* (a))`<br>`n` _rounded up to a multiple of_`a` _._ |
| define  | [**X430\_BUS\_BYTES**](group__x430__types.md#define-x430_bus_bytes)  `/* multi line expression */`<br>_Private state of a bus in bytes; x430\_bus.c checks it at compile time._  |
| define  | [**X430\_BUS\_WORDS**](group__x430__types.md#define-x430_bus_words)  `(([**X430\_BUS\_BYTES**](group__x430__types.md#define-x430_bus_bytes) + 7U) / 8U)`<br>_Size of_ [_**x430\_bus**_](structx430__bus.md) _in 64-bit words._ |
| define  | [**X430\_DEVICE\_BYTES**](group__x430__types.md#define-x430_device_bytes)  `/* multi line expression */`<br>_Private state of a device in bytes; x430\_device.c checks it at compile time._  |
| define  | [**X430\_DEVICE\_WORDS**](group__x430__types.md#define-x430_device_words)  `(([**X430\_DEVICE\_BYTES**](group__x430__types.md#define-x430_device_bytes) + 7U) / 8U)`<br>_Size of_ [_**x430\_device**_](structx430__device.md) _in 64-bit words._ |
| define  | [**X430\_DXL2\_LINK\_BYTES**](group__x430__types.md#define-x430_dxl2_link_bytes)  `/* multi line expression */`<br>_Bytes of struct dxl2\_link of DXL2-C 1.0.0 (configuration, parser, packet, counters), from DXL2\_MAX\_PACKET\_SIZE._  |
| define  | [**X430\_FAST\_READ\_BYTES**](group__x430__types.md#define-x430_fast_read_bytes)  `[**DXL2\_MAX\_PACKET\_SIZE**](group__x430__config.md#define-dxl2_max_packet_size)`<br>_Bytes of the Fast Sync/Bulk Read response buffer of a bus._  |
| define  | [**X430\_NVM\_BACKUP\_SIZE**](group__x430__types.md#define-x430_nvm_backup_size)  `([**X430\_NVM\_HEADER\_SIZE**](group__x430__defs.md#define-x430_nvm_header_size) + X430\_TABLE\_SIZE + 2U)`<br>_Size of the persistent Control Table backup block._  |
| define  | [**X430\_NVM\_BUFFER\_SIZE**](group__x430__types.md#define-x430_nvm_buffer_size)  `[**X430\_NVM\_BACKUP\_SIZE**](group__x430__types.md#define-x430_nvm_backup_size)`<br>_Bytes of the persistent block buffer inside each device._  |
| define  | [**X430\_NVM\_EEPROM\_SIZE**](group__x430__types.md#define-x430_nvm_eeprom_size)  `([**X430\_NVM\_HEADER\_SIZE**](group__x430__defs.md#define-x430_nvm_header_size) + [**X430\_RAM\_START**](group__x430__defs.md#define-x430_ram_start) + 2U)`<br>_Size of the persistent EEPROM block (nvm\_load / nvm\_store)._  |
| define  | [**X430\_POINTER\_BYTES**](group__x430__types.md#define-x430_pointer_bytes)  `4U`<br>_Size of a pointer and of size\_t: 8 on 64-bit hosts, 4 on Cortex-M._  |
| define  | [**X430\_SIM\_BYTES**](group__x430__types.md#define-x430_sim_bytes)  `(24U + [**X430\_POINTER\_BYTES**](group__x430__types.md#define-x430_pointer_bytes))`<br>_Private state of a simulator in bytes; x430\_sim.c checks it at compile time._  |
| define  | [**X430\_SIM\_WORDS**](group__x430__types.md#define-x430_sim_words)  `(([**X430\_SIM\_BYTES**](group__x430__types.md#define-x430_sim_bytes) + 7U) / 8U)`<br>_Size of_ [_**x430\_sim**_](structx430__sim.md) _in 64-bit words._ |

## Public Types Documentation




### typedef x430\_delay\_fn 

_Busy or sleeping delay (_ [_**x430\_port::delay\_us**_](structx430__port.md#variable-delay_us) _)._
```
typedef void(* x430_delay_fn) (void *context, uint32_t us);
```





**Parameters:**


* `context` [**x430\_port::context**](structx430__port.md#variable-context). 
* `us` Delay in microseconds. 




        

<hr>



### typedef x430\_event\_hook 

_Device event (_ [_**x430\_hooks::event**_](structx430__hooks.md#variable-event) _)._
```
typedef uint8_t(* x430_event_hook) (struct x430_device *device, enum x430_event event, void *user_data);
```





**Parameters:**


* `device` Device. 
* `event` What happened. 
* `user_data` [**x430\_device\_config::user\_data**](structx430__device__config.md#variable-user_data). 



**Returns:**

DXL2\_ERR\_NONE, or a DXL2\_ERR\_\* code where the event can be refused (X430\_EVENT\_CLEAR\_MULTI\_TURN); ignored otherwise. 





        

<hr>



### typedef x430\_lock\_fn 

_Take or release the bus lock (_ [_**x430\_port::lock**_](structx430__port.md#variable-lock) _,_[_**x430\_port::unlock**_](structx430__port.md#variable-unlock) _)._
```
typedef void(* x430_lock_fn) (void *context);
```





**Parameters:**


* `context` [**x430\_port::context**](structx430__port.md#variable-context). 




        

<hr>



### typedef x430\_millis\_fn 

_Free-running millisecond counter (_ [_**x430\_port::millis**_](structx430__port.md#variable-millis) _)._
```
typedef uint32_t(* x430_millis_fn) (void *context);
```





**Parameters:**


* `context` [**x430\_port::context**](structx430__port.md#variable-context). 



**Returns:**

Milliseconds; wraps at 2^32. 





        

<hr>



### typedef x430\_nvm\_load\_hook 

_Load a persistent block (_ [_**x430\_hooks::nvm\_load**_](structx430__hooks.md#variable-nvm_load) _)._
```
typedef int(* x430_nvm_load_hook) (struct x430_device *device, enum x430_nvm_slot slot, uint8_t *data, uint16_t size, void *user_data);
```





**Parameters:**


* `device` Device. 
* `slot` Block to load. 
* `data` Destination of `size` bytes. 
* `size` [**X430\_NVM\_EEPROM\_SIZE**](group__x430__types.md#define-x430_nvm_eeprom_size) or [**X430\_NVM\_BACKUP\_SIZE**](group__x430__types.md#define-x430_nvm_backup_size). 
* `user_data` [**x430\_device\_config::user\_data**](structx430__device__config.md#variable-user_data). 



**Return value:**


* `0` `size` bytes were read. 
* `<0` No data (the factory values stay). 




        

<hr>



### typedef x430\_nvm\_store\_hook 

_Store a persistent block (_ [_**x430\_hooks::nvm\_store**_](structx430__hooks.md#variable-nvm_store) _), after the change; may be slow._
```
typedef int(* x430_nvm_store_hook) (struct x430_device *device, enum x430_nvm_slot slot, const uint8_t *data, uint16_t size, void *user_data);
```





**Parameters:**


* `device` Device. 
* `slot` Block to store. 
* `data` `size` bytes: header, payload and CRC, opaque to the application. 
* `size` [**X430\_NVM\_EEPROM\_SIZE**](group__x430__types.md#define-x430_nvm_eeprom_size) or [**X430\_NVM\_BACKUP\_SIZE**](group__x430__types.md#define-x430_nvm_backup_size). 
* `user_data` [**x430\_device\_config::user\_data**](structx430__device__config.md#variable-user_data). 



**Return value:**


* `0` Stored. 
* `<0` Failure, reported to the host as Result Fail. 




        

<hr>



### typedef x430\_read\_hook 

_Data is about to be read (_ [_**x430\_hooks::read**_](structx430__hooks.md#variable-read) _)._
```
typedef void(* x430_read_hook) (struct x430_device *device, uint16_t address, uint16_t length, void *user_data);
```



Called once per contiguous run of real addresses (Indirect Data is resolved to the addresses it points at). The hook may refresh live registers with [**x430\_device\_set()**](group__x430.md#function-x430_device_set). 

**Parameters:**


* `device` Device read. 
* `address` First real address of the run. 
* `length` Bytes in the run. 
* `user_data` [**x430\_device\_config::user\_data**](structx430__device__config.md#variable-user_data). 




        

<hr>



### typedef x430\_write\_hook 

_A validated register write, before the value is stored (_ [_**x430\_hooks::write**_](structx430__hooks.md#variable-write) _)._
```
typedef uint8_t(* x430_write_hook) (struct x430_device *device, uint16_t address, uint8_t size, int32_t value, void *user_data);
```





**Parameters:**


* `device` Device written. 
* `address` First address of the register. 
* `size` Register size in bytes (1, 2 or 4). 
* `value` New value (sign-extended for signed registers). 
* `user_data` [**x430\_device\_config::user\_data**](structx430__device__config.md#variable-user_data). 



**Returns:**

DXL2\_ERR\_NONE to accept, another DXL2\_ERR\_\* code to refuse (the instruction stops at this register and the host gets that error). 





        

<hr>
## Macro Definition Documentation





### define X430\_ALIGN\_UP 

`n` _rounded up to a multiple of_`a` _._
```
#define X430_ALIGN_UP (
    n,
    a
) `(((((n) + (a)) - 1U) / (a)) * (a))`
```




<hr>



### define X430\_BUS\_BYTES 

_Private state of a bus in bytes; x430\_bus.c checks it at compile time._ 
```
#define X430_BUS_BYTES `/* multi line expression */`
```




<hr>



### define X430\_BUS\_WORDS 

_Size of_ [_**x430\_bus**_](structx430__bus.md) _in 64-bit words._
```
#define X430_BUS_WORDS `(( X430_BUS_BYTES + 7U) / 8U)`
```




<hr>



### define X430\_DEVICE\_BYTES 

_Private state of a device in bytes; x430\_device.c checks it at compile time._ 
```
#define X430_DEVICE_BYTES `((3U * X430_POINTER_BYTES ) + 17U + X430_TABLE_SIZE + X430_REG_WRITE_SIZE_U + \ X430_NVM_BUFFER_SIZE )`
```




<hr>



### define X430\_DEVICE\_WORDS 

_Size of_ [_**x430\_device**_](structx430__device.md) _in 64-bit words._
```
#define X430_DEVICE_WORDS `(( X430_DEVICE_BYTES + 7U) / 8U)`
```




<hr>



### define X430\_DXL2\_LINK\_BYTES 

_Bytes of struct dxl2\_link of DXL2-C 1.0.0 (configuration, parser, packet, counters), from DXL2\_MAX\_PACKET\_SIZE._ 
```
#define X430_DXL2_LINK_BYTES `X430_ALIGN_UP ((4U * X430_POINTER_BYTES ) +                                                 \ X430_ALIGN_UP ( DXL2_MAX_PACKET_SIZE , X430_POINTER_BYTES ) +               \ X430_POINTER_BYTES + X430_ALIGN_UP ( DXL2_MAX_PACKET_SIZE - 6U, 4U) + 8U, \ X430_POINTER_BYTES )`
```



Plain arithmetic so that the instance size needs no sizeof (static analysis); x430\_bus.c checks it at compile time. 


        

<hr>



### define X430\_FAST\_READ\_BYTES 

_Bytes of the Fast Sync/Bulk Read response buffer of a bus._ 
```
#define X430_FAST_READ_BYTES `DXL2_MAX_PACKET_SIZE`
```




<hr>



### define X430\_NVM\_BACKUP\_SIZE 

_Size of the persistent Control Table backup block._ 
```
#define X430_NVM_BACKUP_SIZE `( X430_NVM_HEADER_SIZE + X430_TABLE_SIZE + 2U)`
```




<hr>



### define X430\_NVM\_BUFFER\_SIZE 

_Bytes of the persistent block buffer inside each device._ 
```
#define X430_NVM_BUFFER_SIZE `X430_NVM_BACKUP_SIZE`
```




<hr>



### define X430\_NVM\_EEPROM\_SIZE 

_Size of the persistent EEPROM block (nvm\_load / nvm\_store)._ 
```
#define X430_NVM_EEPROM_SIZE `( X430_NVM_HEADER_SIZE + X430_RAM_START + 2U)`
```




<hr>



### define X430\_POINTER\_BYTES 

_Size of a pointer and of size\_t: 8 on 64-bit hosts, 4 on Cortex-M._ 
```
#define X430_POINTER_BYTES `4U`
```




<hr>



### define X430\_SIM\_BYTES 

_Private state of a simulator in bytes; x430\_sim.c checks it at compile time._ 
```
#define X430_SIM_BYTES `(24U + X430_POINTER_BYTES )`
```




<hr>



### define X430\_SIM\_WORDS 

_Size of_ [_**x430\_sim**_](structx430__sim.md) _in 64-bit words._
```
#define X430_SIM_WORDS `(( X430_SIM_BYTES + 7U) / 8U)`
```




<hr>

------------------------------


