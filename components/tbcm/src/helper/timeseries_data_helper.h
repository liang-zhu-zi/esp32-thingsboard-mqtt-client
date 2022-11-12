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

// This file is called by tbc_mqtt_helper.c/.h.

#ifndef _TIMESERIES_DATA_HELPER_H_
#define _TIMESERIES_DATA_HELPER_H_

#include <stdint.h>
#include <stdbool.h>

#include "sys/queue.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//====1.telemetry time-series data=====================================================================================

/**
 * ThingsBoard MQTT Client Helper Telemetry time-series data
 */
typedef struct tbcmh_tsdata
{
     tbcmh_handle_t client; /*!< ThingsBoard MQTT Client Helper */

     char *key; /*!< Key */

     void *context;                /*!< Context of getting/setting value*/
     tbcmh_tsdata_on_get_t on_get; /*!< Callback of getting value from context */

     LIST_ENTRY(tbcmh_tsdata) entry;
} tbcmh_tsdata_t;

tbcmh_tsdata_t *_tbcmh_tsdata_init(tbcmh_handle_t client, const char *key, void *context,
                                   tbcmh_tsdata_on_get_t on_get); /*!< Initialize tbcmh_tsdata of TBCM_JSON */
tbc_err_t _tbcmh_tsdata_destroy(tbcmh_tsdata_t *tsdata);        /*!< Destroys the tbcm key-value handle */
const char *_tbcmh_tsdata_get_key(tbcmh_tsdata_t *tsdata);        /*!< Get key of the tbcm time-series data handle */
tbc_err_t _tbcmh_tsdata_go_get(tbcmh_tsdata_t *tsdata, cJSON *object); /*!< add item value to json object */

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

