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

// This file is called by tbmc_help.c/.h.

#ifndef _TIMESERIES_DATA_HELPER_H_
#define _TIMESERIES_DATA_HELPER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//====1.telemetry time-series data=====================================================================================

/**
 * ThingsBoard MQTT Client telemetry time-series data
 */
typedef struct tbmc_tsdata_
{
     char *key;           /*!< Key */
     tbmc_value_t *value; /*!< Value */

     void *context;                              /*!< Context of getting/setting value*/
     tbmc_tsdata_get_callback_t get_value_cb; /*!< Callback of getting value from context */
     tbmc_tsdata_set_callback_t set_value_cb; /*!< Callback of setting value to context */
} tbmc_tsdata_t;

typedef tbmc_tsdata_t *tbmc_tsdata_handle_t;

tbmc_tsdata_handle_t _tbmc_tsdata_init(const char *key, tbmc_value_type_t type, void *context,
                                             tbmc_tsdata_get_callback_t get_value_cb,
                                             tbmc_tsdata_set_callback_t set_value_cb); /*!< Initialize tbmc_dp of TBMC_JSON */
esp_err_t _tbmc_tsdata_destory(tbmc_tsdata_handle_t dp);                           /*!< Destroys the tbmc key-value handle */

const char *_tbmc_tsdata_get_key(tbmc_tsdata_handle_t dp);              /*!< Get key of the tbmc time-series data handle */
tbmc_value_type_t _tbmc_tsdata_get_value_type(tbmc_tsdata_handle_t dp); /*!< Get value type of tbmc_dp */

esp_err_t _tbmc_tsdata_get_value(tbmc_tsdata_handle_t dp, tbmc_value_t *value);       /*!< Get tbmc_value of tbmc_dp */
esp_err_t _tbmc_tsdata_set_value(tbmc_tsdata_handle_t dp, const tbmc_value_t *value); /*!< Set tbmc_value of tbmc_dp */

//_dps_pack()?/_dps_send()?
//tbmc_telemetry_tsdata_list_send(tsdata_list);   //telemetry_tsdata_list_init()/_destory(), _add(), _pack()/_send()!, _get_name()

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

