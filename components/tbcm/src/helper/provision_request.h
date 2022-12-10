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

#ifndef _TBCMH_PROVISION_OBSERVER_H_
#define _TBCMH_PROVISION_OBSERVER_H_

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ThingsBoard MQTT Client Helper client-RPC
 */
typedef struct provision
{
     tbcmh_handle_t client;        /*!< ThingsBoard MQTT Client Helper */

     ////const char *params_key;   /*!< params key, default "params" */
     ////const char *results_key;  /*!< results key, default "results" */

     //char *method; /*!< method value */
     tbcmh_provision_params_t *params;
     uint32_t request_id;
     uint64_t timestamp; /*!< time stamp at sending request */

     void *context;                             /*!< Context of callback */
     tbcmh_provision_on_response_t on_response; /*!< Callback of provision response success */
     tbcmh_provision_on_timeout_t on_timeout;   /*!< Callback of provision response timeout */

     LIST_ENTRY(provision) entry;
} provision_t;

typedef LIST_HEAD(tbcmh_provision_list, provision) provision_list_t;

void _tbcmh_provision_on_create(tbcmh_handle_t client);
void _tbcmh_provision_on_destroy(tbcmh_handle_t client);
void _tbcmh_provision_on_connected(tbcmh_handle_t client);
void _tbcmh_provision_on_disconnected(tbcmh_handle_t client);
void _tbcmh_provision_on_data(tbcmh_handle_t client, uint32_t request_id, const tbcmh_provision_results_t *provision_results);
void _tbcmh_provision_on_check_timeout(tbcmh_handle_t client, uint64_t timestamp);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
