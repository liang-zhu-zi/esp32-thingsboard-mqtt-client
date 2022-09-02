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

#ifndef _SHARED_ATTRIBUTE_OBSERVER_H_
#define _SHARED_ATTRIBUTE_OBSERVER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//====3.shared attribute===============================================================================================

/**
 * ThingsBoard MQTT Client shared attribute
 */
// typedef tbmch_kv_t tbmch_attribute_t;
//#define tbmch_attribute_ tbmch_kv_
typedef struct tbmch_sharedattribute
{
     char *key;            /*!< Key */
     tbmch_value_t *value; /*!< Value */

     void *context;                               /*!< Context of getting/setting value*/
     tbmch_sharedattribute_on_set_t on_set;       /*!< Callback of setting value to context */
     // tbmch_sharedattribute_on_get_t on_get;    /*!< Callback of getting value from context */

     LIST_ENTRY(tbmch_sharedattribute) entry;
} tbmch_sharedattribute_t;

typedef tbmch_sharedattribute_t *tbmch_sharedattribute_handle_t;

tbmch_sharedattribute_handle_t _tbmch_sharedattribute_init(const char *key, tbmch_value_type_t type, void *context,
                                                           tbmch_sharedattribute_on_set_t on_set);
tbmch_err_t _tbmch_sharedattributee_destroy(tbmch_sharedattribute_handle_t attribute); /*!< Destroys the tbmc key-value handle */

// bool _tbmch_attribute_is_shared(tbmch_sharedattribute_handle_t attribute);                /*!< Is it a shared attribute? */
// bool _tbmch_sharedattribute_has_set_value_cb(tbmch_sharedattribute_handle_t attribute);   /*!< Has it a set value callback? A shared attribute is always true;
//                                                                                              a client-side attribute is true or false. */
const char *_tbmch_sharedattribute_get_key(tbmch_sharedattribute_handle_t attribute);               /*!< Get key of the tbmc tbmch_attribute handle */
tbmch_value_type_t _tbmch_sharedattribute_get_value_type(tbmch_sharedattribute_handle_t attribute); /*!< Get value type of tbmch_attribute */

// tmbch_err_t _tbmch_sharedattribute_get_value(tbmch_sharedattribute_handle_t attribute, tbmch_value_t *value); /*!< Get tbmch_value of client-side attribute */
tmbch_err_t _tbmch_sharedattribute_set_value(tbmch_sharedattribute_handle_t attribute, const tbmch_value_t *value); /*!< Set tbmch_value of tbmch_attribute */

//    _sharedattribute_unpack()/_sharedattribute_deal()

#ifdef __cplusplus
}
#endif //__cplusplus

#endif