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

const static char *TAG = "clientrpc";

/*!< Initialize clientrpc_t */
static clientrpc_t *_clientrpc_create(tbcmh_handle_t client, uint32_t request_id,
                                         const char *method, ////tbcmh_rpc_params_t *params,
                                         void *context,
                                         tbcmh_clientrpc_on_response_t on_response,
                                         tbcmh_clientrpc_on_timeout_t on_timeout)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(method, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_response, NULL);

    clientrpc_t *clientrpc = TBC_MALLOC(sizeof(clientrpc_t));
    if (!clientrpc) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(clientrpc, 0x00, sizeof(clientrpc_t));
    clientrpc->client = client;
    clientrpc->method = TBC_MALLOC(strlen(method)+1);
    if (clientrpc->method) {
        strcpy(clientrpc->method, method);
    }
    clientrpc->request_id = request_id;
    clientrpc->timestamp = (uint64_t)time(NULL);
    clientrpc->context = context;
    clientrpc->on_response = on_response;
    clientrpc->on_timeout = on_timeout;
    return clientrpc;
}

/*!< Destroys the clientrpc_t */
static tbc_err_t _clientrpc_destroy(clientrpc_t *clientrpc)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(clientrpc, ESP_FAIL);

    TBC_FREE(clientrpc->method);
    TBC_FREE(clientrpc);
    return ESP_OK;
}

//============ Client-side RPC ============================================================

void _tbcmh_clientrpc_on_create(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // list create
    memset(&client->clientrpc_list, 0x00, sizeof(client->clientrpc_list)); //client->clientrpc_list = LIST_HEAD_INITIALIZER(client->clientrpc_list);

    // Give semaphore
    // xSemaphoreGiveRecursive(client->_lock);
}

void _tbcmh_clientrpc_on_destroy(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    memset(&client->clientrpc_list, 0x00, sizeof(client->clientrpc_list));

    // Give semaphore
    // xSemaphoreGiveRecursive(client->_lock);
}

void _tbcmh_clientrpc_on_connected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    //TBC_CHECK_PTR(client)
}

void _tbcmh_clientrpc_on_disconnected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // remove all item in clientrpc_list
    _tbcmh_clientrpc_on_check_timeout(client, (uint64_t)time(NULL)+ TB_MQTT_TIMEOUT + 2);
    memset(&client->clientrpc_list, 0x00, sizeof(client->clientrpc_list));

    // Give semaphore
    // xSemaphoreGiveRecursive(client->_lock);
}

