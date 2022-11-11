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

#include "timeseries_data_helper.h"
#include "tbc_utils.h"

const static char *TAG = "timeseries_data";

/*!< Initialize tbcmh_tsdata of TBCM_JSON */
tbcmh_tsdata_t *_tbcmh_tsdata_init(tbcmh_handle_t client, const char *key, void *context, tbcmh_tsdata_on_get_t on_get)
{
    if (!key) {
        TBC_LOGE("key is NULL");
        return NULL;
    }
    if (!on_get) {
        TBC_LOGE("on_get is NULL");
        return NULL;
    }
    
    tbcmh_tsdata_t *tsdata = TBCMH_MALLOC(sizeof(tbcmh_tsdata_t));
    if (!tsdata) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(tsdata, 0x00, sizeof(tbcmh_tsdata_t));
    tsdata->client = client;
    tsdata->key = TBCMH_MALLOC(strlen(key)+1);
    if (tsdata->key) {
        strcpy(tsdata->key, key);
    }
    tsdata->context = context;
    tsdata->on_get = on_get;
    return tsdata;
}

/*!< Destroys the tbmc key-value handle */
tbcmh_err_t _tbcmh_tsdata_destroy(tbcmh_tsdata_t *tsdata)
{
    if (!tsdata) {
        TBC_LOGE("tsdata is NULL");
        return ESP_FAIL;
    }

    TBCMH_FREE(tsdata->key);
    TBCMH_FREE(tsdata);
    return ESP_OK;
}

/*!< Get key of the tbmc time-series data handle */
const char *_tbcmh_tsdata_get_key(tbcmh_tsdata_t *tsdata)
{
    if (!tsdata) {
        TBC_LOGE("tsdata is NULL");
        return NULL;
    }
    return tsdata->key;
}

/*!< add item value to json object */
tbcmh_err_t _tbcmh_tsdata_go_get(tbcmh_tsdata_t *tsdata, cJSON *object)
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
