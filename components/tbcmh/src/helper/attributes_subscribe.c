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
#include "tbc_mqtt_helper_internal.h"

#include "attributes_subscribe.h"

static uint32_t _subscribe_id = 0;

static const char *TAG = "ATTRIBUTES_SUBSCRIBE";

static tbc_err_t _subscribekey_list_append(
                                            subscribekey_list_t *subscribekey_list,
                                            const char *key)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(subscribekey_list, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(key, ESP_FAIL);

    subscribekey_t *subscribekey = TBC_MALLOC(sizeof(subscribekey_t));
    if (!subscribekey) {
        TBC_LOGE("Unable to malloc memeory!");
        return ESP_FAIL;
    }

    memset(subscribekey, 0x00, sizeof(subscribekey_t));
    subscribekey->key = TBC_MALLOC(strlen(key)+1);
    if (subscribekey->key) {
        strcpy(subscribekey->key, key);
    }

    // Insert subscribekey to list
    subscribekey_t *it, *last = NULL;
    if (LIST_FIRST(subscribekey_list) == NULL) {
         // Insert head
         LIST_INSERT_HEAD(subscribekey_list, subscribekey, entry);
    } else {
         // Insert last
         LIST_FOREACH(it, subscribekey_list, entry) {
              last = it;
         }
         if (it == NULL) {
              assert(last);
              LIST_INSERT_AFTER(last, subscribekey, entry);
         }
    }

    return ESP_OK;
}

static tbc_err_t _subscribekey_list_empty(subscribekey_list_t *subscribekey_list)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(subscribekey_list, ESP_FAIL);

    // items empty - remove all item in subscribekey_list
    subscribekey_t *subscribekey = NULL, *next;
    LIST_FOREACH_SAFE(subscribekey, subscribekey_list, entry, next) {
         // remove from subscribekey list and free
         LIST_REMOVE(subscribekey, entry);
         TBC_FREE(subscribekey->key);
         TBC_FREE(subscribekey);
    }

    return ESP_OK;
}

static attributessubscribe_t *_attributessubscribe_create(
                                        void *context,
                                        tbcmh_attributes_on_update_t on_update)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_update, NULL);
    
    attributessubscribe_t *attributessubscribe = TBC_MALLOC(sizeof(attributessubscribe_t));
    if (!attributessubscribe) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(attributessubscribe, 0x00, sizeof(attributessubscribe_t));
    attributessubscribe->subscribe_id = ++_subscribe_id;
    memset(&attributessubscribe->key_list, 0x00, sizeof(attributessubscribe->key_list));
    attributessubscribe->context = context;
    attributessubscribe->on_update = on_update;
    return attributessubscribe;
}

/*!< Destroys attributessubscribe */
static tbc_err_t _attributessubscribe_destroy(attributessubscribe_t *attributessubscribe)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(attributessubscribe, ESP_FAIL);

    _subscribekey_list_empty(&attributessubscribe->key_list);
    TBC_FREE(attributessubscribe);
    return ESP_OK;
}

void _tbcmh_attributessubscribe_on_create(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)

    // Take semaphore
    // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }
    
    // list create
    memset(&client->attributessubscribe_list, 0x00, sizeof(client->attributessubscribe_list)); //client->attributessubscribe_list = LIST_HEAD_INITIALIZER(client->attributessubscribe_list);

    // Give semaphore
    // xSemaphoreGiveRecursive(client->_lock);
}

void _tbcmh_attributessubscribe_on_destroy(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // items empty - remove all item in attributessubscribe_list
    attributessubscribe_t *attributessubscribe = NULL, *next;
    LIST_FOREACH_SAFE(attributessubscribe, &client->attributessubscribe_list, entry, next) {
         // remove from attributessubscribe list and destory
         LIST_REMOVE(attributessubscribe, entry);
         _attributessubscribe_destroy(attributessubscribe);
    }
    memset(&client->attributessubscribe_list, 0x00, sizeof(client->attributessubscribe_list));

    // Give semaphore
    // xSemaphoreGiveRecursive(client->_lock);
}

