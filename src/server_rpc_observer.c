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

#include <string.h>

#include "server_rpc_observer.h"

/*!< Initialize tbmch_serverrpc */
tbmch_serverrpc_t *_tbmch_serverrpc_init(tbmch_handle_t client, const char *method, void *context,
                                         tbmch_serverrpc_request_callback_t on_request)
{
    if (!method) {
        TBMCHLOG_E("method is NULL");
        return NULL;
    }
    if (!on_request) {
        TBMCHLOG_E("on_request is NULL");
        return NULL;
    }
    
    tbmch_serverrpc_t *serverrpc = TBMCH_MALLOC(sizeof(tbmch_serverrpc_t));
    if (!serverrpc) {
        TBMCHLOG_E("Unable to malloc memeory!");
        return NULL;
    }

    memset(serverrpc, 0x00, sizeof(tbmch_serverrpc_t));
    serverrpc->client = client;
    serverrpc->method = TBMCH_MALLOC(strlen(method)+1);
    if (serverrpc->method) {
        strcpy(serverrpc->method, method);
    }
    serverrpc->context = context;
    serverrpc->on_request = on_request;
    return serverrpc;
}

tbmch_serverrpc_t * _tbmch_serverrpc_clone_wo_listentry(tbmch_serverrpc_t *src)
{
    if (!src) {
        TBMCHLOG_E("src is NULL");
        return NULL;
    }

    tbmch_serverrpc_t *serverrpc = TBMCH_MALLOC(sizeof(tbmch_serverrpc_t));
    if (!serverrpc) {
        TBMCHLOG_E("Unable to malloc memeory!");
        return NULL;
    }

    memset(serverrpc, 0x00, sizeof(tbmch_serverrpc_t));
    serverrpc->client = src->client;
    serverrpc->method = TBMCH_MALLOC(strlen(src->method)+1);
    if (serverrpc->method) {
        strcpy(serverrpc->method, src->method);
    }
    serverrpc->context = src->context;
    serverrpc->on_request = src->on_request;
    return serverrpc;
}
/*!< Destroys the tbmch_serverrpc */
tbmch_err_t _tbmch_serverrpc_destroy(tbmch_serverrpc_t *serverrpc)
{
    if (!serverrpc) {
        TBMCHLOG_E("serverrpc is NULL");
        return ESP_FAIL;
    }

    TBMC_FREE(serverrpc->method);
    TBMC_FREE(serverrpc);
    return ESP_OK;
}

const char *_tbmch_serverrpc_get_method(tbmch_serverrpc_t *serverrpc)
{
    if (!serverrpc) {
        TBMCHLOG_E("serverrpc is NULL");
        return NULL;
    }
    return serverrpc->method;
}

tbmch_rpc_results_t *_tbmch_serverrpc_do_request(tbmch_serverrpc_t *serverrpc, int request_id, tbmch_rpc_params_t *params)
{
    if (!serverrpc) {
        TBMCHLOG_E("serverrpc is NULL");
        return NULL;
    }

    return serverrpc->on_request(serverrpc->client, serverrpc->context,
                                 request_id, serverrpc->method, params);
}
