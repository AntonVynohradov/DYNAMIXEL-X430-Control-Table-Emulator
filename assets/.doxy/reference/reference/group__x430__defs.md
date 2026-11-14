

# Group x430\_defs



[**Modules**](modules.md) **>** [**x430\_defs**](group__x430__defs.md)



_Status codes, options, events and the addresses, operating modes and bit fields of the X430 Control Table._ [More...](#detailed-description)


















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**x430\_event**](#enum-x430_event)  <br>_Notifications from a device to the application (_ [_**x430\_hooks::event**_](structx430__hooks.md#variable-event) _)._ |
| enum  | [**x430\_nvm\_slot**](#enum-x430_nvm_slot)  <br>_Persistent storage slots (_ [_**x430\_hooks::nvm\_read**_](structx430__hooks.md#variable-nvm_read) _,_[_**x430\_hooks::nvm\_write**_](structx430__hooks.md#variable-nvm_write) _)._ |















































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**X430\_ADDR\_BACKUP\_READY**](group__x430__defs.md#define-x430_addr_backup_ready)  `147U`<br> |
| define  | [**X430\_ADDR\_BAUD\_RATE**](group__x430__defs.md#define-x430_addr_baud_rate)  `8U`<br> |
| define  | [**X430\_ADDR\_BUS\_WATCHDOG**](group__x430__defs.md#define-x430_addr_bus_watchdog)  `98U`<br> |
| define  | [**X430\_ADDR\_CURRENT\_LIMIT**](group__x430__defs.md#define-x430_addr_current_limit)  `38U`<br> |
| define  | [**X430\_ADDR\_DRIVE\_MODE**](group__x430__defs.md#define-x430_addr_drive_mode)  `10U`<br> |
| define  | [**X430\_ADDR\_FEEDFORWARD\_1ST\_GAIN**](group__x430__defs.md#define-x430_addr_feedforward_1st_gain)  `90U`<br> |
| define  | [**X430\_ADDR\_FEEDFORWARD\_2ND\_GAIN**](group__x430__defs.md#define-x430_addr_feedforward_2nd_gain)  `88U`<br> |
| define  | [**X430\_ADDR\_FIRMWARE\_VERSION**](group__x430__defs.md#define-x430_addr_firmware_version)  `6U`<br> |
| define  | [**X430\_ADDR\_GOAL\_CURRENT**](group__x430__defs.md#define-x430_addr_goal_current)  `102U`<br> |
| define  | [**X430\_ADDR\_GOAL\_POSITION**](group__x430__defs.md#define-x430_addr_goal_position)  `116U`<br> |
| define  | [**X430\_ADDR\_GOAL\_PWM**](group__x430__defs.md#define-x430_addr_goal_pwm)  `100U`<br> |
| define  | [**X430\_ADDR\_GOAL\_VELOCITY**](group__x430__defs.md#define-x430_addr_goal_velocity)  `104U`<br> |
| define  | [**X430\_ADDR\_HARDWARE\_ERROR\_STATUS**](group__x430__defs.md#define-x430_addr_hardware_error_status)  `70U`<br> |
| define  | [**X430\_ADDR\_HOMING\_OFFSET**](group__x430__defs.md#define-x430_addr_homing_offset)  `20U`<br> |
| define  | [**X430\_ADDR\_ID**](group__x430__defs.md#define-x430_addr_id)  `7U`<br> |
| define  | [**X430\_ADDR\_LED**](group__x430__defs.md#define-x430_addr_led)  `65U`<br> |
| define  | [**X430\_ADDR\_MAX\_POSITION\_LIMIT**](group__x430__defs.md#define-x430_addr_max_position_limit)  `48U`<br> |
| define  | [**X430\_ADDR\_MAX\_VOLTAGE\_LIMIT**](group__x430__defs.md#define-x430_addr_max_voltage_limit)  `32U`<br> |
| define  | [**X430\_ADDR\_MIN\_POSITION\_LIMIT**](group__x430__defs.md#define-x430_addr_min_position_limit)  `52U`<br> |
| define  | [**X430\_ADDR\_MIN\_VOLTAGE\_LIMIT**](group__x430__defs.md#define-x430_addr_min_voltage_limit)  `34U`<br> |
| define  | [**X430\_ADDR\_MODEL\_INFORMATION**](group__x430__defs.md#define-x430_addr_model_information)  `2U`<br> |
| define  | [**X430\_ADDR\_MODEL\_NUMBER**](group__x430__defs.md#define-x430_addr_model_number)  `0U`<br> |
| define  | [**X430\_ADDR\_MOVING**](group__x430__defs.md#define-x430_addr_moving)  `122U`<br> |
| define  | [**X430\_ADDR\_MOVING\_STATUS**](group__x430__defs.md#define-x430_addr_moving_status)  `123U`<br> |
| define  | [**X430\_ADDR\_MOVING\_THRESHOLD**](group__x430__defs.md#define-x430_addr_moving_threshold)  `24U`<br> |
| define  | [**X430\_ADDR\_OPERATING\_MODE**](group__x430__defs.md#define-x430_addr_operating_mode)  `11U`<br> |
| define  | [**X430\_ADDR\_POSITION\_D\_GAIN**](group__x430__defs.md#define-x430_addr_position_d_gain)  `80U`<br> |
| define  | [**X430\_ADDR\_POSITION\_I\_GAIN**](group__x430__defs.md#define-x430_addr_position_i_gain)  `82U`<br> |
| define  | [**X430\_ADDR\_POSITION\_P\_GAIN**](group__x430__defs.md#define-x430_addr_position_p_gain)  `84U`<br> |
| define  | [**X430\_ADDR\_POSITION\_TRAJECTORY**](group__x430__defs.md#define-x430_addr_position_trajectory)  `140U`<br> |
| define  | [**X430\_ADDR\_PRESENT\_CURRENT**](group__x430__defs.md#define-x430_addr_present_current)  `126U`<br>_Present Current (XM/XH/XD/XW) or Present Load (XL/XC), same address._  |
| define  | [**X430\_ADDR\_PRESENT\_INPUT\_VOLTAGE**](group__x430__defs.md#define-x430_addr_present_input_voltage)  `144U`<br> |
| define  | [**X430\_ADDR\_PRESENT\_LOAD**](group__x430__defs.md#define-x430_addr_present_load)  `126U`<br> |
| define  | [**X430\_ADDR\_PRESENT\_POSITION**](group__x430__defs.md#define-x430_addr_present_position)  `132U`<br> |
| define  | [**X430\_ADDR\_PRESENT\_PWM**](group__x430__defs.md#define-x430_addr_present_pwm)  `124U`<br> |
| define  | [**X430\_ADDR\_PRESENT\_TEMPERATURE**](group__x430__defs.md#define-x430_addr_present_temperature)  `146U`<br> |
| define  | [**X430\_ADDR\_PRESENT\_VELOCITY**](group__x430__defs.md#define-x430_addr_present_velocity)  `128U`<br> |
| define  | [**X430\_ADDR\_PROFILE\_ACCELERATION**](group__x430__defs.md#define-x430_addr_profile_acceleration)  `108U`<br> |
| define  | [**X430\_ADDR\_PROFILE\_VELOCITY**](group__x430__defs.md#define-x430_addr_profile_velocity)  `112U`<br> |
| define  | [**X430\_ADDR\_PROTOCOL\_TYPE**](group__x430__defs.md#define-x430_addr_protocol_type)  `13U`<br> |
| define  | [**X430\_ADDR\_PWM\_LIMIT**](group__x430__defs.md#define-x430_addr_pwm_limit)  `36U`<br> |
| define  | [**X430\_ADDR\_REALTIME\_TICK**](group__x430__defs.md#define-x430_addr_realtime_tick)  `120U`<br> |
| define  | [**X430\_ADDR\_REGISTERED\_INSTRUCTION**](group__x430__defs.md#define-x430_addr_registered_instruction)  `69U`<br> |
| define  | [**X430\_ADDR\_RETURN\_DELAY\_TIME**](group__x430__defs.md#define-x430_addr_return_delay_time)  `9U`<br> |
| define  | [**X430\_ADDR\_SECONDARY\_ID**](group__x430__defs.md#define-x430_addr_secondary_id)  `12U`<br> |
| define  | [**X430\_ADDR\_SHUTDOWN**](group__x430__defs.md#define-x430_addr_shutdown)  `63U`<br> |
| define  | [**X430\_ADDR\_STARTUP\_CONFIGURATION**](group__x430__defs.md#define-x430_addr_startup_configuration)  `60U`<br> |
| define  | [**X430\_ADDR\_STATUS\_RETURN\_LEVEL**](group__x430__defs.md#define-x430_addr_status_return_level)  `68U`<br> |
| define  | [**X430\_ADDR\_TEMPERATURE\_LIMIT**](group__x430__defs.md#define-x430_addr_temperature_limit)  `31U`<br> |
| define  | [**X430\_ADDR\_TORQUE\_ENABLE**](group__x430__defs.md#define-x430_addr_torque_enable)  `64U`<br> |
| define  | [**X430\_ADDR\_VELOCITY\_I\_GAIN**](group__x430__defs.md#define-x430_addr_velocity_i_gain)  `76U`<br> |
| define  | [**X430\_ADDR\_VELOCITY\_LIMIT**](group__x430__defs.md#define-x430_addr_velocity_limit)  `44U`<br> |
| define  | [**X430\_ADDR\_VELOCITY\_P\_GAIN**](group__x430__defs.md#define-x430_addr_velocity_p_gain)  `78U`<br> |
| define  | [**X430\_ADDR\_VELOCITY\_TRAJECTORY**](group__x430__defs.md#define-x430_addr_velocity_trajectory)  `136U`<br> |
| define  | [**X430\_BACKUP\_RESTORE**](group__x430__defs.md#define-x430_backup_restore)  `0x02U`<br>_BACKUP option: restore the Control Table._  |
| define  | [**X430\_BACKUP\_STORE**](group__x430__defs.md#define-x430_backup_store)  `0x01U`<br>_BACKUP option: store the Control Table._  |
| define  | [**X430\_BUS\_WATCHDOG\_ERROR**](group__x430__defs.md#define-x430_bus_watchdog_error)  `0xFFU`<br>_Bus Watchdog (98) value while a Bus Watchdog error is active (-1)._  |
| define  | [**X430\_BUS\_WATCHDOG\_UNIT\_MS**](group__x430__defs.md#define-x430_bus_watchdog_unit_ms)  `20U`<br>_Bus Watchdog (98) time unit in milliseconds._  |
| define  | [**X430\_CLEAR\_MULTI\_TURN**](group__x430__defs.md#define-x430_clear_multi_turn)  `0x01U`<br>_CLEAR option: clear the multi-turn information of Present Position._  |
| define  | [**X430\_DRIVE\_MODE\_REVERSE**](group__x430__defs.md#define-x430_drive_mode_reverse)  `0x01U`<br>_Reverse mode: positive goals turn clockwise._  |
| define  | [**X430\_DRIVE\_MODE\_TIME\_BASED**](group__x430__defs.md#define-x430_drive_mode_time_based)  `0x04U`<br>_Time-based profile: Profile Velocity/Acceleration are times in ms._  |
| define  | [**X430\_DRIVE\_MODE\_TORQUE\_ON\_BY\_GOAL**](group__x430__defs.md#define-x430_drive_mode_torque_on_by_goal)  `0x08U`<br>_Torque On by Goal Update: writing a goal value enables torque._  |
| define  | [**X430\_ERR\_CONFIG**](group__x430__defs.md#define-x430_err_config)  `(-3)`<br>_Values that are valid alone but not together (bus full, ID already used on the bus)._  |
| define  | [**X430\_ERR\_HAL**](group__x430__defs.md#define-x430_err_hal)  `(-4)`<br>_The byte writer of the bus (_ [_**x430\_bus\_config::write**_](structx430__bus__config.md#variable-write) _) returned an error._ |
| define  | [**X430\_ERR\_PARAM**](group__x430__defs.md#define-x430_err_param)  `(-1)`<br>_NULL pointer or a value outside its documented range._  |
| define  | [**X430\_ERR\_STATE**](group__x430__defs.md#define-x430_err_state)  `(-2)`<br>_Call not allowed in the current state (instance not initialized, device attached)._  |
| define  | [**X430\_ERR\_UNSUPPORTED**](group__x430__defs.md#define-x430_err_unsupported)  `(-5)`<br>_The function belongs to a feature that is not compiled in (X430\_FEATURE\_\*)._  |
| define  | [**X430\_EXTENDED\_POSITION\_LIMIT**](group__x430__defs.md#define-x430_extended_position_limit)  `1048575`<br>_Goal Position range outside Position Control Mode._  |
| define  | [**X430\_FACTORY\_RESET\_ALL**](group__x430__defs.md#define-x430_factory_reset_all)  `0xFFU`<br>_FACTORY\_RESET option: reset everything (refused on the broadcast ID)._  |
| define  | [**X430\_FACTORY\_RESET\_EXCEPT\_ID**](group__x430__defs.md#define-x430_factory_reset_except_id)  `0x01U`<br>_FACTORY\_RESET option: keep the ID._  |
| define  | [**X430\_FACTORY\_RESET\_EXCEPT\_ID\_BAUD**](group__x430__defs.md#define-x430_factory_reset_except_id_baud)  `0x02U`<br>_FACTORY\_RESET option: keep the ID and the Baud Rate._  |
| define  | [**X430\_HW\_ERROR\_ELECTRICAL\_SHOCK**](group__x430__defs.md#define-x430_hw_error_electrical_shock)  `0x10U`<br> |
| define  | [**X430\_HW\_ERROR\_INPUT\_VOLTAGE**](group__x430__defs.md#define-x430_hw_error_input_voltage)  `0x01U`<br> |
| define  | [**X430\_HW\_ERROR\_MOTOR\_ENCODER**](group__x430__defs.md#define-x430_hw_error_motor_encoder)  `0x08U`<br> |
| define  | [**X430\_HW\_ERROR\_OVERHEATING**](group__x430__defs.md#define-x430_hw_error_overheating)  `0x04U`<br> |
| define  | [**X430\_HW\_ERROR\_OVERLOAD**](group__x430__defs.md#define-x430_hw_error_overload)  `0x20U`<br> |
| define  | [**X430\_MODE\_CURRENT**](group__x430__defs.md#define-x430_mode_current)  `0U`<br> |
| define  | [**X430\_MODE\_CURRENT\_BASED\_POSITION**](group__x430__defs.md#define-x430_mode_current_based_position)  `5U`<br> |
| define  | [**X430\_MODE\_EXTENDED\_POSITION**](group__x430__defs.md#define-x430_mode_extended_position)  `4U`<br> |
| define  | [**X430\_MODE\_POSITION**](group__x430__defs.md#define-x430_mode_position)  `3U`<br> |
| define  | [**X430\_MODE\_PWM**](group__x430__defs.md#define-x430_mode_pwm)  `16U`<br> |
| define  | [**X430\_MODE\_VELOCITY**](group__x430__defs.md#define-x430_mode_velocity)  `1U`<br> |
| define  | [**X430\_MOVING\_STATUS\_FOLLOWING\_ERROR**](group__x430__defs.md#define-x430_moving_status_following_error)  `0x08U`<br> |
| define  | [**X430\_MOVING\_STATUS\_IN\_POSITION**](group__x430__defs.md#define-x430_moving_status_in_position)  `0x01U`<br> |
| define  | [**X430\_MOVING\_STATUS\_PROFILE\_ONGOING**](group__x430__defs.md#define-x430_moving_status_profile_ongoing)  `0x02U`<br> |
| define  | [**X430\_MOVING\_STATUS\_PROFILE\_RECTANGULAR**](group__x430__defs.md#define-x430_moving_status_profile_rectangular)  `0x10U`<br> |
| define  | [**X430\_MOVING\_STATUS\_PROFILE\_STEP**](group__x430__defs.md#define-x430_moving_status_profile_step)  `0x00U`<br>_Velocity profile type, bits 4-5._  |
| define  | [**X430\_MOVING\_STATUS\_PROFILE\_TRAPEZOIDAL**](group__x430__defs.md#define-x430_moving_status_profile_trapezoidal)  `0x30U`<br> |
| define  | [**X430\_MOVING\_STATUS\_PROFILE\_TRIANGULAR**](group__x430__defs.md#define-x430_moving_status_profile_triangular)  `0x20U`<br> |
| define  | [**X430\_NVM\_HEADER\_SIZE**](group__x430__defs.md#define-x430_nvm_header_size)  `10U`<br>_Size of the persistent block header: magic, model, format, slot, payload size._  |
| define  | [**X430\_OK**](group__x430__defs.md#define-x430_ok)  `0`<br>_Success._  |
| define  | [**X430\_RAM\_START**](group__x430__defs.md#define-x430_ram_start)  `64U`<br>_First address of the RAM area; addresses below belong to the EEPROM area._  |
| define  | [**X430\_REALTIME\_TICK\_PERIOD**](group__x430__defs.md#define-x430_realtime_tick_period)  `32768U`<br>_Realtime Tick (120) wraps at this value, in ms._  |
| define  | [**X430\_RETURN\_DELAY\_UNIT\_US**](group__x430__defs.md#define-x430_return_delay_unit_us)  `2U`<br>_Return Delay Time (9) unit in microseconds._  |
| define  | [**X430\_SECONDARY\_ID\_DISABLED**](group__x430__defs.md#define-x430_secondary_id_disabled)  `253U`<br>_Secondary ID (12) values from 253 disable the Secondary ID._  |
| define  | [**X430\_STARTUP\_RAM\_RESTORE**](group__x430__defs.md#define-x430_startup_ram_restore)  `0x02U`<br> |
| define  | [**X430\_STARTUP\_TORQUE\_ON**](group__x430__defs.md#define-x430_startup_torque_on)  `0x01U`<br> |
| define  | [**X430\_WRITE\_SECONDARY**](group__x430__defs.md#define-x430_write_secondary)  `0x01U`<br>_Packet addressed by the Secondary ID: the EEPROM area is locked._  |
| define  | [**X430\_WRITE\_VALIDATE\_ONLY**](group__x430__defs.md#define-x430_write_validate_only)  `0x02U`<br>_Validate only: change nothing and call no hook._  |

## Detailed Description


Control Table values come from the ROBOTIS e-Manual of the X430 models ([https://emanual.robotis.com/docs/en/dxl/x/](https://emanual.robotis.com/docs/en/dxl/x/)). Which registers a model really has is decided by its generated table: XL/XC models have no Current Limit (38) and Goal Current (102) and report Present Load at 126; XW models have no LED (65). 


    
## Public Types Documentation




### enum x430\_event 

_Notifications from a device to the application (_ [_**x430\_hooks::event**_](structx430__hooks.md#variable-event) _)._
```
enum x430_event {
    X430_EVENT_REBOOT = 0,
    X430_EVENT_FACTORY_RESET,
    X430_EVENT_CLEAR_MULTI_TURN,
    X430_EVENT_BUS_WATCHDOG,
    X430_EVENT_BAUD_RATE,
    X430_EVENT_TORQUE_SHUTDOWN,
    X430_EVENT_BACKUP_RESTORED
};
```




<hr>



### enum x430\_nvm\_slot 

_Persistent storage slots (_ [_**x430\_hooks::nvm\_read**_](structx430__hooks.md#variable-nvm_read) _,_[_**x430\_hooks::nvm\_write**_](structx430__hooks.md#variable-nvm_write) _)._
```
enum x430_nvm_slot {
    X430_NVM_EEPROM = 0,
    X430_NVM_BACKUP = 1
};
```




<hr>
## Macro Definition Documentation





### define X430\_ADDR\_BACKUP\_READY 

```
#define X430_ADDR_BACKUP_READY `147U`
```




<hr>



### define X430\_ADDR\_BAUD\_RATE 

```
#define X430_ADDR_BAUD_RATE `8U`
```




<hr>



### define X430\_ADDR\_BUS\_WATCHDOG 

```
#define X430_ADDR_BUS_WATCHDOG `98U`
```




<hr>



### define X430\_ADDR\_CURRENT\_LIMIT 

```
#define X430_ADDR_CURRENT_LIMIT `38U`
```




<hr>



### define X430\_ADDR\_DRIVE\_MODE 

```
#define X430_ADDR_DRIVE_MODE `10U`
```




<hr>



### define X430\_ADDR\_FEEDFORWARD\_1ST\_GAIN 

```
#define X430_ADDR_FEEDFORWARD_1ST_GAIN `90U`
```




<hr>



### define X430\_ADDR\_FEEDFORWARD\_2ND\_GAIN 

```
#define X430_ADDR_FEEDFORWARD_2ND_GAIN `88U`
```




<hr>



### define X430\_ADDR\_FIRMWARE\_VERSION 

```
#define X430_ADDR_FIRMWARE_VERSION `6U`
```




<hr>



### define X430\_ADDR\_GOAL\_CURRENT 

```
#define X430_ADDR_GOAL_CURRENT `102U`
```




<hr>



### define X430\_ADDR\_GOAL\_POSITION 

```
#define X430_ADDR_GOAL_POSITION `116U`
```




<hr>



### define X430\_ADDR\_GOAL\_PWM 

```
#define X430_ADDR_GOAL_PWM `100U`
```




<hr>



### define X430\_ADDR\_GOAL\_VELOCITY 

```
#define X430_ADDR_GOAL_VELOCITY `104U`
```




<hr>



### define X430\_ADDR\_HARDWARE\_ERROR\_STATUS 

```
#define X430_ADDR_HARDWARE_ERROR_STATUS `70U`
```




<hr>



### define X430\_ADDR\_HOMING\_OFFSET 

```
#define X430_ADDR_HOMING_OFFSET `20U`
```




<hr>



### define X430\_ADDR\_ID 

```
#define X430_ADDR_ID `7U`
```




<hr>



### define X430\_ADDR\_LED 

```
#define X430_ADDR_LED `65U`
```




<hr>



### define X430\_ADDR\_MAX\_POSITION\_LIMIT 

```
#define X430_ADDR_MAX_POSITION_LIMIT `48U`
```




<hr>



### define X430\_ADDR\_MAX\_VOLTAGE\_LIMIT 

```
#define X430_ADDR_MAX_VOLTAGE_LIMIT `32U`
```




<hr>



### define X430\_ADDR\_MIN\_POSITION\_LIMIT 

```
#define X430_ADDR_MIN_POSITION_LIMIT `52U`
```




<hr>



### define X430\_ADDR\_MIN\_VOLTAGE\_LIMIT 

```
#define X430_ADDR_MIN_VOLTAGE_LIMIT `34U`
```




<hr>



### define X430\_ADDR\_MODEL\_INFORMATION 

```
#define X430_ADDR_MODEL_INFORMATION `2U`
```




<hr>



### define X430\_ADDR\_MODEL\_NUMBER 

```
#define X430_ADDR_MODEL_NUMBER `0U`
```




<hr>



### define X430\_ADDR\_MOVING 

```
#define X430_ADDR_MOVING `122U`
```




<hr>



### define X430\_ADDR\_MOVING\_STATUS 

```
#define X430_ADDR_MOVING_STATUS `123U`
```




<hr>



### define X430\_ADDR\_MOVING\_THRESHOLD 

```
#define X430_ADDR_MOVING_THRESHOLD `24U`
```




<hr>



### define X430\_ADDR\_OPERATING\_MODE 

```
#define X430_ADDR_OPERATING_MODE `11U`
```




<hr>



### define X430\_ADDR\_POSITION\_D\_GAIN 

```
#define X430_ADDR_POSITION_D_GAIN `80U`
```




<hr>



### define X430\_ADDR\_POSITION\_I\_GAIN 

```
#define X430_ADDR_POSITION_I_GAIN `82U`
```




<hr>



### define X430\_ADDR\_POSITION\_P\_GAIN 

```
#define X430_ADDR_POSITION_P_GAIN `84U`
```




<hr>



### define X430\_ADDR\_POSITION\_TRAJECTORY 

```
#define X430_ADDR_POSITION_TRAJECTORY `140U`
```




<hr>



### define X430\_ADDR\_PRESENT\_CURRENT 

_Present Current (XM/XH/XD/XW) or Present Load (XL/XC), same address._ 
```
#define X430_ADDR_PRESENT_CURRENT `126U`
```




<hr>



### define X430\_ADDR\_PRESENT\_INPUT\_VOLTAGE 

```
#define X430_ADDR_PRESENT_INPUT_VOLTAGE `144U`
```




<hr>



### define X430\_ADDR\_PRESENT\_LOAD 

```
#define X430_ADDR_PRESENT_LOAD `126U`
```




<hr>



### define X430\_ADDR\_PRESENT\_POSITION 

```
#define X430_ADDR_PRESENT_POSITION `132U`
```




<hr>



### define X430\_ADDR\_PRESENT\_PWM 

```
#define X430_ADDR_PRESENT_PWM `124U`
```




<hr>



### define X430\_ADDR\_PRESENT\_TEMPERATURE 

```
#define X430_ADDR_PRESENT_TEMPERATURE `146U`
```




<hr>



### define X430\_ADDR\_PRESENT\_VELOCITY 

```
#define X430_ADDR_PRESENT_VELOCITY `128U`
```




<hr>



### define X430\_ADDR\_PROFILE\_ACCELERATION 

```
#define X430_ADDR_PROFILE_ACCELERATION `108U`
```




<hr>



### define X430\_ADDR\_PROFILE\_VELOCITY 

```
#define X430_ADDR_PROFILE_VELOCITY `112U`
```




<hr>



### define X430\_ADDR\_PROTOCOL\_TYPE 

```
#define X430_ADDR_PROTOCOL_TYPE `13U`
```




<hr>



### define X430\_ADDR\_PWM\_LIMIT 

```
#define X430_ADDR_PWM_LIMIT `36U`
```




<hr>



### define X430\_ADDR\_REALTIME\_TICK 

```
#define X430_ADDR_REALTIME_TICK `120U`
```




<hr>



### define X430\_ADDR\_REGISTERED\_INSTRUCTION 

```
#define X430_ADDR_REGISTERED_INSTRUCTION `69U`
```




<hr>



### define X430\_ADDR\_RETURN\_DELAY\_TIME 

```
#define X430_ADDR_RETURN_DELAY_TIME `9U`
```




<hr>



### define X430\_ADDR\_SECONDARY\_ID 

```
#define X430_ADDR_SECONDARY_ID `12U`
```




<hr>



### define X430\_ADDR\_SHUTDOWN 

```
#define X430_ADDR_SHUTDOWN `63U`
```




<hr>



### define X430\_ADDR\_STARTUP\_CONFIGURATION 

```
#define X430_ADDR_STARTUP_CONFIGURATION `60U`
```




<hr>



### define X430\_ADDR\_STATUS\_RETURN\_LEVEL 

```
#define X430_ADDR_STATUS_RETURN_LEVEL `68U`
```




<hr>



### define X430\_ADDR\_TEMPERATURE\_LIMIT 

```
#define X430_ADDR_TEMPERATURE_LIMIT `31U`
```




<hr>



### define X430\_ADDR\_TORQUE\_ENABLE 

```
#define X430_ADDR_TORQUE_ENABLE `64U`
```




<hr>



### define X430\_ADDR\_VELOCITY\_I\_GAIN 

```
#define X430_ADDR_VELOCITY_I_GAIN `76U`
```




<hr>



### define X430\_ADDR\_VELOCITY\_LIMIT 

```
#define X430_ADDR_VELOCITY_LIMIT `44U`
```




<hr>



### define X430\_ADDR\_VELOCITY\_P\_GAIN 

```
#define X430_ADDR_VELOCITY_P_GAIN `78U`
```




<hr>



### define X430\_ADDR\_VELOCITY\_TRAJECTORY 

```
#define X430_ADDR_VELOCITY_TRAJECTORY `136U`
```




<hr>



### define X430\_BACKUP\_RESTORE 

_BACKUP option: restore the Control Table._ 
```
#define X430_BACKUP_RESTORE `0x02U`
```




<hr>



### define X430\_BACKUP\_STORE 

_BACKUP option: store the Control Table._ 
```
#define X430_BACKUP_STORE `0x01U`
```




<hr>



### define X430\_BUS\_WATCHDOG\_ERROR 

_Bus Watchdog (98) value while a Bus Watchdog error is active (-1)._ 
```
#define X430_BUS_WATCHDOG_ERROR `0xFFU`
```




<hr>



### define X430\_BUS\_WATCHDOG\_UNIT\_MS 

_Bus Watchdog (98) time unit in milliseconds._ 
```
#define X430_BUS_WATCHDOG_UNIT_MS `20U`
```




<hr>



### define X430\_CLEAR\_MULTI\_TURN 

_CLEAR option: clear the multi-turn information of Present Position._ 
```
#define X430_CLEAR_MULTI_TURN `0x01U`
```




<hr>



### define X430\_DRIVE\_MODE\_REVERSE 

_Reverse mode: positive goals turn clockwise._ 
```
#define X430_DRIVE_MODE_REVERSE `0x01U`
```




<hr>



### define X430\_DRIVE\_MODE\_TIME\_BASED 

_Time-based profile: Profile Velocity/Acceleration are times in ms._ 
```
#define X430_DRIVE_MODE_TIME_BASED `0x04U`
```




<hr>



### define X430\_DRIVE\_MODE\_TORQUE\_ON\_BY\_GOAL 

_Torque On by Goal Update: writing a goal value enables torque._ 
```
#define X430_DRIVE_MODE_TORQUE_ON_BY_GOAL `0x08U`
```




<hr>



### define X430\_ERR\_CONFIG 

_Values that are valid alone but not together (bus full, ID already used on the bus)._ 
```
#define X430_ERR_CONFIG `(-3)`
```




<hr>



### define X430\_ERR\_HAL 

_The byte writer of the bus (_ [_**x430\_bus\_config::write**_](structx430__bus__config.md#variable-write) _) returned an error._
```
#define X430_ERR_HAL `(-4)`
```




<hr>



### define X430\_ERR\_PARAM 

_NULL pointer or a value outside its documented range._ 
```
#define X430_ERR_PARAM `(-1)`
```




<hr>



### define X430\_ERR\_STATE 

_Call not allowed in the current state (instance not initialized, device attached)._ 
```
#define X430_ERR_STATE `(-2)`
```




<hr>



### define X430\_ERR\_UNSUPPORTED 

_The function belongs to a feature that is not compiled in (X430\_FEATURE\_\*)._ 
```
#define X430_ERR_UNSUPPORTED `(-5)`
```




<hr>



### define X430\_EXTENDED\_POSITION\_LIMIT 

_Goal Position range outside Position Control Mode._ 
```
#define X430_EXTENDED_POSITION_LIMIT `1048575`
```




<hr>



### define X430\_FACTORY\_RESET\_ALL 

_FACTORY\_RESET option: reset everything (refused on the broadcast ID)._ 
```
#define X430_FACTORY_RESET_ALL `0xFFU`
```




<hr>



### define X430\_FACTORY\_RESET\_EXCEPT\_ID 

_FACTORY\_RESET option: keep the ID._ 
```
#define X430_FACTORY_RESET_EXCEPT_ID `0x01U`
```




<hr>



### define X430\_FACTORY\_RESET\_EXCEPT\_ID\_BAUD 

_FACTORY\_RESET option: keep the ID and the Baud Rate._ 
```
#define X430_FACTORY_RESET_EXCEPT_ID_BAUD `0x02U`
```




<hr>



### define X430\_HW\_ERROR\_ELECTRICAL\_SHOCK 

```
#define X430_HW_ERROR_ELECTRICAL_SHOCK `0x10U`
```




<hr>



### define X430\_HW\_ERROR\_INPUT\_VOLTAGE 

```
#define X430_HW_ERROR_INPUT_VOLTAGE `0x01U`
```




<hr>



### define X430\_HW\_ERROR\_MOTOR\_ENCODER 

```
#define X430_HW_ERROR_MOTOR_ENCODER `0x08U`
```




<hr>



### define X430\_HW\_ERROR\_OVERHEATING 

```
#define X430_HW_ERROR_OVERHEATING `0x04U`
```




<hr>



### define X430\_HW\_ERROR\_OVERLOAD 

```
#define X430_HW_ERROR_OVERLOAD `0x20U`
```




<hr>



### define X430\_MODE\_CURRENT 

```
#define X430_MODE_CURRENT `0U`
```




<hr>



### define X430\_MODE\_CURRENT\_BASED\_POSITION 

```
#define X430_MODE_CURRENT_BASED_POSITION `5U`
```




<hr>



### define X430\_MODE\_EXTENDED\_POSITION 

```
#define X430_MODE_EXTENDED_POSITION `4U`
```




<hr>



### define X430\_MODE\_POSITION 

```
#define X430_MODE_POSITION `3U`
```




<hr>



### define X430\_MODE\_PWM 

```
#define X430_MODE_PWM `16U`
```




<hr>



### define X430\_MODE\_VELOCITY 

```
#define X430_MODE_VELOCITY `1U`
```




<hr>



### define X430\_MOVING\_STATUS\_FOLLOWING\_ERROR 

```
#define X430_MOVING_STATUS_FOLLOWING_ERROR `0x08U`
```




<hr>



### define X430\_MOVING\_STATUS\_IN\_POSITION 

```
#define X430_MOVING_STATUS_IN_POSITION `0x01U`
```




<hr>



### define X430\_MOVING\_STATUS\_PROFILE\_ONGOING 

```
#define X430_MOVING_STATUS_PROFILE_ONGOING `0x02U`
```




<hr>



### define X430\_MOVING\_STATUS\_PROFILE\_RECTANGULAR 

```
#define X430_MOVING_STATUS_PROFILE_RECTANGULAR `0x10U`
```




<hr>



### define X430\_MOVING\_STATUS\_PROFILE\_STEP 

_Velocity profile type, bits 4-5._ 
```
#define X430_MOVING_STATUS_PROFILE_STEP `0x00U`
```




<hr>



### define X430\_MOVING\_STATUS\_PROFILE\_TRAPEZOIDAL 

```
#define X430_MOVING_STATUS_PROFILE_TRAPEZOIDAL `0x30U`
```




<hr>



### define X430\_MOVING\_STATUS\_PROFILE\_TRIANGULAR 

```
#define X430_MOVING_STATUS_PROFILE_TRIANGULAR `0x20U`
```




<hr>



### define X430\_NVM\_HEADER\_SIZE 

_Size of the persistent block header: magic, model, format, slot, payload size._ 
```
#define X430_NVM_HEADER_SIZE `10U`
```




<hr>



### define X430\_OK 

_Success._ 
```
#define X430_OK `0`
```




<hr>



### define X430\_RAM\_START 

_First address of the RAM area; addresses below belong to the EEPROM area._ 
```
#define X430_RAM_START `64U`
```




<hr>



### define X430\_REALTIME\_TICK\_PERIOD 

_Realtime Tick (120) wraps at this value, in ms._ 
```
#define X430_REALTIME_TICK_PERIOD `32768U`
```




<hr>



### define X430\_RETURN\_DELAY\_UNIT\_US 

_Return Delay Time (9) unit in microseconds._ 
```
#define X430_RETURN_DELAY_UNIT_US `2U`
```




<hr>



### define X430\_SECONDARY\_ID\_DISABLED 

_Secondary ID (12) values from 253 disable the Secondary ID._ 
```
#define X430_SECONDARY_ID_DISABLED `253U`
```




<hr>



### define X430\_STARTUP\_RAM\_RESTORE 

```
#define X430_STARTUP_RAM_RESTORE `0x02U`
```




<hr>



### define X430\_STARTUP\_TORQUE\_ON 

```
#define X430_STARTUP_TORQUE_ON `0x01U`
```




<hr>



### define X430\_WRITE\_SECONDARY 

_Packet addressed by the Secondary ID: the EEPROM area is locked._ 
```
#define X430_WRITE_SECONDARY `0x01U`
```




<hr>



### define X430\_WRITE\_VALIDATE\_ONLY 

_Validate only: change nothing and call no hook._ 
```
#define X430_WRITE_VALIDATE_ONLY `0x02U`
```




<hr>

------------------------------


