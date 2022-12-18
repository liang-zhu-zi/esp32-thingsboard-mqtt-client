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

#include "tbc_extension_clientattributes.h"

/**
 * Client-side attribute
 */
typedef struct clientattribute
{
     char *key; /*!< Key */
     void *context;                         /*!< Context of getting/setting value*/
     tbce_clientattribute_on_get_t on_get; /*!< Callback of getting value from context */
     tbce_clientattribute_on_set_t on_set; /*!< Callback of setting value to context */

     LIST_ENTRY(clientattribute) entry;
} clientattribute_t;

typedef LIST_HEAD(tbce_clientattribute_list, clientattribute) clientattribute_list_t;

/**
 * Client-side attribute set
 */
typedef struct tbce_clientattributes
{
     clientattribute_list_t clientattribute_list; /*!< client-attribute list */
} tbce_clientattributes_t;

#define MAX_KEYS_LEN (256)

const static char *TAG = "clientattribute";

static clientattribute_t *_clientattribute_create(
                                            const char *key, void *context,
                                            tbce_clientattribute_on_get_t on_get,
                                            tbce_clientattribute_on_set_t on_set)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(key, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_get, NULL);
    
    clientattribute_t *clientattribute = TBC_MALLOC(sizeof(clientattribute_t));
    if (!clientattribute) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(clientattribute, 0x00, sizeof(clientattribute_t));
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

static tbc_err_t _clientattribute_register(tbce_clientattributes_handle_t clientattributes,
                                                  const char *key, void *context,
                                                  tbce_clientattribute_on_get_t on_get,
                                                  tbce_clientattribute_on_set_t on_set)
{
     // Create clientattribute
     clientattribute_t *clientattribute = _clientattribute_create(key, context, on_get, on_set);
     if (!clientattribute) {
          TBC_LOGE("Init clientattribute failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert clientattribute to list
     clientattribute_t *it, *last = NULL;
     if (LIST_FIRST(&clientattributes->clientattribute_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&clientattributes->clientattribute_list, clientattribute, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &clientattributes->clientattribute_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, clientattribute, entry);
          }
     }

     return ESP_OK;
}

tbce_clientattributes_handle_t tbce_clientattributes_create(void)
{
    tbce_clientattributes_t *clientattributes = TBC_MALLOC(sizeof(tbce_clientattributes_t));
    if (!clientattributes) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(clientattributes, 0x00, sizeof(tbce_clientattributes_t));
    // list create
    // memset(&clientattributes->clientattribute_list, 0x00, sizeof(clientattributes->clientattribute_list)); //clientattributes->clientattribute_list = LIST_HEAD_INITIALIZER(clientattributes->clientattribute_list);

    return clientattributes;
}

void tbce_clientattributes_destroy(tbce_clientattributes_handle_t clientattributes)
{
  TBC_CHECK_PTR(clientattributes);

  // items empty - remove all item in clientattribute_list
  clientattribute_t *clientattribute = NULL, *next;
  LIST_FOREACH_SAFE(clientattribute, &clientattributes->clientattribute_list, entry, next) {
       // remove from clientattribute list and destory
       LIST_REMOVE(clientattribute, entry);
       _clientattribute_destroy(clientattribute);
  }
  // list destroy
  memset(&clientattributes->clientattribute_list, 0x00, sizeof(clientattributes->clientattribute_list));
}

tbc_err_t tbce_clientattributes_register_with_set(tbce_clientattributes_handle_t clientattributes,
                                        const char *key, void *context,
                                        tbce_clientattribute_on_get_t on_get,
                                        tbce_clientattribute_on_set_t on_set)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(on_set, ESP_FAIL);
     return _clientattribute_register(clientattributes, key, context, on_get, on_set);
}

tbc_err_t tbce_clientattributes_register(tbce_clientattributes_handle_t clientattributes,
                                         const char *key, void *context,
                                         tbce_clientattribute_on_get_t on_get)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(clientattributes, ESP_FAIL);
     return _clientattribute_register(clientattributes, key, context, on_get, NULL);
}

