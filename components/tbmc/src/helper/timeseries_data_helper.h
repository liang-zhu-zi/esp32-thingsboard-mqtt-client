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
typedef struct tbmch_tsdata
{
     tbmch_handle_t client; /*!< ThingsBoard MQTT Client Helper */

     char *key; /*!< Key */

     void *context;                /*!< Context of getting/setting value*/
     tbmch_tsdata_on_get_t on_get; /*!< Callback of getting value from context */

     LIST_ENTRY(tbmch_tsdata) entry;
} tbmch_tsdata_t;

tbmch_tsdata_t *_tbmch_tsdata_init(tbmch_handle_t client, const char *key, void *context,
                                   tbmch_tsdata_on_get_t on_get); /*!< Initialize tbmch_tsdata of TBMC_JSON */
tbmch_err_t _tbmch_tsdata_destroy(tbmch_tsdata_t *tsdata);        /*!< Destroys the tbmc key-value handle */
const char *_tbmch_tsdata_get_key(tbmch_tsdata_t *tsdata);        /*!< Get key of the tbmc time-series data handle */
tbmch_err_t _tbmch_tsdata_go_get(tbmch_tsdata_t *tsdata, cJSON *object); /*!< add item value to json object */

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

