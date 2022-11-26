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

// TODO: remove it!
extern void _tbcmh_otaupdate_on_sharedattributes(tbcmh_handle_t client, tbcmh_otaupdate_type_t ota_type,
                                                 const char *ota_title, const char *ota_version, int ota_size,
                                                 const char *ota_checksum, const char *ota_checksum_algorithm);


const static char *TAG = "sharedattribute";

static sharedattribute_t *_sharedattribute_create(tbcmh_handle_t client,
                                            const char *key, void *context,
                                            tbcmh_sharedattribute_on_set_t on_set)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(key, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_set, NULL);
    
    sharedattribute_t *sharedattribute = TBC_MALLOC(sizeof(sharedattribute_t));
    if (!sharedattribute) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(sharedattribute, 0x00, sizeof(sharedattribute_t));
    sharedattribute->client = client;
    sharedattribute->key = TBC_MALLOC(strlen(key)+1);
    if (sharedattribute->key) {
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

//Call it before connect()
tbc_err_t tbcmh_sharedattribute_register(tbcmh_handle_t client,
                                                  const char *key, void *context,
                                                  tbcmh_sharedattribute_on_set_t on_set)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Create sharedattribute
     sharedattribute_t *sharedattribute = _sharedattribute_create(client, key, context, on_set);
     if (!sharedattribute) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init sharedattribute failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert sharedattribute to list
     sharedattribute_t *it, *last = NULL;
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

// remove sharedattribute from tbcmh_shared_attribute_list_t
tbc_err_t tbcmh_sharedattribute_unregister(tbcmh_handle_t client, const char *key)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(key, ESP_FAIL);

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Search item
     sharedattribute_t *sharedattribute = NULL, *next;
     LIST_FOREACH_SAFE(sharedattribute, &client->sharedattribute_list, entry, next) {
          if (sharedattribute && strcmp(sharedattribute->key, key)==0) {
              // Remove form list
              LIST_REMOVE(sharedattribute, entry);
              _sharedattribute_destroy(sharedattribute);
              break;
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);

     if (!sharedattribute) {
          TBC_LOGW("Unable to remove shared-attribute:%s! %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }
     return ESP_OK;
}

void _tbcmh_sharedattribute_on_create(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)

    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }
    
    // list create
    memset(&client->sharedattribute_list, 0x00, sizeof(client->sharedattribute_list)); //client->sharedattribute_list = LIST_HEAD_INITIALIZER(client->sharedattribute_list);

    // Give semaphore
    // xSemaphoreGive(client->_lock);
}

void _tbcmh_sharedattribute_on_destroy(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // TODO: How to add lock??
    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // items empty - remove all item in sharedattribute_list
    sharedattribute_t *sharedattribute = NULL, *next;
    LIST_FOREACH_SAFE(sharedattribute, &client->sharedattribute_list, entry, next) {
         // remove from sharedattribute list and destory
         LIST_REMOVE(sharedattribute, entry);
         _sharedattribute_destroy(sharedattribute);
    }
    memset(&client->sharedattribute_list, 0x00, sizeof(client->sharedattribute_list));

    // Give semaphore
    // xSemaphoreGive(client->_lock);
}

void _tbcmh_sharedattribute_on_connected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);
    int msg_id = tbcm_subscribe(client->tbmqttclient, TB_MQTT_TOPIC_SHARED_ATTRIBUTES, 0);
    TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s",
            msg_id, TB_MQTT_TOPIC_SHARED_ATTRIBUTES);
}

void _tbcmh_sharedattribute_on_disconnected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);
    //...
}

//on received: unpack & deal
void _tbcmh_sharedattribute_on_data(tbcmh_handle_t client, const cJSON *object)
{
     TBC_CHECK_PTR(client);
     TBC_CHECK_PTR(object);

     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
     //      return;
     // }

     // foreach itme to set value of sharedattribute in lock/unlodk.  Don't call tbcmh's funciton in set value callback!
     sharedattribute_t *sharedattribute = NULL;
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
     // xSemaphoreGive(client->_lock);
     // return;

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
}