tbc_err_t tbce_clientattributes_unregister(tbce_clientattributes_handle_t clientattributes,
                                        const char *key)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(clientattributes, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(key, ESP_FAIL);

     // Search item
     clientattribute_t *clientattribute = NULL, *next;;
     LIST_FOREACH_SAFE(clientattribute, &clientattributes->clientattribute_list, entry, next) {
          if (clientattribute && strcmp(clientattribute->key, key)==0) {
             // Remove from list and destroy
             LIST_REMOVE(clientattribute, entry);
             _clientattribute_destroy(clientattribute);
             break;
          }
     }

     if (!clientattribute) {
          TBC_LOGW("Unable to remove client attribute: %s! %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }
     return ESP_OK;
}

bool tbce_clientattributes_is_contained(tbce_clientattributes_handle_t clientattributes,
                                        const char *key)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(clientattributes, false);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(key, false);

     // Search item
     clientattribute_t *clientattribute = NULL, *next;;
     LIST_FOREACH_SAFE(clientattribute, &clientattributes->clientattribute_list, entry, next) {
          if (clientattribute && strcmp(clientattribute->key, key)==0) {
             return true;
          }
     }

     return false;
}

//Initialize client-side attributes from the server
//on received init value in attributes response: unpack & deal
//Note: Only client attributes that have an on_set() callback will be initialized.
static void _tbce_clientattributes_on_initialized(tbcmh_handle_t client,
                                         void *context,
                                         const cJSON *client_attributes,
                                         const cJSON *shared_attributes)
{
     tbce_clientattributes_handle_t clientattributes = (tbce_clientattributes_handle_t)context;
     const cJSON *object = client_attributes;
     TBC_CHECK_PTR(clientattributes);
     TBC_CHECK_PTR(object);

     // foreach item to set value of clientattribute in lock/unlodk. 
     // Don't call tbcmh's funciton in set value callback!
     clientattribute_t *clientattribute = NULL, *next;
     tbc_err_t result = 0;
     LIST_FOREACH_SAFE(clientattribute, &clientattributes->clientattribute_list, entry, next) {
          if (clientattribute && clientattribute->key && clientattribute->on_set) {
               cJSON *value = cJSON_GetObjectItem(object, clientattribute->key);
               if (value) {
                   result = clientattribute->on_set(clientattribute->context, value);
                   if (result == 2) { // called tbcmh_disconnect()/tbcmh_destroy() inside on_set()
                        break;
                   }
                   if (result == 1) { // called tbce_clientattributes_unregister() inside on_set()
                        break;
                   }
               }
          }
    }
}

//Initialize client-side attributes from the server
//on received init value in attributes response: unpack & deal
tbc_err_t tbce_clientattributes_initialize(
                        tbce_clientattributes_handle_t clientattributes,
                        tbcmh_handle_t client,
                        uint32_t max_attributes_per_request)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(clientattributes, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);

    char *client_keys = TBC_MALLOC(MAX_KEYS_LEN);
    if (!client_keys) {
        TBC_LOGE("Unable to malloc memeory!");
        return ESP_FAIL;
    }
    memset(client_keys, 0x00, MAX_KEYS_LEN);

    // Get client_keys from clientattributes
    int i = 0;
    clientattribute_t *clientattribute = NULL, *next;
    LIST_FOREACH_SAFE(clientattribute, &clientattributes->clientattribute_list, entry, next) {
        if (clientattribute && clientattribute->key && clientattribute->on_set) {
             // copy key to client_keys
             if (strlen(client_keys)==0) {
                  strncpy(client_keys, clientattribute->key, MAX_KEYS_LEN-1);
             } else {
                  strncat(client_keys, ",", MAX_KEYS_LEN-1);                         
                  strncat(client_keys, clientattribute->key, MAX_KEYS_LEN-1);
             }

             i++;
             if (i>=max_attributes_per_request) {
                 tbcmh_attributes_request(client,
                      clientattributes/*context*/,
                      _tbce_clientattributes_on_initialized/*on_response*/,
                      NULL/*on_timeout*/,
                      client_keys, NULL);
                 i = 0;
                 memset(client_keys, 0x00, MAX_KEYS_LEN);
             }
        }
    }

    // last attributes request
    if (i>0) {
        tbcmh_attributes_request(client,
             clientattributes/*context*/,
             _tbce_clientattributes_on_initialized/*on_response*/,
             NULL/*on_timeout*/,
             client_keys, NULL);
        i = 0;
        memset(client_keys, 0x00, MAX_KEYS_LEN);
    }

    TBC_FREE(client_keys);
    return ESP_OK;
}

tbc_err_t tbce_clientattributes_update(tbce_clientattributes_handle_t clientattributes,
                                    tbcmh_handle_t client,
                                    int count, /*const char *key,*/ ...)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(clientattributes, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
     if (count <= 0) {
          TBC_LOGE("count(%d) is error! %s()", count, __FUNCTION__);
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
          LIST_FOREACH(clientattribute, &clientattributes->clientattribute_list, entry) {
               if (clientattribute && strcmp(clientattribute->key, key)==0) {
                    break;
               }
          }

          /// Add clientattribute to package
          if (clientattribute && clientattribute->on_get) {
                // add item to json object
                cJSON *value = clientattribute->on_get(clientattribute->context);
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
         msg_id = tbcmh_attributes_update_ex(client, object, 1/*qos*/, 0/*retain*/);
     }
     cJSON_Delete(object); // delete json object

     return (msg_id > -1) ? ESP_OK : ESP_FAIL;
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
