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

#include <stdint.h>
#include <stdbool.h>

#include "sys/queue.h"

#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//====5.Server-side RPC================================================================================================
/**
 * ThingsBoard MQTT Client Helper server-RPC
 */
typedef struct tbcmh_serverrpc
{
     tbcmh_handle_t client;        /*!< ThingsBoard MQTT Client Helper */

     char *method; /*!< method value */
     ////char *method_key;   /*!< method key, default "method" */
     ////char *params_key;   /*!< params key, default "params" */
     ////char *results_key;  /*!< results key, default "results" */

     void *context;                           /*!< Context of callback */
     tbcmh_serverrpc_on_request_t on_request; /*!< Callback of server-rpc request */

     LIST_ENTRY(tbcmh_serverrpc) entry;
} tbcmh_serverrpc_t;

tbcmh_serverrpc_t *_tbcmh_serverrpc_init(tbcmh_handle_t client, const char *method, void *context,
                                         tbcmh_serverrpc_on_request_t on_request); /*!< Initialize tbcmh_serverrpc */
tbcmh_serverrpc_t *_tbcmh_serverrpc_clone_wo_listentry(tbcmh_serverrpc_t *src);
tbc_err_t _tbcmh_serverrpc_destroy(tbcmh_serverrpc_t *serverrpc); /*!< Destroys the tbcmh_serverrpc */

const char *_tbcmh_serverrpc_get_method(tbcmh_serverrpc_t *serverrpc);

tbcmh_rpc_results_t *_tbcmh_serverrpc_do_request(tbcmh_serverrpc_t *serverrpc, int request_id, tbcmh_rpc_params_t *params);

//0.   Subscribe topic: server-side RPC request;

//1.   tbcmh_serverrpc_observer_append(...);
//1.1  tbcmh_serverrpc_t *_tbcmh_serverrpc_init(const char* method, void *context, tbcmh_serverrpc_request_callback_t on_request);
//1.2  create to add to LIST_ENTRY(tbcmh_serverrpc_)
//1.3  tbmqttclient_addServerRpcEvent()???

//2.     _tbcm.on_serverrpc_request()
//2.1    _tbcm.on_serverrpc_request_unpack(): parse payload* to cJSON*, then push it to queue;
//2.2    _tbcm.on_serverrpc_request_deal(): call a server RPC's on_request callback by method name, then send a replay if on_request callback has a return value of tbcmh_rpc_results_t.
//2.3   send serverrpc response, option:
//2.3.1  _tbcm.serverrpc_response_pack(...);
//2.3.2  _tbcm.serverrpc_response_send(...); //tbc_err_t tbcmh_serverrpc_response(tbcmh_client_handle_t client, int request_id, const char* results); //tbmqttclient_sendServerRpcReply()

//3.    tbcmh_client_destroy(...)
//3.x   tbc_err_t _tbcmh_serverrpc_destroy(tbcmh_serverrpc_t *serverrpc);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
