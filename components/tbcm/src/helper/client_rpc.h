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

#ifndef _CLIENT_RPC_OBSERVER_H_
#define _CLIENT_RPC_OBSERVER_H_

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ThingsBoard MQTT Client Helper client-RPC
 */
typedef struct clientrpc
{
     tbcmh_handle_t client;        /*!< ThingsBoard MQTT Client Helper */

     ////const char *method_key;   /*!< method key, default "method" */
     ////const char *params_key;   /*!< params key, default "params" */
     ////const char *results_key;  /*!< results key, default "results" */

     char *method; /*!< method value */
     ////tbcmh_rpc_params_t *params;
     uint32_t request_id;
     uint64_t timestamp; /*!< time stamp at sending request */

     void *context;                             /*!< Context of callback */
     tbcmh_clientrpc_on_response_t on_response; /*!< Callback of client-rpc response success */
     tbcmh_clientrpc_on_timeout_t on_timeout;   /*!< Callback of client-rpc response timeout */

     LIST_ENTRY(clientrpc) entry;
} clientrpc_t;

typedef LIST_HEAD(tbcmh_clientrpc_list, clientrpc) clientrpc_list_t;

void _tbcmh_clientrpc_on_create(tbcmh_handle_t client);
void _tbcmh_clientrpc_on_destroy(tbcmh_handle_t client);
void _tbcmh_clientrpc_on_connected(tbcmh_handle_t client);
void _tbcmh_clientrpc_on_disconnected(tbcmh_handle_t client);
void _tbcmh_clientrpc_on_data(tbcmh_handle_t client, uint32_t request_id, const cJSON *object);
void _tbcmh_clientrpc_on_check_timeout(tbcmh_handle_t client, uint64_t timestamp);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
