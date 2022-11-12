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

#ifndef _SHARED_ATTRIBUTE_OBSERVER_H_
#define _SHARED_ATTRIBUTE_OBSERVER_H_

#include <stdint.h>
#include <stdbool.h>

#include "sys/queue.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//====3.shared attribute===============================================================================================

/**
 * ThingsBoard MQTT Client Helper shared attribute
 */
typedef struct tbcmh_sharedattribute
{
     tbcmh_handle_t client; /*!< ThingsBoard MQTT Client Helper */

     char *key; /*!< Key */

     void *context;                         /*!< Context of getting/setting value*/
     tbcmh_sharedattribute_on_set_t on_set; /*!< Callback of setting value to context */

     LIST_ENTRY(tbcmh_sharedattribute) entry;
} tbcmh_sharedattribute_t;

tbcmh_sharedattribute_t *_tbcmh_sharedattribute_init(tbcmh_handle_t client, const char *key, void *context,
                                                    tbcmh_sharedattribute_on_set_t on_set);
tbc_err_t _tbcmh_sharedattribute_destroy(tbcmh_sharedattribute_t *sharedattribute); /*!< Destroys the tbcm key-value handle */

const char *_tbcmh_sharedattribute_get_key(tbcmh_sharedattribute_t *sharedattribute); /*!< Get key of the tbcm tbcmh_attribute handle */
tbc_err_t _tbcmh_sharedattribute_do_set(tbcmh_sharedattribute_t *sharedattribute, cJSON *value); /*!< add item value to json object */

#ifdef __cplusplus
}
#endif //__cplusplus

#endif