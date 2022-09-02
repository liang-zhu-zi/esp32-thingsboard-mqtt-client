// Copyright 2022 liangzhuzhi2020@gmail.com, https://github.com/liang-zhu-zi/thingsboard-mqttclient-basedon-espmqtt
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

// This file is called by tb_mqtt_client_helper.c/.h.

#include <string.h>

#include "timeseries_data_helper.h"

/*!< Initialize tbmch_tsdata of TBMC_JSON */
tbmch_tsdata_t *_tbmch_tsdata_init(const char *key, void *context, tbmch_tsdata_on_get_t on_get)
{
    if (!key) {
        TBMCHLOG_E("key is NULL");
        return NULL;
    }
    if (!on_get) {
        TBMCHLOG_E("on_get is NULL");
        return NULL;
    }
    
    tbmch_tsdata_t *tsdata = TBMCH_MALLOC(sizeof(tbmch_tsdata_t));
    if (!tsdata) {
        TBMCHLOG_E("Unable to malloc memeory!");
        return NULL;
    }

    memset(tsdata, 0x00, sizeof(tbmch_tsdata_t));
    tsdata->key = TBMCH_MALLOC(strlen(key)+1);
    if (tsdata->key) {
        strcpy(tsdata->key, key);
    }
    tsdata->context = context;
    tsdata->on_get = on_get;
    return tsdata;
}

/*!< Destroys the tbmc key-value handle */
tbmch_err_t _tbmch_tsdata_destroy(tbmch_tsdata_t *tsdata)
{
    if (!tsdata) {
        TBMCHLOG_E("tsdata is NULL");
        return ESP_FAIL;
    }

    TBMC_FREE(tsdata->key);
    TBMC_FREE(tsdata);
    return ESP_OK;
}

/*!< Get key of the tbmc time-series data handle */
const char *_tbmch_tsdata_get_key(tbmch_tsdata_t *tsdata)
{
    if (!tsdata) {
        TBMCHLOG_E("tsdata is NULL");
        return NULL;
    }
    return tsdata->key;
}

/*!< add item value to json object */
tbmch_err_t _tbmch_tsdata_value_to_pack(tbmch_handle_t client, tbmch_tsdata_t *tsdata, cJSON *object)
{
    if (!tsdata) {
        TBMCHLOG_E("tsdata is NULL");
        return ESP_FAIL;
    }
    if (!object) {
        TBMCHLOG_E("object is NULL");
        return ESP_FAIL;
    }

    cJSON *value = tsdata->on_get(client, tsdata->context);
    if (!value) {
        TBMCHLOG_W("value is NULL! key=%s", tsdata->key);
        return ESP_FAIL;
    }

    cJSON_bool result = cJSON_AddItemToObject(object, tsdata->key, value);
    return result ? ESP_OK : ESP_FAIL;
}
