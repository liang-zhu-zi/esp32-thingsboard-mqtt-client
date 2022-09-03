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

#ifndef _SERVER_RPC_OBSERVER_H_
#define _SERVER_RPC_OBSERVER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//====5.Server-side RPC================================================================================================
/**
 * ThingsBoard MQTT Client Helper server-RPC
 */
typedef struct tbmch_serverrpc
{
     tbmch_handle_t client;        /*!< ThingsBoard MQTT Client Helper */

     const char *method; /*!< method value */
     ////const char *method_key;   /*!< method key, default "method" */
     ////const char *params_key;   /*!< params key, default "params" */
     ////const char *results_key;  /*!< results key, default "results" */

     void *context;                                 /*!< Context of callback */
     tbmch_serverrpc_on_request_t on_request; /*!< Callback of server-rpc request */

     LIST_ENTRY(tbmch_serverrpc) entry;
} tbmch_serverrpc_t;

tbmch_serverrpc_t *_tbmch_serverrpc_init(tbmch_handle_t client, const char *method, void *context,
                                         tbmch_serverrpc_on_request_t on_request); /*!< Initialize tbmch_serverrpc */
tbmch_err_t _tbmch_serverrpc_destroy(tbmch_serverrpc_t *serverrpc);                      /*!< Destroys the tbmch_serverrpc */

const char *_tbmch_serverrpc_get_method(tbmch_serverrpc_t *serverrpc);

tbmch_rpc_results_t *_tbmch_serverrpc_do_request(tbmch_serverrpc_t *serverrpc, int request_id, tbmch_rpc_params_t *params);

//0.   Subscribe topic: server-side RPC request;

//1.   tbmch_serverrpc_observer_append(...);
//1.1  tbmch_serverrpc_t *_tbmch_serverrpc_init(const char* method, void *context, tbmch_serverrpc_request_callback_t on_request);
//1.2  create to add to LIST_ENTRY(tbmch_serverrpc_)
//1.3  tbmqttclient_addServerRpcEvent()???

//2.     _tbmc.on_serverrpc_request()
//2.1    _tbmc.on_serverrpc_request_unpack(): parse payload* to cJSON*, then push it to queue;
//2.2    _tbmc.on_serverrpc_request_deal(): call a server RPC's on_request callback by method name, then send a replay if on_request callback has a return value of tbmch_rpc_results_t.
//2.3   send serverrpc response, option:
//2.3.1  _tbmc.serverrpc_response_pack(...);
//2.3.2  _tbmc.serverrpc_response_send(...); //tbmch_err_t tbmch_serverrpc_response(tbmch_client_handle_t client, int request_id, const char* results); //tbmqttclient_sendServerRpcReply()

//3.    tbmch_client_destroy(...)
//3.x   tbmch_err_t _tbmch_serverrpc_destroy(tbmch_serverrpc_t *serverrpc);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
