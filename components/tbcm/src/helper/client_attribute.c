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

//#include "client_attribute.h"
#include "tbc_mqtt_helper_internal.h"

const static char *TAG = "client_attribute";

static client_attribute_t *_client_attribute_create(tbcmh_handle_t client, const char *key, void *context,
                                                    tbcmh_clientattribute_on_get_t on_get,
                                                    tbcmh_clientattribute_on_set_t on_set)
{
    if (!key) {
        TBC_LOGE("key is NULL");
        return NULL;
    }
    if (!on_get) {
        TBC_LOGE("on_get is NULL");
        return NULL;
    }
    
    client_attribute_t *clientattribute = TBC_MALLOC(sizeof(client_attribute_t));
    if (!clientattribute) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(clientattribute, 0x00, sizeof(client_attribute_t));
    clientattribute->client = client;
    clientattribute->key = TBC_MALLOC(strlen(key)+1);
    if (clientattribute->key) {
        strcpy(clientattribute->key, key);
    }
    clientattribute->context = context;
    clientattribute->on_get = on_get;
    clientattribute->on_set = on_set;
    return clientattribute;
}

/*!< Destroys client_attribute */
static tbc_err_t _client_attribute_destroy(client_attribute_t *clientattribute)
{
    if (!clientattribute) {
        TBC_LOGE("clientattribute is NULL");
        return ESP_FAIL;
    }

    TBC_FREE(clientattribute->key);
    TBC_FREE(clientattribute);
    return ESP_OK;
}

// tbcmh_attribute_of_clientside_init()
static tbc_err_t _tbcmh_clientattribute_xx_append(tbcmh_handle_t client, const char *key, void *context,
                                                  tbcmh_clientattribute_on_get_t on_get,
                                                  tbcmh_clientattribute_on_set_t on_set)
{
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Create clientattribute
     client_attribute_t *clientattribute = _client_attribute_create(client, key, context, on_get, on_set);
     if (!clientattribute) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init clientattribute failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert clientattribute to list
     client_attribute_t *it, *last = NULL;
     if (LIST_FIRST(&client->clientattribute_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->clientattribute_list, clientattribute, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->clientattribute_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, clientattribute, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

tbc_err_t tbcmh_clientattribute_append_with_set(tbcmh_handle_t client, const char *key, void *context,
                                                  tbcmh_clientattribute_on_get_t on_get,
                                                  tbcmh_clientattribute_on_set_t on_set)
{
     if (!on_set)  {
          TBC_LOGE("on_set is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     return _tbcmh_clientattribute_xx_append(client, key, context, on_get, on_set);
}

tbc_err_t tbcmh_clientattribute_append(tbcmh_handle_t client, const char *key, void *context,
                                         tbcmh_clientattribute_on_get_t on_get)
{
     return _tbcmh_clientattribute_xx_append(client, key, context, on_get, NULL);
}

tbc_err_t tbcmh_clientattribute_clear(tbcmh_handle_t client, const char *key)
{
     if (!client || !key) {
          TBC_LOGE("client or key is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Search item
     client_attribute_t *clientattribute = NULL;
     LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
          if (clientattribute && strcmp(clientattribute->key, key)==0) {
               break;
          }
     }
     if (!clientattribute) {
          TBC_LOGW("Unable to remove client attribute: %s! %s()", key, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(clientattribute, entry);
     _client_attribute_destroy(clientattribute);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

tbc_err_t _tbcmh_clientattribute_empty(tbcmh_handle_t client)
{
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

     // remove all item in clientattribute_list
     client_attribute_t *clientattribute = NULL, *next;
     LIST_FOREACH_SAFE(clientattribute, &client->clientattribute_list, entry, next) {
          // remove from clientattribute list and destory
          LIST_REMOVE(clientattribute, entry);
          _client_attribute_destroy(clientattribute);
     }
     memset(&client->clientattribute_list, 0x00, sizeof(client->clientattribute_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

tbc_err_t tbcmh_clientattribute_send(tbcmh_handle_t client, int count, /*const char *key,*/ ...)
{
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (count <= 0) {
          TBC_LOGE("count(%d) is error! %s()", count, __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     int i;
     va_list ap;
     va_start(ap, count);
     cJSON_bool result = false;
     cJSON *object = cJSON_CreateObject(); // create json object
     for (i=0; i<count; i++) {
          const char *key = va_arg(ap, const char*);

          // Search item
          client_attribute_t *clientattribute = NULL;
          LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
               if (clientattribute && strcmp(clientattribute->key, key)==0) {
                    break;
               }
          }

          /// Add clientattribute to package
          if (clientattribute) {
                //_client_attribute_do_get(clientattribute, object); // add item to json object
                cJSON *value = clientattribute->on_get(clientattribute->client, clientattribute->context);
                if (value) {
                    result = cJSON_AddItemToObject(object, clientattribute->key, value);
                } else {
                    TBC_LOGW("value is NULL! key=%s", clientattribute->key);                    
                }
          } else {
               TBC_LOGW("Unable to find client-side attribute:%s! %s()", key, __FUNCTION__);
          }
     }
     va_end(ap);

     // send package...
     int msg_id = -1;
     if (result) {
         char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
         msg_id = tbcm_clientattributes_publish(client->tbmqttclient, pack, 1/*qos*/, 0/*retain*/);
         cJSON_free(pack); // free memory
     }
     cJSON_Delete(object); // delete json object

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return (msg_id > -1) ? ESP_OK : ESP_FAIL;
}

void _tbcmh_clientattribute_on_create(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)
    memset(&client->clientattribute_list, 0x00, sizeof(client->clientattribute_list)); //client->clientattribute_list = LIST_HEAD_INITIALIZER(client->clientattribute_list);
}

void _tbcmh_clientattribute_on_destroy(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)
    _tbcmh_clientattribute_empty(client);
}

void _tbcmh_clientattribute_on_connected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)
    // TODO: ......
}

void _tbcmh_clientattribute_on_disconnected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)
    // TODO: ...
}

//unpack & deal
void _tbcmh_clientattribute_on_received(tbcmh_handle_t client, const cJSON *object)
{
     if (!client || !object) {
          TBC_LOGE("client or object is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // foreach item to set value of clientattribute in lock/unlodk.  Don't call tbcmh's funciton in set value callback!
     client_attribute_t *clientattribute = NULL;
     const char* key = NULL;
     LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
          if (clientattribute) {
               key = clientattribute->key;
               if (cJSON_HasObjectItem(object, key)) {
                    //_client_attribute_do_set(clientattribute, cJSON_GetObjectItem(object, key));
                    clientattribute->on_set(clientattribute->client, clientattribute->context,
                        cJSON_GetObjectItem(object, key));
               }
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return;// ESP_OK;
}

