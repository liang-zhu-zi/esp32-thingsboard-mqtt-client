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

// ThingsBoard MQTT Client high layer API

#ifndef _TB_MQTT_CLIENT_HELPER_LOG_H_
#define _TB_MQTT_CLIENT_HELPER_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"

#define TBMCH_LOGE(format, ...)  ESP_LOGE(TAG, format, ##__VA_ARGS__) //"[TBMCH][E] "
#define TBMCH_LOGW(format, ...)  ESP_LOGW(TAG, format, ##__VA_ARGS__) //"[TBMCH][W] "
#define TBMCH_LOGI(format, ...)  ESP_LOGI(TAG, format, ##__VA_ARGS__) //"[TBMCH][I] "
#define TBMCH_LOGD(format, ...)  ESP_LOGD(TAG, format, ##__VA_ARGS__) //"[TBMCH][D] "
#define TBMCH_LOGV(format, ...)  ESP_LOGV(TAG, format, ##__VA_ARGS__) //"[TBMCH][V] "

#define TBMCH_CHECK_PTR_WITH_RETURN_VALUE(pointer, returnValue) \
     if (!pointer) { \
          TBMCH_LOGE(#pointer" is NULL! %s()", __FUNCTION__); \
          return returnValue; \
     }

#define TBMCH_CHECK_PTR(pointer) \
     if (!pointer) { \
          TBMCH_LOGE(pointer" is NULL! %s()", __FUNCTION__); \
          return; \
     }

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
