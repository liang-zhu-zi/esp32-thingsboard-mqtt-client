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

// This file is called by tb_mqtt_client_helper.c/.h.

#include <string.h>
#include "esp_err.h"

#include "client_rpc_observer.h"
#include "tb_mqtt_client_helper_log.h"

const static char *TAG = "client_rpc";

/*!< Initialize tbmch_clientrpc_t */
tbmch_clientrpc_t *_tbmch_clientrpc_init(tbmch_handle_t client, int request_id,
                                         const char *method, ////tbmch_rpc_params_t *params,
                                         void *context,
                                         tbmch_clientrpc_on_response_t on_response,
                                         tbmch_clientrpc_on_timeout_t on_timeout)
{
    if (!method) {
        TBMCH_LOGE("method is NULL");
        return NULL;
    }
    if (!on_response) {
        TBMCH_LOGE("on_response is NULL");
        return NULL;
    }
    
    tbmch_clientrpc_t *clientrpc = TBMCH_MALLOC(sizeof(tbmch_clientrpc_t));
    if (!clientrpc) {
        TBMCH_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(clientrpc, 0x00, sizeof(tbmch_clientrpc_t));
    clientrpc->client = client;
    clientrpc->method = TBMCH_MALLOC(strlen(method)+1);
    if (clientrpc->method) {
        strcpy(clientrpc->method, method);
    }
    clientrpc->request_id = request_id;
    clientrpc->context = context;
    clientrpc->on_response = on_response;
    clientrpc->on_timeout = on_timeout;
    return clientrpc;
}

tbmch_clientrpc_t *_tbmch_clientrpc_clone_wo_listentry(tbmch_clientrpc_t *src)
{
    if (!src) {
        TBMCH_LOGE("src is NULL");
        return NULL;
    }
    
    tbmch_clientrpc_t *clientrpc = TBMCH_MALLOC(sizeof(tbmch_clientrpc_t));
    if (!clientrpc) {
        TBMCH_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(clientrpc, 0x00, sizeof(tbmch_clientrpc_t));
    clientrpc->client = src->client;
    clientrpc->method = TBMCH_MALLOC(strlen(src->method)+1);
    if (clientrpc->method) {
        strcpy(clientrpc->method, src->method);
    }
    clientrpc->request_id = src->request_id;
    clientrpc->context = src->context;
    clientrpc->on_response = src->on_response;
    clientrpc->on_timeout = src->on_timeout;
    return clientrpc;
}

int _tbmch_clientrpc_get_request_id(tbmch_clientrpc_t *clientrpc)
{
    if (!clientrpc) {
        TBMCH_LOGE("clientrpc is NULL");
        return -1;
    }

    return clientrpc->request_id;
}

/*!< Destroys the tbmch_clientrpc_t */
tbmch_err_t _tbmch_clientrpc_destroy(tbmch_clientrpc_t *clientrpc)
{
    if (!clientrpc) {
        TBMCH_LOGE("clientrpc is NULL");
        return ESP_FAIL;
    }

    TBMCH_FREE(clientrpc->method);
    TBMCH_FREE(clientrpc);
    return ESP_OK;
}

void _tbmch_clientrpc_do_response(tbmch_clientrpc_t *clientrpc, const tbmch_rpc_results_t *results)
{
    if (!clientrpc) {
        TBMCH_LOGE("clientrpc is NULL");
        return; // ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, clientrpc->key);;
    if (!value) {
        TBMCH_LOGW("value is NULL! key=%s", clientrpc->key);
        return; // ESP_FAIL;
    }*/

    clientrpc->on_response(clientrpc->client, clientrpc->context, clientrpc->request_id, clientrpc->method, results);
    return; // ESP_OK;
}

void _tbmch_clientrpc_do_timeout(tbmch_clientrpc_t *clientrpc)
{
    if (!clientrpc) {
        TBMCH_LOGE("clientrpc is NULL");
        return; // ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, clientrpc->key);;
    if (!value) {
        TBMCH_LOGW("value is NULL! key=%s", clientrpc->key);
        return; // ESP_FAIL;
    }*/

    if (clientrpc->on_timeout) {
        clientrpc->on_timeout(clientrpc->client, clientrpc->context, clientrpc->request_id, clientrpc->method);
    }
    return; // ESP_OK;
}
