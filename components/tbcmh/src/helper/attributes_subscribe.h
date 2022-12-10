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

#ifndef _ATTRIBUTES_SUBSCRIBE_H_
#define _ATTRIBUTES_SUBSCRIBE_H_

#include <stdint.h>
#include <stdbool.h>

#include "sys/queue.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct subscribekey
{
     char *key; /*!< Key */
     LIST_ENTRY(subscribekey) entry;
} subscribekey_t;

typedef LIST_HEAD(subscribekey_list, subscribekey) subscribekey_list_t;

/**
 * ThingsBoard MQTT Client Helper shared attribute
 */
typedef struct attributessubscribe
{
     uint32_t subscribe_id;

     subscribekey_list_t key_list;         /*!< A list of some keys */
     void *context;                        /*!< Context of getting/setting value*/
     tbcmh_attributes_on_update_t on_update; /*!< Callback of setting value to context */

     LIST_ENTRY(attributessubscribe) entry;
} attributessubscribe_t;

typedef LIST_HEAD(tbcmh_attributessubscribe_list, attributessubscribe) attributessubscribe_list_t;

void _tbcmh_attributessubscribe_on_create(tbcmh_handle_t client);
void _tbcmh_attributessubscribe_on_destroy(tbcmh_handle_t client);
void _tbcmh_attributessubscribe_on_connected(tbcmh_handle_t client);
void _tbcmh_attributessubscribe_on_disconnected(tbcmh_handle_t client);
int  _tbcmh_attributessubscribe_on_data(tbcmh_handle_t client, const cJSON *object);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif
