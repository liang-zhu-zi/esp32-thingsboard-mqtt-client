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

//#include "client_rpc_observer.h"
#include "tbc_mqtt_helper_internal.h"

const static char *TAG = "client_rpc";

/*!< Initialize tbcmh_clientrpc_t */
static tbcmh_clientrpc_t *_tbcmh_clientrpc_init(tbcmh_handle_t client, int request_id,
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
    
    tbcmh_clientrpc_t *clientrpc = TBC_MALLOC(sizeof(tbcmh_clientrpc_t));
    if (!clientrpc) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(clientrpc, 0x00, sizeof(tbcmh_clientrpc_t));
    clientrpc->client = client;
    clientrpc->method = TBC_MALLOC(strlen(method)+1);
    if (clientrpc->method) {
        strcpy(clientrpc->method, method);
    }
    clientrpc->request_id = request_id;
    clientrpc->context = context;
    clientrpc->on_response = on_response;
    clientrpc->on_timeout = on_timeout;
    return clientrpc;
}

static tbcmh_clientrpc_t *_tbcmh_clientrpc_clone_wo_listentry(tbcmh_clientrpc_t *src)
{
    if (!src) {
        TBC_LOGE("src is NULL");
        return NULL;
    }
    
    tbcmh_clientrpc_t *clientrpc = TBC_MALLOC(sizeof(tbcmh_clientrpc_t));
    if (!clientrpc) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(clientrpc, 0x00, sizeof(tbcmh_clientrpc_t));
    clientrpc->client = src->client;
    clientrpc->method = TBC_MALLOC(strlen(src->method)+1);
    if (clientrpc->method) {
        strcpy(clientrpc->method, src->method);
    }
    clientrpc->request_id = src->request_id;
    clientrpc->context = src->context;
    clientrpc->on_response = src->on_response;
    clientrpc->on_timeout = src->on_timeout;
    return clientrpc;
}

static int _tbcmh_clientrpc_get_request_id(tbcmh_clientrpc_t *clientrpc)
{
    if (!clientrpc) {
        TBC_LOGE("clientrpc is NULL");
        return -1;
    }

    return clientrpc->request_id;
}

/*!< Destroys the tbcmh_clientrpc_t */
static tbc_err_t _tbcmh_clientrpc_destroy(tbcmh_clientrpc_t *clientrpc)
{
    if (!clientrpc) {
        TBC_LOGE("clientrpc is NULL");
        return ESP_FAIL;
    }

    TBC_FREE(clientrpc->method);
    TBC_FREE(clientrpc);
    return ESP_OK;
}

static void _tbcmh_clientrpc_do_response(tbcmh_clientrpc_t *clientrpc, const tbcmh_rpc_results_t *results)
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

static void _tbcmh_clientrpc_do_timeout(tbcmh_clientrpc_t *clientrpc)
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

