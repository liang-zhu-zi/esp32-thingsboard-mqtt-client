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

#define MAX_KEYS_LEN (256)

const static char *TAG = "attributesrequest";

/*!< Initialize attributesrequest */
static attributesrequest_t *_attributesrequest_create(tbcmh_handle_t client, int request_id, void *context,
                                                         tbcmh_attributesrequest_on_response_t on_response,
                                                         tbcmh_attributesrequest_on_timeout_t on_timeout)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_response, NULL);
    
    attributesrequest_t *attributesrequest = TBC_MALLOC(sizeof(attributesrequest_t));
    if (!attributesrequest) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(attributesrequest, 0x00, sizeof(attributesrequest_t));
    attributesrequest->client = client;
    attributesrequest->request_id = request_id;
    attributesrequest->timestamp = (uint64_t)time(NULL);
    attributesrequest->context = context;
    attributesrequest->on_response = on_response;
    attributesrequest->on_timeout = on_timeout;
    return attributesrequest;
}

static attributesrequest_t *_attributesrequest_clone_wo_listentry(attributesrequest_t *src)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(src, NULL);
    
    attributesrequest_t *attributesrequest = TBC_MALLOC(sizeof(attributesrequest_t));
    if (!attributesrequest) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(attributesrequest, 0x00, sizeof(attributesrequest_t));
    attributesrequest->client = src->client;
    attributesrequest->request_id = src->request_id;
    attributesrequest->context = src->context;
    attributesrequest->on_response = src->on_response;
    attributesrequest->on_timeout = src->on_timeout;
    return attributesrequest;
}

/*!< Destroys the attributesrequest */
static tbc_err_t _attributesrequest_destroy(attributesrequest_t *attributesrequest)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(attributesrequest, ESP_FAIL);

    TBC_FREE(attributesrequest);
    return ESP_OK;
}

////return request_id on successful, otherwise return -1/ESP_FAIL
int tbcmh_attributesrequest_send(tbcmh_handle_t client,
                                 void *context,
                                 tbcmh_attributesrequest_on_response_t on_response,
                                 tbcmh_attributesrequest_on_timeout_t on_timeout,
                                 int count, /*const char *key,*/...)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
     if (count <= 0) {
          TBC_LOGE("count(%d) is error! %s()", count, __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore, malloc client_keys & shared_keys
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     char *client_keys = TBC_MALLOC(MAX_KEYS_LEN);
     char *shared_keys = TBC_MALLOC(MAX_KEYS_LEN);
     if (!client_keys || !shared_keys) {
          goto attributesrequest_fail;
     }
     memset(client_keys, 0x00, MAX_KEYS_LEN);
     memset(shared_keys, 0x00, MAX_KEYS_LEN);

     // Get client_keys & shared_keys
     int i = 0;
     va_list ap;
     va_start(ap, count);
next_attribute_key:
     while (i<count) {
          i++;
          const char *key = va_arg(ap, const char*);

          // Search item in clientattribute
          clientattribute_t *clientattribute = NULL;
          LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
               if (clientattribute && strcmp(clientattribute->key, key)==0) {
                    // copy key to client_keys
                    if (strlen(client_keys)==0) {
                         strncpy(client_keys, key, MAX_KEYS_LEN-1);
                    } else {
                         strncat(client_keys, ",", MAX_KEYS_LEN-1);                         
                         strncat(client_keys, key, MAX_KEYS_LEN-1);
                    }
                    goto next_attribute_key;
               }
          }

          // Search item in sharedattribute
          sharedattribute_t *sharedattribute = NULL;
          LIST_FOREACH(sharedattribute, &client->sharedattribute_list, entry) {
               if (sharedattribute && strcmp(sharedattribute->key, key)==0) {
                    // copy key to shared_keys
                    if (strlen(shared_keys)==0) {
                         strncpy(shared_keys, key, MAX_KEYS_LEN-1);
                    } else {
                         strncat(shared_keys, ",", MAX_KEYS_LEN-1);                         
                         strncat(shared_keys, key, MAX_KEYS_LEN-1);
                    }
                    goto next_attribute_key;
               }
          }

          TBC_LOGW("Unable to find attribute in request:%s! %s()", key, __FUNCTION__);
     }
     va_end(ap);

     // Send msg to server
     int request_id = _tbcmh_get_request_id(client);
     if (request_id <= 0) {
          TBC_LOGE("failure to getting request id!");
          return -1;
     }
     int msg_id = tbcm_attributes_request_ex(client->tbmqttclient, client_keys, shared_keys,
                               request_id,
                               1/*qos*/, 0/*retain*/);
     if (msg_id<0) {
          TBC_LOGE("Init tbcm_attributes_request failure! %s()", __FUNCTION__);
          goto attributesrequest_fail;
     }

     // Create attributesrequest
     attributesrequest_t *attributesrequest = _attributesrequest_create(client, request_id,
                                context, on_response, on_timeout);
     if (!attributesrequest) {
          TBC_LOGE("Init attributesrequest failure! %s()", __FUNCTION__);
          goto attributesrequest_fail;
     }

     // Insert attributesrequest to list
     attributesrequest_t *it, *last = NULL;
     if (LIST_FIRST(&client->attributesrequest_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->attributesrequest_list, attributesrequest, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->attributesrequest_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, attributesrequest, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     
     TBC_FREE(client_keys);
     TBC_FREE(shared_keys);
     return request_id;

attributesrequest_fail:
     xSemaphoreGive(client->_lock);
     if (!client_keys) {
          TBC_FREE(client_keys);
     }
     if (!shared_keys) {
          TBC_FREE(shared_keys);
     }
     return ESP_FAIL;
}

// TODO: merge to tbcmh_attributesrequest_send()
////return request_id on successful, otherwise return -1/ESP_FAIL
int tbcmh_attributesrequest_send_4_ota_sharedattributes(tbcmh_handle_t client,
                                  void *context,
                                  tbcmh_attributesrequest_on_response_t on_response,
                                  tbcmh_attributesrequest_on_timeout_t on_timeout,
                                  int count, /*const char *key,*/...)
{
      // this funciton is in client->_lock !

      TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
      if (count <= 0) {
           TBC_LOGE("count(%d) is error! %s()", count, __FUNCTION__);
           return ESP_FAIL;
      }
 
      // Take semaphore, malloc client_keys & shared_keys
      //if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
      //     TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
      //     return ESP_FAIL;
      //}
      char *shared_keys = TBC_MALLOC(MAX_KEYS_LEN);
      if (!shared_keys) {
           goto attributesrequest_fail;
      }
      memset(shared_keys, 0x00, MAX_KEYS_LEN);
 
      // Get shared_keys
      int i = 0;
      va_list ap;
      va_start(ap, count);
      while (i<count) {
        i++;
        const char *key = va_arg(ap, const char*);
        if (strlen(key)>0) {
            // copy key to shared_keys
            if (strlen(shared_keys)==0) {
                strncpy(shared_keys, key, MAX_KEYS_LEN-1);
            } else {
                strncat(shared_keys, ",", MAX_KEYS_LEN-1);                         
                strncat(shared_keys, key, MAX_KEYS_LEN-1);
            }
        }
      }
      va_end(ap);
 
      // Send msg to server
      // Send msg to server
      int request_id = _tbcmh_get_request_id(client);
      if (request_id <= 0) {
           TBC_LOGE("failure to getting request id!");
           return -1;
      }
      int msg_id = tbcm_attributes_request_ex(client->tbmqttclient, NULL, shared_keys,
                                request_id,
                                1/*qos*/, 0/*retain*/);
      if (msg_id<0) {
           TBC_LOGE("Init tbcm_attributes_request failure! %s()", __FUNCTION__);
           goto attributesrequest_fail;
      }
 
      // Create attributesrequest
      attributesrequest_t *attributesrequest = _attributesrequest_create(client, request_id,
                                    context, on_response, on_timeout);
      if (!attributesrequest) {
           TBC_LOGE("Init attributesrequest failure! %s()", __FUNCTION__);
           goto attributesrequest_fail;
      }
 
      // Insert attributesrequest to list
      attributesrequest_t *it, *last = NULL;
      if (LIST_FIRST(&client->attributesrequest_list) == NULL) {
           // Insert head
           LIST_INSERT_HEAD(&client->attributesrequest_list, attributesrequest, entry);
      } else {
           // Insert last
           LIST_FOREACH(it, &client->attributesrequest_list, entry) {
                last = it;
           }
           if (it == NULL) {
                assert(last);
                LIST_INSERT_AFTER(last, attributesrequest, entry);
           }
      }
 
      // Give semaphore
      //xSemaphoreGive(client->_lock);
      TBC_FREE(shared_keys);
      return request_id;
 
 attributesrequest_fail:
      //xSemaphoreGive(client->_lock);
      if (!shared_keys) {
           TBC_FREE(shared_keys);
      }
      return ESP_FAIL;
}

void _tbcmh_attributesrequest_on_create(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // list create
    memset(&client->attributesrequest_list, 0x00, sizeof(client->attributesrequest_list)); //client->attributesrequest_list = LIST_HEAD_INITIALIZER(client->attributesrequest_list);

    // Give semaphore
    // xSemaphoreGive(client->_lock);
}

void _tbcmh_attributesrequest_on_destroy(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // TODO: How to add lock??
    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    memset(&client->attributesrequest_list, 0x00, sizeof(client->attributesrequest_list));

    // Give semaphore
    // xSemaphoreGive(client->_lock);
}

void _tbcmh_attributesrequest_on_connected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)
    int msg_id = tbcm_subscribe(client->tbmqttclient, TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_SUBSCRIRBE, 0);
    TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s",
                msg_id, TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_SUBSCRIRBE);
}

