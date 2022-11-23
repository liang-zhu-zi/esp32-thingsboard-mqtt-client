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

//#include "shared_attribute.h"
#include "tbc_mqtt_helper_internal.h"

// TODO: remove it!
extern void _tbcmh_otaupdate_on_sharedattributes(tbcmh_handle_t client, tbcmh_otaupdate_type_t ota_type,
                                                 const char *ota_title, const char *ota_version, int ota_size,
                                                 const char *ota_checksum, const char *ota_checksum_algorithm);


const static char *TAG = "shared_attribute";

static shared_attribute_t *_shared_attribute_create(tbcmh_handle_t client, const char *key, void *context,
                                                     tbcmh_sharedattribute_on_set_t on_set)
{
    if (!key) {
        TBC_LOGE("key is NULL");
        return NULL;
    }
    
    shared_attribute_t *sharedattribute = TBC_MALLOC(sizeof(shared_attribute_t));
    if (!sharedattribute) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(sharedattribute, 0x00, sizeof(shared_attribute_t));
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
static tbc_err_t _shared_attribute_destroy(shared_attribute_t *sharedattribute)
{
    if (!sharedattribute) {
        TBC_LOGE("sharedattribute is NULL");
        return ESP_FAIL;
    }

    TBC_FREE(sharedattribute->key);
    TBC_FREE(sharedattribute);
    return ESP_OK;
}

//==== Subscribe to shared device attribute updates from the server ================================
//Call it before connect() //tbcmh_shared_attribute_list_t
tbc_err_t tbcmh_sharedattribute_append(tbcmh_handle_t client, const char *key, void *context,
                                         tbcmh_sharedattribute_on_set_t on_set)
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

     // Create sharedattribute
     shared_attribute_t *sharedattribute = _shared_attribute_create(client, key, context, on_set);
     if (!sharedattribute) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init sharedattribute failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert sharedattribute to list
     shared_attribute_t *it, *last = NULL;
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
tbc_err_t tbcmh_sharedattribute_clear(tbcmh_handle_t client, const char *key)
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
     shared_attribute_t *sharedattribute = NULL;
     LIST_FOREACH(sharedattribute, &client->sharedattribute_list, entry) {
          if (sharedattribute && strcmp(sharedattribute->key, key)==0) {
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
     _shared_attribute_destroy(sharedattribute);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

tbc_err_t _tbcmh_sharedattribute_empty(tbcmh_handle_t client)
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

     // remove all item in sharedattribute_list
     shared_attribute_t *sharedattribute = NULL, *next;
     LIST_FOREACH_SAFE(sharedattribute, &client->sharedattribute_list, entry, next) {
          // remove from sharedattribute list and destory
          LIST_REMOVE(sharedattribute, entry);
          _shared_attribute_destroy(sharedattribute);
     }
     memset(&client->sharedattribute_list, 0x00, sizeof(client->sharedattribute_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

//unpack & deal
void _tbcmh_sharedattribute_on_received(tbcmh_handle_t client, const cJSON *object)
{
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
     shared_attribute_t *sharedattribute = NULL;
     const char* key = NULL;
     LIST_FOREACH(sharedattribute, &client->sharedattribute_list, entry) {
          if (sharedattribute) {
               key = sharedattribute->key;
               if (cJSON_HasObjectItem(object, key)) {
                    cJSON *value = cJSON_GetObjectItem(object, key);
                    if (sharedattribute->on_set && value) {
                        sharedattribute->on_set(sharedattribute->client, sharedattribute->context, value);
                    }
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
          _tbcmh_otaupdate_on_sharedattributes(client, TBCMH_OTAUPDATE_TYPE_FW, ota_title, ota_version, ota_size, ota_checksum, ota_checksum_algorithm);
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
          _tbcmh_otaupdate_on_sharedattributes(client, TBCMH_OTAUPDATE_TYPE_SW, sw_title, sw_version, sw_size, sw_checksum, sw_checksum_algorithm);
     }

     return;// ESP_OK;
}

