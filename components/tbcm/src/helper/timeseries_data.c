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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "esp_err.h"

//#include "timeseriesdata.h"
#include "tbc_mqtt_helper_internal.h"

const static char *TAG = "timeseriesdata";

/*!< Initialize timeseriesdata of TBCM_JSON */
static timeseriesdata_t *_timeseriesdata_create(tbcmh_handle_t client, const char *key,
                            void *context, tbcmh_tsdata_on_get_t on_get)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(key, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_get, NULL);
    
    timeseriesdata_t *tsdata = TBC_MALLOC(sizeof(timeseriesdata_t));
    if (!tsdata) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(tsdata, 0x00, sizeof(timeseriesdata_t));
    tsdata->client = client;
    tsdata->key = TBC_MALLOC(strlen(key)+1);
    if (tsdata->key) {
        strcpy(tsdata->key, key);
    }
    tsdata->context = context;
    tsdata->on_get = on_get;
    return tsdata;
}

/*!< Destroys the tbcm key-value handle */
static tbc_err_t _timeseriesdata_destroy(timeseriesdata_t *tsdata)
{
    if (!tsdata) {
        TBC_LOGE("tsdata is NULL");
        return ESP_FAIL;
    }

    TBC_FREE(tsdata->key);
    TBC_FREE(tsdata);
    return ESP_OK;
}

//====10.Publish Telemetry time-series data==============================================================================
tbc_err_t tbcmh_timeseriesdata_register(tbcmh_handle_t client, const char *key,
                    void *context, tbcmh_tsdata_on_get_t on_get)
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

     // Create tsdata
     timeseriesdata_t *tsdata = _timeseriesdata_create(client, key/*, type*/, context, on_get/*, on_set*/);
     if (!tsdata) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init tsdata failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert tsdata to list
     timeseriesdata_t *it, *last = NULL;
     if (LIST_FIRST(&client->timeseriesdata_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->timeseriesdata_list, tsdata, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->timeseriesdata_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, tsdata, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

tbc_err_t tbcmh_timeseriesdata_unregister(tbcmh_handle_t client, const char *key)
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
     timeseriesdata_t *tsdata = NULL, *next;
     LIST_FOREACH_SAFE(tsdata, &client->timeseriesdata_list, entry, next) {
          if (tsdata && strcmp(tsdata->key, key)==0) {
             // remove from tsdata list and destory
             LIST_REMOVE(tsdata, entry);
             _timeseriesdata_destroy(tsdata);
             break;
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);

     if (!tsdata) {
          TBC_LOGW("Unable to remove time-series data:%s! %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }
     return ESP_OK;
}

tbc_err_t tbcmh_timeseriesdata_update(tbcmh_handle_t client, int count, /*const char *key,*/ ...)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL)
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
          timeseriesdata_t *tsdata = NULL;
          LIST_FOREACH(tsdata, &client->timeseriesdata_list, entry) {
               if (tsdata && strcmp(tsdata->key, key)==0) {
                    break;
               }
          }

          /// Add tsdata to package
          if (tsdata && tsdata->on_get) {
               cJSON *value = tsdata->on_get(tsdata->client, tsdata->context);
               if (value) {
                   result |= cJSON_AddItemToObject(object, tsdata->key, value);
               }
          } else {
               TBC_LOGW("Unable to find&send time-series data:%s! %s()", key, __FUNCTION__);
          }
     }
     va_end(ap);

     // send package...
     int msg_id = -1;
     if (result) {
         char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
         msg_id = tbcm_telemetry_publish(client->tbmqttclient, pack, 1/*qos*/, 0/*retain*/);
         cJSON_free(pack); // free memory
     }
     cJSON_Delete(object); // delete json object

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return (msg_id > -1) ? ESP_OK : ESP_FAIL;
}

void _tbcmh_timeseriesdata_on_create(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)

    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return ESP_FAIL;
    // }
    
    // list create
    memset(&client->timeseriesdata_list, 0x00, sizeof(client->timeseriesdata_list)); //client->timeseriesdata_list = LIST_HEAD_INITIALIZER(client->timeseriesdata_list);

    // Give semaphore
    // xSemaphoreGive(client->_lock);
}

void _tbcmh_timeseriesdata_on_destroy(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // TODO: How to add lock??
    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return ESP_FAIL;
    // }
    
    // items empty - remove all item in timeseriesdata_list
    timeseriesdata_t *tsdata = NULL, *next;
    LIST_FOREACH_SAFE(tsdata, &client->timeseriesdata_list, entry, next) {
         // remove from tsdata list and destory
         LIST_REMOVE(tsdata, entry);
         _timeseriesdata_destroy(tsdata);
    }
    // list destroy
    memset(&client->timeseriesdata_list, 0x00, sizeof(client->timeseriesdata_list));
    
    // Give semaphore
    // xSemaphoreGive(client->_lock);
}

void _tbcmh_timeseriesdata_on_connected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)
    //......
}

void _tbcmh_timeseriesdata_on_disconnected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)
    // ...
}

