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

#include "cJSON.h"

#include "provision_observer.h"
#include "tbc_mqtt.h"
#include "tbc_utils.h"

const static char *TAG = "provision";

/*!< Initialize tbmch_provision_t */
tbmch_provision_t *_tbmch_provision_init(tbmch_handle_t client, int request_id,
                                         const tbmch_provision_params_t *params,
                                         void *context,
                                         tbmch_provision_on_response_t on_response,
                                         tbmch_provision_on_timeout_t on_timeout)
{
    if (!on_response) {
        TBC_LOGE("on_response is NULL");
        return NULL;
    }
    
    tbmch_provision_t *provision = TBMCH_MALLOC(sizeof(tbmch_provision_t));
    if (!provision) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(provision, 0x00, sizeof(tbmch_provision_t));
    provision->client = client;
    provision->params = cJSON_Duplicate(params, true);
    provision->request_id = request_id;
    provision->context = context;
    provision->on_response = on_response;
    provision->on_timeout = on_timeout;
    return provision;
}

tbmch_provision_t *_tbmch_provision_clone_wo_listentry(tbmch_provision_t *src)
{
    if (!src) {
        TBC_LOGE("src is NULL");
        return NULL;
    }
    
    tbmch_provision_t *provision = TBMCH_MALLOC(sizeof(tbmch_provision_t));
    if (!provision) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(provision, 0x00, sizeof(tbmch_provision_t));
    provision->client = src->client;
    provision->params = cJSON_Duplicate(src->params, true);
    provision->request_id = src->request_id;
    provision->context = src->context;
    provision->on_response = src->on_response;
    provision->on_timeout = src->on_timeout;
    return provision;
}

int _tbmch_provision_get_request_id(tbmch_provision_t *provision)
{
    if (!provision) {
        TBC_LOGE("provision is NULL");
        return -1;
    }

    return provision->request_id;
}

/*!< Destroys the tbmch_provision_t */
tbmch_err_t _tbmch_provision_destroy(tbmch_provision_t *provision)
{
    if (!provision) {
        TBC_LOGE("provision is NULL");
        return ESP_FAIL;
    }

    cJSON_Delete(provision->params);
    provision->params = NULL;
    TBMCH_FREE(provision);
    return ESP_OK;
}

static char *_parse_string_item(const cJSON *object, const char* key)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(object, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(key, NULL);

    cJSON *item = cJSON_GetObjectItem(object, key);
    if (!item) {
        TBC_LOGW("%s is not existed!", key);
        return NULL;
    }
    char *string_value = cJSON_GetStringValue(item);
    if (!string_value) {
        TBC_LOGW("%s is NULL!", key);
        return NULL;
    }
    return string_value;
}

