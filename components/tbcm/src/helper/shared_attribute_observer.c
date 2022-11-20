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

//#include "shared_attribute_observer.h"
#include "tbc_mqtt_helper_internal.h"

const static char *TAG = "shared_attribute";

static tbcmh_sharedattribute_t *_tbcmh_sharedattribute_init(tbcmh_handle_t client, const char *key, void *context,
                                                     tbcmh_sharedattribute_on_set_t on_set)
{
    if (!key) {
        TBC_LOGE("key is NULL");
        return NULL;
    }
    
    tbcmh_sharedattribute_t *sharedattribute = TBC_MALLOC(sizeof(tbcmh_sharedattribute_t));
    if (!sharedattribute) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(sharedattribute, 0x00, sizeof(tbcmh_sharedattribute_t));
    sharedattribute->client = client;
    sharedattribute->key = TBC_MALLOC(strlen(key)+1);
    if (sharedattribute->key) {
        strcpy(sharedattribute->key, key);
    }
    sharedattribute->context = context;
    sharedattribute->on_set = on_set;
    return sharedattribute;
}

/*!< Destroys the tbcm key-value handle */
static tbc_err_t _tbcmh_sharedattribute_destroy(tbcmh_sharedattribute_t *sharedattribute)
{
    if (!sharedattribute) {
        TBC_LOGE("sharedattribute is NULL");
        return ESP_FAIL;
    }

    TBC_FREE(sharedattribute->key);
    TBC_FREE(sharedattribute);
    return ESP_OK;
}

/*!< Get key of the tbcm tbcmh_attribute handle */
const char *_tbcmh_sharedattribute_get_key(tbcmh_sharedattribute_t *sharedattribute)
{
    if (!sharedattribute) {
        TBC_LOGE("sharedattribute is NULL");
        return NULL;
    }
    return sharedattribute->key;
}

/*!< add item value to json object */
static tbc_err_t _tbcmh_sharedattribute_do_set(tbcmh_sharedattribute_t *sharedattribute, cJSON *value)                                               
{
    if (!sharedattribute) {
        TBC_LOGE("sharedattribute is NULL");
        return ESP_FAIL;
    }
    if (!value) {
        TBC_LOGE("value is NULL");
        return ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, sharedattribute->key);;
    if (!value) {
        TBC_LOGW("value is NULL! key=%s", sharedattribute->key);
        return ESP_FAIL;
    }*/

    sharedattribute->on_set(sharedattribute->client, sharedattribute->context, value);
    return ESP_OK;
}


//====21.Subscribe to shared device attribute updates from the server===================================================
//Call it before connect() //tbcmh_shared_attribute_list_t
tbc_err_t tbcmh_sharedattribute_append(tbcmh_handle_t client_, const char *key, void *context,
                                         tbcmh_sharedattribute_on_set_t on_set)
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

     // Create sharedattribute
     tbcmh_sharedattribute_t *sharedattribute = _tbcmh_sharedattribute_init(client_, key, context, on_set);
     if (!sharedattribute) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init sharedattribute failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert sharedattribute to list
     tbcmh_sharedattribute_t *it, *last = NULL;
     if (LIST_FIRST(&client->sharedattribute_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->sharedattribute_list, sharedattribute, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->sharedattribute_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, sharedattribute, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

// remove shared_attribute from tbcmh_shared_attribute_list_t
tbc_err_t tbcmh_sharedattribute_clear(tbcmh_handle_t client_, const char *key)
{
     tbcmh_t *client = (tbcmh_t *)client_;
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
     tbcmh_sharedattribute_t *sharedattribute = NULL;
     LIST_FOREACH(sharedattribute, &client->sharedattribute_list, entry) {
          if (sharedattribute && strcmp(_tbcmh_sharedattribute_get_key(sharedattribute), key)==0) {
               break;
          }
     }
     if (!sharedattribute) {
          TBC_LOGW("Unable to remove shared-attribute:%s! %s()", key, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(sharedattribute, entry);
     _tbcmh_sharedattribute_destroy(sharedattribute);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}
/*static*/ tbc_err_t _tbcmh_sharedattribute_empty(tbcmh_handle_t client_)
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

     // remove all item in sharedattribute_list
     tbcmh_sharedattribute_t *sharedattribute = NULL, *next;
     LIST_FOREACH_SAFE(sharedattribute, &client->sharedattribute_list, entry, next) {
          // remove from sharedattribute list and destory
          LIST_REMOVE(sharedattribute, entry);
          _tbcmh_sharedattribute_destroy(sharedattribute);
     }
     memset(&client->sharedattribute_list, 0x00, sizeof(client->sharedattribute_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

//unpack & deal
/*static*/ void _tbcmh_sharedattribute_on_received(tbcmh_handle_t client_, const cJSON *object)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !object) {
          TBC_LOGE("client or object is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Remove it from request list
     

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // foreach itme to set value of sharedattribute in lock/unlodk.  Don't call tbcmh's funciton in set value callback!
     tbcmh_sharedattribute_t *sharedattribute = NULL;
     const char* key = NULL;
     LIST_FOREACH(sharedattribute, &client->sharedattribute_list, entry) {
          if (sharedattribute) {
               key = _tbcmh_sharedattribute_get_key(sharedattribute);
               if (cJSON_HasObjectItem(object, key)) {
                    _tbcmh_sharedattribute_do_set(sharedattribute, cJSON_GetObjectItem(object, key));
               }
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);

     // special process for otaupdate
     if (cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_TITLE) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_VERSION) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_SIZE) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_CHECKSUM) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_CHECKSUM_ALG))
     {
          char *ota_title = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_TITLE));
          char *ota_version = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_VERSION));
          int ota_size = cJSON_GetNumberValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_SIZE));
          char *ota_checksum = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_CHECKSUM));
          char *ota_checksum_algorithm = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_CHECKSUM_ALG));
          _tbcmh_otaupdate_on_sharedattributes(client_, TBCMH_OTAUPDATE_TYPE_FW, ota_title, ota_version, ota_size, ota_checksum, ota_checksum_algorithm);
     } else if (cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_TITLE) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_VERSION) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_SIZE) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_CHECKSUM) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_CHECKSUM_ALG))
     {
          char *sw_title = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_TITLE));
          char *sw_version = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_VERSION));
          int sw_size = cJSON_GetNumberValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_SIZE));
          char *sw_checksum = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_CHECKSUM));
          char *sw_checksum_algorithm = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_CHECKSUM_ALG));
          _tbcmh_otaupdate_on_sharedattributes(client_, TBCMH_OTAUPDATE_TYPE_SW, sw_title, sw_version, sw_size, sw_checksum, sw_checksum_algorithm);
     }

     return;// ESP_OK;
}
