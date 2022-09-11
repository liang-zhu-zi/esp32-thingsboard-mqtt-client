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

#include "shared_attribute_observer.h"

tbmch_sharedattribute_t *_tbmch_sharedattribute_init(tbmch_handle_t client, const char *key, void *context,
                                                     tbmch_sharedattribute_on_set_t on_set)
{
    if (!key) {
        TBMCHLOG_E("key is NULL");
        return NULL;
    }
    
    tbmch_sharedattribute_t *sharedattribute = TBMCH_MALLOC(sizeof(tbmch_sharedattribute_t));
    if (!sharedattribute) {
        TBMCHLOG_E("Unable to malloc memeory!");
        return NULL;
    }

    memset(sharedattribute, 0x00, sizeof(tbmch_sharedattribute_t));
    sharedattribute->client = client;
    sharedattribute->key = TBMCH_MALLOC(strlen(key)+1);
    if (sharedattribute->key) {
        strcpy(sharedattribute->key, key);
    }
    sharedattribute->context = context;
    sharedattribute->on_set = on_set;
    return sharedattribute;
}

/*!< Destroys the tbmc key-value handle */
tbmch_err_t _tbmch_sharedattributee_destroy(tbmch_sharedattribute_t *sharedattribute)
{
    if (!sharedattribute) {
        TBMCHLOG_E("sharedattribute is NULL");
        return ESP_FAIL;
    }

    TBMC_FREE(sharedattribute->key);
    TBMC_FREE(sharedattribute);
    return ESP_OK;
}

/*!< Get key of the tbmc tbmch_attribute handle */
const char *_tbmch_sharedattribute_get_key(tbmch_sharedattribute_t *sharedattribute)
{
    if (!sharedattribute) {
        TBMCHLOG_E("sharedattribute is NULL");
        return NULL;
    }
    return sharedattribute->key;
}

/*!< add item value to json object */
tbmch_err_t _tbmch_sharedattribute_do_set(tbmch_sharedattribute_t *sharedattribute, cJSON *value)                                               
{
    if (!sharedattribute) {
        TBMCHLOG_E("sharedattribute is NULL");
        return ESP_FAIL;
    }
    if (!value) {
        TBMCHLOG_E("value is NULL");
        return ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, sharedattribute->key);;
    if (!value) {
        TBMCHLOG_W("value is NULL! key=%s", sharedattribute->key);
        return ESP_FAIL;
    }*/

    sharedattribute->on_set(sharedattribute->client, sharedattribute->context, value);
    return ESP_OK;
}
