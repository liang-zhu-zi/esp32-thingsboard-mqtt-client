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

#include "tbc_utils.h"

//#include "server_rpc_observer.h"
#include "tbc_mqtt_helper_internal.h"


const static char *TAG = "server_rpc";

/*!< Initialize tbcmh_serverrpc */
static tbcmh_serverrpc_t *_tbcmh_serverrpc_init(tbcmh_handle_t client, const char *method, void *context,
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
    
    tbcmh_serverrpc_t *serverrpc = TBC_MALLOC(sizeof(tbcmh_serverrpc_t));
    if (!serverrpc) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(serverrpc, 0x00, sizeof(tbcmh_serverrpc_t));
    serverrpc->client = client;
    serverrpc->method = TBC_MALLOC(strlen(method)+1);
    if (serverrpc->method) {
        strcpy(serverrpc->method, method);
    }
    serverrpc->context = context;
    serverrpc->on_request = on_request;
    return serverrpc;
}

static tbcmh_serverrpc_t * _tbcmh_serverrpc_clone_wo_listentry(tbcmh_serverrpc_t *src)
{
    if (!src) {
        TBC_LOGE("src is NULL");
        return NULL;
    }

    tbcmh_serverrpc_t *serverrpc = TBC_MALLOC(sizeof(tbcmh_serverrpc_t));
    if (!serverrpc) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(serverrpc, 0x00, sizeof(tbcmh_serverrpc_t));
    serverrpc->client = src->client;
    serverrpc->method = TBC_MALLOC(strlen(src->method)+1);
    if (serverrpc->method) {
        strcpy(serverrpc->method, src->method);
    }
    serverrpc->context = src->context;
    serverrpc->on_request = src->on_request;
    return serverrpc;
}
/*!< Destroys the tbcmh_serverrpc */
static tbc_err_t _tbcmh_serverrpc_destroy(tbcmh_serverrpc_t *serverrpc)
{
    if (!serverrpc) {
        TBC_LOGE("serverrpc is NULL");
        return ESP_FAIL;
    }

    TBC_FREE(serverrpc->method);
    TBC_FREE(serverrpc);
    return ESP_OK;
}

static const char *_tbcmh_serverrpc_get_method(tbcmh_serverrpc_t *serverrpc)
{
    if (!serverrpc) {
        TBC_LOGE("serverrpc is NULL");
        return NULL;
    }
    return serverrpc->method;
}

static tbcmh_rpc_results_t *_tbcmh_serverrpc_do_request(tbcmh_serverrpc_t *serverrpc, int request_id, tbcmh_rpc_params_t *params)
{
    if (!serverrpc) {
        TBC_LOGE("serverrpc is NULL");
        return NULL;
    }

    return serverrpc->on_request(serverrpc->client, serverrpc->context,
                                 request_id, serverrpc->method, params);
}


//====30.Server-side RPC================================================================================================
//Call it before connect()
tbc_err_t tbcmh_serverrpc_append(tbcmh_handle_t client_, const char *method,
                                   void *context,
                                   tbcmh_serverrpc_on_request_t on_request)
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Create serverrpc
     tbcmh_serverrpc_t *serverrpc = _tbcmh_serverrpc_init(client, method, context, on_request);
     if (!serverrpc) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init serverrpc failure! method=%s. %s()", method, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert serverrpc to list
     tbcmh_serverrpc_t *it, *last = NULL;
     if (LIST_FIRST(&client->serverrpc_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->serverrpc_list, serverrpc, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->serverrpc_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, serverrpc, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

// remove from LIST_ENTRY(tbcmh_serverrpc_) & delete
tbc_err_t tbcmh_serverrpc_clear(tbcmh_handle_t client_, const char *method)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !method) {
          TBC_LOGE("client or method is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Search item
     tbcmh_serverrpc_t *serverrpc = NULL;
     LIST_FOREACH(serverrpc, &client->serverrpc_list, entry) {
          if (serverrpc && strcmp(_tbcmh_serverrpc_get_method(serverrpc), method)==0) {
               break;
          }
     }
     if (!serverrpc)  {
          TBC_LOGW("Unable to remove server-rpc:%s! %s()", method, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(serverrpc, entry);
     _tbcmh_serverrpc_destroy(serverrpc);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

/*static*/ tbc_err_t _tbcmh_serverrpc_empty(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in serverrpc_list
     tbcmh_serverrpc_t *serverrpc = NULL, *next;
     LIST_FOREACH_SAFE(serverrpc, &client->serverrpc_list, entry, next) {
          // remove from serverrpc list and destory
          LIST_REMOVE(serverrpc, entry);
          _tbcmh_serverrpc_destroy(serverrpc);
     }
     memset(&client->serverrpc_list, 0x00, sizeof(client->serverrpc_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

/*static*/ void _tbcmh_serverrpc_on_request(tbcmh_handle_t client_, int request_id, const cJSON *object)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !object) {
          TBC_LOGE("client or object is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     const char *method = NULL;
     if (cJSON_HasObjectItem(object, TB_MQTT_KEY_RPC_METHOD)) {
          cJSON *methodItem = cJSON_GetObjectItem(object, TB_MQTT_KEY_RPC_METHOD);
          method = cJSON_GetStringValue(methodItem);
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search item
     tbcmh_serverrpc_t *serverrpc = NULL;
     LIST_FOREACH(serverrpc, &client->serverrpc_list, entry) {
          if (serverrpc && strcmp(_tbcmh_serverrpc_get_method(serverrpc), method)==0) {
               break;
          }
     }
     if (!serverrpc) {
          TBC_LOGW("Unable to deal server-rpc:%s! %s()", method, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return;// ESP_OK;
     }

     // Clone serverrpc
     tbcmh_serverrpc_t *cache = _tbcmh_serverrpc_clone_wo_listentry(serverrpc);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do request
     tbcmh_rpc_results_t *result = _tbcmh_serverrpc_do_request(cache, request_id,
                                                               cJSON_GetObjectItem(object, TB_MQTT_KEY_RPC_PARAMS));
     // Send reply
     if (result) {
          #if 0
          cJSON* reply = cJSON_CreateObject();
          cJSON_AddStringToObject(reply, TB_MQTT_KEY_RPC_METHOD, method);
          cJSON_AddItemToObject(reply, TB_MQTT_KEY_RPC_RESULTS, result);
          const char *response = cJSON_PrintUnformatted(reply); //cJSON_Print()
          tbcm_serverrpc_response(client_->tbmqttclient, request_id, response, 1/*qos*/, 0/*retain*/);
          cJSON_free(response); // free memory
          cJSON_Delete(reply); // delete json object
          #else
          char *response = cJSON_PrintUnformatted(result); //cJSON_Print(result);
          tbcm_serverrpc_response(client_->tbmqttclient, request_id, response, 1/*qos*/, 0/*retain*/);
          cJSON_free(response); // free memory
          cJSON_Delete(result); // delete json object
          #endif
     }
     // Free serverrpc
     _tbcmh_serverrpc_destroy(cache);

     return;// ESP_OK;
}

