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

#ifndef _ATTRIBUTES_REQUEST_OBSERVER_H_
#define _ATTRIBUTES_REQUEST_OBSERVER_H_

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//==== attributes request for client-side_attribute and sharedattribute =================

/**
 * ThingsBoard MQTT Client Helper attributes request
 */
typedef struct attributesrequest
{
     tbcmh_handle_t client; /*!< ThingsBoard MQTT Client Helper */

     uint32_t request_id;
     uint64_t timestamp;    /*!< time stamp at sending request */

     void *context;                                     /*!< Context of callback*/
     tbcmh_attributes_on_response_t on_response; /*!< Callback of dealing successful */
     tbcmh_attributes_on_timeout_t on_timeout;   /*!< Callback of response timeout */

     LIST_ENTRY(attributesrequest) entry;
} attributesrequest_t;

typedef LIST_HEAD(tbcmh_attributesrequest_list, attributesrequest) attributesrequest_list_t;

void _tbcmh_attributesrequest_on_create(tbcmh_handle_t client);
void _tbcmh_attributesrequest_on_destroy(tbcmh_handle_t client);
void _tbcmh_attributesrequest_on_connected(tbcmh_handle_t client);
void _tbcmh_attributesrequest_on_disconnected(tbcmh_handle_t client);
void _tbcmh_attributesrequest_on_data(tbcmh_handle_t client, uint32_t request_id, const cJSON *object);
void _tbcmh_attributesrequest_on_check_timeout(tbcmh_handle_t client, uint64_t timestamp);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
