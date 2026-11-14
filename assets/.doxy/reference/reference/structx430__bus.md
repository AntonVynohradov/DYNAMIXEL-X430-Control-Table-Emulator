

# Struct x430\_bus



[**ClassList**](annotated.md) **>** [**x430\_bus**](structx430__bus.md)



_One serial line with up to_ [_**X430\_MAX\_DEVICES**_](group__x430__config.md#define-x430_max_devices) _emulated devices._[More...](#detailed-description)

* `#include <x430_types.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint64\_t | [**opaque**](#variable-opaque)  <br>_Private state._  |












































## Detailed Description


Allocated by the application (static, stack or pool) and initialized by [**x430\_bus\_init()**](group__x430.md#function-x430_bus_init). The content is private: use the functions of x430.h. 


    
## Public Attributes Documentation




### variable opaque 

_Private state._ 
```C++
uint64_t x430_bus::opaque[X430_BUS_WORDS];
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/x430_types.h`

