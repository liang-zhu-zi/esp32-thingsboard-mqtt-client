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

// ThingsBoard Client transport config API

#ifndef _TBC_UTIL_H_
#define _TBC_UTIL_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TBC_LOGE(format, ...)  ESP_LOGE(TAG, format, ##__VA_ARGS__) //"[TBC][E] "
#define TBC_LOGW(format, ...)  ESP_LOGW(TAG, format, ##__VA_ARGS__) //"[TBC][W] "
#define TBC_LOGI(format, ...)  ESP_LOGI(TAG, format, ##__VA_ARGS__) //"[TBC][I] "
#define TBC_LOGD(format, ...)  ESP_LOGD(TAG, format, ##__VA_ARGS__) //"[TBC][D] "
#define TBC_LOGV(format, ...)  ESP_LOGV(TAG, format, ##__VA_ARGS__) //"[TBC][V] "

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
                    action;                                                                                         \
                }

#define TBC_CHECK_PTR_WITH_RETURN_VALUE(pointer, returnValue) \
                 if (!pointer) { \
                      ESP_LOGE(TAG, #pointer" is NULL! %s()", __FUNCTION__); \
                      return returnValue; \
                 }
            
#define TBC_CHECK_PTR(pointer) \
                 if (!pointer) { \
                      ESP_LOGE(TAG, #pointer" is NULL! %s()", __FUNCTION__); \
                      return; \
                 }


#ifdef __cplusplus
}
#endif //__cplusplus

#endif
