// Copyright 2022 liangzhuzhi2020@gmail.com, https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// ThingsBoard Client utils API

#ifndef _TBC_UTILS_H_
#define _TBC_UTILS_H_

#include <stdint.h>
#include <stdbool.h>

#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

//====tbcm data=============================================================================
/* Definitions for error constants. */
// #define TBCM_OK    0   /*!< tbc_err_t value indicating success (no error) */
// #define TBCM_FAIL -1   /*!< Generic tbc_err_t code indicating failure */

// #define TBCM_ERR_NO_MEM              0x101   /*!< Out of memory */
// #define TBCM_ERR_INVALID_ARG         0x102   /*!< Invalid argument */
// #define TBCM_ERR_INVALID_STATE       0x103   /*!< Invalid state */
// #define TBCM_ERR_INVALID_SIZE        0x104   /*!< Invalid size */
// #define TBCM_ERR_NOT_FOUND           0x105   /*!< Requested resource not found */
// #define TBCM_ERR_NOT_SUPPORTED       0x106   /*!< Operation or feature not supported */
// #define TBCM_ERR_TIMEOUT             0x107   /*!< Operation timed out */
// #define TBCM_ERR_INVALID_RESPONSE    0x108   /*!< Received response was invalid */
// #define TBCM_ERR_INVALID_CRC         0x109   /*!< CRC or checksum was invalid */
// #define TBCM_ERR_INVALID_VERSION     0x10A   /*!< Version was invalid */
// #define TBCM_ERR_INVALID_MAC         0x10B   /*!< MAC address was invalid */
// #define TBCM_ERR_NOT_FINISHED        0x10C   /*!< There are items remained to retrieve */

#define TBC_MALLOC   malloc
#define TBC_FREE     free

typedef int tbc_err_t;

//============================================================================================
#define TBC_LOGE(format, ...)  ESP_LOGE(TAG, format, ##__VA_ARGS__) //"[TBC][E] "
#define TBC_LOGW(format, ...)  ESP_LOGW(TAG, format, ##__VA_ARGS__) //"[TBC][W] "
#define TBC_LOGI(format, ...)  ESP_LOGI(TAG, format, ##__VA_ARGS__) //"[TBC][I] "
#define TBC_LOGD(format, ...)  ESP_LOGD(TAG, format, ##__VA_ARGS__) //"[TBC][D] "
#define TBC_LOGV(format, ...)  ESP_LOGV(TAG, format, ##__VA_ARGS__) //"[TBC][V] "

//============================================================================================
#define TBC_FIELD_STRDUP(dest, src) \
                if (src) { \
                    dest = strdup(src);\
                }

#define TBC_FIELD_MEMDUP(dest, src, size) \
                if (src && size>0) { \
                    dest = malloc(size); \
                    if (dest) { \
                        memcpy(dest, src, size); \
                    } \
                }

#define TBC_FIELD_FREE(field) \
                if (field) { \
                    free(field); \
                    field = NULL; \
                }

#define TBC_MEM_CHECK(TAG, a, action) \
                if (!(a)) {                                                      \
                    ESP_LOGE(TAG, "%s(%d): %s",  __FUNCTION__, __LINE__, "Memory exhausted"); \
                    action; \
                }

#define TBC_CHECK_PTR_WITH_RETURN_VALUE(pointer, returnValue) \
                 if (!pointer) { \
                      ESP_LOGE(TAG, #pointer" is NULL! %s() %d", __FUNCTION__, __LINE__); \
                      return returnValue; \
                 }
            
#define TBC_CHECK_PTR(pointer) \
                 if (!pointer) { \
                      ESP_LOGE(TAG, #pointer" is NULL! %s() %d", __FUNCTION__, __LINE__); \
                      return; \
                 }

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
