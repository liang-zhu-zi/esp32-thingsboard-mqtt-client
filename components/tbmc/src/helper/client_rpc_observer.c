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

#include "client_rpc_observer.h"
#include "tbc_utils.h"

const static char *TAG = "client_rpc";

/*!< Initialize tbcmh_clientrpc_t */
tbcmh_clientrpc_t *_tbcmh_clientrpc_init(tbcmh_handle_t client, int request_id,
                                         const char *method, ////tbcmh_rpc_params_t *params,
                                         void *context,
                                         tbcmh_clientrpc_on_response_t on_response,
                                         tbcmh_clientrpc_on_timeout_t on_timeout)
{
    if (!method) {
        TBC_LOGE("method is NULL");
        return NULL;
    }
    if (!on_response) {
        TBC_LOGE("on_response is NULL");
        return NULL;
    }
    
    tbcmh_clientrpc_t *clientrpc = TBCMH_MALLOC(sizeof(tbcmh_clientrpc_t));
    if (!clientrpc) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(clientrpc, 0x00, sizeof(tbcmh_clientrpc_t));
    clientrpc->client = client;
    clientrpc->method = TBCMH_MALLOC(strlen(method)+1);
    if (clientrpc->method) {
        strcpy(clientrpc->method, method);
    }
    clientrpc->request_id = request_id;
    clientrpc->context = context;
    clientrpc->on_response = on_response;
    clientrpc->on_timeout = on_timeout;
    return clientrpc;
}

tbcmh_clientrpc_t *_tbcmh_clientrpc_clone_wo_listentry(tbcmh_clientrpc_t *src)
{
    if (!src) {
        TBC_LOGE("src is NULL");
        return NULL;
    }
    
    tbcmh_clientrpc_t *clientrpc = TBCMH_MALLOC(sizeof(tbcmh_clientrpc_t));
    if (!clientrpc) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(clientrpc, 0x00, sizeof(tbcmh_clientrpc_t));
    clientrpc->client = src->client;
    clientrpc->method = TBCMH_MALLOC(strlen(src->method)+1);
    if (clientrpc->method) {
        strcpy(clientrpc->method, src->method);
    }
    clientrpc->request_id = src->request_id;
    clientrpc->context = src->context;
    clientrpc->on_response = src->on_response;
    clientrpc->on_timeout = src->on_timeout;
    return clientrpc;
}

int _tbcmh_clientrpc_get_request_id(tbcmh_clientrpc_t *clientrpc)
{
    if (!clientrpc) {
        TBC_LOGE("clientrpc is NULL");
        return -1;
    }

    return clientrpc->request_id;
}

/*!< Destroys the tbcmh_clientrpc_t */
tbcmh_err_t _tbcmh_clientrpc_destroy(tbcmh_clientrpc_t *clientrpc)
{
    if (!clientrpc) {
        TBC_LOGE("clientrpc is NULL");
        return ESP_FAIL;
    }

    TBCMH_FREE(clientrpc->method);
    TBCMH_FREE(clientrpc);
    return ESP_OK;
}

void _tbcmh_clientrpc_do_response(tbcmh_clientrpc_t *clientrpc, const tbcmh_rpc_results_t *results)
{
    if (!clientrpc) {
        TBC_LOGE("clientrpc is NULL");
        return; // ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, clientrpc->key);;
    if (!value) {
        TBC_LOGW("value is NULL! key=%s", clientrpc->key);
        return; // ESP_FAIL;
    }*/

    clientrpc->on_response(clientrpc->client, clientrpc->context, clientrpc->request_id, clientrpc->method, results);
    return; // ESP_OK;
}

void _tbcmh_clientrpc_do_timeout(tbcmh_clientrpc_t *clientrpc)
{
    if (!clientrpc) {
        TBC_LOGE("clientrpc is NULL");
        return; // ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, clientrpc->key);;
    if (!value) {
        TBC_LOGW("value is NULL! key=%s", clientrpc->key);
        return; // ESP_FAIL;
    }*/

    if (clientrpc->on_timeout) {
        clientrpc->on_timeout(clientrpc->client, clientrpc->context, clientrpc->request_id, clientrpc->method);
    }
    return; // ESP_OK;
}
