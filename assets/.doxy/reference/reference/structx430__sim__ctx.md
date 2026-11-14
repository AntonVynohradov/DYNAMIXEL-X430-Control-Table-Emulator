

# Struct x430\_sim\_ctx



[**ClassList**](annotated.md) **>** [**x430\_sim\_ctx**](structx430__sim__ctx.md)



_State behind the opaque_ [_**x430\_sim**_](structx430__sim.md) _._[More...](#detailed-description)

* `#include <x430_internal.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  struct [**x430\_sim\_config**](structx430__sim__config.md) | [**config**](#variable-config)  <br>_Copy of the configuration._  |
|  struct [**x430\_device**](structx430__device.md) \* | [**device**](#variable-device)  <br>_Simulated device._  |
|  uint32\_t | [**ready**](#variable-ready)  <br>_X430\_READY after_ [_**x430\_sim\_init()**_](group__x430.md#function-x430_sim_init) _._ |
|  int64\_t | [**shaft\_mpulse**](#variable-shaft_mpulse)  <br>_Shaft position [1/1000 pulse]._  |
|  int64\_t | [**velocity\_mpulse**](#variable-velocity_mpulse)  <br>_Shaft velocity [1/1000 pulse per second]._  |












































## Detailed Description


Its layout may change in any release. 


    
## Public Attributes Documentation




### variable config 

_Copy of the configuration._ 
```C++
struct x430_sim_config x430_sim_ctx::config;
```




<hr>



### variable device 

_Simulated device._ 
```C++
struct x430_device* x430_sim_ctx::device;
```




<hr>



### variable ready 

_X430\_READY after_ [_**x430\_sim\_init()**_](group__x430.md#function-x430_sim_init) _._
```C++
uint32_t x430_sim_ctx::ready;
```




<hr>



### variable shaft\_mpulse 

_Shaft position [1/1000 pulse]._ 
```C++
int64_t x430_sim_ctx::shaft_mpulse;
```




<hr>



### variable velocity\_mpulse 

_Shaft velocity [1/1000 pulse per second]._ 
```C++
int64_t x430_sim_ctx::velocity_mpulse;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/x430_internal.h`

