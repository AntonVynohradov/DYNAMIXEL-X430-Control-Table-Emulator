

# Struct x430\_turn



[**ClassList**](annotated.md) **>** [**x430\_turn**](structx430__turn.md)



_One hosted device of a SYNC\_READ or BULK\_READ that has not answered yet._ [More...](#detailed-description)

* `#include <x430_internal.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**address**](#variable-address)  <br>_First address to read._  |
|  uint8\_t | [**after**](#variable-after)  <br>_ID whose status comes first, or X430\_TURN\_NOW._  |
|  struct [**x430\_device**](structx430__device.md) \* | [**device**](#variable-device)  <br>_Hosted device._  |
|  uint16\_t | [**length**](#variable-length)  <br>_Bytes to read._  |












































## Detailed Description


It answers when [**x430\_turn::after**](structx430__turn.md#variable-after) is X430\_TURN\_NOW: at once for the first ID of the request and after a hosted device, else after the status packet of the ID `after` (a device of another bus). 


    
## Public Attributes Documentation




### variable address 

_First address to read._ 
```C++
uint16_t x430_turn::address;
```




<hr>



### variable after 

_ID whose status comes first, or X430\_TURN\_NOW._ 
```C++
uint8_t x430_turn::after;
```




<hr>



### variable device 

_Hosted device._ 
```C++
struct x430_device* x430_turn::device;
```




<hr>



### variable length 

_Bytes to read._ 
```C++
uint16_t x430_turn::length;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/x430_internal.h`

