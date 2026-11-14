

# Struct x430\_hooks



[**ClassList**](annotated.md) **>** [**x430\_hooks**](structx430__hooks.md)



_Application hooks of a device._ [More...](#detailed-description)

* `#include <x430_types.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**x430\_event\_hook**](group__x430__types.md#typedef-x430_event_hook) | [**event**](#variable-event)  <br>_Reboot, factory reset, watchdog, ..._  |
|  [**x430\_nvm\_read\_hook**](group__x430__types.md#typedef-x430_nvm_read_hook) | [**nvm\_read**](#variable-nvm_read)  <br>_Read part of the EEPROM block or of the backup._  |
|  [**x430\_nvm\_write\_hook**](group__x430__types.md#typedef-x430_nvm_write_hook) | [**nvm\_write**](#variable-nvm_write)  <br>_Write part of the EEPROM block or of the backup._  |
|  [**x430\_read\_hook**](group__x430__types.md#typedef-x430_read_hook) | [**read**](#variable-read)  <br>_Before data is read._  |
|  [**x430\_write\_hook**](group__x430__types.md#typedef-x430_write_hook) | [**write**](#variable-write)  <br>_Every accepted register write._  |












































## Detailed Description


Every member may be NULL.


Hooks run in the bus context with the bus lock held. They may call the lock-free device functions ([**x430\_device\_get()**](group__x430.md#function-x430_device_get), [**x430\_device\_set()**](group__x430.md#function-x430_device_set), [**x430\_device\_id()**](group__x430.md#function-x430_device_id), [**x430\_device\_raise\_hardware\_error()**](group__x430.md#function-x430_device_raise_hardware_error)), but no bus function. 


    
## Public Attributes Documentation




### variable event 

_Reboot, factory reset, watchdog, ..._ 
```C++
x430_event_hook x430_hooks::event;
```




<hr>



### variable nvm\_read 

_Read part of the EEPROM block or of the backup._ 
```C++
x430_nvm_read_hook x430_hooks::nvm_read;
```




<hr>



### variable nvm\_write 

_Write part of the EEPROM block or of the backup._ 
```C++
x430_nvm_write_hook x430_hooks::nvm_write;
```




<hr>



### variable read 

_Before data is read._ 
```C++
x430_read_hook x430_hooks::read;
```




<hr>



### variable write 

_Every accepted register write._ 
```C++
x430_write_hook x430_hooks::write;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/x430_types.h`

