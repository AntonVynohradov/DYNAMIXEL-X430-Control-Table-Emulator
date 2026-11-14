

# Struct x430\_hooks



[**ClassList**](annotated.md) **>** [**x430\_hooks**](structx430__hooks.md)



_Application hooks of a device._ [More...](#detailed-description)

* `#include <x430_types.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**x430\_event\_hook**](group__x430__types.md#typedef-x430_event_hook) | [**event**](#variable-event)  <br>_Reboot, factory reset, watchdog, ..._  |
|  [**x430\_nvm\_load\_hook**](group__x430__types.md#typedef-x430_nvm_load_hook) | [**nvm\_load**](#variable-nvm_load)  <br>_Load the EEPROM area or the backup._  |
|  [**x430\_nvm\_store\_hook**](group__x430__types.md#typedef-x430_nvm_store_hook) | [**nvm\_store**](#variable-nvm_store)  <br>_Store the EEPROM area or the backup._  |
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



### variable nvm\_load 

_Load the EEPROM area or the backup._ 
```C++
x430_nvm_load_hook x430_hooks::nvm_load;
```




<hr>



### variable nvm\_store 

_Store the EEPROM area or the backup._ 
```C++
x430_nvm_store_hook x430_hooks::nvm_store;
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

