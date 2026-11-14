

# Struct x430\_device\_config



[**ClassList**](annotated.md) **>** [**x430\_device\_config**](structx430__device__config.md)



_Device configuration, copied by_ [_**x430\_device\_init()**_](group__x430.md#function-x430_device_init) _._

* `#include <x430_types.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**firmware\_version**](#variable-firmware_version)  <br>_Firmware Version (6); 0 =_ [_**X430\_FIRMWARE\_VERSION**_](group__x430__config.md#define-x430_firmware_version) _._ |
|  const struct [**x430\_hooks**](structx430__hooks.md) \* | [**hooks**](#variable-hooks)  <br>_Application hooks; NULL = pure register device._  |
|  uint8\_t | [**id**](#variable-id)  <br>_Factory ID of this device, 0..252 (0 = model default 1)._  |
|  uint32\_t | [**model\_information**](#variable-model_information)  <br>_Model Information (2), free for the application (e.g._  |
|  void \* | [**user\_data**](#variable-user_data)  <br>_Passed to every hook._  |












































## Public Attributes Documentation




### variable firmware\_version 

_Firmware Version (6); 0 =_ [_**X430\_FIRMWARE\_VERSION**_](group__x430__config.md#define-x430_firmware_version) _._
```C++
uint8_t x430_device_config::firmware_version;
```




<hr>



### variable hooks 

_Application hooks; NULL = pure register device._ 
```C++
const struct x430_hooks* x430_device_config::hooks;
```




<hr>



### variable id 

_Factory ID of this device, 0..252 (0 = model default 1)._ 
```C++
uint8_t x430_device_config::id;
```



Stored EEPROM wins. 


        

<hr>



### variable model\_information 

_Model Information (2), free for the application (e.g._ 
```C++
uint32_t x430_device_config::model_information;
```



its firmware version). 


        

<hr>



### variable user\_data 

_Passed to every hook._ 
```C++
void* x430_device_config::user_data;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/x430_types.h`

