

# File x430\_config.h

[**File List**](files.md) **>** [**inc**](dir_bfccd401955b95cf8c75461437045ac0.md) **>** [**x430\_config.h**](x430__config_8h.md)

[Go to the documentation of this file](x430__config_8h.md)


```C++

#ifndef X430_CONFIG_H
#define X430_CONFIG_H

#include <stdint.h>

#if defined(X430_HAS_AUTOCONF)
#include "x430_autoconf.h"
#endif

#include "x430_model_info.h"

#ifndef DXL2_MAX_PACKET_SIZE
#if defined(CONFIG_X430_DXL2_MAX_PACKET_SIZE)
#define DXL2_MAX_PACKET_SIZE (CONFIG_X430_DXL2_MAX_PACKET_SIZE)
#else
#define DXL2_MAX_PACKET_SIZE 768U
#endif
#endif

#ifndef X430_MAX_DEVICES
#if defined(CONFIG_X430_MAX_DEVICES)
#define X430_MAX_DEVICES (CONFIG_X430_MAX_DEVICES)
#else
#define X430_MAX_DEVICES (4U)
#endif
#endif

#ifndef X430_REG_WRITE_SIZE
#if defined(CONFIG_X430_REG_WRITE_SIZE)
#define X430_REG_WRITE_SIZE (CONFIG_X430_REG_WRITE_SIZE)
#else
#define X430_REG_WRITE_SIZE (32U)
#endif
#endif

#ifndef X430_TURN_TIMEOUT_MS
#if defined(CONFIG_X430_TURN_TIMEOUT_MS)
#define X430_TURN_TIMEOUT_MS (CONFIG_X430_TURN_TIMEOUT_MS)
#else
#define X430_TURN_TIMEOUT_MS (100U)
#endif
#endif

#ifndef X430_FIRMWARE_VERSION
#if defined(CONFIG_X430_FIRMWARE_VERSION)
#define X430_FIRMWARE_VERSION (CONFIG_X430_FIRMWARE_VERSION)
#else
#define X430_FIRMWARE_VERSION (52U)
#endif
#endif

#ifndef X430_MODEL_NUMBER
#if defined(CONFIG_X430_MODEL_NUMBER_OVERRIDE) && (CONFIG_X430_MODEL_NUMBER_OVERRIDE != 0)
#define X430_MODEL_NUMBER (CONFIG_X430_MODEL_NUMBER_OVERRIDE)
#else
#define X430_MODEL_NUMBER (X430_MODEL_NUMBER_DEFAULT)
#endif
#endif

#ifndef X430_BAUD_RATE_CODE_MAX
#if defined(CONFIG_X430_BAUD_RATE_CODE_MAX)
#define X430_BAUD_RATE_CODE_MAX (CONFIG_X430_BAUD_RATE_CODE_MAX)
#else
#define X430_BAUD_RATE_CODE_MAX (7U)
#endif
#endif

#ifndef X430_FEATURE_BULK
#if defined(CONFIG_X430_BULK) || !defined(X430_HAS_AUTOCONF)
#define X430_FEATURE_BULK 1
#else
#define X430_FEATURE_BULK 0
#endif
#endif

#ifndef X430_FEATURE_FAST_READ
#if defined(CONFIG_X430_FAST_READ) || !defined(X430_HAS_AUTOCONF)
#define X430_FEATURE_FAST_READ 1
#else
#define X430_FEATURE_FAST_READ 0
#endif
#endif

#ifndef X430_FEATURE_BACKUP
#if defined(CONFIG_X430_BACKUP) || !defined(X430_HAS_AUTOCONF)
#define X430_FEATURE_BACKUP 1
#else
#define X430_FEATURE_BACKUP 0
#endif
#endif

#ifndef X430_FEATURE_SIMULATOR
#if defined(CONFIG_X430_SIMULATOR) || !defined(X430_HAS_AUTOCONF)
#define X430_FEATURE_SIMULATOR 1
#else
#define X430_FEATURE_SIMULATOR 0
#endif
#endif

#define X430_MAX_DEVICES_U ((uint32_t)(X430_MAX_DEVICES))
#define X430_REG_WRITE_SIZE_U ((uint32_t)(X430_REG_WRITE_SIZE))
#define X430_BAUD_RATE_CODE_MAX_U ((uint32_t)(X430_BAUD_RATE_CODE_MAX))
#define X430_TURN_TIMEOUT_MS_U ((uint32_t)(X430_TURN_TIMEOUT_MS))

#if (X430_MAX_DEVICES < 1U) || (X430_MAX_DEVICES > 253U)
#error "X430_MAX_DEVICES must be in the range 1..253"
#endif

#if (X430_REG_WRITE_SIZE < 4U) || (X430_REG_WRITE_SIZE > X430_TABLE_SIZE)
#error "X430_REG_WRITE_SIZE must be in the range 4..X430_TABLE_SIZE"
#endif

#if (X430_TURN_TIMEOUT_MS < 1U) || (X430_TURN_TIMEOUT_MS > 60000U)
#error "X430_TURN_TIMEOUT_MS must be in the range 1..60000"
#endif

#endif /* X430_CONFIG_H */

```


