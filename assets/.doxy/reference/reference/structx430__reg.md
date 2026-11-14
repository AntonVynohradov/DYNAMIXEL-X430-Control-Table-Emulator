

# Struct x430\_reg



[**ClassList**](annotated.md) **>** [**x430\_reg**](structx430__reg.md)



_One Control Table item._ 

* `#include <x430_model.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**address**](#variable-address)  <br>_First byte._  |
|  uint8\_t | [**check**](#variable-check)  <br>_enum x430\_check._  |
|  uint8\_t | [**flags**](#variable-flags)  <br>_X430\_REG\_\* bits._  |
|  int32\_t | [**initial**](#variable-initial)  <br>_Value after a factory reset._  |
|  int32\_t | [**max**](#variable-max)  <br>_RANGE/INDIRECT upper bound, MASK allowed bits._  |
|  int32\_t | [**min**](#variable-min)  <br>_RANGE/INDIRECT lower bound._  |
|  uint16\_t | [**ref**](#variable-ref)  <br>_Limit register address for SYMMETRIC._  |
|  uint8\_t | [**size**](#variable-size)  <br>_1, 2 or 4 bytes._  |












































## Public Attributes Documentation




### variable address 

_First byte._ 
```C++
uint16_t x430_reg::address;
```




<hr>



### variable check 

_enum x430\_check._ 
```C++
uint8_t x430_reg::check;
```




<hr>



### variable flags 

_X430\_REG\_\* bits._ 
```C++
uint8_t x430_reg::flags;
```




<hr>



### variable initial 

_Value after a factory reset._ 
```C++
int32_t x430_reg::initial;
```




<hr>



### variable max 

_RANGE/INDIRECT upper bound, MASK allowed bits._ 
```C++
int32_t x430_reg::max;
```




<hr>



### variable min 

_RANGE/INDIRECT lower bound._ 
```C++
int32_t x430_reg::min;
```




<hr>



### variable ref 

_Limit register address for SYMMETRIC._ 
```C++
uint16_t x430_reg::ref;
```




<hr>



### variable size 

_1, 2 or 4 bytes._ 
```C++
uint8_t x430_reg::size;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/x430_model.h`

