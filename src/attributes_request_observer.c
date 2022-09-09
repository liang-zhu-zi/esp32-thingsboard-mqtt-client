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

#include "attributes_request_observer.h"

/*!< Initialize tbmch_attributesrequest */
tbmch_attributesrequest_t *_tbmch_attributesrequest_init(tbmch_handle_t client, int request_id, void *context,
                                                         tbmch_attributesrequest_on_response_t on_response,
                                                         tbmch_attributesrequest_on_timeout_t on_timeout)
{
    if (!on_response) {
        TBMCHLOG_E("on_response is NULL");
        return NULL;
    }
    
    tbmch_attributesrequest_t *attributesrequest = TBMCH_MALLOC(sizeof(tbmch_attributesrequest_t));
    if (!attributesrequest) {
        TBMCHLOG_E("Unable to malloc memeory!");
        return NULL;
    }

    memset(attributesrequest, 0x00, sizeof(tbmch_attributesrequest_t));
    attributesrequest->client = client;
    attributesrequest->request_id = request_id;
    attributesrequest->context = context;
    attributesrequest->on_response = on_response;
    attributesrequest->on_timeout = on_timeout;
    return attributesrequest;
}

tbmch_attributesrequest_t *_tbmch_attributesrequest_clone_wo_listentry(tbmch_attributesrequest_t *src)
{
    if (!src) {
        TBMCHLOG_E("src is NULL");
        return NULL;
    }
    
    tbmch_attributesrequest_t *attributesrequest = TBMCH_MALLOC(sizeof(tbmch_attributesrequest_t));
    if (!attributesrequest) {
        TBMCHLOG_E("Unable to malloc memeory!");
        return NULL;
    }

    memset(attributesrequest, 0x00, sizeof(tbmch_attributesrequest_t));
    attributesrequest->client = src->client;
    attributesrequest->request_id = src->request_id;
    attributesrequest->context = src->context;
    attributesrequest->on_response = src->on_response;
    attributesrequest->on_timeout = src->on_timeout;
    return attributesrequest;
}

/*!< Destroys the tbmch_attributesrequest */
tbmch_err_t _tbmch_attributesrequest_destroy(tbmch_attributesrequest_t *attributesrequest)
{
    if (!attributesrequest) {
        TBMCHLOG_E("attributesrequest is NULL");
        return ESP_FAIL;
    }

    TBMC_FREE(attributesrequest);
    return ESP_OK;
}

int _tbmch_attributesrequest_get_request_id(tbmch_attributesrequest_t *attributesrequest)
{
    if (!attributesrequest) {
        TBMCHLOG_E("attributesrequest is NULL");
        return NULL;
    }
    return attributesrequest->key;
}

//(none/resend/destroy/_destroy_all_attributes)?
void _tbmch_attributesrequest_do_response(tbmch_attributesrequest_t *attributesrequest)
{
    if (!attributesrequest) {
        TBMCHLOG_E("attributesrequest is NULL");
        return; // ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, attributesrequest->key);;
    if (!value) {
        TBMCHLOG_W("value is NULL! key=%s", attributesrequest->key);
        return; // ESP_FAIL;
    }*/

    attributesrequest->on_response(attributesrequest->client, attributesrequest->context, attributesrequest->request_id); //(none/resend/destroy/_destroy_all_attributes)?
    return; // ESP_OK;
}

//(none/resend/destroy/_destroy_all_attributes)? 
void _tbmch_attributesrequest_do_timeout(tbmch_attributesrequest_t *attributesrequest)
{
    if (!attributesrequest) {
        TBMCHLOG_E("attributesrequest is NULL");
        return; // ESP_FAIL;
    }

    /*cJSON *value = cJSON_GetObjectItem(object, attributesrequest->key);;
    if (!value) {
        TBMCHLOG_W("value is NULL! key=%s", attributesrequest->key);
        return; // ESP_FAIL;
    }*/

    attributesrequest->on_timeout(attributesrequest->client, attributesrequest->context, attributesrequest->request_id); //(none/resend/destroy/_destroy_all_attributes)?
    return; // ESP_OK;
}