static int _parse_provision_response(const tbmch_provision_results_t *results,
                                          tbc_transport_credentials_config_t *credentials)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(results, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(credentials, ESP_FAIL);

    bool isSuccess = false;
    cJSON *status = cJSON_GetObjectItem(results, TB_MQTT_KEY_PROVISION_STATUS);
    if (status) {
        if (strcmp(cJSON_GetStringValue(status), TB_MQTT_VALUE_PROVISION_SUCCESS) ==0) {
            isSuccess = true;
        }
    }
    cJSON *provisionDeviceStatus = cJSON_GetObjectItem(results, TB_MQTT_KEY_PROVISION_DEVICE_STATUS);
    if (provisionDeviceStatus) {
        if (strcmp(cJSON_GetStringValue(provisionDeviceStatus), TB_MQTT_VALUE_PROVISION_SUCCESS) ==0) {
            isSuccess = true;
        }
    }
    if (!isSuccess) {
        char *results_str = cJSON_PrintUnformatted(results); //cJSON_Print(object);
        TBC_LOGW("provision response is failure! %s", results_str);
        cJSON_free(results_str); // free memory
        return ESP_FAIL;
    }

    char *credentialsTypeStr = _parse_string_item(results, TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE);
    if (!credentialsTypeStr) {
        TBC_LOGW("credentialsType is error!");
        return ESP_FAIL;
    }

    if (strcmp(credentialsTypeStr, TB_MQTT_VALUE_PROVISION_ACCESS_TOKEN) == 0) {
        //{
        //  "credentialsType":"ACCESS_TOKEN",
        //  "credentialsValue":"sLzc0gDAZPkGMzFVTyUY"
        //  "status":"SUCCESS"
        //}
        credentials->type = TBC_TRANSPORT_CREDENTIALS_TYPE_ACCESS_TOKEN;
        credentials->token = _parse_string_item(results, TB_MQTT_KEY_PROVISION_CREDENTIALS_VALUE);
        if (!credentials->token){
            TBC_LOGW("credentialsValue is error!");
            return ESP_FAIL;
        }

    } else if (strcmp(credentialsTypeStr, TB_MQTT_VALUE_PROVISION_MQTT_BASIC) == 0) {
        //{
        //  "credentialsType":"MQTT_BASIC",
        //  "credentialsValue": {
        //    "clientId":"DEVICE_CLIENT_ID_HERE",
        //    "userName":"DEVICE_USERNAME_HERE",
        //    "password":"DEVICE_PASSWORD_HERE"
        //    },
        //  "status":"SUCCESS"
        //}
        credentials->type = TBC_TRANSPORT_CREDENTIALS_TYPE_BASIC_MQTT;
        cJSON *credentialsValue = cJSON_GetObjectItem(results, TB_MQTT_KEY_PROVISION_CREDENTIALS_VALUE);
        if (!credentialsValue){
            TBC_LOGW("credentialsValue is NOT existed!");
            return ESP_FAIL;
        }
        credentials->client_id = _parse_string_item(credentialsValue, TB_MQTT_KEY_PROVISION_CLIENT_ID);
        credentials->username  = _parse_string_item(credentialsValue, TB_MQTT_KEY_PROVISION_USERNAME);
        if (!credentials->username) {
            credentials->username  = _parse_string_item(credentialsValue, TB_MQTT_KEY_PROVISION_USERNAME2);
        }
        credentials->password  = _parse_string_item(credentialsValue, TB_MQTT_KEY_PROVISION_PASSWORD);

    } else if (strcmp(credentialsTypeStr, TB_MQTT_VALUE_PROVISION_X509_CERTIFICATE) == 0) {
        //{
        //  "deviceId":"3b829220-232f-11eb-9d5c-e9ed3235dff8",
        //  "credentialsType":"X509_CERTIFICATE",
        //  "credentialsId":"f307a1f717a12b32c27203cf77728d305d29f64694a8311be921070dd1259b3a",
        //  "credentialsValue":"MIIB........AQAB",
        //  "provisionDeviceStatus":"SUCCESS"
        //}
        credentials->type = TBC_TRANSPORT_CREDENTIALS_TYPE_X509;
        credentials->token = _parse_string_item(results, TB_MQTT_KEY_PROVISION_CREDENTIALS_VALUE);
        if (!credentials->token){
            TBC_LOGW("credentialsValue is error!");
            return ESP_FAIL;
        }

    } else {
        TBC_LOGW("credentialsType(%s) is error!", credentialsTypeStr);
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

void _tbmch_provision_do_response(tbmch_provision_t *provision, const tbmch_provision_results_t *results)
{
    if (!provision) {
        TBC_LOGE("provision is NULL");
        return; // ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, provision->key);;
    if (!value) {
        TBC_LOGW("value is NULL! key=%s", provision->key);
        return; // ESP_FAIL;
    }*/

    // parse results - provision response
    tbc_transport_credentials_config_t credentials = {0};
    int result = _parse_provision_response(results, &credentials);
    if (result == ESP_OK) {
        provision->on_response(provision->client, provision->context, provision->request_id, &credentials);
    } else {
        provision->on_timeout(provision->client, provision->context, provision->request_id); // TODO: a new faiure callback?
    }
    return; // ESP_OK;
}

void _tbmch_provision_do_timeout(tbmch_provision_t *provision)
{
    if (!provision) {
        TBC_LOGE("provision is NULL");
        return; // ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, provision->key);;
    if (!value) {
        TBC_LOGW("value is NULL! key=%s", provision->key);
        return; // ESP_FAIL;
    }*/

    if (provision->on_timeout) {
        provision->on_timeout(provision->client, provision->context, provision->request_id);
    }
    return; // ESP_OK;
}