//add list
//return 0/ESP_OK on successful, otherwise return -1/ESP_FAIL
tbc_err_t tbcmh_oneway_clientrpc_request(tbcmh_handle_t client, const char *method,
                                        const tbcmh_rpc_params_t *params)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(method, ESP_FAIL);

     if (!tbcmh_is_connected(client)) {
         TBC_LOGW("It still not connnected to servers! %s()", __FUNCTION__);
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
     // Send msg to server
     uint32_t request_id = _tbcmh_get_request_id(client);
     // if (request_id <= 0) {
     //      TBC_LOGE("failure to getting request id!");
     //      return -1;
     // }
     int msg_id;
     if (params) {
         char *params_str = cJSON_PrintUnformatted(params); //cJSON_Print(object);
         msg_id = tbcm_clientrpc_request_ex(client->tbmqttclient, method, params_str,
                          request_id,
                          1/*qos*/, 0/*retain*/);
         cJSON_free(params_str); // free memory
     } else {
         msg_id = tbcm_clientrpc_request_ex(client->tbmqttclient, method, "{}",
                          request_id,
                          1/*qos*/, 0/*retain*/);     
     }
     //cJSON_Delete(object); // delete json object
     if (msg_id<0) {
          TBC_LOGE("Init tbcm_clientrpc_request failure! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     return ESP_OK;
}

//create to add to LIST_ENTRY(tbcmh_clientrpc_)
//return 0/ESP_OK on successful, otherwise return -1/ESP_FAIL
tbc_err_t tbcmh_twoway_clientrpc_request(tbcmh_handle_t client, const char *method, 
                                       const tbcmh_rpc_params_t *params,
                                       void *context,
                                       tbcmh_clientrpc_on_response_t on_response,
                                       tbcmh_clientrpc_on_timeout_t on_timeout)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(method, ESP_FAIL);

     // Take semaphore
     if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     if (!tbcmh_is_connected(client)) {
         TBC_LOGW("It still not connnected to servers! %s()", __FUNCTION__);
         xSemaphoreGiveRecursive(client->_lock);
         return ESP_FAIL;
     }

     // NOTE: It must subscribe response topic, then send request!
     // Subscript topic <===  empty->non-empty
     if (tbcmh_is_connected(client) && LIST_EMPTY(&client->clientrpc_list)) {
         int msg_id = tbcm_subscribe(client->tbmqttclient,
                            TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE, 0);
         TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s",
                            msg_id, TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE);
     }

     // Send msg to server
     //cJSON *object = cJSON_CreateObject(); // create json object
     //cJSON_AddStringToObject(object, TB_MQTT_KEY_RPC_METHOD, method);
     //if (params)
     //     cJSON_AddItemReferenceToObject(object, TB_MQTT_KEY_RPC_PARAMS, params);
     //else 
     //     cJSON_AddNullToObject(object, TB_MQTT_KEY_RPC_PARAMS);
     //char *params_str = cJSON_PrintUnformatted(object); //cJSON_Print(object);
     // Send msg to server
     uint32_t request_id = _tbcmh_get_request_id(client);
     int msg_id;
     if (params) {
         char *params_str = cJSON_PrintUnformatted(params); //cJSON_Print(object);
         msg_id = tbcm_clientrpc_request_ex(client->tbmqttclient, method, params_str,
                                  request_id,
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
          xSemaphoreGiveRecursive(client->_lock);
          return ESP_FAIL;
     }

     // Create clientrpc
     clientrpc_t *clientrpc = _clientrpc_create(client, request_id, method, context, on_response, on_timeout);
     if (!clientrpc) {
          TBC_LOGE("Init clientrpc failure! %s()", __FUNCTION__);
          xSemaphoreGiveRecursive(client->_lock);
          return ESP_FAIL;
     }

     // Insert clientrpc to list
     clientrpc_t *it, *last = NULL;
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
     xSemaphoreGiveRecursive(client->_lock);
     return ESP_OK; //request_id;
}

//on response
void _tbcmh_clientrpc_on_data(tbcmh_handle_t client, uint32_t request_id, const cJSON *object)
{
     TBC_CHECK_PTR(client);
     TBC_CHECK_PTR(object);

     // Take semaphore
     // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
     //      return;
     // }

     bool isEmptyBefore = LIST_EMPTY(&client->clientrpc_list);
    
     // Search clientrpc
     clientrpc_t *clientrpc = NULL;
     LIST_FOREACH(clientrpc, &client->clientrpc_list, entry) {
          if (clientrpc && (clientrpc->request_id==request_id)) {
              LIST_REMOVE(clientrpc, entry);
              break;
          }
     }

     // Subscript topic <===  empty->non-empty
     if (tbcmh_is_connected(client) && !isEmptyBefore && LIST_EMPTY(&client->clientrpc_list)) {
        int msg_id = tbcm_unsubscribe(client->tbmqttclient,
                            TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE);
        TBC_LOGI("sent unsubscribe successful, msg_id=%d, topic=%s",
                            msg_id, TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE);
     }

     // Give semaphore
     // xSemaphoreGiveRecursive(client->_lock);

     if (!clientrpc) {
          TBC_LOGW("Unable to find client-rpc:%u! %s()", request_id, __FUNCTION__);
          return;
     }

     // Do response
     if (clientrpc->on_response) {
        clientrpc->on_response(clientrpc->client, clientrpc->context,
                            clientrpc->method, //clientrpc->request_id,
                            cJSON_GetObjectItem(object, TB_MQTT_KEY_RPC_RESULTS));
     }

     // Free cache
     _clientrpc_destroy(clientrpc);
}

void _tbcmh_clientrpc_on_check_timeout(tbcmh_handle_t client, uint64_t timestamp)
{
     TBC_CHECK_PTR(client);

     // Take semaphore
     // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
     //      return;
     // }

     bool isEmptyBefore = LIST_EMPTY(&client->clientrpc_list);

     // Search & move timeout item to timeout_list
     clientrpc_list_t timeout_list = LIST_HEAD_INITIALIZER(timeout_list);
     clientrpc_t *request = NULL, *next;
     LIST_FOREACH_SAFE(request, &client->clientrpc_list, entry, next) {
          if (request && request->timestamp + TB_MQTT_TIMEOUT <= timestamp) {
               LIST_REMOVE(request, entry);
               // append to timeout list
               clientrpc_t *it, *last = NULL;
               if (LIST_FIRST(&timeout_list) == NULL) {
                    LIST_INSERT_HEAD(&timeout_list, request, entry);
               } else {
                    LIST_FOREACH(it, &timeout_list, entry) {
                         last = it;
                    }
                    if (it == NULL) {
                         assert(last);
                         LIST_INSERT_AFTER(last, request, entry);
                    }
               }
          }
     }

     // Subscript topic <===  empty->non-empty
     if (tbcmh_is_connected(client) && !isEmptyBefore && LIST_EMPTY(&client->clientrpc_list)) {
        int msg_id = tbcm_unsubscribe(client->tbmqttclient,
                            TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE);
        TBC_LOGI("sent unsubscribe successful, msg_id=%d, topic=%s",
                            msg_id, TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE);
     }

     // Give semaphore
     // xSemaphoreGiveRecursive(client->_lock);

     // Deal timeout
     bool clientIsValid = true;
     LIST_FOREACH_SAFE(request, &timeout_list, entry, next) {
          int result = 0;
          if (clientIsValid && request->on_timeout) {
              result = request->on_timeout(request->client, request->context,
                                    request->method); //request->request_id,
          }
          if (result == 2) { // result is equal to 2 if calling tbcmh_disconnect()/tbcmh_destroy() inside on_timeout()
              clientIsValid = false;
          }

          LIST_REMOVE(request, entry);
          _clientrpc_destroy(request);
     }
}

