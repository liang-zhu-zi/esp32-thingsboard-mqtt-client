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

#ifndef _SHARED_ATTRIBUTE_OBSERVER_H_
#define _SHARED_ATTRIBUTE_OBSERVER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//====3.shared attribute===============================================================================================

/**
 * ThingsBoard MQTT Client shared attribute
 */
//typedef tbmc_kv_t tbmc_attribute_t;
//#define tbmc_attribute_ tbmc_kv_
typedef struct tbmc_sharedattribute_
{
     char *key;           /*!< Key */
     tbmc_value_t *value; /*!< Value */

     void *context;                                      /*!< Context of getting/setting value*/
     //tbmc_sharedattribute_get_callback_t get_value_cb; /*!< Callback of getting value from context */
     tbmc_sharedattribute_set_callback_t set_value_cb;   /*!< Callback of setting value to context */
} tbmc_sharedattribute_t;

typedef tbmc_sharedattribute_t *tbmc_sharedattribute_handle_t;

tbmc_sharedattribute_handle_t _tbmc_sharedattribute_init(const char *key, tbmc_value_type_t type, void *context,
                                                         tbmc_sharedattribute_set_callback_t set_value_cb);
tbmc_err_t _tbmc_sharedattributee_destory(tbmc_sharedattribute_handle_t attribute); /*!< Destroys the tbmc key-value handle */

//bool _tbmc_attribute_is_shared(tbmc_sharedattribute_handle_t attribute);                /*!< Is it a shared attribute? */
//bool _tbmc_sharedattribute_has_set_value_cb(tbmc_sharedattribute_handle_t attribute);   /*!< Has it a set value callback? A shared attribute is always true;
//                                                                                             a client-side attribute is true or false. */
const char *_tbmc_sharedattribute_get_key(tbmc_sharedattribute_handle_t attribute);              /*!< Get key of the tbmc tbmc_attribute handle */
tbmc_value_type_t _tbmc_sharedattribute_get_value_type(tbmc_sharedattribute_handle_t attribute); /*!< Get value type of tbmc_attribute */

//tbmc_err_t _tbmc_sharedattribute_get_value(tbmc_sharedattribute_handle_t attribute, tbmc_value_t *value); /*!< Get tbmc_value of client-side attribute */
tbmc_err_t _tbmc_sharedattribute_set_value(tbmc_sharedattribute_handle_t attribute, const tbmc_value_t *value);        /*!< Set tbmc_value of tbmc_attribute */

//                            _as_unpack()/_as_deal()

#ifdef __cplusplus
}
#endif //__cplusplus

#endif