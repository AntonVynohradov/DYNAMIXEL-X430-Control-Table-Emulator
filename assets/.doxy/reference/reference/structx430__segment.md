

# Struct x430\_segment



[**ClassList**](annotated.md) **>** [**x430\_segment**](structx430__segment.md)



_Segment of the compact Control Table image of a device: Control Table addresses address ._ [More...](#detailed-description)

* `#include <x430_model.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**address**](#variable-address)  <br>_First Control Table address._  |
|  uint16\_t | [**length**](#variable-length)  <br>_Bytes._  |
|  uint16\_t | [**offset**](#variable-offset)  <br>_Index of the first byte in the image._  |












































## Detailed Description


. address + length - 1 are image bytes offset .. offset + length - 1. Addresses outside every segment are reserved and read 0. 


    
## Public Attributes Documentation




### variable address 

_First Control Table address._ 
```C++
uint16_t x430_segment::address;
```




<hr>



### variable length 

_Bytes._ 
```C++
uint16_t x430_segment::length;
```




<hr>



### variable offset 

_Index of the first byte in the image._ 
```C++
uint16_t x430_segment::offset;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/x430_model.h`

