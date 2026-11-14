

# Struct x430\_bus\_config



[**ClassList**](annotated.md) **>** [**x430\_bus\_config**](structx430__bus__config.md)



_Bus configuration, copied by_ [_**x430\_bus\_init()**_](group__x430.md#function-x430_bus_init) _._

* `#include <x430_types.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const struct [**x430\_port**](structx430__port.md) \* | [**port**](#variable-port)  <br>_Platform services, NULL = none._  |
|  dxl2\_write\_fn | [**write**](#variable-write)  <br>_Writes a complete packet to the line (required)._  |
|  void \* | [**write\_context**](#variable-write_context)  <br>_Passed to write._  |












































## Public Attributes Documentation




### variable port 

_Platform services, NULL = none._ 
```C++
const struct x430_port* x430_bus_config::port;
```




<hr>



### variable write 

_Writes a complete packet to the line (required)._ 
```C++
dxl2_write_fn x430_bus_config::write;
```




<hr>



### variable write\_context 

_Passed to write._ 
```C++
void* x430_bus_config::write_context;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/x430_types.h`