int tbcmh_attributes_subscribe(tbcmh_handle_t client,
                                        void *context,
                                        tbcmh_attributes_on_update_t on_update,
                                        int count, /*const char *key,*/...)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_update, ESP_FAIL);

    // Take semaphore
    if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
         TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
         return ESP_FAIL;
    }

    // Create attributessubscribe
    attributessubscribe_t *attributessubscribe = _attributessubscribe_create(context, on_update);
    if (!attributessubscribe) {
         // Give semaphore
         xSemaphoreGiveRecursive(client->_lock);
         TBC_LOGE("Init attributessubscribe failure! %s()", __FUNCTION__);
         return ESP_FAIL;
    }

    // Append key
    if (count>0) {
        va_list ap;
        va_start(ap, count);
        int i = 0;
        for (i=0; i<count; i++) {
            // insert key to attributessubscribe
            const char *key = va_arg(ap, const char*);
            _subscribekey_list_append(&attributessubscribe->key_list, key);
        }
        va_end(ap);
    }

    bool isEmptyBefore = LIST_EMPTY(&client->attributessubscribe_list);

    // Insert attributessubscribe to list
    attributessubscribe_t *it, *last = NULL;
    if (LIST_FIRST(&client->attributessubscribe_list) == NULL) {
         // Insert head
         LIST_INSERT_HEAD(&client->attributessubscribe_list, attributessubscribe, entry);
    } else {
         // Insert last
         LIST_FOREACH(it, &client->attributessubscribe_list, entry) {
              last = it;
         }
         if (it == NULL) {
              assert(last);
              LIST_INSERT_AFTER(last, attributessubscribe, entry);
         }
    }

    // Subscript topic <===  empty->non-empty
    if (tbcmh_is_connected(client) && isEmptyBefore && !LIST_EMPTY(&client->attributessubscribe_list))
    {
        int msg_id = tbcm_subscribe(client->tbmqttclient, TB_MQTT_TOPIC_SHARED_ATTRIBUTES, 0);
        TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s",
                msg_id, TB_MQTT_TOPIC_SHARED_ATTRIBUTES);
    }

    // Give semaphore
    xSemaphoreGiveRecursive(client->_lock);
    return attributessubscribe->subscribe_id;
}

int tbcmh_attributes_subscribe_of_array(tbcmh_handle_t client, //int qos /*=0*/,
                                        void *context,
                                        tbcmh_attributes_on_update_t on_update,
                                        int count, const char *keys[])
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_update, ESP_FAIL);

    // Take semaphore
    if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
         TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
         return ESP_FAIL;
    }

    // Create attributessubscribe
    attributessubscribe_t *attributessubscribe = _attributessubscribe_create(context, on_update);
    if (!attributessubscribe) {
         // Give semaphore
         xSemaphoreGiveRecursive(client->_lock);
         TBC_LOGE("Init attributessubscribe failure! %s()", __FUNCTION__);
         return ESP_FAIL;
    }
    // Append key
    int i = 0;
    for (i=0; keys && i<count; i++) {
        // insert key to attributessubscribe
        _subscribekey_list_append(&attributessubscribe->key_list, keys[i]);
    }

    bool isEmptyBefore = LIST_EMPTY(&client->attributessubscribe_list);

    // Insert attributessubscribe to list
    attributessubscribe_t *it, *last = NULL;
    if (LIST_FIRST(&client->attributessubscribe_list) == NULL) {
         // Insert head
         LIST_INSERT_HEAD(&client->attributessubscribe_list, attributessubscribe, entry);
    } else {
         // Insert last
         LIST_FOREACH(it, &client->attributessubscribe_list, entry) {
              last = it;
         }
         if (it == NULL) {
              assert(last);
              LIST_INSERT_AFTER(last, attributessubscribe, entry);
         }
    }

    // Subscript topic <===  empty->non-empty
    if (tbcmh_is_connected(client) && isEmptyBefore && !LIST_EMPTY(&client->attributessubscribe_list))
    {
        int msg_id = tbcm_subscribe(client->tbmqttclient, TB_MQTT_TOPIC_SHARED_ATTRIBUTES, 0);
        TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s",
                msg_id, TB_MQTT_TOPIC_SHARED_ATTRIBUTES);
    }

    // Give semaphore
    xSemaphoreGiveRecursive(client->_lock);
    return attributessubscribe->subscribe_id;
}

