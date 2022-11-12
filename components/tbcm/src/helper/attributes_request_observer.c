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

#include "attributes_request_observer.h"
#include "tbc_utils.h"

const static char *TAG = "attributes_request";

/*!< Initialize tbcmh_attributesrequest */
tbcmh_attributesrequest_t *_tbcmh_attributesrequest_init(tbcmh_handle_t client, int request_id, void *context,
                                                         tbcmh_attributesrequest_on_response_t on_response,
                                                         tbcmh_attributesrequest_on_timeout_t on_timeout)
{
    if (!on_response) {
        TBC_LOGE("on_response is NULL");
        return NULL;
    }
    
    tbcmh_attributesrequest_t *attributesrequest = TBC_MALLOC(sizeof(tbcmh_attributesrequest_t));
    if (!attributesrequest) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(attributesrequest, 0x00, sizeof(tbcmh_attributesrequest_t));
    attributesrequest->client = client;
    attributesrequest->request_id = request_id;
    attributesrequest->context = context;
    attributesrequest->on_response = on_response;
    attributesrequest->on_timeout = on_timeout;
    return attributesrequest;
}

tbcmh_attributesrequest_t *_tbcmh_attributesrequest_clone_wo_listentry(tbcmh_attributesrequest_t *src)
{
    if (!src) {
        TBC_LOGE("src is NULL");
        return NULL;
    }
    
    tbcmh_attributesrequest_t *attributesrequest = TBC_MALLOC(sizeof(tbcmh_attributesrequest_t));
    if (!attributesrequest) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(attributesrequest, 0x00, sizeof(tbcmh_attributesrequest_t));
    attributesrequest->client = src->client;
    attributesrequest->request_id = src->request_id;
    attributesrequest->context = src->context;
    attributesrequest->on_response = src->on_response;
    attributesrequest->on_timeout = src->on_timeout;
    return attributesrequest;
}

/*!< Destroys the tbcmh_attributesrequest */
tbc_err_t _tbcmh_attributesrequest_destroy(tbcmh_attributesrequest_t *attributesrequest)
{
    if (!attributesrequest) {
        TBC_LOGE("attributesrequest is NULL");
        return ESP_FAIL;
    }

    TBC_FREE(attributesrequest);
    return ESP_OK;
}

int _tbcmh_attributesrequest_get_request_id(tbcmh_attributesrequest_t *attributesrequest)
{
    if (!attributesrequest) {
        TBC_LOGE("attributesrequest is NULL");
        return -1;
    }
    return attributesrequest->request_id;
}

//(none/resend/destroy/_destroy_all_attributes)?
void _tbcmh_attributesrequest_do_response(tbcmh_attributesrequest_t *attributesrequest)
{
    if (!attributesrequest) {
        TBC_LOGE("attributesrequest is NULL");
        return; // ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, attributesrequest->key);;
    if (!value) {
        TBC_LOGW("value is NULL! key=%s", attributesrequest->key);
        return; // ESP_FAIL;
    }*/

    attributesrequest->on_response(attributesrequest->client, attributesrequest->context, attributesrequest->request_id); //(none/resend/destroy/_destroy_all_attributes)?
    return; // ESP_OK;
}

//(none/resend/destroy/_destroy_all_attributes)? 
void _tbcmh_attributesrequest_do_timeout(tbcmh_attributesrequest_t *attributesrequest)
{
    if (!attributesrequest) {
        TBC_LOGE("attributesrequest is NULL");
        return; // ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, attributesrequest->key);;
    if (!value) {
        TBC_LOGW("value is NULL! key=%s", attributesrequest->key);
        return; // ESP_FAIL;
    }*/

    if (attributesrequest->on_timeout) {
        attributesrequest->on_timeout(attributesrequest->client, attributesrequest->context, attributesrequest->request_id); //(none/resend/destroy/_destroy_all_attributes)?
    }
    return; // ESP_OK;
}
