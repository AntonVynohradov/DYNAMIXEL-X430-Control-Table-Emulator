

# Struct x430\_device\_ctx



[**ClassList**](annotated.md) **>** [**x430\_device\_ctx**](structx430__device__ctx.md)



_State behind the opaque_ [_**x430\_device**_](structx430__device.md) _._[More...](#detailed-description)

* `#include <x430_internal.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**baud\_changed**](#variable-baud_changed)  <br>_Deferred_ X430\_EVENT\_BAUD\_RATE _._ |
|  struct [**x430\_bus**](structx430__bus.md) \* | [**bus**](#variable-bus)  <br>_Owning bus, NULL before attach._  |
|  struct [**x430\_device\_config**](structx430__device__config.md) | [**config**](#variable-config)  <br>_Copy of the configuration._  |
|  uint8\_t | [**nvm**](#variable-nvm)  <br>_Persistent block buffer._  |
|  uint32\_t | [**ready**](#variable-ready)  <br>_X430\_READY after_ [_**x430\_device\_init()**_](group__x430.md#function-x430_device_init) _._ |
|  uint16\_t | [**reg\_address**](#variable-reg_address)  <br>_REG\_WRITE address._  |
|  uint8\_t | [**reg\_data**](#variable-reg_data)  <br>_REG\_WRITE data._  |
|  uint16\_t | [**reg\_length**](#variable-reg_length)  <br>_REG\_WRITE length, 0 = none._  |
|  uint8\_t | [**table**](#variable-table)  <br>_Control Table image, little endian._  |












































## Detailed Description


Its layout may change in any release. 


    
## Public Attributes Documentation




### variable baud\_changed 

_Deferred_ X430\_EVENT\_BAUD\_RATE _._
```C++
bool x430_device_ctx::baud_changed;
```




<hr>



### variable bus 

_Owning bus, NULL before attach._ 
```C++
struct x430_bus* x430_device_ctx::bus;
```




<hr>



### variable config 

_Copy of the configuration._ 
```C++
struct x430_device_config x430_device_ctx::config;
```




<hr>



### variable nvm 

_Persistent block buffer._ 
```C++
uint8_t x430_device_ctx::nvm[X430_NVM_BUFFER_SIZE];
```




<hr>



### variable ready 

_X430\_READY after_ [_**x430\_device\_init()**_](group__x430.md#function-x430_device_init) _._
```C++
uint32_t x430_device_ctx::ready;
```




<hr>



### variable reg\_address 

_REG\_WRITE address._ 
```C++
uint16_t x430_device_ctx::reg_address;
```




<hr>



### variable reg\_data 

_REG\_WRITE data._ 
```C++
uint8_t x430_device_ctx::reg_data[X430_REG_WRITE_SIZE];
```




<hr>



### variable reg\_length 

_REG\_WRITE length, 0 = none._ 
```C++
uint16_t x430_device_ctx::reg_length;
```




<hr>



### variable table 

_Control Table image, little endian._ 
```C++
uint8_t x430_device_ctx::table[X430_TABLE_SIZE];
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/x430_internal.h`

