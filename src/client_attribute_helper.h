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

#ifndef _CLIENT_ATTRIBUTE_HELPER_H_
#define _CLIENT_ATTRIBUTE_HELPER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//====2.client-side attribute==========================================================================================
/**
 * ThingsBoard MQTT Client client-side attribute
 */
// typedef tbmc_kv_t tbmc_attribute_t;
//#define tbmc_attribute_ tbmc_kv_
typedef struct tbmc_clientattribute_
{
     char *key;           /*!< Key */
     tbmc_value_t *value; /*!< Value */

     void *context;                                    /*!< Context of getting/setting value*/
     tbmc_clientattribute_get_callback_t get_value_cb; /*!< Callback of getting value from context */
     tbmc_clientattribute_set_callback_t set_value_cb; /*!< Callback of setting value to context */
} tbmc_clientattribute_t;

typedef tbmc_clientattribute_t *tbmc_clientattribute_handle_t;

tbmc_clientattribute_handle_t _tbmc_clientattribute_init(const char *key, tbmc_value_type_t type, void *context,
                                                          tbmc_clientattribute_get_callback_t get_value_cb,
                                                          tbmc_clientattribute_set_callback_t set_value_cb);
tbmc_err_t _tbmc_clientattribute_destory(tbmc_clientattribute_handle_t attribute);               /*!< Destroys the tbmc key-value handle */

// bool _tbmc_attribute_is_clientside(tbmc_clientattribute_handle_t attribute);                  /*!< Is it a client-side attribute? */
bool _tbmc_clientattribute_has_set_value_cb(tbmc_clientattribute_handle_t attribute);            /*!< Has it a set value callback? A shared attribute is always true;
                                                                                                a client-side attribute is true or false. */
const char *_tbmc_clientattribute_get_key(tbmc_clientattribute_handle_t attribute);              /*!< Get key of the tbmc tbmc_attribute handle */
tbmc_value_type_t _tbmc_clientattribute_get_value_type(tbmc_clientattribute_handle_t attribute); /*!< Get value type of tbmc_attribute */

tbmc_err_t _tbmc_clientattribute_get_value(tbmc_clientattribute_handle_t clientside_attribute, tbmc_value_t *value); /*!< Get tbmc_value of client-side attribute */
tbmc_err_t _tbmc_clientattribute_set_value(tbmc_clientattribute_handle_t attribute, const tbmc_value_t *value);      /*!< Set tbmc_value of tbmc_attribute */

//_csas_pack()/_csas_send()!, _as_unpack()/_as_deal()

// tbmc_clientside_attribute_list_send(clientside_attribute_list);//clientside_attribute_list_init()/_destory(), _add(), _pack()/_send()!,                    _get_name(), _get_keys()
// shared_attribute_list_init()/_destory(), _add(),                   _unpack()/_deal()?,_get_name(), _get_keys()

#ifdef __cplusplus
}
#endif //__cplusplus

#endif