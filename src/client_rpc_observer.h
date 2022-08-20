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

#ifndef _CLIENT_RPC_OBSERVER_H_
#define _CLIENT_RPC_OBSERVER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//====6.Client-side RPC================================================================================================
typedef tbmc_clientrpc_t* tbmc_clientrpc_handle_t;

/**
 * ThingsBoard MQTT Client client-RPC
 */
typedef struct tbmc_clientrpc_
{
     const char *method_value; /*!< method value */
     const char *method_key;   /*!< method key, default "method" */
     const char *params_key;   /*!< params key, default "params" */
     const char *results_key;  /*!< results key, default "results" */
     void *context;            /*!< Context of callback */

     tbmc_rpc_params_t *params;
     tbmc_clientrpc_success_callback_t on_success; /*!< Callback of client-rpc response success */
     tbmc_clientrpc_timeout_callback_t on_timeout; /*!< Callback of client-rpc response timeout */
} tbmc_clientrpc_t;

//const char *_tbmc_clientrpc_get_method(tbmc_clientrpc_handle_t clientrpc);

//0.   Subscribe topic: client-side RPC response;

//1.    tbmc_clientrpc_of_oneway_request(...)/tbmc_clientrpc_of_oneway_request(...)
//1.1   tbmc_clientrpc_handle_t _tbmc_clientrpc_init(tbmc_client_handle_t client, const char* method, tbmc_rpc_params_t *params, void *context, tbmc_clientrpc_response_callback_t on_response);
//1.1  _tbmc.clientrpc_request_pack(...) 
//1.2  _tbmc.clientrpc_request_send(...); //tbmqttclient_sendClientRpcRequest()

//2    _tbmc.on_clientrpc_response()
//2.1  _tbmc.on_clientrpc_response_unpack(): parse payload* to cJSON*, then push it to queue;
//2.2  _tbmc.on_clientrpc_response_deal(on_response): call a server RPC's on_request callback by method name, then send a replay if on_request callback has a return value of tbmc_rpc_results_t.

//3.   _tbmc.on_clientrpc_timeout(on_timeout)
//3.1  _tbmc.on_clientrpc_response_timeout(on_timeout)

//2.f/3.f tbmc_err_t _tbmc_clientrpc_destory(tbmc_clientrpc_handle_t clientrpc)

//4     tbmc_client_destory(...)
//4.x   tbmc_err_t _tbmc_serverrpc_destory(tbmc_serverrpc_handle_t serverrpc)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