//====31.Client-side RPC================================================================================================
/*static*/ tbc_err_t _tbcmh_clientrpc_empty(tbcmh_handle_t client_)
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

     // remove all item in clientrpc_list
     tbcmh_clientrpc_t *clientrpc = NULL, *next;
     LIST_FOREACH_SAFE(clientrpc, &client->clientrpc_list, entry, next) {
          // exec timeout callback
          _tbcmh_clientrpc_do_timeout(clientrpc);

          // remove from clientrpc list and destory
          LIST_REMOVE(clientrpc, entry);
          _tbcmh_clientrpc_destroy(clientrpc);
     }
     memset(&client->clientrpc_list, 0x00, sizeof(client->clientrpc_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

//add list
int tbcmh_clientrpc_of_oneway_request(tbcmh_handle_t client_, const char *method,
                                                           /*const*/ tbcmh_rpc_params_t *params)
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (!method) {
          TBC_LOGE("method is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Send msg to server
     //cJSON *object = cJSON_CreateObject(); // create json object
     //cJSON_AddStringToObject(object, TB_MQTT_KEY_RPC_METHOD, method);
     //if (params) {
     //     cJSON_AddItemReferenceToObject(object, TB_MQTT_KEY_RPC_PARAMS, params);
     //} else  {
     //     cJSON_AddNullToObject(object, TB_MQTT_KEY_RPC_PARAMS);
     //}
     //char *params_str = cJSON_PrintUnformatted(object); //cJSON_Print(object);
     int request_id = _request_list_create_and_append(client, TBCMH_REQUEST_CLIENTRPC, 0/*request_id*/);
     if (request_id <= 0) {
          TBC_LOGE("Unable to create request!");
          return -1;
     }
     int msg_id;
     if (params) {
         char *params_str = cJSON_PrintUnformatted(params); //cJSON_Print(object);
         msg_id = tbcm_clientrpc_request_ex(client->tbmqttclient, method, params_str,
                          request_id,
                          //client,
                          //NULL, //_tbcmh_on_clientrpc_response,
                          //NULL, //_tbcmh_on_clientrpc_timeout,
                          1/*qos*/, 0/*retain*/);
         cJSON_free(params_str); // free memory
     } else {
         msg_id = tbcm_clientrpc_request_ex(client->tbmqttclient, method, "{}",
                          request_id,
                          //client,
                          //NULL, //_tbcmh_on_clientrpc_response,
                          //NULL, //_tbcmh_on_clientrpc_timeout,
                          1/*qos*/, 0/*retain*/);     
     }
     //cJSON_Delete(object); // delete json object
     if (msg_id<0) {
          TBC_LOGE("Init tbcm_clientrpc_request failure! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     return request_id;
}
//create to add to LIST_ENTRY(tbcmh_clientrpc_)
int tbcmh_clientrpc_of_twoway_request(tbcmh_handle_t client_, const char *method, 
                                                           /*const*/ tbcmh_rpc_params_t *params,
                                                           void *context,
                                                           tbcmh_clientrpc_on_response_t on_response,
                                                           tbcmh_clientrpc_on_timeout_t on_timeout)
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (!method) {
          TBC_LOGE("method is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Send msg to server
     //cJSON *object = cJSON_CreateObject(); // create json object
     //cJSON_AddStringToObject(object, TB_MQTT_KEY_RPC_METHOD, method);
     //if (params)
     //     cJSON_AddItemReferenceToObject(object, TB_MQTT_KEY_RPC_PARAMS, params);
     //else 
     //     cJSON_AddNullToObject(object, TB_MQTT_KEY_RPC_PARAMS);
     //char *params_str = cJSON_PrintUnformatted(object); //cJSON_Print(object);
     int request_id = _request_list_create_and_append(client, TBCMH_REQUEST_CLIENTRPC, 0/*request_id*/);
     if (request_id <= 0) {
          TBC_LOGE("Unable to create request!");
          return -1;
     }
     int msg_id;
     if (params) {
         char *params_str = cJSON_PrintUnformatted(params); //cJSON_Print(object);
         msg_id = tbcm_clientrpc_request_ex(client->tbmqttclient, method, params_str,
                                  request_id,
                                  //client,
                                  //_tbcmh_on_clientrpc_response,
                                  //_tbcmh_on_clientrpc_timeout,
                                  1/*qos*/, 0/*retain*/);
         cJSON_free(params_str); // free memory
     } else {
         msg_id = tbcm_clientrpc_request_ex(client->tbmqttclient, method, "{}",
                                  request_id,
                                  //client,
                                  //_tbcmh_on_clientrpc_response,
                                  //_tbcmh_on_clientrpc_timeout,
                                  1/*qos*/, 0/*retain*/);
     }
     //cJSON_Delete(object); // delete json object
     if (msg_id<0) {
          TBC_LOGE("Init tbcm_clientrpc_request failure! %s()", __FUNCTION__);
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Create clientrpc
     tbcmh_clientrpc_t *clientrpc = _tbcmh_clientrpc_init(client, request_id, method, context, on_response, on_timeout);
     if (!clientrpc) {
          TBC_LOGE("Init clientrpc failure! %s()", __FUNCTION__);
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Insert clientrpc to list
     tbcmh_clientrpc_t *it, *last = NULL;
     if (LIST_FIRST(&client->clientrpc_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->clientrpc_list, clientrpc, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->clientrpc_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, clientrpc, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return request_id;
}

/*static*/ void _tbcmh_clientrpc_on_response(tbcmh_handle_t client_, int request_id, const cJSON *object)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !object) {
          TBC_LOGE("client or object is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Remove it from request list
     _request_list_search_and_remove(client, request_id);

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search clientrpc
     tbcmh_clientrpc_t *clientrpc = NULL;
     LIST_FOREACH(clientrpc, &client->clientrpc_list, entry) {
          if (clientrpc && (_tbcmh_clientrpc_get_request_id(clientrpc)==request_id)) {
               break;
          }
     }
     if (!clientrpc) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGW("Unable to find client-rpc:%d! %s()", request_id, __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Cache and remove clientrpc
     tbcmh_clientrpc_t *cache = _tbcmh_clientrpc_clone_wo_listentry(clientrpc);
     LIST_REMOVE(clientrpc, entry);
     _tbcmh_clientrpc_destroy(clientrpc);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do response
     _tbcmh_clientrpc_do_response(cache, cJSON_GetObjectItem(object, TB_MQTT_KEY_RPC_RESULTS));
     // Free cache
     _tbcmh_clientrpc_destroy(cache);

     return;// ESP_OK;
}

/*static*/ void _tbcmh_clientrpc_on_timeout(tbcmh_handle_t client_, int request_id)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search clientrpc
     tbcmh_clientrpc_t *clientrpc = NULL;
     LIST_FOREACH(clientrpc, &client->clientrpc_list, entry) {
          if (clientrpc && (_tbcmh_clientrpc_get_request_id(clientrpc)==request_id)) {
               break;
          }
     }
     if (!clientrpc) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGW("Unable to find client-rpc:%d! %s()", request_id, __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Cache and remove clientrpc
     tbcmh_clientrpc_t *cache = _tbcmh_clientrpc_clone_wo_listentry(clientrpc);
     LIST_REMOVE(clientrpc, entry);
     _tbcmh_clientrpc_destroy(clientrpc);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do timeout
     _tbcmh_clientrpc_do_timeout(cache);
     // Free clientrpc
     _tbcmh_clientrpc_destroy(cache);

     return;// ESP_OK;
}
