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

#include "provision_observer.h"
#include "tb_mqtt_client_helper_log.h"

const static char *TAG = "provision";

/*!< Initialize tbmch_provision_t */
tbmch_provision_t *_tbmch_provision_init(tbmch_handle_t client, int request_id,
                                         void *context,
                                         tbmch_provision_on_response_t on_response,
                                         tbmch_provision_on_timeout_t on_timeout)
{
    if (!on_response) {
        TBMCH_LOGE("on_response is NULL");
        return NULL;
    }
    
    tbmch_provision_t *provision = TBMCH_MALLOC(sizeof(tbmch_provision_t));
    if (!provision) {
        TBMCH_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(provision, 0x00, sizeof(tbmch_provision_t));
    provision->client = client;
    provision->request_id = request_id;
    provision->context = context;
    provision->on_response = on_response;
    provision->on_timeout = on_timeout;
    return provision;
}

tbmch_provision_t *_tbmch_provision_clone_wo_listentry(tbmch_provision_t *src)
{
    if (!src) {
        TBMCH_LOGE("src is NULL");
        return NULL;
    }
    
    tbmch_provision_t *provision = TBMCH_MALLOC(sizeof(tbmch_provision_t));
    if (!provision) {
        TBMCH_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(provision, 0x00, sizeof(tbmch_provision_t));
    provision->client = src->client;
    provision->request_id = src->request_id;
    provision->context = src->context;
    provision->on_response = src->on_response;
    provision->on_timeout = src->on_timeout;
    return provision;
}

int _tbmch_provision_get_request_id(tbmch_provision_t *provision)
{
    if (!provision) {
        TBMCH_LOGE("provision is NULL");
        return -1;
    }

    return provision->request_id;
}

/*!< Destroys the tbmch_provision_t */
tbmch_err_t _tbmch_provision_destroy(tbmch_provision_t *provision)
{
    if (!provision) {
        TBMCH_LOGE("provision is NULL");
        return ESP_FAIL;
    }

    TBMCH_FREE(provision);
    return ESP_OK;
}

void _tbmch_provision_do_response(tbmch_provision_t *provision, const tbmch_provision_results_t *results)
{
    if (!provision) {
        TBMCH_LOGE("provision is NULL");
        return; // ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, provision->key);;
    if (!value) {
        TBMCH_LOGW("value is NULL! key=%s", provision->key);
        return; // ESP_FAIL;
    }*/

    provision->on_response(provision->client, provision->context, provision->request_id, results);
    return; // ESP_OK;
}

void _tbmch_provision_do_timeout(tbmch_provision_t *provision)
{
    if (!provision) {
        TBMCH_LOGE("provision is NULL");
        return; // ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, provision->key);;
    if (!value) {
        TBMCH_LOGW("value is NULL! key=%s", provision->key);
        return; // ESP_FAIL;
    }*/

    if (provision->on_timeout) {
        provision->on_timeout(provision->client, provision->context, provision->request_id);
    }
    return; // ESP_OK;
}
