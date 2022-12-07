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
#include <sys/queue.h>

#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"

#include "tbc_mqtt_helper.h"
#include "tbc_extension_timeseriesdata.h"

/**
 * Time-series axis
 */
typedef struct timeseriesaxis
{
     char *key;                           /*!< Key */
     void *context;                       /*!< Context of getting/setting value*/
     tbce_timeseriesdata_on_get_t on_get; /*!< Callback of getting value from context */
     LIST_ENTRY(timeseriesaxis) entry;
} timeseriesaxis_t;

typedef LIST_HEAD(tbce_timeseriesaxis_list, timeseriesaxis) timeseriesaxis_list_t;

/**
 * Time-series data
 */
typedef struct tbce_timeseriesdata
{
     timeseriesaxis_list_t timeseriesdata_list; /*!< time-series data list */
} tbce_timeseriesdata_t;

const static char *TAG = "extension_timeseriesdata";

static timeseriesaxis_t *_timeseriesdata_create(const char *key, void *context,
                                                 tbce_timeseriesdata_on_get_t on_get)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(key, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_get, NULL);
    
    timeseriesaxis_t *tsdata = TBC_MALLOC(sizeof(timeseriesaxis_t));
    if (!tsdata) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(tsdata, 0x00, sizeof(timeseriesaxis_t));
    tsdata->key = TBC_MALLOC(strlen(key)+1);
    if (tsdata->key) {
        strcpy(tsdata->key, key);
    }
    tsdata->context = context;
    tsdata->on_get = on_get;
    return tsdata;
}

/*!< Destroys timeseries_data */
static void _timeseriesdata_destroy(timeseriesaxis_t *tsdata)
{
    TBC_CHECK_PTR(tsdata);

    TBC_FREE(tsdata->key);
    TBC_FREE(tsdata);
}

tbce_timeseriesdata_handle_t tbce_timeseriesdata_create(void)
{
    tbce_timeseriesdata_t *telemetry = TBC_MALLOC(sizeof(tbce_timeseriesdata_t));
    if (!telemetry) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(telemetry, 0x00, sizeof(tbce_timeseriesdata_t));
    // list create
    // memset(&telemetry->timeseriesdata_list, 0x00, sizeof(telemetry->timeseriesdata_list)); //telemetry->timeseriesdata_list = LIST_HEAD_INITIALIZER(telemetry->timeseriesdata_list);

    return telemetry;
}

void tbce_timeseriesdata_destroy(tbce_timeseriesdata_handle_t telemetry)
{
    TBC_CHECK_PTR(telemetry);

    // items empty - remove all item in timeseriesdata_list
    timeseriesaxis_t *tsdata = NULL, *next;
    LIST_FOREACH_SAFE(tsdata, &telemetry->timeseriesdata_list, entry, next) {
         // remove from tsdata list and destory
         LIST_REMOVE(tsdata, entry);
         _timeseriesdata_destroy(tsdata);
    }
    // list destroy
    memset(&telemetry->timeseriesdata_list, 0x00, sizeof(telemetry->timeseriesdata_list));
}

tbc_err_t tbce_timeseriesdata_register(tbce_timeseriesdata_handle_t telemetry,
                                          const char *key,
                                          void *context,
                                          tbce_timeseriesdata_on_get_t on_get)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(telemetry, ESP_FAIL);

     // Create tsdata
     timeseriesaxis_t *tsdata = _timeseriesdata_create(key, context, on_get);
     if (!tsdata) {
          TBC_LOGE("Init tsdata failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert tsdata to list
     timeseriesaxis_t *it, *last = NULL;
     if (LIST_FIRST(&telemetry->timeseriesdata_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&telemetry->timeseriesdata_list, tsdata, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &telemetry->timeseriesdata_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, tsdata, entry);
          }
     }

     return ESP_OK;
}

tbc_err_t tbce_timeseriesdata_unregister(tbce_timeseriesdata_handle_t telemetry,
                                    const char *key)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(telemetry, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(key, ESP_FAIL);

     // Search item
     timeseriesaxis_t *tsdata = NULL, *next;
     LIST_FOREACH_SAFE(tsdata, &telemetry->timeseriesdata_list, entry, next) {
          if (tsdata && strcmp(tsdata->key, key)==0) {
             // Remove from list and destroy
             LIST_REMOVE(tsdata, entry);
             _timeseriesdata_destroy(tsdata);
             break;
          }
     }

     if (!tsdata) {
          TBC_LOGW("Unable to remove time-series data:%s! %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     return ESP_OK;
}

tbc_err_t tbce_timeseriesdata_upload(tbce_timeseriesdata_handle_t telemetry,
                                      tbcmh_handle_t client,
                                      int count, /*const char *key,*/...)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(telemetry, ESP_FAIL);
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
          timeseriesaxis_t *tsdata = NULL;
          LIST_FOREACH(tsdata, &telemetry->timeseriesdata_list, entry) {
               if (tsdata && strcmp(tsdata->key, key)==0) {
                    break;
               }
          }

          /// Add tsdata to package
          if (tsdata && tsdata->on_get) {
                // add item to json object
                cJSON *value = tsdata->on_get(tsdata->context);
                if (value) {
                    result |= cJSON_AddItemToObject(object, tsdata->key, value);
                } else {
                    TBC_LOGW("value is NULL! key=%s", tsdata->key);                    
                }
          } else {
               TBC_LOGW("Unable to find&send time-series data:%s! %s()", key, __FUNCTION__);
          }
     }
     va_end(ap);

     // send package...
     int msg_id = -1;
     if (result) {
         msg_id = tbcmh_telemetry_upload_ex(client, object, 1/*qos*/, 0/*retain*/);
     }
     cJSON_Delete(object); // delete json object

     return (msg_id > -1) ? ESP_OK : ESP_FAIL;
}

