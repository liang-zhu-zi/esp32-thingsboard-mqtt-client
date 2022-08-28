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

// This file is called by tb_mqtt_client_helper.c/.h.

#ifndef _TIMESERIES_DATA_HELPER_H_
#define _TIMESERIES_DATA_HELPER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//====1.telemetry time-series data=====================================================================================

/**
 * ThingsBoard MQTT Client telemetry time-series data
 */
typedef struct tbmch_tsdata
{
     char *key;            /*!< Key */
     tbmch_value_t *value; /*!< Value */

     void *context;                /*!< Context of getting/setting value*/
     tbmch_tsdata_on_get_t on_get; /*!< Callback of getting value from context */
     tbmch_tsdata_on_set_t on_set; /*!< Callback of setting value to context */

     LIST_ENTRY(tbmch_tsdata) entry;
} tbmch_tsdata_t;

typedef tbmch_tsdata_t *tbmch_tsdata_handle_t;

tbmch_tsdata_handle_t _tbmch_tsdata_init(const char *key, tbmch_value_type_t type, void *context,
                                         tbmch_tsdata_on_get_t on_get,
                                         tbmch_tsdata_on_set_t on_set); /*!< Initialize tbmch_tsdata of TBMC_JSON */
esp_err_t _tbmch_tsdata_destory(tbmch_tsdata_handle_t tsdata);          /*!< Destroys the tbmc key-value handle */

const char *_tbmch_tsdata_get_key(tbmch_tsdata_handle_t tsdata);               /*!< Get key of the tbmc time-series data handle */
tbmch_value_type_t _tbmch_tsdata_get_value_type(tbmch_tsdata_handle_t tsdata); /*!< Get value type of tbmch_tsdata */

esp_err_t _tbmch_tsdata_get_value(tbmch_tsdata_handle_t tsdata, tbmch_value_t *value);       /*!< Get tbmch_value of tbmch_tsdata */
esp_err_t _tbmch_tsdata_set_value(tbmch_tsdata_handle_t tsdata, const tbmch_value_t *value); /*!< Set tbmch_value of tbmch_tsdata */

//_tsdata_pack()?/_tsdata_send()?
//tbmch_telemetry_tsdata_list_send(tsdata_list);   //telemetry_tsdata_list_init()/_destory(), _add(), _pack()/_send()!, _get_name()

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

