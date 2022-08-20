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

#ifndef _TELEMETRY_DATAPOINT_HELPER_H_
#define _TELEMETRY_DATAPOINT_HELPER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//====1.telemetry_datapoint============================================================================================

/**
 * ThingsBoard MQTT Client telemetry datapoint
 */
typedef struct tbmc_datapoint_
{
     char *key;           /*!< Key */
     tbmc_value_t *value; /*!< Value */

     void *context;                              /*!< Context of getting/setting value*/
     tbmc_datapoint_get_callback_t get_value_cb; /*!< Callback of getting value from context */
     tbmc_datapoint_set_callback_t set_value_cb; /*!< Callback of setting value to context */
} tbmc_datapoint_t;

typedef tbmc_datapoint_t *tbmc_datapoint_handle_t;

tbmc_datapoint_handle_t _tbmc_datapoint_init(const char *key, tbmc_value_type_t type, void *context,
                                             tbmc_datapoint_get_callback_t get_value_cb,
                                             tbmc_datapoint_set_callback_t set_value_cb); /*!< Initialize tbmc_dp of TBMC_JSON */
tbmc_err_t _tbmc_datapoint_destory(tbmc_datapoint_handle_t dp);                           /*!< Destroys the tbmc key-value handle */

const char *_tbmc_datapoint_get_key(tbmc_datapoint_handle_t dp);              /*!< Get key of the tbmc datapoint handle */
tbmc_value_type_t _tbmc_datapoint_get_value_type(tbmc_datapoint_handle_t dp); /*!< Get value type of tbmc_dp */

tbmc_err_t _tbmc_datapoint_get_value(tbmc_datapoint_handle_t dp, tbmc_value_t *value);       /*!< Get tbmc_value of tbmc_dp */
tbmc_err_t _tbmc_datapoint_set_value(tbmc_datapoint_handle_t dp, const tbmc_value_t *value); /*!< Set tbmc_value of tbmc_dp */

//_dps_pack()?/_dps_send()?
//tbmc_telemetry_datapoint_list_send(datapoint_list);   //telemetry_datapoint_list_init()/_destory(), _add(), _pack()/_send()!, _get_name()

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

