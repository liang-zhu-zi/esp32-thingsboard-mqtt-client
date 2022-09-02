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
 * ThingsBoard MQTT Client Helper client-side attribute
 */
typedef struct tbmch_clientattribute
{
     char *key;            /*!< Key */

     void *context;                         /*!< Context of getting/setting value*/
     tbmch_clientattribute_on_get_t on_get; /*!< Callback of getting value from context */
     tbmch_clientattribute_on_set_t on_set; /*!< Callback of setting value to context */

     LIST_ENTRY(tbmch_clientattribute) entry;
} tbmch_clientattribute_t;

tbmch_clientattribute_t *_tbmch_clientattribute_init(const char *key, void *context,
                                                    tbmch_clientattribute_on_get_t on_get,
                                                    tbmch_clientattribute_on_set_t on_set);
tbmch_err_t _tbmch_clientattribute_destroy(tbmch_clientattribute_t *clientattribute); /*!< Destroys the tbmc key-value handle */

bool _tbmch_clientattribute_has_set_value_cb(tbmch_clientattribute_t *clientattribute); /*!< Has it a set value callback? A shared attribute is always true;
                                                                                                 a client-side attribute is true or false. */
const char *_tbmch_clientattribute_get_key(tbmch_clientattribute_t *clientattribute);   /*!< Get key of the tbmc tbmch_attribute handle */

tbmch_err_t _tbmch_clientattribute_value_to_pack(tbmch_handle_t client, tbmch_clientattribute_t *clientattribute, cJSON *object);     /*!< add item value to json object */
tbmch_err_t _tbmch_clientattribute_value_from_unpack(tbmch_handle_t client, tbmch_clientattribute_t *clientattribute, cJSON *object); /*!< add item value to json object */

#ifdef __cplusplus
}
#endif //__cplusplus

#endif