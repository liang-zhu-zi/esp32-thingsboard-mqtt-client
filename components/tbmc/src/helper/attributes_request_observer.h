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

//====4.attributes request for client-side_attribute and shared_attribute==============================================

//#include "sys/queue.h"
//struct track_; // Forward declaration
//typedef STAILQ_HEAD(track_list, track_) track_list_t;

/**
 * ThingsBoard MQTT Client Helper attributes request
 */
typedef struct tbcmh_attributesrequest
{
     tbcmh_handle_t client;        /*!< ThingsBoard MQTT Client Helper */

     int request_id;

     void *context;                                     /*!< Context of callback*/
     tbcmh_attributesrequest_on_response_t on_response; /*!< Callback of dealing successful */
     tbcmh_attributesrequest_on_timeout_t on_timeout;   /*!< Callback of response timeout */

     ////LIST_HEAD(tbcmh_clientattribute_list, tbcmh_clientattribute) clientattribute_list; /*!< client attributes entries */
     ////LIST_HEAD(tbcmh_sharedattribute_list, tbcmh_sharedattribute) sharedattribute_list; /*!< shared attributes entries */

     LIST_ENTRY(tbcmh_attributesrequest) entry;
} tbcmh_attributesrequest_t;

tbcmh_attributesrequest_t *_tbcmh_attributesrequest_init(tbcmh_handle_t client, int request_id, void *context,
                                                               tbcmh_attributesrequest_on_response_t on_response,
                                                               tbcmh_attributesrequest_on_timeout_t on_timeout); /*!< Initialize tbcmh_attributesrequest */
tbcmh_attributesrequest_t *_tbcmh_attributesrequest_clone_wo_listentry(tbcmh_attributesrequest_t *src);
tbcmh_err_t _tbcmh_attributesrequest_destroy(tbcmh_attributesrequest_t *attributesrequest);                          /*!< Destroys the tbcmh_attributesrequest */

int _tbcmh_attributesrequest_get_request_id(tbcmh_attributesrequest_t *attributesrequest);

void _tbcmh_attributesrequest_do_response(tbcmh_attributesrequest_t *attributesrequest); //(none/resend/destroy/_destroy_all_attributes)?
void _tbcmh_attributesrequest_do_timeout(tbcmh_attributesrequest_t *attributesrequest); //(none/resend/destroy/_destroy_all_attributes)?


////tbcmh_err_t _tbcmh_attributesrequest_add(tbcmh_attributesrequest_t *attributesrequest, int count, /*tbcmh_attribute_handle_t attribute,*/ ...);

////tbcmh_err_t _tbcmh_attributesrequest_get_client_keys(tbcmh_attributesrequest_t *attributesrequest, char *buffer, int size);
////tbcmh_err_t _tbcmh_attributesrequest_get_shared_keys(tbcmh_attributesrequest_t * attributesrequest, char *buffer, int size);

////tbcmh_attribute_handle_t _tbcmh_attributesrequest_search_clientattribute(tbcmh_attributesrequest_t *attributesrequest, const char *clientside_attribute_name); /*!< Search the client-side attribute in request */
////tbcmh_attribute_handle_t _tbcmh_attributesrequest_search_sharedattribute(tbcmh_attributesrequest_t *attributesrequest, const char *clientside_attribute_name); /*!< Search the shared attribute in request */

//_pack()/_send()!, _unpack()/deal() //_onAttributesResponse()[unpack->queue], [queue->attribute deal],
//???_destroy_all_attributes(),

#ifdef __cplusplus
}
#endif //__cplusplus

#endif