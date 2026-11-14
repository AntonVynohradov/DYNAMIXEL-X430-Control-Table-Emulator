

# Struct x430\_bus\_ctx



[**ClassList**](annotated.md) **>** [**x430\_bus\_ctx**](structx430__bus__ctx.md)



_State behind the opaque_ [_**x430\_bus**_](structx430__bus.md) _._[More...](#detailed-description)

* `#include <x430_internal.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  struct [**x430\_bus\_config**](structx430__bus__config.md) | [**config**](#variable-config)  <br>_Copy of the configuration._  |
|  uint8\_t | [**device\_count**](#variable-device_count)  <br>_Entries in devices._  |
|  struct [**x430\_device**](structx430__device.md) \* | [**devices**](#variable-devices)  <br>_Attached devices._  |
|  uint32\_t | [**last\_rx\_ms**](#variable-last_rx_ms)  <br>_Time of the last instruction packet._  |
|  struct dxl2\_link | [**link**](#variable-link)  <br>_DXL2-C parser/encoder._  |
|  uint32\_t | [**ready**](#variable-ready)  <br>_X430\_READY after_ [_**x430\_bus\_init()**_](group__x430.md#function-x430_bus_init) _._ |
|  uint8\_t | [**tx**](#variable-tx)  <br>_Status packet, built and framed in place._  |
|  bool | [**write\_failed**](#variable-write_failed)  <br>_The writer failed since the last call._  |












































## Detailed Description


Its layout may change in any release. 


    
## Public Attributes Documentation




### variable config 

_Copy of the configuration._ 
```C++
struct x430_bus_config x430_bus_ctx::config;
```




<hr>



### variable device\_count 

_Entries in devices._ 
```C++
uint8_t x430_bus_ctx::device_count;
```




<hr>



### variable devices 

_Attached devices._ 
```C++
struct x430_device* x430_bus_ctx::devices[X430_MAX_DEVICES];
```




<hr>



### variable last\_rx\_ms 

_Time of the last instruction packet._ 
```C++
uint32_t x430_bus_ctx::last_rx_ms;
```




<hr>



### variable link 

_DXL2-C parser/encoder._ 
```C++
struct dxl2_link x430_bus_ctx::link;
```




<hr>



### variable ready 

_X430\_READY after_ [_**x430\_bus\_init()**_](group__x430.md#function-x430_bus_init) _._
```C++
uint32_t x430_bus_ctx::ready;
```




<hr>



### variable tx 

_Status packet, built and framed in place._ 
```C++
uint8_t x430_bus_ctx::tx[DXL2_MAX_PACKET_SIZE];
```




<hr>



### variable write\_failed 

_The writer failed since the last call._ 
```C++
bool x430_bus_ctx::write_failed;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/x430_internal.h`

