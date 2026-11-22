

# Group x430\_config



[**Modules**](modules.md) **>** [**x430\_config**](group__x430__config.md)



_Build-time configuration: Kconfig symbols or plain compiler definitions._ [More...](#detailed-description)

































































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**X430\_BAUD\_RATE\_CODE\_MAX**](group__x430__config.md#define-x430_baud_rate_code_max)  `(7U)`<br>_Highest Baud Rate (address 8) code accepted by a write._  |
| define  | [**X430\_BAUD\_RATE\_CODE\_MAX\_U**](group__x430__config.md#define-x430_baud_rate_code_max_u)  `((uint32\_t)([**X430\_BAUD\_RATE\_CODE\_MAX**](group__x430__config.md#define-x430_baud_rate_code_max)))`<br> |
| define  | [**X430\_FEATURE\_BACKUP**](group__x430__config.md#define-x430_feature_backup)  `1`<br>_Control Table BACKUP instruction, Backup Ready and Startup RAM Restore._  |
| define  | [**X430\_FEATURE\_BULK**](group__x430__config.md#define-x430_feature_bulk)  `1`<br>_BULK\_READ and BULK\_WRITE instructions._  |
| define  | [**X430\_FEATURE\_FAST\_READ**](group__x430__config.md#define-x430_feature_fast_read)  `1`<br>_FAST\_SYNC\_READ and FAST\_BULK\_READ instructions (used by ROS 2 by default)._  |
| define  | [**X430\_FEATURE\_SIMULATOR**](group__x430__config.md#define-x430_feature_simulator)  `1`<br>_Built-in simulator backend (x430\_sim.h)._  |
| define  | [**X430\_FIRMWARE\_VERSION**](group__x430__config.md#define-x430_firmware_version)  `(52U)`<br>_Firmware Version (address 6) used when the device configuration gives 0._  |
| define  | [**X430\_MAX\_DEVICES**](group__x430__config.md#define-x430_max_devices)  `(4U)`<br>_Devices (DYNAMIXEL IDs) one bus can host._  |
| define  | [**X430\_MAX\_DEVICES\_U**](group__x430__config.md#define-x430_max_devices_u)  `((uint32\_t)([**X430\_MAX\_DEVICES**](group__x430__config.md#define-x430_max_devices)))`<br> |
| define  | [**X430\_MODEL\_NUMBER**](group__x430__config.md#define-x430_model_number)  `(X430\_MODEL\_NUMBER\_DEFAULT)`<br>_Model Number reported by PING and address 0._  |
| define  | [**X430\_REG\_WRITE\_SIZE**](group__x430__config.md#define-x430_reg_write_size)  `(32U)`<br>_Largest data block a REG\_WRITE can register, in bytes._  |
| define  | [**X430\_REG\_WRITE\_SIZE\_U**](group__x430__config.md#define-x430_reg_write_size_u)  `((uint32\_t)([**X430\_REG\_WRITE\_SIZE**](group__x430__config.md#define-x430_reg_write_size)))`<br> |
| define  | [**X430\_TURN\_TIMEOUT\_MS**](group__x430__config.md#define-x430_turn_timeout_ms)  `(100U)`<br>_Longest wait, in milliseconds, for the status packet of the device before a hosted device in a SYNC\_READ or BULK\_READ (default 100)._  |
| define  | [**X430\_TURN\_TIMEOUT\_MS\_U**](group__x430__config.md#define-x430_turn_timeout_ms_u)  `((uint32\_t)([**X430\_TURN\_TIMEOUT\_MS**](group__x430__config.md#define-x430_turn_timeout_ms)))`<br> |

## Detailed Description


The values come from three places, highest priority first: 
* a compiler definition of the X430\_\* macro itself (for example `-DX430_MAX_DEVICES=8U`); 
* the Kconfig header x430\_autoconf.h (CONFIG\_X430\_\* symbols), included when the build defines X430\_HAS\_AUTOCONF; 
* the defaults below.

The emulated model is not configured here: CMake generates its Control Table (x430\_model\_table.c) and x430\_model\_info.h for the model selected with Kconfig or with the X430\_MODEL cache variable. 


    
## Macro Definition Documentation





### define X430\_BAUD\_RATE\_CODE\_MAX 

_Highest Baud Rate (address 8) code accepted by a write._ 
```
#define X430_BAUD_RATE_CODE_MAX `(7U)`
```



The X430 table defines 0..7. A larger value lets an application use its own codes; the application validates them in the write hook. 


        

<hr>



### define X430\_BAUD\_RATE\_CODE\_MAX\_U 

```
#define X430_BAUD_RATE_CODE_MAX_U `((uint32_t)( X430_BAUD_RATE_CODE_MAX ))`
```




<hr>



### define X430\_FEATURE\_BACKUP 

_Control Table BACKUP instruction, Backup Ready and Startup RAM Restore._ 
```
#define X430_FEATURE_BACKUP `1`
```




<hr>



### define X430\_FEATURE\_BULK 

_BULK\_READ and BULK\_WRITE instructions._ 
```
#define X430_FEATURE_BULK `1`
```




<hr>



### define X430\_FEATURE\_FAST\_READ 

_FAST\_SYNC\_READ and FAST\_BULK\_READ instructions (used by ROS 2 by default)._ 
```
#define X430_FEATURE_FAST_READ `1`
```




<hr>



### define X430\_FEATURE\_SIMULATOR 

_Built-in simulator backend (x430\_sim.h)._ 
```
#define X430_FEATURE_SIMULATOR `1`
```




<hr>



### define X430\_FIRMWARE\_VERSION 

_Firmware Version (address 6) used when the device configuration gives 0._ 
```
#define X430_FIRMWARE_VERSION `(52U)`
```




<hr>



### define X430\_MAX\_DEVICES 

_Devices (DYNAMIXEL IDs) one bus can host._ 
```
#define X430_MAX_DEVICES `(4U)`
```




<hr>



### define X430\_MAX\_DEVICES\_U 

```
#define X430_MAX_DEVICES_U `((uint32_t)( X430_MAX_DEVICES ))`
```




<hr>



### define X430\_MODEL\_NUMBER 

_Model Number reported by PING and address 0._ 
```
#define X430_MODEL_NUMBER `(X430_MODEL_NUMBER_DEFAULT)`
```



CONFIG\_X430\_MODEL\_NUMBER\_OVERRIDE (non-zero) replaces the number of the ROBOTIS documentation, for hosts that only know another number. 


        

<hr>



### define X430\_REG\_WRITE\_SIZE 

_Largest data block a REG\_WRITE can register, in bytes._ 
```
#define X430_REG_WRITE_SIZE `(32U)`
```




<hr>



### define X430\_REG\_WRITE\_SIZE\_U 

```
#define X430_REG_WRITE_SIZE_U `((uint32_t)( X430_REG_WRITE_SIZE ))`
```




<hr>



### define X430\_TURN\_TIMEOUT\_MS 

_Longest wait, in milliseconds, for the status packet of the device before a hosted device in a SYNC\_READ or BULK\_READ (default 100)._ 
```
#define X430_TURN_TIMEOUT_MS `(100U)`
```



On a bus shared with devices of other buses (other MCUs, real servos), a hosted device answers a SYNC\_READ or BULK\_READ only after it has received the status packet of the ID listed just before it, like an X series actuator. When that status does not arrive within this time after the last progress of the read, the remaining hosted devices of the read stay silent. A new instruction packet ends the wait as well. The time comes from the millisecond clock of [**x430\_port**](structx430__port.md); without a clock the wait ends only with the next instruction packet. Keep it longer than one status packet at the lowest baud rate in use, plus the Return Delay Time. 


        

<hr>



### define X430\_TURN\_TIMEOUT\_MS\_U 

```
#define X430_TURN_TIMEOUT_MS_U `((uint32_t)( X430_TURN_TIMEOUT_MS ))`
```




<hr>

------------------------------


