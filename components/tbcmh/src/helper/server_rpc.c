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

#include "tbc_mqtt_helper_internal.h"

const static char *TAG = "serverrpc";

/*!< Initialize serverrpc */
static serverrpc_t *_serverrpc_create(tbcmh_handle_t client,
                                            const char *method, void *context,
                                            tbcmh_serverrpc_on_request_t on_request)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(method, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_request, NULL);
    
    serverrpc_t *serverrpc = TBC_MALLOC(sizeof(serverrpc_t));
    if (!serverrpc) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(serverrpc, 0x00, sizeof(serverrpc_t));
    serverrpc->client = client;
    serverrpc->method = TBC_MALLOC(strlen(method)+1);
    if (serverrpc->method) {
        strcpy(serverrpc->method, method);
    }
    serverrpc->context = context;
    serverrpc->on_request = on_request;
    return serverrpc;
}

static serverrpc_t * _serverrpc_clone_wo_listentry(serverrpc_t *src)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(src, NULL);

    serverrpc_t *serverrpc = TBC_MALLOC(sizeof(serverrpc_t));
    if (!serverrpc) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(serverrpc, 0x00, sizeof(serverrpc_t));
    serverrpc->client = src->client;
    serverrpc->method = TBC_MALLOC(strlen(src->method)+1);
    if (serverrpc->method) {
        strcpy(serverrpc->method, src->method);
    }
    serverrpc->context = src->context;
    serverrpc->on_request = src->on_request;
    return serverrpc;
}

/*!< Destroys the serverrpc */
static tbc_err_t _serverrpc_destroy(serverrpc_t *serverrpc)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(serverrpc, ESP_FAIL);

    TBC_FREE(serverrpc->method);
    TBC_FREE(serverrpc);
    return ESP_OK;
}

void _tbcmh_serverrpc_on_create(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // list create
    memset(&client->serverrpc_list, 0x00, sizeof(client->serverrpc_list)); //client->serverrpc_list = LIST_HEAD_INITIALIZER(client->serverrpc_list);

    // Give semaphore
    // xSemaphoreGiveRecursive(client->_lock);
}

void _tbcmh_serverrpc_on_destroy(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // items empty - remove all item in serverrpc_list
    serverrpc_t *serverrpc = NULL, *next;
    LIST_FOREACH_SAFE(serverrpc, &client->serverrpc_list, entry, next) {
         // remove from serverrpc list and destory
         LIST_REMOVE(serverrpc, entry);
         _serverrpc_destroy(serverrpc);
    }
    memset(&client->serverrpc_list, 0x00, sizeof(client->serverrpc_list));

    // Give semaphore
    // xSemaphoreGiveRecursive(client->_lock);
}

//Call it before connect()
//return 0/ESP_OK on successful, otherwise return -1/ESP_FAIL
tbc_err_t tbcmh_serverrpc_subscribe(tbcmh_handle_t client,
                                   const char *method, void *context,
                                   tbcmh_serverrpc_on_request_t on_request)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);

     // Take semaphore
     if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Create serverrpc
     serverrpc_t *serverrpc = _serverrpc_create(client, method, context, on_request);
     if (!serverrpc) {
          // Give semaphore
          xSemaphoreGiveRecursive(client->_lock);
          TBC_LOGE("Init serverrpc failure! method=%s. %s()", method, __FUNCTION__);
          return ESP_FAIL;
     }

     bool isEmptyBefore = LIST_EMPTY(&client->serverrpc_list);

     // Insert serverrpc to list
     serverrpc_t *it, *last = NULL;
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

     // Subscript topic <===  empty->non-empty
     if (tbcmh_is_connected(client) && isEmptyBefore && !LIST_EMPTY(&client->serverrpc_list)) {
        int msg_id = tbcm_subscribe(client->tbmqttclient,
                        TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE, 0);
        TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s",
                        msg_id, TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE);
     }

     // Give semaphore
     xSemaphoreGiveRecursive(client->_lock);
     return ESP_OK;
}

// remove from LIST_ENTRY(tbcmh_serverrpc_) & delete
//return 0/ESP_OK on successful, otherwise return -1/ESP_FAIL
tbc_err_t tbcmh_serverrpc_unsubscribe(tbcmh_handle_t client, const char *method)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(method, ESP_FAIL);

     // Take semaphore
     if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     bool isEmptyBefore = LIST_EMPTY(&client->serverrpc_list);

     // Search item
     serverrpc_t *serverrpc = NULL, *next;
     LIST_FOREACH_SAFE(serverrpc, &client->serverrpc_list, entry, next) {
          if (serverrpc && strcmp(serverrpc->method, method)==0) {
             // Remove from list and destroy
             LIST_REMOVE(serverrpc, entry);
             _serverrpc_destroy(serverrpc);
             break;
          }
     }

     // Unsubscript topic <===  non-empty->empty
     if (tbcmh_is_connected(client) && !isEmptyBefore && LIST_EMPTY(&client->serverrpc_list)) {
         int msg_id = tbcm_unsubscribe(client->tbmqttclient,
                            TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE);
         TBC_LOGI("sent unsubscribe successful, msg_id=%d, topic=%s",
                            msg_id, TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE);
     }

     // Give semaphore
     xSemaphoreGiveRecursive(client->_lock);

     if (!serverrpc)  {
          TBC_LOGW("Unable to remove server-rpc:%s! %s()", method, __FUNCTION__);
          return ESP_FAIL;
     }
     return ESP_OK;
}


void _tbcmh_serverrpc_on_connected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)

    if (tbcmh_is_connected(client) && !LIST_EMPTY(&client->serverrpc_list)) {
        int msg_id = tbcm_subscribe(client->tbmqttclient, TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE, 0);
        TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s",
                msg_id, TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE);
    }
}

void _tbcmh_serverrpc_on_disconnected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);
    // ...
}

//on request.
void _tbcmh_serverrpc_on_data(tbcmh_handle_t client, uint32_t request_id, const cJSON *object)
{
     TBC_CHECK_PTR(client);
     TBC_CHECK_PTR(object);

     const char *method = NULL;
     if (cJSON_HasObjectItem(object, TB_MQTT_KEY_RPC_METHOD)) {
          cJSON *methodItem = cJSON_GetObjectItem(object, TB_MQTT_KEY_RPC_METHOD);
          method = cJSON_GetStringValue(methodItem);
     }

     // Take semaphore
     // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
     //      return;
     // }

     serverrpc_t *serverrpc = NULL, *cache = NULL;
     LIST_FOREACH(serverrpc, &client->serverrpc_list, entry) {
          if (serverrpc && strcmp(serverrpc->method, method)==0) {
              // Clone serverrpc
              cache = _serverrpc_clone_wo_listentry(serverrpc);
              break;
          }
     }

     // Give semaphore
     // xSemaphoreGiveRecursive(client->_lock);

     if (!cache) {
          TBC_LOGW("Unable to deal server-rpc:%s! %s()", method, __FUNCTION__);
          return;
     }

     // Do request
     tbcmh_rpc_results_t *result = NULL;
     if (cache && cache->on_request) {
         result = cache->on_request(cache->client, cache->context, request_id, cache->method,
                                    cJSON_GetObjectItem(object, TB_MQTT_KEY_RPC_PARAMS));
     }
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
          tbcm_serverrpc_response(client->tbmqttclient, request_id, response, 1/*qos*/, 0/*retain*/);
          cJSON_free(response); // free memory
          cJSON_Delete(result); // delete json object
          #endif
     }
     // Free serverrpc
     _serverrpc_destroy(cache);

     return;
}

