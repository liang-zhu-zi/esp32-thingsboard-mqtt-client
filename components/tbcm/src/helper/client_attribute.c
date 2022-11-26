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

const static char *TAG = "clientattribute";

static clientattribute_t *_clientattribute_create(tbcmh_handle_t client,
                                            const char *key, void *context,
                                            tbcmh_clientattribute_on_get_t on_get,
                                            tbcmh_clientattribute_on_set_t on_set)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(key, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_get, NULL);
    
    clientattribute_t *clientattribute = TBC_MALLOC(sizeof(clientattribute_t));
    if (!clientattribute) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(clientattribute, 0x00, sizeof(clientattribute_t));
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

/*!< Destroys clientattribute */
static tbc_err_t _clientattribute_destroy(clientattribute_t *clientattribute)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(clientattribute, ESP_FAIL);

    TBC_FREE(clientattribute->key);
    TBC_FREE(clientattribute);
    return ESP_OK;
}

//Call it before connect()
static tbc_err_t _clientattribute_register(tbcmh_handle_t client,
                                                  const char *key, void *context,
                                                  tbcmh_clientattribute_on_get_t on_get,
                                                  tbcmh_clientattribute_on_set_t on_set)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Create clientattribute
     clientattribute_t *clientattribute = _clientattribute_create(client, key, context, on_get, on_set);
     if (!clientattribute) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init clientattribute failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert clientattribute to list
     clientattribute_t *it, *last = NULL;
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

tbc_err_t tbcmh_clientattribute_register_with_set(tbcmh_handle_t client,
                                        const char *key, void *context,
                                        tbcmh_clientattribute_on_get_t on_get,
                                        tbcmh_clientattribute_on_set_t on_set)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(on_set, ESP_FAIL);
     return _clientattribute_register(client, key, context, on_get, on_set);
}

tbc_err_t tbcmh_clientattribute_register(tbcmh_handle_t client,
                                         const char *key, void *context,
                                         tbcmh_clientattribute_on_get_t on_get)
{
     return _clientattribute_register(client, key, context, on_get, NULL);
}

tbc_err_t tbcmh_clientattribute_unregister(tbcmh_handle_t client, const char *key)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(key, ESP_FAIL);

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Search item
     clientattribute_t *clientattribute = NULL, *next;;
     LIST_FOREACH_SAFE(clientattribute, &client->clientattribute_list, entry, next) {
          if (clientattribute && strcmp(clientattribute->key, key)==0) {
             // Remove from list and destroy
             LIST_REMOVE(clientattribute, entry);
             _clientattribute_destroy(clientattribute);
             break;
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);

     if (!clientattribute) {
          TBC_LOGW("Unable to remove client attribute: %s! %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }
     return ESP_OK;
}

tbc_err_t tbcmh_clientattribute_update(tbcmh_handle_t client,
                                    int count, /*const char *key,*/ ...)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
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
     for (i=0; object && i<count; i++) {
          const char *key = va_arg(ap, const char*);

          // Search item
          clientattribute_t *clientattribute = NULL;
          LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
               if (clientattribute && strcmp(clientattribute->key, key)==0) {
                    break;
               }
          }

          /// Add clientattribute to package
          if (clientattribute && clientattribute->on_get) {
                // add item to json object
                cJSON *value = clientattribute->on_get(clientattribute->client, clientattribute->context);
                if (value) {
                    result |= cJSON_AddItemToObject(object, clientattribute->key, value);
                } else {
                    TBC_LOGW("value is NULL! key=%s", clientattribute->key);                    
                }
          } else {
               TBC_LOGW("Unable to find&send client-side attribute:%s! %s()", key, __FUNCTION__);
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
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // list create
    memset(&client->clientattribute_list, 0x00, sizeof(client->clientattribute_list)); //client->clientattribute_list = LIST_HEAD_INITIALIZER(client->clientattribute_list);

    // Give semaphore
    // xSemaphoreGive(client->_lock);
}

void _tbcmh_clientattribute_on_destroy(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // TODO: How to add lock??
    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // items empty - remove all item in clientattribute_list
    clientattribute_t *clientattribute = NULL, *next;
    LIST_FOREACH_SAFE(clientattribute, &client->clientattribute_list, entry, next) {
         // remove from clientattribute list and destory
         LIST_REMOVE(clientattribute, entry);
         _clientattribute_destroy(clientattribute);
    }
    memset(&client->clientattribute_list, 0x00, sizeof(client->clientattribute_list));

    // Give semaphore
    // xSemaphoreGive(client->_lock);
}

void _tbcmh_clientattribute_on_connected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);
    //......
}

void _tbcmh_clientattribute_on_disconnected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);
    // ...
}

//on received init value in attributes response: unpack & deal
void _tbcmh_clientattribute_on_data(tbcmh_handle_t client, const cJSON *object)
{
     TBC_CHECK_PTR(client);
     TBC_CHECK_PTR(object);

     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
     //      return;
     // }

     // foreach item to set value of clientattribute in lock/unlodk.  Don't call tbcmh's funciton in set value callback!
     clientattribute_t *clientattribute = NULL;
     const char* key = NULL;
     LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
          if (clientattribute) {
               key = clientattribute->key;
               if (cJSON_HasObjectItem(object, key)) {
                    cJSON *value = cJSON_GetObjectItem(object, key);
                    if (clientattribute->on_set && value) {
                         clientattribute->on_set(clientattribute->client, clientattribute->context, value);
                    }
            }
        }
     }

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     // return;
}

