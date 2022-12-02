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

#ifndef _SERVER_RPC_OBSERVER_H_
#define _SERVER_RPC_OBSERVER_H_

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ThingsBoard MQTT Client Helper server-RPC
 */
typedef struct serverrpc
{
     tbcmh_handle_t client;        /*!< ThingsBoard MQTT Client Helper */

     char *method; /*!< method value */
     ////char *method_key;   /*!< method key, default "method" */
     ////char *params_key;   /*!< params key, default "params" */
     ////char *results_key;  /*!< results key, default "results" */

     void *context;                           /*!< Context of callback */
     tbcmh_serverrpc_on_request_t on_request; /*!< Callback of server-rpc request */

     LIST_ENTRY(serverrpc) entry;
} serverrpc_t;

typedef LIST_HEAD(tbcmh_serverrpc_list, serverrpc) serverrpc_list_t;

void _tbcmh_serverrpc_on_create(tbcmh_handle_t client);
void _tbcmh_serverrpc_on_destroy(tbcmh_handle_t client);
void _tbcmh_serverrpc_on_connected(tbcmh_handle_t client);
void _tbcmh_serverrpc_on_disconnected(tbcmh_handle_t client);
void _tbcmh_serverrpc_on_data(tbcmh_handle_t client, uint32_t request_id, const cJSON *object);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
