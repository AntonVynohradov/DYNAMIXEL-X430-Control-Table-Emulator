

# Group x430



[**Modules**](modules.md) **>** [**x430**](group__x430.md)



_Device-side emulation of DYNAMIXEL X430 actuators over Protocol 2.0: the firmware moves any motor, the host sees a genuine X430._ [More...](#detailed-description)












## Modules

| Type | Name |
| ---: | :--- |
| module | [**Configuration**](group__x430__config.md) <br>_Build-time configuration: Kconfig symbols or plain compiler definitions._  |
| module | [**Definitions**](group__x430__defs.md) <br>_Status codes, options, events and the addresses, operating modes and bit fields of the X430 Control Table._  |
| module | [**Model descriptor**](group__x430__model.md) <br>_Types of the generated Control Table (one model per build)._  |
| module | [**Types**](group__x430__types.md) <br>_Callbacks, configuration structures and the opaque instances._  |










## Public Attributes

| Type | Name |
| ---: | :--- |
|  const struct [**x430\_hooks**](structx430__hooks.md) | [**x430\_sim\_hooks**](#variable-x430_sim_hooks)  <br>_Hooks implementing the simulated actuator: pass them with_ [_**x430\_device\_config::user\_data**_](structx430__device__config.md#variable-user_data) _pointing at an initialized_[_**x430\_sim**_](structx430__sim.md) _._ |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  int | [**x430\_bus\_attach**](#function-x430_bus_attach) (struct [**x430\_bus**](structx430__bus.md) \* bus, struct [**x430\_device**](structx430__device.md) \* device) <br>_Attach an initialized device to an initialized bus._  |
|  int | [**x430\_bus\_find**](#function-x430_bus_find) (const struct [**x430\_bus**](structx430__bus.md) \* bus, uint8\_t id, struct [**x430\_device**](structx430__device.md) \*\* device) <br>_Attached device with the primary ID_ `id` _(lock-free)._ |
|  int | [**x430\_bus\_handle\_packet**](#function-x430_bus_handle_packet) (struct [**x430\_bus**](structx430__bus.md) \* bus, const struct dxl2\_packet \* packet) <br>_Handle one decoded instruction packet, for applications that run their own DXL2-C link._  |
|  int | [**x430\_bus\_init**](#function-x430_bus_init) (struct [**x430\_bus**](structx430__bus.md) \* bus, const struct [**x430\_bus\_config**](structx430__bus__config.md) \* config) <br>_Initialize a bus._  |
|  int | [**x430\_bus\_lock**](#function-x430_bus_lock) (struct [**x430\_bus**](structx430__bus.md) \* bus) <br>_Take the bus lock (_ [_**x430\_port::lock**_](structx430__port.md#variable-lock) _; nothing without it)._ |
|  int | [**x430\_bus\_poll**](#function-x430_bus_poll) (struct [**x430\_bus**](structx430__bus.md) \* bus) <br>_Periodic service: Bus Watchdog of every device._  |
|  int | [**x430\_bus\_receive**](#function-x430_bus_receive) (struct [**x430\_bus**](structx430__bus.md) \* bus, const uint8\_t \* data, size\_t length) <br>_Feed received bytes in any chunking; answers every complete instruction packet._  |
|  int | [**x430\_bus\_unlock**](#function-x430_bus_unlock) (struct [**x430\_bus**](structx430__bus.md) \* bus) <br>_Release the bus lock (_ [_**x430\_port::unlock**_](structx430__port.md#variable-unlock) _)._ |
|  int | [**x430\_device\_action**](#function-x430_device_action) (struct [**x430\_device**](structx430__device.md) \* device, uint8\_t \* error) <br>_ACTION: execute the registered write._  |
|  int | [**x430\_device\_backup**](#function-x430_device_backup) (struct [**x430\_device**](structx430__device.md) \* device, uint8\_t option, uint8\_t \* error) <br>_Control Table BACKUP: store, or restore with the torque off._  |
|  int | [**x430\_device\_clear**](#function-x430_device_clear) (struct [**x430\_device**](structx430__device.md) \* device, uint8\_t option, uint8\_t \* error) <br>_CLEAR with option_ [_**X430\_CLEAR\_MULTI\_TURN**_](group__x430__defs.md#define-x430_clear_multi_turn) _(event_X430\_EVENT\_CLEAR\_MULTI\_TURN _)._ |
|  int | [**x430\_device\_factory\_reset**](#function-x430_device_factory_reset) (struct [**x430\_device**](structx430__device.md) \* device, uint8\_t option, uint8\_t \* error) <br>_FACTORY\_RESET, then like REBOOT; event_ X430\_EVENT\_FACTORY\_RESET _._ |
|  int | [**x430\_device\_flush\_events**](#function-x430_device_flush_events) (struct [**x430\_device**](structx430__device.md) \* device) <br>_Deliver deferred events (_ X430\_EVENT\_BAUD\_RATE _) after the status packet._ |
|  int | [**x430\_device\_get**](#function-x430_device_get) (const struct [**x430\_device**](structx430__device.md) \* device, uint16\_t address, int32\_t \* value) <br>_Value of the register that starts at_ `address` _(lock-free)._ |
|  int | [**x430\_device\_id**](#function-x430_device_id) (const struct [**x430\_device**](structx430__device.md) \* device, uint8\_t \* id) <br>_Current ID (7) of the device (lock-free)._  |
|  int | [**x430\_device\_init**](#function-x430_device_init) (struct [**x430\_device**](structx430__device.md) \* device, const struct [**x430\_device\_config**](structx430__device__config.md) \* config) <br>_Initialize a device: factory values, stored EEPROM area (nvm\_load), the RAM area and the Startup Configuration (which may call the write hook)._  |
|  int | [**x430\_device\_raise\_hardware\_error**](#function-x430_device_raise_hardware_error) (struct [**x430\_device**](structx430__device.md) \* device, uint8\_t bits) <br>_Report hardware errors (Hardware Error Status bits, lock-free)._  |
|  int | [**x430\_device\_read**](#function-x430_device_read) (struct [**x430\_device**](structx430__device.md) \* device, uint16\_t address, uint16\_t length, uint8\_t \* data, uint8\_t \* error) <br>_READ semantics: read hook, Indirect Data, Realtime Tick._  |
|  int | [**x430\_device\_reboot**](#function-x430_device_reboot) (struct [**x430\_device**](structx430__device.md) \* device) <br>_REBOOT: RAM area reset, errors cleared, Startup Configuration applied, event_ X430\_EVENT\_REBOOT _._ |
|  int | [**x430\_device\_reg\_write**](#function-x430_device_reg_write) (struct [**x430\_device**](structx430__device.md) \* device, uint16\_t address, uint16\_t length, const uint8\_t \* data, uint8\_t options, uint8\_t \* error) <br>_REG\_WRITE: validate a write and register it for ACTION._  |
|  int | [**x430\_device\_set**](#function-x430_device_set) (struct [**x430\_device**](structx430__device.md) \* device, uint16\_t address, int32\_t value) <br>_Store a value into the register at_ `address` _, ignoring access rights (lock-free)._ |
|  int | [**x430\_device\_write**](#function-x430_device_write) (struct [**x430\_device**](structx430__device.md) \* device, uint16\_t address, uint16\_t length, const uint8\_t \* data, uint8\_t options, uint8\_t \* error) <br>_WRITE semantics: checks every touched register, then calls the write hook and stores in address order; stores the EEPROM area when it changed._  |
|  int | [**x430\_sim\_init**](#function-x430_sim_init) (struct [**x430\_sim**](structx430__sim.md) \* sim, struct [**x430\_device**](structx430__device.md) \* device, const struct [**x430\_sim\_config**](structx430__sim__config.md) \* config) <br>_Prepare a simulator for_ `device` _._ |
|  int | [**x430\_sim\_step**](#function-x430_sim_step) (struct [**x430\_sim**](structx430__sim.md) \* sim, uint32\_t elapsed\_ms) <br>_Advance the simulation by_ `elapsed_ms` _one-millisecond steps and refresh the live registers._ |




























## Detailed Description


**Parts.** A _bus_ ([**x430\_bus**](structx430__bus.md)) decodes the instructions of one serial line with DXL2-C and answers for its _devices_ ([**x430\_device**](structx430__device.md)). A device holds the Control Table of the model selected at build time and enforces the rules of the real actuator; the application connects it to its motor through [**x430\_hooks**](structx430__hooks.md). The optional _simulator_ ([**x430\_sim**](structx430__sim.md)) replaces the motor.


**Execution model.** Everything runs in one task context (an RX thread or the main loop), never in an interrupt: [**x430\_bus\_receive()**](group__x430.md#function-x430_bus_receive) with the bytes from the UART, [**x430\_bus\_poll()**](group__x430.md#function-x430_bus_poll) every few milliseconds (Bus Watchdog), and the live register updates of the application between [**x430\_bus\_lock()**](group__x430.md#function-x430_bus_lock) and [**x430\_bus\_unlock()**](group__x430.md#function-x430_bus_unlock). Hooks run inside [**x430\_bus\_receive()**](group__x430.md#function-x430_bus_receive) with the bus lock held.


**Platform access** is done only through [**x430\_bus\_config::write**](structx430__bus__config.md#variable-write) (the bytes of a status packet), the optional [**x430\_port**](structx430__port.md) (lock, millisecond clock, delay) and the hooks of each device.


**Initialization.** Every function except the three \*\_init() functions returns [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) for an instance without a successful initialization and calls no hook. An initialization checks every argument before it changes the instance: a refused initialization leaves the instance as it was. 


    
## Public Attributes Documentation




### variable x430\_sim\_hooks 

_Hooks implementing the simulated actuator: pass them with_ [_**x430\_device\_config::user\_data**_](structx430__device__config.md#variable-user_data) _pointing at an initialized_[_**x430\_sim**_](structx430__sim.md) _._
```
const struct x430_hooks x430_sim_hooks;
```



All members are NULL when [**X430\_FEATURE\_SIMULATOR**](group__x430__config.md#define-x430_feature_simulator) is 0. 


        

<hr>
## Public Functions Documentation




### function x430\_bus\_attach 

_Attach an initialized device to an initialized bus._ 
```
int x430_bus_attach (
    struct x430_bus * bus,
    struct x430_device * device
) 
```



Takes the bus lock. 

**Parameters:**


* `bus` Bus. 
* `device` Device, not attached to any bus. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) Attached. 
* [**X430\_ERR\_PARAM**](group__x430__defs.md#define-x430_err_param) NULL pointer. 
* [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) Bus or device not initialized, or the device is attached. 
* [**X430\_ERR\_CONFIG**](group__x430__defs.md#define-x430_err_config) Bus full ([**X430\_MAX\_DEVICES**](group__x430__config.md#define-x430_max_devices)) or the ID is used on the bus. 




        

<hr>



### function x430\_bus\_find 

_Attached device with the primary ID_ `id` _(lock-free)._
```
int x430_bus_find (
    const struct x430_bus * bus,
    uint8_t id,
    struct x430_device ** device
) 
```





**Parameters:**


* `bus` Bus. 
* `id` DYNAMIXEL ID. 
* `device` Set to the device, or to NULL when no device has that ID. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) Found. 
* [**X430\_ERR\_PARAM**](group__x430__defs.md#define-x430_err_param) NULL pointer, or no attached device has the ID. 
* [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) Bus not initialized. 




        

<hr>



### function x430\_bus\_handle\_packet 

_Handle one decoded instruction packet, for applications that run their own DXL2-C link._ 
```
int x430_bus_handle_packet (
    struct x430_bus * bus,
    const struct dxl2_packet * packet
) 
```



Takes the bus lock; runs the hooks. 

**Parameters:**


* `bus` Bus. 
* `packet` Packet decoded by DXL2-C. Status packets are ignored. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL pointer), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (bus not initialized), [**X430\_ERR\_HAL**](group__x430__defs.md#define-x430_err_hal) (the writer failed). 




        

<hr>



### function x430\_bus\_init 

_Initialize a bus._ 
```
int x430_bus_init (
    struct x430_bus * bus,
    const struct x430_bus_config * config
) 
```



Calls no callback except [**x430\_port::millis**](structx430__port.md#variable-millis). 

**Parameters:**


* `bus` Bus. 
* `config` Configuration; copied (the port it points at is not). 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) Bus ready. 
* [**X430\_ERR\_PARAM**](group__x430__defs.md#define-x430_err_param) NULL pointer or no writer. 




        

<hr>



### function x430\_bus\_lock 

_Take the bus lock (_ [_**x430\_port::lock**_](structx430__port.md#variable-lock) _; nothing without it)._
```
int x430_bus_lock (
    struct x430_bus * bus
) 
```



Use it around the live register updates of the application. 

**Parameters:**


* `bus` Bus. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_bus\_poll 

_Periodic service: Bus Watchdog of every device._ 
```
int x430_bus_poll (
    struct x430_bus * bus
) 
```



Call every few milliseconds (the watchdog unit is 20 ms). Takes the bus lock. 

**Parameters:**


* `bus` Bus. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_bus\_receive 

_Feed received bytes in any chunking; answers every complete instruction packet._ 
```
int x430_bus_receive (
    struct x430_bus * bus,
    const uint8_t * data,
    size_t length
) 
```



Takes the bus lock; runs the hooks. 

**Parameters:**


* `bus` Bus. 
* `data` Received bytes (may be NULL when `length` is 0). 
* `length` Number of bytes. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) Bytes processed. 
* [**X430\_ERR\_PARAM**](group__x430__defs.md#define-x430_err_param) NULL bus, or NULL data with a non-zero length. 
* [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) Bus not initialized. 
* [**X430\_ERR\_HAL**](group__x430__defs.md#define-x430_err_hal) The writer failed for at least one status packet. 




        

<hr>



### function x430\_bus\_unlock 

_Release the bus lock (_ [_**x430\_port::unlock**_](structx430__port.md#variable-unlock) _)._
```
int x430_bus_unlock (
    struct x430_bus * bus
) 
```





**Parameters:**


* `bus` Bus. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_device\_action 

_ACTION: execute the registered write._ 
```
int x430_device_action (
    struct x430_device * device,
    uint8_t * error
) 
```





**Parameters:**


* `device` Device. 
* `error` Protocol result; DXL2\_ERR\_INSTRUCTION when nothing is registered. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_device\_backup 

_Control Table BACKUP: store, or restore with the torque off._ 
```
int x430_device_backup (
    struct x430_device * device,
    uint8_t option,
    uint8_t * error
) 
```





**Parameters:**


* `device` Device. 
* `option` [**X430\_BACKUP\_STORE**](group__x430__defs.md#define-x430_backup_store) or [**X430\_BACKUP\_RESTORE**](group__x430__defs.md#define-x430_backup_restore). 
* `error` Protocol result. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized), [**X430\_ERR\_UNSUPPORTED**](group__x430__defs.md#define-x430_err_unsupported) ([**X430\_FEATURE\_BACKUP**](group__x430__config.md#define-x430_feature_backup) is 0). 




        

<hr>



### function x430\_device\_clear 

_CLEAR with option_ [_**X430\_CLEAR\_MULTI\_TURN**_](group__x430__defs.md#define-x430_clear_multi_turn) _(event_X430\_EVENT\_CLEAR\_MULTI\_TURN _)._
```
int x430_device_clear (
    struct x430_device * device,
    uint8_t option,
    uint8_t * error
) 
```





**Parameters:**


* `device` Device. 
* `option` [**X430\_CLEAR\_MULTI\_TURN**](group__x430__defs.md#define-x430_clear_multi_turn). 
* `error` Protocol result: DXL2\_ERR\_RESULT\_FAIL while moving, DXL2\_ERR\_DATA\_RANGE for another option. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_device\_factory\_reset 

_FACTORY\_RESET, then like REBOOT; event_ X430\_EVENT\_FACTORY\_RESET _._
```
int x430_device_factory_reset (
    struct x430_device * device,
    uint8_t option,
    uint8_t * error
) 
```





**Parameters:**


* `device` Device. 
* `option` [**X430\_FACTORY\_RESET\_ALL**](group__x430__defs.md#define-x430_factory_reset_all), [**X430\_FACTORY\_RESET\_EXCEPT\_ID**](group__x430__defs.md#define-x430_factory_reset_except_id) or [**X430\_FACTORY\_RESET\_EXCEPT\_ID\_BAUD**](group__x430__defs.md#define-x430_factory_reset_except_id_baud). 
* `error` Protocol result; DXL2\_ERR\_DATA\_RANGE for another option. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_device\_flush\_events 

_Deliver deferred events (_ X430\_EVENT\_BAUD\_RATE _) after the status packet._
```
int x430_device_flush_events (
    struct x430_device * device
) 
```



The bus calls it; call it after [**x430\_device\_write()**](group__x430.md#function-x430_device_write) of your own. 

**Parameters:**


* `device` Device. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_device\_get 

_Value of the register that starts at_ `address` _(lock-free)._
```
int x430_device_get (
    const struct x430_device * device,
    uint16_t address,
    int32_t * value
) 
```





**Parameters:**


* `device` Device. 
* `address` First address of a register. 
* `value` Sign-extended value. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) Value read. 
* [**X430\_ERR\_PARAM**](group__x430__defs.md#define-x430_err_param) NULL pointer, or no register starts at `address`. 
* [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) Device not initialized. 




        

<hr>



### function x430\_device\_id 

_Current ID (7) of the device (lock-free)._ 
```
int x430_device_id (
    const struct x430_device * device,
    uint8_t * id
) 
```





**Parameters:**


* `device` Device. 
* `id` ID. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_device\_init 

_Initialize a device: factory values, stored EEPROM area (nvm\_load), the RAM area and the Startup Configuration (which may call the write hook)._ 
```
int x430_device_init (
    struct x430_device * device,
    const struct x430_device_config * config
) 
```



Initialize a device before [**x430\_bus\_attach()**](group__x430.md#function-x430_bus_attach), never while a bus serves it: the content of the instance before the call is not trusted (it may be the garbage of a stack instance), so an attachment cannot be detected. 

**Parameters:**


* `device` Device, not attached to a bus. 
* `config` Configuration; copied (the hooks it points at are not). 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) Device ready. 
* [**X430\_ERR\_PARAM**](group__x430__defs.md#define-x430_err_param) NULL pointer or ID above 252. 




        

<hr>



### function x430\_device\_raise\_hardware\_error 

_Report hardware errors (Hardware Error Status bits, lock-free)._ 
```
int x430_device_raise_hardware_error (
    struct x430_device * device,
    uint8_t bits
) 
```



The bits stay set until REBOOT. Bits enabled in Shutdown (63) switch the torque off (event X430\_EVENT\_TORQUE\_SHUTDOWN) and keep it off until REBOOT. 

**Parameters:**


* `device` Device. 
* `bits` ::X430\_HW\_ERROR\_\* bits. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_device\_read 

_READ semantics: read hook, Indirect Data, Realtime Tick._ 
```
int x430_device_read (
    struct x430_device * device,
    uint16_t address,
    uint16_t length,
    uint8_t * data,
    uint8_t * error
) 
```





**Parameters:**


* `device` Device. 
* `address` First address. 
* `length` Bytes to read. 
* `data` Destination of `length` bytes. 
* `error` Protocol result: DXL2\_ERR\_NONE, DXL2\_ERR\_DATA\_LENGTH (length 0) or DXL2\_ERR\_ACCESS (beyond the table). 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_device\_reboot 

_REBOOT: RAM area reset, errors cleared, Startup Configuration applied, event_ X430\_EVENT\_REBOOT _._
```
int x430_device_reboot (
    struct x430_device * device
) 
```





**Parameters:**


* `device` Device. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_device\_reg\_write 

_REG\_WRITE: validate a write and register it for ACTION._ 
```
int x430_device_reg_write (
    struct x430_device * device,
    uint16_t address,
    uint16_t length,
    const uint8_t * data,
    uint8_t options,
    uint8_t * error
) 
```





**Parameters:**


* `device` Device. 
* `address` First address. 
* `length` Bytes, at most [**X430\_REG\_WRITE\_SIZE**](group__x430__config.md#define-x430_reg_write_size). 
* `data` `length` bytes. 
* `options` [**X430\_WRITE\_SECONDARY**](group__x430__defs.md#define-x430_write_secondary). 
* `error` Protocol result. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_device\_set 

_Store a value into the register at_ `address` _, ignoring access rights (lock-free)._
```
int x430_device_set (
    struct x430_device * device,
    uint16_t address,
    int32_t value
) 
```



For live registers (Present Position, Moving, ...): no hook is called, no range is checked. 

**Parameters:**


* `device` Device. 
* `address` First address of a register. 
* `value` Value; truncated to the register size. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) Value stored. 
* [**X430\_ERR\_PARAM**](group__x430__defs.md#define-x430_err_param) NULL pointer, or no register starts at `address`. 
* [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) Device not initialized. 




        

<hr>



### function x430\_device\_write 

_WRITE semantics: checks every touched register, then calls the write hook and stores in address order; stores the EEPROM area when it changed._ 
```
int x430_device_write (
    struct x430_device * device,
    uint16_t address,
    uint16_t length,
    const uint8_t * data,
    uint8_t options,
    uint8_t * error
) 
```





**Parameters:**


* `device` Device. 
* `address` First address. 
* `length` Bytes to write. 
* `data` `length` bytes. 
* `options` [**X430\_WRITE\_SECONDARY**](group__x430__defs.md#define-x430_write_secondary), [**X430\_WRITE\_VALIDATE\_ONLY**](group__x430__defs.md#define-x430_write_validate_only). 
* `error` Protocol result: DXL2\_ERR\_NONE or the DXL2\_ERR\_\* of the first failure. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (not initialized). 




        

<hr>



### function x430\_sim\_init 

_Prepare a simulator for_ `device` _._
```
int x430_sim_init (
    struct x430_sim * sim,
    struct x430_device * device,
    const struct x430_sim_config * config
) 
```



Call it before [**x430\_device\_init()**](group__x430.md#function-x430_device_init) of a device whose hooks are [**x430\_sim\_hooks**](group__x430.md#variable-x430_sim_hooks) and whose user data is `sim`. 

**Parameters:**


* `sim` Simulator. 
* `device` Device it simulates (may be initialized later). 
* `config` Configuration, NULL for the defaults. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL sim or device), [**X430\_ERR\_UNSUPPORTED**](group__x430__defs.md#define-x430_err_unsupported) ([**X430\_FEATURE\_SIMULATOR**](group__x430__config.md#define-x430_feature_simulator) is 0). 




        

<hr>



### function x430\_sim\_step 

_Advance the simulation by_ `elapsed_ms` _one-millisecond steps and refresh the live registers._
```
int x430_sim_step (
    struct x430_sim * sim,
    uint32_t elapsed_ms
) 
```



Takes the bus lock of the device when it is attached. The time is proportional to `elapsed_ms:` call it periodically. 

**Parameters:**


* `sim` Simulator. 
* `elapsed_ms` Time since the previous call [ms]. 



**Return value:**


* [**X430\_OK**](group__x430__defs.md#define-x430_ok) (NULL), [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state) (simulator or device not initialized), [**X430\_ERR\_UNSUPPORTED**](group__x430__defs.md#define-x430_err_unsupported) ([**X430\_FEATURE\_SIMULATOR**](group__x430__config.md#define-x430_feature_simulator) is 0). 




        

<hr>

------------------------------


