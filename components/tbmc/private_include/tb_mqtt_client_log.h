// Copyright 2022 liangzhuzhi2020@gmail.com, https://github.com/liang-zhu-zi/thingsboard-mqttclient-basedon-espmqtt
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

// ThingsBoard MQTT Client low layer API

#ifndef _TB_MQTT_CLIENT_LOG_H_
#define _TB_MQTT_CLIENT_LOG_H_

#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TBMC_LOGE(format, ...)  ESP_LOGE(TAG, format, ##__VA_ARGS__) //"[TBMC][E] "
#define TBMC_LOGW(format, ...)  ESP_LOGW(TAG, format, ##__VA_ARGS__) //"[TBMC][W] "
#define TBMC_LOGI(format, ...)  ESP_LOGI(TAG, format, ##__VA_ARGS__) //"[TBMC][I] "
#define TBMC_LOGD(format, ...)  ESP_LOGD(TAG, format, ##__VA_ARGS__) //"[TBMC][D] "
#define TBMC_LOGV(format, ...)  ESP_LOGV(TAG, format, ##__VA_ARGS__) //"[TBMC][V] "

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
