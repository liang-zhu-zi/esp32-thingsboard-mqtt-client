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

// This file is called by tb_mqtt_client_helper.c/.h.

#ifndef _ATTRIBUTES_REQUEST_OBSERVER_H_
#define _ATTRIBUTES_REQUEST_OBSERVER_H_

#include <stdint.h>
#include <stdbool.h>

#include "sys/queue.h"
#include "tb_mqtt_client_helper.h"

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
typedef struct tbmch_attributesrequest
{
     tbmch_handle_t client;        /*!< ThingsBoard MQTT Client Helper */

     int request_id;

     void *context;                                     /*!< Context of callback*/
     tbmch_attributesrequest_on_response_t on_response; /*!< Callback of dealing successful */
     tbmch_attributesrequest_on_timeout_t on_timeout;   /*!< Callback of response timeout */

     ////LIST_HEAD(tbmch_clientattribute_list, tbmch_clientattribute) clientattribute_list; /*!< client attributes entries */
     ////LIST_HEAD(tbmch_sharedattribute_list, tbmch_sharedattribute) sharedattribute_list; /*!< shared attributes entries */

     LIST_ENTRY(tbmch_attributesrequest) entry;
} tbmch_attributesrequest_t;

tbmch_attributesrequest_t *_tbmch_attributesrequest_init(tbmch_handle_t client, int request_id, void *context,
                                                               tbmch_attributesrequest_on_response_t on_response,
                                                               tbmch_attributesrequest_on_timeout_t on_timeout); /*!< Initialize tbmch_attributesrequest */
tbmch_attributesrequest_t *_tbmch_attributesrequest_clone_wo_listentry(tbmch_attributesrequest_t *src);
tbmch_err_t _tbmch_attributesrequest_destroy(tbmch_attributesrequest_t *attributesrequest);                          /*!< Destroys the tbmch_attributesrequest */

int _tbmch_attributesrequest_get_request_id(tbmch_attributesrequest_t *attributesrequest);

void _tbmch_attributesrequest_do_response(tbmch_attributesrequest_t *attributesrequest); //(none/resend/destroy/_destroy_all_attributes)?
void _tbmch_attributesrequest_do_timeout(tbmch_attributesrequest_t *attributesrequest); //(none/resend/destroy/_destroy_all_attributes)?


////tbmch_err_t _tbmch_attributesrequest_add(tbmch_attributesrequest_t *attributesrequest, int count, /*tbmch_attribute_handle_t attribute,*/ ...);

////tbmch_err_t _tbmch_attributesrequest_get_client_keys(tbmch_attributesrequest_t *attributesrequest, char *buffer, int size);
////tbmch_err_t _tbmch_attributesrequest_get_shared_keys(tbmch_attributesrequest_t * attributesrequest, char *buffer, int size);

////tbmch_attribute_handle_t _tbmch_attributesrequest_search_clientattribute(tbmch_attributesrequest_t *attributesrequest, const char *clientside_attribute_name); /*!< Search the client-side attribute in request */
////tbmch_attribute_handle_t _tbmch_attributesrequest_search_sharedattribute(tbmch_attributesrequest_t *attributesrequest, const char *clientside_attribute_name); /*!< Search the shared attribute in request */

//_pack()/_send()!, _unpack()/deal() //_onAttributesResponse()[unpack->queue], [queue->attribute deal],
//???_destroy_all_attributes(),

#ifdef __cplusplus
}
#endif //__cplusplus

#endif