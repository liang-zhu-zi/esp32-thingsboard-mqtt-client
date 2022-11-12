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

#ifndef _CLIENT_ATTRIBUTE_HELPER_H_
#define _CLIENT_ATTRIBUTE_HELPER_H_

#include <stdint.h>
#include <stdbool.h>

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//====2.client-side attribute==========================================================================================
/**
 * ThingsBoard MQTT Client Helper client-side attribute
 */
typedef struct tbcmh_clientattribute
{
     tbcmh_handle_t client; /*!< ThingsBoard MQTT Client Helper */

     char *key; /*!< Key */

     void *context;                         /*!< Context of getting/setting value*/
     tbcmh_clientattribute_on_get_t on_get; /*!< Callback of getting value from context */
     tbcmh_clientattribute_on_set_t on_set; /*!< Callback of setting value to context */

     LIST_ENTRY(tbcmh_clientattribute) entry;
} tbcmh_clientattribute_t;

// static tbcmh_clientattribute_t *_tbcmh_clientattribute_init(tbcmh_handle_t client, const char *key, void *context,
//                                                     tbcmh_clientattribute_on_get_t on_get,
//                                                     tbcmh_clientattribute_on_set_t on_set);
// static tbc_err_t _tbcmh_clientattribute_destroy(tbcmh_clientattribute_t *clientattribute); /*!< Destroys the tbcm key-value handle */

// static bool _tbcmh_clientattribute_has_set_value_cb(tbcmh_clientattribute_t *clientattribute); /*!< Has it a set value callback? A shared attribute is always true;
//                                                                                                  a client-side attribute is true or false. */
/*static*/ const char *_tbcmh_clientattribute_get_key(tbcmh_clientattribute_t *clientattribute);   /*!< Get key of the tbcm tbcmh_attribute handle */

// static tbc_err_t _tbcmh_clientattribute_do_get(tbcmh_clientattribute_t *clientattribute, cJSON *object);     /*!< add item value to json object */
// static tbc_err_t _tbcmh_clientattribute_do_set(tbcmh_clientattribute_t *clientattribute, cJSON *value); /*!< add item value to json object */

/*static*/ void _tbcmh_clientattribute_on_received(tbcmh_handle_t client_, const cJSON *object);

/*static*/ tbc_err_t _tbcmh_clientattribute_empty(tbcmh_handle_t client_);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif