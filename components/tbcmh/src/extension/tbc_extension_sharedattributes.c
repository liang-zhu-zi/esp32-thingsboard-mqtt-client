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

// This file is part of the ThingsBoard Client Extension (TBCE) API.

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"
#include "sys/queue.h"

#include "tbc_mqtt_helper.h"

#include "tbc_extension_sharedattributes.h"

/**
 * Shared attribute
 */
typedef struct sharedattribute
{
    int subscribe_id;                     /*!< Default is -1 before it's subscribed */

    char *key;                            /*!< Key */
    void *context;                        /*!< Context of getting/setting value*/
    tbce_sharedattribute_on_set_t on_set; /*!< Callback of setting value to context */

    LIST_ENTRY(sharedattribute) entry;
} sharedattribute_t;

typedef LIST_HEAD(tbce_sharedattribute_list, sharedattribute) sharedattribute_list_t;

/**
 * Shared attribute set
 */
typedef struct tbce_sharedattributes
{
     tbcmh_handle_t client; /*!< ThingsBoard MQTT Client Helper. Default is NULL before it's subscribed */

     sharedattribute_list_t sharedattribute_list; /*!< shared-attribute list */
} tbce_sharedattributes_t;

#define MAX_KEYS_LEN (256)

static int _tbce_sharedattributes_on_update(tbcmh_handle_t client, void *context, const cJSON *object);

const static char *TAG = "sharedattributes";

static sharedattribute_t *_sharedattribute_create(
                                                  const char *key, void *context,
                                                  tbce_sharedattribute_on_set_t on_set)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(key, NULL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(on_set, NULL);

     sharedattribute_t *sharedattribute = TBC_MALLOC(sizeof(sharedattribute_t));
     if (!sharedattribute)
     {
          TBC_LOGE("Unable to malloc memeory!");
          return NULL;
     }

     memset(sharedattribute, 0x00, sizeof(sharedattribute_t));
     sharedattribute->subscribe_id = -1;
     sharedattribute->key = TBC_MALLOC(strlen(key) + 1);
     if (sharedattribute->key)
     {
          strcpy(sharedattribute->key, key);
     }
     sharedattribute->context = context;
     sharedattribute->on_set = on_set;
     return sharedattribute;
}

/*!< Destroys sharedattribute */
static tbc_err_t _sharedattribute_destroy(sharedattribute_t *sharedattribute)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(sharedattribute, ESP_FAIL);

     TBC_FREE(sharedattribute->key);
     TBC_FREE(sharedattribute);
     return ESP_OK;
}

tbce_sharedattributes_handle_t tbce_sharedattributes_create(void)
{
    tbce_sharedattributes_t *sharedattributes = TBC_MALLOC(sizeof(tbce_sharedattributes_t));
    if (!sharedattributes) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(sharedattributes, 0x00, sizeof(tbce_sharedattributes_t));
    // list create
    // memset(&sharedattributes->sharedattribute_list, 0x00, sizeof(sharedattributes->sharedattribute_list)); //sharedattributes->sharedattribute_list = LIST_HEAD_INITIALIZER(sharedattribute_list->clientattribute_list);

    return sharedattributes;
}

void tbce_sharedattributes_destroy(tbce_sharedattributes_handle_t sharedattributes)
{
     // This function is in semaphore/client->_lock!!!
     TBC_CHECK_PTR(sharedattributes);

     // items empty - remove all item in sharedattribute_list
     sharedattribute_t *sharedattribute = NULL, *next;
     LIST_FOREACH_SAFE(sharedattribute, &sharedattributes->sharedattribute_list, entry, next)
     {
          // remove from sharedattribute list and destory
          LIST_REMOVE(sharedattribute, entry);
          _sharedattribute_destroy(sharedattribute);
     }
     // list destroy
     memset(&sharedattributes->sharedattribute_list, 0x00, sizeof(sharedattributes->sharedattribute_list));
}

tbc_err_t tbce_sharedattributes_register(tbce_sharedattributes_handle_t sharedattributes,
                                         const char *key, void *context,
                                         tbce_sharedattribute_on_set_t on_set)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(sharedattributes, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(key, ESP_FAIL);

     // Create sharedattribute
     sharedattribute_t *sharedattribute = _sharedattribute_create(key, context, on_set);
     if (!sharedattribute)
     {
          TBC_LOGE("Init sharedattribute failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert sharedattribute to list
     sharedattribute_t *it, *last = NULL;
     if (LIST_FIRST(&sharedattributes->sharedattribute_list) == NULL)
     {
          // Insert head
          LIST_INSERT_HEAD(&sharedattributes->sharedattribute_list, sharedattribute, entry);
     }
     else
     {
          // Insert last
          LIST_FOREACH(it, &sharedattributes->sharedattribute_list, entry)
          {
               last = it;
          }
          if (it == NULL)
          {
               assert(last);
               LIST_INSERT_AFTER(last, sharedattribute, entry);
          }
     }

     return ESP_OK;
}

// remove sharedattribute from tbcmh_shared_attribute_list_t
tbc_err_t tbce_sharedattributes_unregister(tbce_sharedattributes_handle_t sharedattributes,
                                           const char *key)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(sharedattributes, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(key, ESP_FAIL);

     // Search item
     sharedattribute_t *sharedattribute = NULL, *next;
     LIST_FOREACH_SAFE(sharedattribute, &sharedattributes->sharedattribute_list, entry, next)
     {
          if (sharedattribute && strcmp(sharedattribute->key, key) == 0)
          {
               // Remove form list
               LIST_REMOVE(sharedattribute, entry);
               _sharedattribute_destroy(sharedattribute);
               break;
          }
     }

     if (!sharedattribute)
     {
          TBC_LOGW("Unable to remove shared-attribute:%s! %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }
     return ESP_OK;
}

void tbce_sharedattributes_subscribe(tbce_sharedattributes_handle_t sharedattributes,
                                     tbcmh_handle_t client, uint32_t max_attributes_per_subscribe)
{
     TBC_CHECK_PTR(sharedattributes);
     TBC_CHECK_PTR(client);
     if (max_attributes_per_subscribe==0) {
        TBC_LOGE("max_attributes_per_subscribe is equql to 0!");
        return;
     }

     // unsubscribe if it's already subscribed.
     if (sharedattributes->client && sharedattributes->client != client) {
        tbce_sharedattributes_unsubscribe(sharedattributes);
     }

     // Search item
     int max_attributes = (max_attributes_per_subscribe<10) ? max_attributes_per_subscribe : 10;
     sharedattribute_t *attribute_array[10] = {0};
     const char *key_array[10] = {0};
     sharedattribute_t *sharedattribute = NULL, *next;
     int count = 0;
     LIST_FOREACH_SAFE(sharedattribute, &sharedattributes->sharedattribute_list, entry, next) {
          if (sharedattribute && (sharedattribute->subscribe_id<0)) {
               attribute_array[count] = sharedattribute;
               key_array[count++] = sharedattribute->key;
               if (count>=max_attributes) {
                    // subscribe
                    int subscribe_id = tbcmh_attributes_subscribe_of_array(client,
                                              sharedattributes /*context*/,
                                              _tbce_sharedattributes_on_update,
                                              count, &key_array[0]);
                    // update subscribe_id
                    if (subscribe_id>=0) {
                        for (int j=0; j<count; j++) {
                            attribute_array[j]->subscribe_id = subscribe_id;
                        }
                    }
                    count = 0;
               }
          }
     }

     if (count>0) {
         // subscribe
         int subscribe_id = tbcmh_attributes_subscribe_of_array(client,
                                   sharedattributes /*context*/,
                                   _tbce_sharedattributes_on_update,
                                   count, &key_array[0]);
         // update subscribe_id
         if (subscribe_id>=0) {
             for (int j=0; j<count; j++) {
                 attribute_array[j]->subscribe_id = subscribe_id;
             }
         }
         count = 0;
     }

     sharedattributes->client = client;
}

void tbce_sharedattributes_unsubscribe(tbce_sharedattributes_handle_t sharedattributes)
{
     TBC_CHECK_PTR(sharedattributes);
     TBC_CHECK_PTR(sharedattributes->client);

     // Search item
     sharedattribute_t *sharedattribute = NULL, *next;
     LIST_FOREACH_SAFE(sharedattribute, &sharedattributes->sharedattribute_list, entry, next)
     {
          if (sharedattribute && (sharedattribute->subscribe_id>=0))
          {
               tbcmh_attributes_unsubscribe(sharedattributes->client, sharedattribute->subscribe_id);
               sharedattribute->subscribe_id = -1;
          }
     }

     sharedattributes->client = NULL;
}

// on received: unpack & deal
//  return 2 if calling tbcmh_disconnect()/tbcmh_destroy() inside on_set()
//  return 1 if calling tbce_sharedattributes_unregister() inside on_set()
//  return 0 otherwise
static int _tbce_sharedattributes_on_update(tbcmh_handle_t client, void *context, const cJSON *object)
{
     tbce_sharedattributes_handle_t sharedattributes = (tbce_sharedattributes_handle_t)context;

     //TBC_CHECK_PTR_WITH_RETURN_VALUE(client, 0);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(context, 0);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(object, 0);

     // foreach itme to set value of sharedattribute in lock/unlodk.  Don't call tbcmh's funciton in set value callback!
     sharedattribute_t *sharedattribute = NULL, *next;
     tbc_err_t result = 0;
     LIST_FOREACH_SAFE(sharedattribute, &sharedattributes->sharedattribute_list, entry, next) {
          if (sharedattribute) {
               if ((sharedattribute->subscribe_id>=0) &&
                    sharedattribute->key &&
                    sharedattribute->on_set &&
                    cJSON_HasObjectItem(object, sharedattribute->key))
               {
                    cJSON *value = cJSON_GetObjectItem(object, sharedattribute->key);
                    if (value) {
                         result = sharedattribute->on_set( sharedattribute->context, value);
                         if (result == 2) { // called tbcmh_disconnect()/tbcmh_destroy() inside on_set()
                              break;
                         }
                         if (result == 1) { // called tbce_sharedattributes_unregister() inside on_set()
                              break;
                         }
                    }
               }
          }
     }

     return result;
}

static void _tbce_sharedattributes_on_initialized(tbcmh_handle_t client,
                                         void *context,
                                         const cJSON *client_attributes,
                                         const cJSON *shared_attributes)
{
    _tbce_sharedattributes_on_update(client, context, shared_attributes);
}

tbc_err_t tbce_sharedattributes_initialized(tbce_sharedattributes_handle_t sharedattributes,
                                                  tbcmh_handle_t client,
                                                  uint32_t max_attributes_per_request)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(sharedattributes, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);

    char *shared_keys = TBC_MALLOC(MAX_KEYS_LEN);
    if (!shared_keys) {
        return ESP_FAIL;
    }
    memset(shared_keys, 0x00, MAX_KEYS_LEN);

    // Get shared_keys from sharedattribute
    int i = 0;
    sharedattribute_t *sharedattribute = NULL, *next;
    LIST_FOREACH_SAFE(sharedattribute, &sharedattributes->sharedattribute_list, entry, next) {
        if (sharedattribute && sharedattribute->key) {
             // copy key to shared_keys
             if (strlen(shared_keys)==0) {
                  strncpy(shared_keys, sharedattribute->key, MAX_KEYS_LEN-1);
             } else {
                  strncat(shared_keys, ",", MAX_KEYS_LEN-1);                         
                  strncat(shared_keys, sharedattribute->key, MAX_KEYS_LEN-1);
             }

             i++;
             if (i>=max_attributes_per_request) {
                 tbcmh_attributes_request(client,
                              sharedattributes/*context*/,
                              _tbce_sharedattributes_on_initialized/*on_response*/,
                              NULL/*on_timeout*/,
                              NULL/*client_keys*/, shared_keys);
                 memset(shared_keys, 0x00, MAX_KEYS_LEN);
                 i=0;
             }
        }
    }

    // last attributes request
    if (i>0) {
        tbcmh_attributes_request(client,
                     sharedattributes/*context*/,
                     _tbce_sharedattributes_on_initialized/*on_response*/,
                     NULL/*on_timeout*/,
                     NULL/*client_keys*/, shared_keys);
        memset(shared_keys, 0x00, MAX_KEYS_LEN);
        i=0;
    }
 
    TBC_FREE(shared_keys);
    return ESP_OK;
}

#if 0
//return 0/ESP_OK on successful, otherwise return -1/ESP_FAIL
tbc_err_t tbcmh_attributes_request(tbcmh_handle_t client,
                                 void *context,
                                 tbcmh_attributes_on_response_t on_response,
                                 tbcmh_attributes_on_timeout_t on_timeout,
                                 int count, /*const char *key,*/...)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
     if (count <= 0) {
          TBC_LOGE("count(%d) is error! %s()", count, __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore, malloc client_keys & shared_keys
     if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
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
    
          // TODO: call ....
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

          // TODO: call ....
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
     uint32_t request_id = _tbcmh_get_request_id(client);
     // if (request_id <= 0) {
     //      TBC_LOGE("failure to getting request id!");
     //      return -1;
     // }
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
     xSemaphoreGiveRecursive(client->_lock);

     TBC_FREE(client_keys);
     TBC_FREE(shared_keys);
     return ESP_OK;

attributesrequest_fail:
     xSemaphoreGiveRecursive(client->_lock);
     if (!client_keys) {
          TBC_FREE(client_keys);
     }
     if (!shared_keys) {
          TBC_FREE(shared_keys);
     }
     return ESP_FAIL;
}
#endif

