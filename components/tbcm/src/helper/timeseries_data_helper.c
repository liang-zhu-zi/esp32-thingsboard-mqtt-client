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

//#include "timeseries_data_helper.h"
#include "tbc_mqtt_helper_internal.h"

const static char *TAG = "timeseries_data";

/*!< Initialize tbcmh_tsdata of TBCM_JSON */
static tbcmh_tsdata_t *_tbcmh_tsdata_init(tbcmh_handle_t client, const char *key, void *context, tbcmh_tsdata_on_get_t on_get)
{
    if (!key) {
        TBC_LOGE("key is NULL");
        return NULL;
    }
    if (!on_get) {
        TBC_LOGE("on_get is NULL");
        return NULL;
    }
    
    tbcmh_tsdata_t *tsdata = TBC_MALLOC(sizeof(tbcmh_tsdata_t));
    if (!tsdata) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(tsdata, 0x00, sizeof(tbcmh_tsdata_t));
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
static tbc_err_t _tbcmh_tsdata_destroy(tbcmh_tsdata_t *tsdata)
{
    if (!tsdata) {
        TBC_LOGE("tsdata is NULL");
        return ESP_FAIL;
    }

    TBC_FREE(tsdata->key);
    TBC_FREE(tsdata);
    return ESP_OK;
}

/*!< Get key of the tbcm time-series data handle */
static const char *_tbcmh_tsdata_get_key(tbcmh_tsdata_t *tsdata)
{
    if (!tsdata) {
        TBC_LOGE("tsdata is NULL");
        return NULL;
    }
    return tsdata->key;
}

/*!< add item value to json object */
static tbc_err_t _tbcmh_tsdata_go_get(tbcmh_tsdata_t *tsdata, cJSON *object)
{
    if (!tsdata) {
        TBC_LOGE("tsdata is NULL");
        return ESP_FAIL;
    }
    if (!object) {
        TBC_LOGE("object is NULL");
        return ESP_FAIL;
    }

    cJSON *value = tsdata->on_get(tsdata->client, tsdata->context);
    if (!value) {
        TBC_LOGW("value is NULL! key=%s", tsdata->key);
        return ESP_FAIL;
    }

    cJSON_bool result = cJSON_AddItemToObject(object, tsdata->key, value);
    return result ? ESP_OK : ESP_FAIL;
}

//====10.Publish Telemetry time-series data==============================================================================
tbc_err_t tbcmh_telemetry_append(tbcmh_handle_t client_, const char *key, void *context, tbcmh_tsdata_on_get_t on_get)
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

     // Create tsdata
     tbcmh_tsdata_t *tsdata = _tbcmh_tsdata_init(client_, key/*, type*/, context, on_get/*, on_set*/);
     if (!tsdata) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init tsdata failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert tsdata to list
     tbcmh_tsdata_t *it, *last = NULL;
     if (LIST_FIRST(&client->tsdata_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->tsdata_list, tsdata, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->tsdata_list, entry) {
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

tbc_err_t tbcmh_telemetry_clear(tbcmh_handle_t client_, const char *key)
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
     tbcmh_tsdata_t *tsdata = NULL;
     LIST_FOREACH(tsdata, &client->tsdata_list, entry) {
          if (tsdata && strcmp(_tbcmh_tsdata_get_key(tsdata), key)==0) {
               break;
          }
     }
     if (!tsdata) {
          TBC_LOGW("Unable to remove time-series data:%s! %s()", key, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(tsdata, entry);
     _tbcmh_tsdata_destroy(tsdata);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

/*static*/ tbc_err_t _tbcmh_telemetry_empty(tbcmh_handle_t client_)
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

     // remove all item in tsdata_list
     tbcmh_tsdata_t *tsdata = NULL, *next;
     LIST_FOREACH_SAFE(tsdata, &client->tsdata_list, entry, next) {
          // remove from tsdata list and destory
          LIST_REMOVE(tsdata, entry);
          _tbcmh_tsdata_destroy(tsdata);
     }
     memset(&client->tsdata_list, 0x00, sizeof(client->tsdata_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

tbc_err_t tbcmh_telemetry_send(tbcmh_handle_t client_, int count, /*const char *key,*/ ...)
{
     tbcmh_t *client = (tbcmh_t *)client_;
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
     cJSON *object = cJSON_CreateObject(); // create json object
     for (i=0; i<count; i++) {
          const char *key = va_arg(ap, const char*);

          // Search item
          tbcmh_tsdata_t *tsdata = NULL;
          LIST_FOREACH(tsdata, &client->tsdata_list, entry) {
               if (tsdata && strcmp(_tbcmh_tsdata_get_key(tsdata), key)==0) {
                    break;
               }
          }

          /// Add tsdata to package
          if (tsdata) {
               _tbcmh_tsdata_go_get(tsdata, object); // add item to json object
          } else {
               TBC_LOGW("Unable to find&send time-series data:%s! %s()", key, __FUNCTION__);
          }
     }
     va_end(ap);

     // send package...
     char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
     int result = tbcm_telemetry_publish(client->tbmqttclient, pack, 1/*qos*/, 0/*retain*/);
     cJSON_free(pack); // free memory
     cJSON_Delete(object); // delete json object

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return (result > -1) ? ESP_OK : ESP_FAIL;
}

