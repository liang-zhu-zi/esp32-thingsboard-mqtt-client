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

// This file is called by tb_mqtt_client_helper.c/.h.

#ifndef _CLIENT_RPC_OBSERVER_H_
#define _CLIENT_RPC_OBSERVER_H_

#include <stdint.h>
#include <stdbool.h>

#include "sys/queue.h"
#include "tb_mqtt_client_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//====6.Client-side RPC================================================================================================
/**
 * ThingsBoard MQTT Client Helper client-RPC
 */
typedef struct tbmch_clientrpc
{
     tbmch_handle_t client;        /*!< ThingsBoard MQTT Client Helper */

     ////const char *method_key;   /*!< method key, default "method" */
     ////const char *params_key;   /*!< params key, default "params" */
     ////const char *results_key;  /*!< results key, default "results" */

     char *method; /*!< method value */
     ////tbmch_rpc_params_t *params;
     int request_id;
     void *context;                             /*!< Context of callback */
     tbmch_clientrpc_on_response_t on_response; /*!< Callback of client-rpc response success */
     tbmch_clientrpc_on_timeout_t on_timeout;   /*!< Callback of client-rpc response timeout */

     LIST_ENTRY(tbmch_clientrpc) entry;
} tbmch_clientrpc_t;

tbmch_clientrpc_t *_tbmch_clientrpc_init(tbmch_handle_t client, int request_id,
                                         const char *method, ////tbmch_rpc_params_t *params,
                                         void *context,
                                         tbmch_clientrpc_on_response_t on_response,
                                         tbmch_clientrpc_on_timeout_t on_timeout); /*!< Initialize tbmch_clientrpc_t */
tbmch_clientrpc_t *_tbmch_clientrpc_clone_wo_listentry(tbmch_clientrpc_t *src);
tbmch_err_t _tbmch_clientrpc_destroy(tbmch_clientrpc_t *clientrpc);                /*!< Destroys the tbmch_clientrpc_t */

int _tbmch_clientrpc_get_request_id(tbmch_clientrpc_t *clientrpc);
void _tbmch_clientrpc_do_response(tbmch_clientrpc_t *clientrpc, const tbmch_rpc_results_t *results);
void _tbmch_clientrpc_do_timeout(tbmch_clientrpc_t *clientrpc);

//const char *_tbmch_clientrpc_get_method(tbmch_clientrpc_t *clientrpc);

//0.   Subscribe topic: client-side RPC response;

//1.    tbmch_clientrpc_of_oneway_request(...)/tbmch_clientrpc_of_oneway_request(...)
//1.1   tbmch_clientrpc_t *_tbmch_clientrpc_init(tbmch_client_handle_t client, const char* method, tbmch_rpc_params_t *params, void *context, tbmch_clientrpc_response_callback_t on_response);
//1.1  _tbmc.clientrpc_request_pack(...) 
//1.2  _tbmc.clientrpc_request_send(...); //tbmqttclient_sendClientRpcRequest()

//2    _tbmc.on_clientrpc_response()
//2.1  _tbmc.on_clientrpc_response_unpack(): parse payload* to cJSON*, then push it to queue;
//2.2  _tbmc.on_clientrpc_response_deal(on_response): call a server RPC's on_request callback by method name, then send a replay if on_request callback has a return value of tbmch_rpc_results_t.

//3.   _tbmc.on_clientrpc_timeout(on_timeout)
//3.1  _tbmc.on_clientrpc_response_timeout(on_timeout)

//2.f/3.f tbmch_err_t _tbmch_clientrpc_destroy(tbmch_clientrpc_t *clientrpc)

//4     tbmch_client_destroy(...)
//4.x   tbmch_err_t _tbmch_serverrpc_destroy(tbmch_serverrpc_handle_t serverrpc)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
