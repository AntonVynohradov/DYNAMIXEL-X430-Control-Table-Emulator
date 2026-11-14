

# Struct x430\_port



[**ClassList**](annotated.md) **>** [**x430\_port**](structx430__port.md)



_Platform services._ [More...](#detailed-description)

* `#include <x430_types.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**context**](#variable-context)  <br>_Passed to every function._  |
|  [**x430\_delay\_fn**](group__x430__types.md#typedef-x430_delay_fn) | [**delay\_us**](#variable-delay_us)  <br>_Delay before a status packet._  |
|  [**x430\_lock\_fn**](group__x430__types.md#typedef-x430_lock_fn) | [**lock**](#variable-lock)  <br>_Take the bus lock._  |
|  [**x430\_millis\_fn**](group__x430__types.md#typedef-x430_millis_fn) | [**millis**](#variable-millis)  <br>_Free-running millisecond counter._  |
|  [**x430\_lock\_fn**](group__x430__types.md#typedef-x430_lock_fn) | [**unlock**](#variable-unlock)  <br>_Release the bus lock._  |












































## Detailed Description


Every member may be NULL; the library then works without that service:



|Member   |Used for   |NULL means    |
|-----|-----|-----|
|lock   |mutual exclusion of the Control Tables   |single context (bare metal)    |
|unlock   |(pair of lock)   ||
|millis   |Realtime Tick (120), Bus Watchdog (98)   |tick 0, no watchdog timeout    |
|delay\_us   |Return Delay Time (9) before a status   |status sent at once   |






The bus keeps a pointer to the port: keep it alive as long as the bus. 


    
## Public Attributes Documentation




### variable context 

_Passed to every function._ 
```C++
void* x430_port::context;
```




<hr>



### variable delay\_us 

_Delay before a status packet._ 
```C++
x430_delay_fn x430_port::delay_us;
```




<hr>



### variable lock 

_Take the bus lock._ 
```C++
x430_lock_fn x430_port::lock;
```




<hr>



### variable millis 

_Free-running millisecond counter._ 
```C++
x430_millis_fn x430_port::millis;
```




<hr>



### variable unlock 

_Release the bus lock._ 
```C++
x430_lock_fn x430_port::unlock;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/x430_types.h`

