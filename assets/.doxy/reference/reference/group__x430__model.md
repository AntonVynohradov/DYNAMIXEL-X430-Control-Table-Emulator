

# Group x430\_model



[**Modules**](modules.md) **>** [**x430\_model**](group__x430__model.md)



_Types of the generated Control Table (one model per build)._ [More...](#detailed-description)
















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**x430\_indirect\_block**](structx430__indirect__block.md) <br>_Indirect Address block: count addresses of 2 bytes and count data bytes._  |
| struct | [**x430\_model**](structx430__model.md) <br>_Control Table and capabilities of one model._  |
| struct | [**x430\_reg**](structx430__reg.md) <br>_One Control Table item (16 bytes of flash)._  |
| struct | [**x430\_segment**](structx430__segment.md) <br>_Segment of the compact Control Table image of a device: Control Table addresses address ._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**x430\_check**](#enum-x430_check)  <br>_Write check of a register (upper four bits of_ [_**x430\_reg::flags**_](structx430__reg.md#variable-flags) _)._ |




## Public Attributes

| Type | Name |
| ---: | :--- |
|  const struct [**x430\_model**](structx430__model.md) | [**x430\_model\_active**](#variable-x430_model_active)  <br>_The model selected at build time (x430\_model\_table.c)._  |











































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**X430\_REG\_CHECK**](group__x430__model.md#define-x430_reg_check) (check) `((uint32\_t)(check) &lt;&lt; [**X430\_REG\_CHECK\_SHIFT**](group__x430__model.md#define-x430_reg_check_shift))`<br>_Write check_ `check` _as_[_**x430\_reg::flags**_](structx430__reg.md#variable-flags) _bits._ |
| define  | [**X430\_REG\_CHECK\_SHIFT**](group__x430__model.md#define-x430_reg_check_shift)  `4U`<br>_Shift of the write check (enum x430\_check) in the upper four bits of_ [_**x430\_reg::flags**_](structx430__reg.md#variable-flags) _._ |
| define  | [**X430\_REG\_EEPROM**](group__x430__model.md#define-x430_reg_eeprom)  `0x02U`<br>_EEPROM area: persistent, locked while Torque Enable is 1._  |
| define  | [**X430\_REG\_GOAL**](group__x430__model.md#define-x430_reg_goal)  `0x08U`<br>_Goal value: read-only while a Bus Watchdog error is active._  |
| define  | [**X430\_REG\_SIGNED**](group__x430__model.md#define-x430_reg_signed)  `0x04U`<br>_Two's complement value._  |
| define  | [**X430\_REG\_WRITABLE**](group__x430__model.md#define-x430_reg_writable)  `0x01U`<br>_Writable by an instruction (RW); without it the register is read-only._  |

## Detailed Description


tools/x430\_gen\_model.py writes x430\_model\_table.c with the constant [**x430\_model\_active**](group__x430__model.md#variable-x430_model_active) from the YAML description in models/. The library reads the descriptor only; it never changes it. 


    
## Public Types Documentation




### enum x430\_check 

_Write check of a register (upper four bits of_ [_**x430\_reg::flags**_](structx430__reg.md#variable-flags) _)._
```
enum x430_check {
    X430_CHECK_NONE = 0,
    X430_CHECK_RANGE,
    X430_CHECK_MASK,
    X430_CHECK_OPMODE,
    X430_CHECK_SYMMETRIC,
    X430_CHECK_GOAL_POSITION,
    X430_CHECK_INDIRECT
};
```




<hr>
## Public Attributes Documentation




### variable x430\_model\_active 

_The model selected at build time (x430\_model\_table.c)._ 
```
const struct x430_model x430_model_active;
```




<hr>
## Macro Definition Documentation





### define X430\_REG\_CHECK 

_Write check_ `check` _as_[_**x430\_reg::flags**_](structx430__reg.md#variable-flags) _bits._
```
#define X430_REG_CHECK (
    check
) `((uint32_t)(check) << X430_REG_CHECK_SHIFT )`
```




<hr>



### define X430\_REG\_CHECK\_SHIFT 

_Shift of the write check (enum x430\_check) in the upper four bits of_ [_**x430\_reg::flags**_](structx430__reg.md#variable-flags) _._
```
#define X430_REG_CHECK_SHIFT `4U`
```




<hr>



### define X430\_REG\_EEPROM 

_EEPROM area: persistent, locked while Torque Enable is 1._ 
```
#define X430_REG_EEPROM `0x02U`
```




<hr>



### define X430\_REG\_GOAL 

_Goal value: read-only while a Bus Watchdog error is active._ 
```
#define X430_REG_GOAL `0x08U`
```




<hr>



### define X430\_REG\_SIGNED 

_Two's complement value._ 
```
#define X430_REG_SIGNED `0x04U`
```




<hr>



### define X430\_REG\_WRITABLE 

_Writable by an instruction (RW); without it the register is read-only._ 
```
#define X430_REG_WRITABLE `0x01U`
```




<hr>

------------------------------


