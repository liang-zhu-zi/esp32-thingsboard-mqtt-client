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

#ifndef _ATTRIBUTES_REQUEST_OBSERVER_H_
#define _ATTRIBUTES_REQUEST_OBSERVER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//====4.attributes request for client-side_attribute and shared_attribute==============================================

//#include "sys/queue.h"
//struct track_; // Forward declaration
//typedef STAILQ_HEAD(track_list, track_) track_list_t;
//tbmc_clientside_attribute_list_t
//tbmc_shared_attribute_list_t

/**
 * ThingsBoard MQTT Client attributes request
 */
typedef struct tbmc_attributes_request_
{
     void *context;                                         /*!< Context of callback*/
     tbmc_attributes_request_success_callback_t success_cb; /*!< Callback of dealing successful */
     tbmc_attributes_request_timeout_callback_t timeout_cb; /*!< Callback of response timeout */

     struct
     {
          LIST_ENTRY(tbmc_clientattribute_) helper_entries; /*!< client attributes entries */
     } clientattributes;
     struct
     {
          LIST_ENTRY(tbmc_sharedattribute_) observer_entries; /*!< shared attributes entries */
     } sharedattributes;

     int times;               /*!< times of sending. default value is 0 */
} tbmc_attributes_request_t;

typedef tbmc_attributes_request_t *tbmc_attributes_request_handle_t;

tbmc_attributes_request_handle_t _tbmc_attributes_request_init(void *context,
                                                              tbmc_attributes_request_success_callback_t success_cb,
                                                              tbmc_attributes_request_timeout_callback_t timeout_cb); /*!< Initialize tbmc_attributes_request */
esp_err_t _tbmc_attributes_request_destory(tbmc_attributes_request_handle_t request);                                 /*!< Destroys the tbmc_attributes_request */

esp_err_t _tbmc_attributes_request_add(tbmc_attributes_request_handle_t request, tbmc_attribute_handle_t attribute, ...);

esp_err_t _tbmc_attributes_request_get_client_keys(tbmc_attributes_request_handle_t request, char *buffer, int size);
esp_err_t _tbmc_attributes_request_get_shared_keys(tbmc_attributes_request_handle_t request, char *buffer, int size);

tbmc_attribute_handle_t _tbmc_attributes_request_search_clientattribute(tbmc_attributes_request_handle_t request, const char *clientside_attribute_name); /*!< Search the client-side attribute in request */
tbmc_attribute_handle_t _tbmc_attributes_request_search_sharedattribute(tbmc_attributes_request_handle_t request, const char *clientside_attribute_name); /*!< Search the shared attribute in request */
//_pack()/_send()!, _unpack()/deal() //_onAttributesResponse()[unpack->queue], [queue->attribute deal], 
//???_destory_all_attributes(),

#ifdef __cplusplus
}
#endif //__cplusplus

#endif