/**
 * @brief Unsubscribe the client to defined topic with defined qos
 *
 * Notes:
 * - Client must be connected to send subscribe message
 * - This API is could be executed from a user task or
 * from a mqtt event callback i.e. internal mqtt task
 * (API is protected by internal mutex, so it might block
 * if a longer data receive operation is in progress.
 *
 * @param client    mqtt client handle
 * @param topic
 * @param qos
 *
 * @return  0/ESP_OK   on success
 *         -1/ESP_FAIL on failure
 */
// remove attributessubscribe from tbcmh_attributessubscribe_list_t
tbc_err_t tbcmh_attributes_unsubscribe(tbcmh_handle_t client, int attributes_subscribe_id)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);

    // Take semaphore
    if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
         TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
         return ESP_FAIL;
    }

    bool isEmptyBefore = LIST_EMPTY(&client->attributessubscribe_list);
    
    // Search item
    attributessubscribe_t *attributessubscribe = NULL, *next;
    LIST_FOREACH_SAFE(attributessubscribe, &client->attributessubscribe_list, entry, next) {
         if (attributessubscribe && attributessubscribe->subscribe_id == attributes_subscribe_id) {
             // Remove form list
             LIST_REMOVE(attributessubscribe, entry);
             _attributessubscribe_destroy(attributessubscribe);
             break;
         }
    }
    
    // Unsubscript topic <===  non-empty->empty
    if (tbcmh_is_connected(client) && !isEmptyBefore && LIST_EMPTY(&client->attributessubscribe_list)) {
        int msg_id = tbcm_unsubscribe(client->tbmqttclient, TB_MQTT_TOPIC_SHARED_ATTRIBUTES);
        TBC_LOGI("sent unsubscribe successful, msg_id=%d, topic=%s",
                msg_id, TB_MQTT_TOPIC_SHARED_ATTRIBUTES);
    }

    // Give semaphore
    xSemaphoreGiveRecursive(client->_lock);
    return ESP_OK;  
}

void _tbcmh_attributessubscribe_on_connected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    if (tbcmh_is_connected(client) && !LIST_EMPTY(&client->attributessubscribe_list)) {
        int msg_id = tbcm_subscribe(client->tbmqttclient, TB_MQTT_TOPIC_SHARED_ATTRIBUTES, 0);
        TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s",
                msg_id, TB_MQTT_TOPIC_SHARED_ATTRIBUTES);
    }
}

void _tbcmh_attributessubscribe_on_disconnected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);
    //no code
}

//on received: unpack & deal
// return 2 if calling tbcmh_disconnect()/tbcmh_destroy() inside on_update()
// return 1 if calling tbcmh_sharedattribute_unregister()/tbcmh_attributes_unsubscribe inside on_update()
// return 0 otherwise
int _tbcmh_attributessubscribe_on_data(tbcmh_handle_t client, const cJSON *object)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, 0);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(object, 0);

     // Take semaphore
     // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
     //      return 0;
     // }

     // foreach itme to set value of attributessubscribe in lock/unlodk.  Don't call tbcmh's funciton in set value callback!
     tbc_err_t result = 0;
     attributessubscribe_t *attributessubscribe = NULL, *next;
     LIST_FOREACH_SAFE(attributessubscribe, &client->attributessubscribe_list, entry, next) {
          if (attributessubscribe) {
               if (LIST_EMPTY(&attributessubscribe->key_list)) {
                    attributessubscribe->on_update(client, attributessubscribe->context, object);
                    continue;
               }

               subscribekey_t *subscribekey = NULL, *next;
               LIST_FOREACH_SAFE(subscribekey, &attributessubscribe->key_list, entry, next) {
                   if (cJSON_HasObjectItem(object, subscribekey->key)) {
                        if (attributessubscribe->on_update) {
                            result = attributessubscribe->on_update(client, attributessubscribe->context, object); //cJSON *value = cJSON_GetObjectItem(object, key);
                            if (result==2) { //called tbcmh_disconnect()/tbcmh_destroy() inside on_set()
                                // Give semaphore
                                // xSemaphoreGiveRecursive(client->_lock);
                                return 2;
                            }
                            if (result==1) { //called tbcmh_attributes_unsubscribe() inside on_set()
                                return 1;
                            }
                        }
                        break;
                   }
               }
          }
     }

     // Give semaphore
     // xSemaphoreGiveRecursive(client->_lock);
     return result;
}