void _tbcmh_attributesrequest_on_disconnected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)

    // TODO: How to add lock??
    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // remove all item in attributesrequest_list
    attributesrequest_t *attributesrequest = NULL, *next;
    LIST_FOREACH_SAFE(attributesrequest, &client->attributesrequest_list, entry, next) {
        // exec timeout callback
        if (attributesrequest->on_timeout) {
            attributesrequest->on_timeout(attributesrequest->client, attributesrequest->context, attributesrequest->request_id); //(none/resend/destroy/_destroy_all_attributes)?
        }

        // remove from attributesrequest list and destory
        LIST_REMOVE(attributesrequest, entry);
        _attributesrequest_destroy(attributesrequest);
    }
    memset(&client->attributesrequest_list, 0x00, sizeof(client->attributesrequest_list));

    // Give semaphore
    // xSemaphoreGive(client->_lock);
}


//on response.
void _tbcmh_attributesrequest_on_data(tbcmh_handle_t client, int request_id, const cJSON *object)
{
     TBC_CHECK_PTR(client);
     TBC_CHECK_PTR(object);

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;
     }

     // Search attributesrequest
     attributesrequest_t *attributesrequest = NULL; 
     LIST_FOREACH(attributesrequest, &client->attributesrequest_list, entry) {
          if (attributesrequest && (attributesrequest->request_id==request_id)) {
               break;
          }
     }
     if (!attributesrequest) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGW("Unable to find attribute request:%d! %s()", request_id, __FUNCTION__);
          return;
     }

     // Cache and remove attributesrequest
     attributesrequest_t *cache = _attributesrequest_clone_wo_listentry(attributesrequest);
     LIST_REMOVE(attributesrequest, entry);
     _attributesrequest_destroy(attributesrequest);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // foreach item to set value of clientattribute in lock/unlodk.  Don't call tbcmh's funciton in set value callback!
     if (cJSON_HasObjectItem(object, TB_MQTT_KEY_ATTRIBUTES_RESPONSE_CLIENT)) {
          _tbcmh_clientattribute_on_data(client, cJSON_GetObjectItem(object, TB_MQTT_KEY_ATTRIBUTES_RESPONSE_CLIENT));
     }
     // foreach item to set value of sharedattribute in lock/unlodk.  Don't call tbcmh's funciton in set value callback!
     if (cJSON_HasObjectItem(object, TB_MQTT_KEY_ATTRIBUTES_RESPONSE_SHARED)) {
          _tbcmh_sharedattribute_on_data(client, cJSON_GetObjectItem(object, TB_MQTT_KEY_ATTRIBUTES_RESPONSE_SHARED));
     }

     // Do response
     if (cache->on_response) {
        cache->on_response(cache->client, cache->context, cache->request_id); //(none/resend/destroy/_destroy_all_attributes)?
     }

     // Free cache
     _attributesrequest_destroy(cache);

     return;
}

void _tbcmh_attributesrequest_on_check_timeout(tbcmh_handle_t client, uint64_t timestamp)
{
     TBC_CHECK_PTR(client);

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;
     }

     // Search & move timeout item to timeout_list
     attributesrequest_list_t timeout_list = LIST_HEAD_INITIALIZER(timeout_list);
     attributesrequest_t *request = NULL, *next;
     LIST_FOREACH_SAFE(request, &client->attributesrequest_list, entry, next) {
          if (request && request->timestamp + TB_MQTT_TIMEOUT <= timestamp) {
               LIST_REMOVE(request, entry);
               // append to timeout list
               attributesrequest_t *it, *last = NULL;
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

     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Deal timeout
     LIST_FOREACH_SAFE(request, &timeout_list, entry, next) {
          if (request->on_timeout) {
              request->on_timeout(request->client, request->context, request->request_id);
          }
          LIST_REMOVE(request, entry);
          _attributesrequest_destroy(request);
     }
}

