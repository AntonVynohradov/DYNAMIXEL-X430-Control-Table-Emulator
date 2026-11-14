

# File x430\_model.h

[**File List**](files.md) **>** [**inc**](dir_bfccd401955b95cf8c75461437045ac0.md) **>** [**x430\_model.h**](x430__model_8h.md)

[Go to the documentation of this file](x430__model_8h.md)


```C++

#ifndef X430_MODEL_H
#define X430_MODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define X430_REG_WRITABLE 0x01U
#define X430_REG_EEPROM 0x02U
#define X430_REG_SIGNED 0x04U
#define X430_REG_GOAL 0x08U
#define X430_REG_CHECK_SHIFT 4U
#define X430_REG_CHECK(check) ((uint32_t)(check) << X430_REG_CHECK_SHIFT)

enum x430_check
{
    X430_CHECK_NONE = 0,      
    X430_CHECK_RANGE,         
    X430_CHECK_MASK,          
    X430_CHECK_OPMODE,        
    X430_CHECK_SYMMETRIC,     
    X430_CHECK_GOAL_POSITION, 
    X430_CHECK_INDIRECT       
};

struct x430_reg
{
    uint16_t address; 
    uint8_t size;     
    uint8_t flags;    
    int32_t initial;  
    int32_t min;      
    int32_t max; 
};

struct x430_indirect_block
{
    uint16_t address; 
    uint16_t data;    
    uint16_t count;   
};

struct x430_segment
{
    uint16_t address; 
    uint16_t length;  
    uint16_t offset;  
};

struct x430_model
{
    const char* name;                           
    const struct x430_reg* registers;           
    const struct x430_indirect_block* indirect; 
    const struct x430_segment* segments;        
    uint32_t operating_modes;                   
    uint16_t model_number;                      
    uint16_t table_size;                        
    uint16_t register_count;                    
    uint16_t current_unit_ua;                   
    uint8_t indirect_count;                     
    uint8_t segment_count;                      
    uint8_t axes;                               
};

extern const struct x430_model x430_model_active;

#ifdef __cplusplus
}
#endif

#endif /* X430_MODEL_H */

```


