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

#include <stdint.h>
#include <stdbool.h>

#include "sys/queue.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//==== attributes request for client-side_attribute and shared_attribute =================

/**
 * ThingsBoard MQTT Client Helper attributes request
 */
typedef struct attributes_request
{
     tbcmh_handle_t client;        /*!< ThingsBoard MQTT Client Helper */

     int request_id;

     void *context;                                     /*!< Context of callback*/
     tbcmh_attributesrequest_on_response_t on_response; /*!< Callback of dealing successful */
     tbcmh_attributesrequest_on_timeout_t on_timeout;   /*!< Callback of response timeout */

     ////LIST_HEAD(tbcmh_clientattribute_list, client_attribute) clientattribute_list; /*!< client attributes entries */
     ////LIST_HEAD(tbcmh_sharedattribute_list, shared_attribute) sharedattribute_list; /*!< shared attributes entries */

     LIST_ENTRY(attributes_request) entry;
} attributes_request_t;

// TODO: merge to tbcmh_attributesrequest_send()
int       _tbcmh_attributesrequest_send_4_ota_sharedattributes(tbcmh_handle_t client,
                                  void *context,
                                  tbcmh_attributesrequest_on_response_t on_response,
                                  tbcmh_attributesrequest_on_timeout_t on_timeout,
                                  int count, /*const char *key,*/...);
tbc_err_t _tbcmh_attributesrequest_empty(tbcmh_handle_t client);
void      _tbcmh_attributesrequest_on_response(tbcmh_handle_t client, int request_id, const cJSON *object);
void      _tbcmh_attributesrequest_on_timeout(tbcmh_handle_t client, int request_id);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
