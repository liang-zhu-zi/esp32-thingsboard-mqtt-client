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

#include <string.h>

#include "esp_err.h"

#include "server_rpc_observer.h"
#include "tbc_utils.h"

const static char *TAG = "server_rpc";

/*!< Initialize tbcmh_serverrpc */
tbcmh_serverrpc_t *_tbcmh_serverrpc_init(tbcmh_handle_t client, const char *method, void *context,
                                         tbcmh_serverrpc_on_request_t on_request)
{
    if (!method) {
        TBC_LOGE("method is NULL");
        return NULL;
    }
    if (!on_request) {
        TBC_LOGE("on_request is NULL");
        return NULL;
    }
    
    tbcmh_serverrpc_t *serverrpc = TBCMH_MALLOC(sizeof(tbcmh_serverrpc_t));
    if (!serverrpc) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(serverrpc, 0x00, sizeof(tbcmh_serverrpc_t));
    serverrpc->client = client;
    serverrpc->method = TBCMH_MALLOC(strlen(method)+1);
    if (serverrpc->method) {
        strcpy(serverrpc->method, method);
    }
    serverrpc->context = context;
    serverrpc->on_request = on_request;
    return serverrpc;
}

tbcmh_serverrpc_t * _tbcmh_serverrpc_clone_wo_listentry(tbcmh_serverrpc_t *src)
{
    if (!src) {
        TBC_LOGE("src is NULL");
        return NULL;
    }

    tbcmh_serverrpc_t *serverrpc = TBCMH_MALLOC(sizeof(tbcmh_serverrpc_t));
    if (!serverrpc) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(serverrpc, 0x00, sizeof(tbcmh_serverrpc_t));
    serverrpc->client = src->client;
    serverrpc->method = TBCMH_MALLOC(strlen(src->method)+1);
    if (serverrpc->method) {
        strcpy(serverrpc->method, src->method);
    }
    serverrpc->context = src->context;
    serverrpc->on_request = src->on_request;
    return serverrpc;
}
/*!< Destroys the tbcmh_serverrpc */
tbcmh_err_t _tbcmh_serverrpc_destroy(tbcmh_serverrpc_t *serverrpc)
{
    if (!serverrpc) {
        TBC_LOGE("serverrpc is NULL");
        return ESP_FAIL;
    }

    TBCMH_FREE(serverrpc->method);
    TBCMH_FREE(serverrpc);
    return ESP_OK;
}

const char *_tbcmh_serverrpc_get_method(tbcmh_serverrpc_t *serverrpc)
{
    if (!serverrpc) {
        TBC_LOGE("serverrpc is NULL");
        return NULL;
    }
    return serverrpc->method;
}

tbcmh_rpc_results_t *_tbcmh_serverrpc_do_request(tbcmh_serverrpc_t *serverrpc, int request_id, tbcmh_rpc_params_t *params)
{
    if (!serverrpc) {
        TBC_LOGE("serverrpc is NULL");
        return NULL;
    }

    return serverrpc->on_request(serverrpc->client, serverrpc->context,
                                 request_id, serverrpc->method, params);
}
