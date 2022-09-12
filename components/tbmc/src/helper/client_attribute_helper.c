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

#include "esp_err.h"

#include "client_attribute_helper.h"
#include "tb_mqtt_client_helper_log.h"

const static char *TAG = "client_attribute_helper";

tbmch_clientattribute_t *_tbmch_clientattribute_init(tbmch_handle_t client, const char *key, void *context,
                                                    tbmch_clientattribute_on_get_t on_get,
                                                    tbmch_clientattribute_on_set_t on_set)
{
    if (!key) {
        TBMCH_LOGE("key is NULL");
        return NULL;
    }
    if (!on_get) {
        TBMCH_LOGE("on_get is NULL");
        return NULL;
    }
    
    tbmch_clientattribute_t *clientattribute = TBMCH_MALLOC(sizeof(tbmch_clientattribute_t));
    if (!clientattribute) {
        TBMCH_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(clientattribute, 0x00, sizeof(tbmch_clientattribute_t));
    clientattribute->client = client;
    clientattribute->key = TBMCH_MALLOC(strlen(key)+1);
    if (clientattribute->key) {
        strcpy(clientattribute->key, key);
    }
    clientattribute->context = context;
    clientattribute->on_get = on_get;
    clientattribute->on_set = on_set;
    return clientattribute;
}

/*!< Destroys tbmch_clientattribute */
tbmch_err_t _tbmch_clientattribute_destroy(tbmch_clientattribute_t *clientattribute)
{
    if (!clientattribute) {
        TBMCH_LOGE("clientattribute is NULL");
        return ESP_FAIL;
    }

    TBMCH_FREE(clientattribute->key);
    TBMCH_FREE(clientattribute);
    return ESP_OK;
}

/*!< Has it a set value callback? A shared attribute is always true; a client-side attribute is true or false. */
bool _tbmch_clientattribute_has_set_value_cb(tbmch_clientattribute_t *clientattribute)
{
    if (!clientattribute) {
        TBMCH_LOGE("clientattribute is NULL");
        return false;
    }

    return clientattribute->on_set ? true : false;
}

/*!< Get key of the tbmc tbmch_attribute handle */
const char *_tbmch_clientattribute_get_key(tbmch_clientattribute_t *clientattribute)
{
    if (!clientattribute) {
        TBMCH_LOGE("clientattribute is NULL");
        return NULL;
    }
    return clientattribute->key;
}

/*!< add item value to json object */
tbmch_err_t _tbmch_clientattribute_do_get(tbmch_clientattribute_t *clientattribute, cJSON *object)
{
    if (!clientattribute) {
        TBMCH_LOGE("clientattribute is NULL");
        return ESP_FAIL;
    }
    if (!object) {
        TBMCH_LOGE("object is NULL");
        return ESP_FAIL;
    }

    cJSON *value = clientattribute->on_get(clientattribute->client, clientattribute->context);
    if (!value) {
        TBMCH_LOGW("value is NULL! key=%s", clientattribute->key);
        return ESP_FAIL;
    }

    cJSON_bool result = cJSON_AddItemToObject(object, clientattribute->key, value);
    return result ? ESP_OK : ESP_FAIL;
}

/*!< add item value to json object */
tbmch_err_t _tbmch_clientattribute_do_set(tbmch_clientattribute_t *clientattribute, cJSON *value)
{
    if (!clientattribute) {
        TBMCH_LOGE("clientattribute is NULL");
        return ESP_FAIL;
    }
    if (!value) {
        TBMCH_LOGE("value is NULL");
        return ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, clientattribute->key);;
    if (!value) {
        TBMCH_LOGW("value is NULL! key=%s", clientattribute->key);
        return ESP_FAIL;
    }*/

    clientattribute->on_set(clientattribute->client, clientattribute->context, value);
    return ESP_OK;
}
