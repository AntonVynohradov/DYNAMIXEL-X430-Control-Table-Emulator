

# Struct x430\_model



[**ClassList**](annotated.md) **>** [**x430\_model**](structx430__model.md)



_Control Table and capabilities of one model._ 

* `#include <x430_model.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**axes**](#variable-axes)  <br>_2 for 2XL430/2XC430._  |
|  uint16\_t | [**current\_unit\_ua**](#variable-current_unit_ua)  <br>_Current unit in uA, 0 = no current._  |
|  const struct [**x430\_indirect\_block**](structx430__indirect__block.md) \* | [**indirect**](#variable-indirect)  <br>_Indirect blocks._  |
|  uint8\_t | [**indirect\_count**](#variable-indirect_count)  <br>_Entries in indirect._  |
|  uint16\_t | [**model\_number**](#variable-model_number)  <br>_Number from the documentation._  |
|  const char \* | [**name**](#variable-name)  <br>_e.g._  |
|  uint32\_t | [**operating\_modes**](#variable-operating_modes)  <br>_Bit n set: Operating Mode n allowed._  |
|  uint16\_t | [**register\_count**](#variable-register_count)  <br>_Entries in registers._  |
|  const uint8\_t \* | [**register\_map**](#variable-register_map)  <br>_Index + 1 per address, 0 = reserved._  |
|  const struct [**x430\_reg**](structx430__reg.md) \* | [**registers**](#variable-registers)  <br>_Sorted by address._  |
|  uint16\_t | [**table\_size**](#variable-table_size)  <br>_Addresses 0 ._  |












































## Public Attributes Documentation




### variable axes 

_2 for 2XL430/2XC430._ 
```C++
uint8_t x430_model::axes;
```




<hr>



### variable current\_unit\_ua 

_Current unit in uA, 0 = no current._ 
```C++
uint16_t x430_model::current_unit_ua;
```




<hr>



### variable indirect 

_Indirect blocks._ 
```C++
const struct x430_indirect_block* x430_model::indirect;
```




<hr>



### variable indirect\_count 

_Entries in indirect._ 
```C++
uint8_t x430_model::indirect_count;
```




<hr>



### variable model\_number 

_Number from the documentation._ 
```C++
uint16_t x430_model::model_number;
```




<hr>



### variable name 

_e.g._ 
```C++
const char* x430_model::name;
```



"XM430-W350". 


        

<hr>



### variable operating\_modes 

_Bit n set: Operating Mode n allowed._ 
```C++
uint32_t x430_model::operating_modes;
```




<hr>



### variable register\_count 

_Entries in registers._ 
```C++
uint16_t x430_model::register_count;
```




<hr>



### variable register\_map 

_Index + 1 per address, 0 = reserved._ 
```C++
const uint8_t* x430_model::register_map;
```




<hr>



### variable registers 

_Sorted by address._ 
```C++
const struct x430_reg* x430_model::registers;
```




<hr>



### variable table\_size 

_Addresses 0 ._ 
```C++
uint16_t x430_model::table_size;
```



. table\_size - 1. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `inc/x430_model.h`

