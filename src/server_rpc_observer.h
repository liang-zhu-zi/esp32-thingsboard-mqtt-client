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

#ifndef _SERVER_RPC_OBSERVER_H_
#define _SERVER_RPC_OBSERVER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//====5.Server-side RPC================================================================================================
typedef tbmc_serverrpc_t *tbmc_serverrpc_handle_t;

/**
 * ThingsBoard MQTT Client server-RPC
 */
typedef struct tbmc_serverrpc_
{
     const char *method_value; /*!< method value */
     const char *method_key;   /*!< method key, default "method" */
     const char *params_key;   /*!< params key, default "params" */
     const char *results_key;  /*!< results key, default "results" */
     void *context;            /*!< Context of callback */

     tbmc_serverrpc_request_callback_t on_request; /*!< Callback of server-rpc request */
} tbmc_serverrpc_t;

//const char *_tbmc_serverrpc_get_method(tbmc_serverrpc_handle_t serverrpc);

//0.   Subscribe topic: server-side RPC request;

//1.   tbmc_serverrpc_observer_append(...);
//1.1  tbmc_serverrpc_handle_t _tbmc_serverrpc_init(const char* method, void *context, tbmc_serverrpc_request_callback_t on_request);
//1.2  create to add to LIST_ENTRY(tbmc_serverrpc_)
//1.3  tbmqttclient_addServerRpcEvent()???

//2.     _tbmc.on_serverrpc_request()
//2.1    _tbmc.on_serverrpc_request_unpack(): parse payload* to cJSON*, then push it to queue;
//2.2    _tbmc.on_serverrpc_request_deal(): call a server RPC's on_request callback by method name, then send a replay if on_request callback has a return value of tbmc_rpc_results_t.
//2.3   send serverrpc response, option:
//2.3.1  _tbmc.serverrpc_response_pack(...);
//2.3.2  _tbmc.serverrpc_response_send(...); //esp_err_t tbmc_serverrpc_response(tbmc_client_handle_t client, int request_id, const char* results); //tbmqttclient_sendServerRpcReply()

//3.    tbmc_client_destory(...)
//3.x   esp_err_t _tbmc_serverrpc_destory(tbmc_serverrpc_handle_t serverrpc);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
