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

#ifndef _CLIENT_ATTRIBUTE_HELPER_H_
#define _CLIENT_ATTRIBUTE_HELPER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//====2.client-side attribute==========================================================================================
/**
 * ThingsBoard MQTT Client client-side attribute
 */
// typedef tbmch_kv_t tbmch_attribute_t;
//#define tbmch_attribute_ tbmch_kv_
typedef struct tbmch_clientattribute
{
     char *key;            /*!< Key */
     tbmch_value_t *value; /*!< Value */

     void *context;                         /*!< Context of getting/setting value*/
     tbmch_clientattribute_on_get_t on_get; /*!< Callback of getting value from context */
     tbmch_clientattribute_on_set_t on_set; /*!< Callback of setting value to context */

     LIST_ENTRY(tbmch_clientattribute) entry;
} tbmch_clientattribute_t;

typedef tbmch_clientattribute_t *tbmch_clientattribute_handle_t;

tbmch_clientattribute_handle_t _tbmch_clientattribute_init(const char *key, tbmch_value_type_t type, void *context,
                                                           tbmch_clientattribute_on_get_t on_get,
                                                           tbmch_clientattribute_on_set_t on_set);
esp_err_t _tbmch_clientattribute_destory(tbmch_clientattribute_handle_t attribute); /*!< Destroys the tbmc key-value handle */

// bool _tbmch_attribute_is_clientside(tbmch_clientattribute_handle_t attribute);                  /*!< Is it a client-side attribute? */
bool _tbmch_clientattribute_has_set_value_cb(tbmch_clientattribute_handle_t attribute);             /*!< Has it a set value callback? A shared attribute is always true;
                                                                                                 a client-side attribute is true or false. */
const char *_tbmch_clientattribute_get_key(tbmch_clientattribute_handle_t attribute);               /*!< Get key of the tbmc tbmch_attribute handle */
tbmch_value_type_t _tbmch_clientattribute_get_value_type(tbmch_clientattribute_handle_t attribute); /*!< Get value type of tbmch_attribute */

esp_err_t _tbmch_clientattribute_get_value(tbmch_clientattribute_handle_t clientside_attribute, tbmch_value_t *value); /*!< Get tbmch_value of client-side attribute */
esp_err_t _tbmch_clientattribute_set_value(tbmch_clientattribute_handle_t attribute, const tbmch_value_t *value);      /*!< Set tbmch_value of tbmch_attribute */

//_clientattribute_pack()/_clientattribute_send()!, _clientattribute_unpack()/_clientattribute_deal()

// tbmch_clientside_attribute_list_send(clientside_attribute_list);//clientside_attribute_list_init()/_destory(), _add(), _pack()/_send()!,                    _get_name(), _get_keys()
// shared_attribute_list_init()/_destory(), _add(),                   _unpack()/_deal()?,_get_name(), _get_keys()

#ifdef __cplusplus
}
#endif //__cplusplus

#endif