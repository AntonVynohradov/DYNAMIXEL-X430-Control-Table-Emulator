

# Struct x430\_device



[**ClassList**](annotated.md) **>** [**x430\_device**](structx430__device.md)



_One emulated actuator (one DYNAMIXEL ID): Control Table image and rules._ [More...](#detailed-description)

* `#include <x430_types.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint64\_t | [**opaque**](#variable-opaque)  <br>_Private state._  |












































## Detailed Description


Allocated by the application and initialized by [**x430\_device\_init()**](group__x430.md#function-x430_device_init). The content is private: use the functions of x430.h. 


    
## Public Attributes Documentation




### variable opaque 

_Private state._ 
```C++
uint64_t x430_device::opaque[X430_DEVICE_WORDS];
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/x430_types.h`

