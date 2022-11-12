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

//#include "cJSON.h"
//#include "tbc_mqtt.h"

#include "tbc_utils.h"
//#include "provision_observer.h"
#include "tbc_mqtt_helper_internal.h"

// TODO: remove it!
extern void _tbcmh_on_provision_response(void *context, int request_id, const char* payload, int length);
extern void _tbcmh_on_provision_timeout(void *context, int request_id);

const static char *TAG = "provision";

/*!< Initialize tbcmh_provision_t */
static tbcmh_provision_t *_tbcmh_provision_init(tbcmh_handle_t client, int request_id,
                                         const tbcmh_provision_params_t *params,
                                         void *context,
                                         tbcmh_provision_on_response_t on_response,
                                         tbcmh_provision_on_timeout_t on_timeout)
{
    if (!on_response) {
        TBC_LOGE("on_response is NULL");
        return NULL;
    }
    
    tbcmh_provision_t *provision = TBC_MALLOC(sizeof(tbcmh_provision_t));
    if (!provision) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(provision, 0x00, sizeof(tbcmh_provision_t));
    provision->client = client;
    provision->params = cJSON_Duplicate(params, true);
    provision->request_id = request_id;
    provision->context = context;
    provision->on_response = on_response;
    provision->on_timeout = on_timeout;
    return provision;
}

static tbcmh_provision_t *_tbcmh_provision_clone_wo_listentry(tbcmh_provision_t *src)
{
    if (!src) {
        TBC_LOGE("src is NULL");
        return NULL;
    }
    
    tbcmh_provision_t *provision = TBC_MALLOC(sizeof(tbcmh_provision_t));
    if (!provision) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(provision, 0x00, sizeof(tbcmh_provision_t));
    provision->client = src->client;
    provision->params = cJSON_Duplicate(src->params, true);
    provision->request_id = src->request_id;
    provision->context = src->context;
    provision->on_response = src->on_response;
    provision->on_timeout = src->on_timeout;
    return provision;
}

static int _tbcmh_provision_get_request_id(tbcmh_provision_t *provision)
{
    if (!provision) {
        TBC_LOGE("provision is NULL");
        return -1;
    }

    return provision->request_id;
}

/*!< Destroys the tbcmh_provision_t */
static tbc_err_t _tbcmh_provision_destroy(tbcmh_provision_t *provision)
{
    if (!provision) {
        TBC_LOGE("provision is NULL");
        return ESP_FAIL;
    }

    cJSON_Delete(provision->params);
    provision->params = NULL;
    TBC_FREE(provision);
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

static int _parse_provision_response(const tbcmh_provision_results_t *results,
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

static void _tbcmh_provision_do_response(tbcmh_provision_t *provision, const tbcmh_provision_results_t *results)
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

static void _tbcmh_provision_do_timeout(tbcmh_provision_t *provision)
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

//====50.Device provisioning=======================================================================
/*static*/ tbc_err_t _tbcmh_provision_empty(tbcmh_handle_t client_)
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

     // remove all item in provision_list
     tbcmh_provision_t *provision = NULL, *next;
     LIST_FOREACH_SAFE(provision, &client->provision_list, entry, next) {
          // exec timeout callback
          _tbcmh_provision_do_timeout(provision);

          // remove from provision list and destory
          LIST_REMOVE(provision, entry);
          _tbcmh_provision_destroy(provision);
     }
     memset(&client->provision_list, 0x00, sizeof(client->provision_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

// return ESP_OK on successful, ESP_FAIL on failure
static int _params_of_credentials_generated_by_server(tbcmh_provision_params_t *params, const tbc_provison_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    // TBC_CHECK_PTR_WITH_RETURN_VALUE(config->deviceName, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceKey, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceSecret, ESP_FAIL);

    if (config->deviceName) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_DEVICE_NAME, config->deviceName);
    }
    if (config->provisionDeviceKey) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY, config->provisionDeviceKey);
    }
    if (config->provisionDeviceSecret) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET, config->provisionDeviceSecret);
    }
    return ESP_OK;
}

// return ESP_OK on successful, ESP_FAIL on failure
static int _params_of_devices_supplies_access_token(tbcmh_provision_params_t *params, const tbc_provison_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    // TBC_CHECK_PTR_WITH_RETURN_VALUE(config->deviceName, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceKey, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceSecret, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->token, ESP_FAIL);

    if (config->deviceName) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_DEVICE_NAME, config->deviceName);
    }
    if (config->provisionDeviceKey) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY, config->provisionDeviceKey);
    }
    if (config->provisionDeviceSecret) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET, config->provisionDeviceSecret);
    }
    cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE, TB_MQTT_VALUE_PROVISION_ACCESS_TOKEN); //Credentials type parameter.
    if (config->token) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_TOKEN, config->token);
    }
    return ESP_OK;
}

// return ESP_OK on successful, ESP_FAIL on failure
static int _params_of_devices_supplies_basic_mqtt_credentials(tbcmh_provision_params_t *params, const tbc_provison_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    // TBC_CHECK_PTR_WITH_RETURN_VALUE(config->deviceName, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceKey, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceSecret, ESP_FAIL);
    if (!config->clientId && !config->username) {
         TBC_LOGE("config->clientId and config->username are NULL! %s()", __FUNCTION__);
         return ESP_FAIL;
    }

    if (config->deviceName) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_DEVICE_NAME, config->deviceName);
    }
    if (config->provisionDeviceKey) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY, config->provisionDeviceKey);
    }
    if (config->provisionDeviceSecret) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET, config->provisionDeviceSecret);
    }
    cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE, TB_MQTT_VALUE_PROVISION_MQTT_BASIC); //Credentials type parameter.
    if (config->clientId) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_CLIENT_ID, config->clientId);
    }
    if (config->username) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_USERNAME, config->username);
    }
    if (config->password) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PASSWORD, config->password);
    }
    return ESP_OK;
}

// hash - Public key X509 hash for device in ThingsBoard.
// return ESP_OK on successful, ESP_FAIL on failure
static int _params_of_devices_supplies_x509_certificate(tbcmh_provision_params_t *params, const tbc_provison_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    // TBC_CHECK_PTR_WITH_RETURN_VALUE(config->deviceName, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceKey, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceSecret, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->hash, ESP_FAIL);

    if (config->deviceName) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_DEVICE_NAME, config->deviceName);
    }
    if (config->provisionDeviceKey) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY, config->provisionDeviceKey);
    }
    if (config->provisionDeviceSecret) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET, config->provisionDeviceSecret);
    }
    cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE, TB_MQTT_VALUE_PROVISION_X509_CERTIFICATE); //Credentials type parameter.
    if (config->hash) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_HASH, config->hash);  ////Public key X509 hash for device in ThingsBoard.
    }
    return ESP_OK;
}

static int _tbcmh_provision_request(tbcmh_handle_t client_,
                                const tbcmh_provision_params_t *params,
                                void *context,
                                tbcmh_provision_on_response_t on_response,
                                tbcmh_provision_on_timeout_t on_timeout)
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (!params) {
          TBC_LOGE("params is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Send msg to server
     //cJSON *object = cJSON_CreateObject(); // create json object
     //cJSON_AddStringToObject(object, TB_MQTT_TEXT_PROVISION_METHOD, method);
     //if (params)
     //     cJSON_AddItemReferenceToObject(object, TB_MQTT_TEXT_PROVISION_PARAMS, params);
     //else 
     //     cJSON_AddNullToObject(object, TB_MQTT_TEXT_PROVISION_PARAMS);
     //char *params_str = cJSON_PrintUnformatted(object); //cJSON_Print(object);
     int request_id;
     char *params_str = cJSON_PrintUnformatted(params); //cJSON_Print(object);
     request_id = tbcm_provision_request(client->tbmqttclient, params_str,
                              client,
                              _tbcmh_on_provision_response,
                              _tbcmh_on_provision_timeout,
                               1/*qos*/, 0/*retain*/);
     cJSON_free(params_str); // free memory
     //cJSON_Delete(object); // delete json object
     if (request_id<0) {
          TBC_LOGE("Init tbcm_provision_request failure! %s()", __FUNCTION__);
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Create provision
     tbcmh_provision_t *provision = _tbcmh_provision_init(client, request_id, params, context, on_response, on_timeout);
     if (!provision) {
          TBC_LOGE("Init provision failure! %s()", __FUNCTION__);
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Insert provision to list
     tbcmh_provision_t *it, *last = NULL;
     if (LIST_FIRST(&client->provision_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->provision_list, provision, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->provision_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, provision, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return request_id;
}

// return request_id or ESP_FAIL
int tbcmh_provision_request(tbcmh_handle_t client_,
                                    const tbc_provison_config_t *config,
                                    void *context,
                                    tbcmh_provision_on_response_t on_response,
                                    tbcmh_provision_on_timeout_t on_timeout)
{
    //TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client_, ESP_FAIL);

    tbcmh_provision_params_t *params = cJSON_CreateObject();
    if (!params) {
         TBC_LOGE("create params is error(NULL)!");
         return ESP_FAIL;
    }
    int ret = ESP_FAIL;
    if (config->provisionType == TBC_PROVISION_TYPE_SERVER_GENERATES_CREDENTIALS) { // Credentials generated by the ThingsBoard server
         ret = _params_of_credentials_generated_by_server(params, config);
    } else if (config->provisionType == TBC_PROVISION_TYPE_DEVICE_SUPPLIES_ACCESS_TOKEN) { // Devices supplies Access Token
         ret = _params_of_devices_supplies_access_token(params, config);
    } else if (config->provisionType == TBC_PROVISION_TYPE_DEVICE_SUPPLIES_BASIC_MQTT_CREDENTIALS) { // Devices supplies Basic MQTT Credentials
         ret = _params_of_devices_supplies_basic_mqtt_credentials(params, config);
    } else if (config->provisionType == TBC_PROVISION_TYPE_DEVICE_SUPPLIES_X509_CREDENTIALS) { // Devices supplies X.509 Certificate)
         ret = _params_of_devices_supplies_x509_certificate(params, config);
    } else {
         TBC_LOGE("config->provisionType(%d) is error!", config->provisionType);
         ret = ESP_FAIL;
    }

    if (ret == ESP_OK) {
         // request_id
         ret = _tbcmh_provision_request(client_, params, context,
                                       on_response, on_timeout);
    } else {
         // TBC_LOGE("ret is error!", ret);
         ret = ESP_FAIL;
    }

    cJSON_Delete(params); // delete json object     
    return ret;
}

/*static*/ void _tbcmh_provision_on_response(tbcmh_handle_t client_, int request_id, const cJSON *object)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !object) {
          TBC_LOGE("client or object is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search provision
     tbcmh_provision_t *provision = NULL;
     LIST_FOREACH(provision, &client->provision_list, entry) {
          if (provision && (_tbcmh_provision_get_request_id(provision)==request_id)) {
               break;
          }
     }
     if (!provision) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGW("Unable to find provision:%d! %s()", request_id, __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Cache and remove provision
     tbcmh_provision_t *cache = _tbcmh_provision_clone_wo_listentry(provision);
     LIST_REMOVE(provision, entry);
     _tbcmh_provision_destroy(provision);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do response
     _tbcmh_provision_do_response(cache, object);
     // Free cache
     _tbcmh_provision_destroy(cache);

     return;// ESP_OK;
}

/*static*/ void _tbcmh_provision_on_timeout(tbcmh_handle_t client_, int request_id)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search provision
     tbcmh_provision_t *provision = NULL;
     LIST_FOREACH(provision, &client->provision_list, entry) {
          if (provision && (_tbcmh_provision_get_request_id(provision)==request_id)) {
               break;
          }
     }
     if (!provision) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGW("Unable to find provision:%d! %s()", request_id, __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Cache and remove provision
     tbcmh_provision_t *cache = _tbcmh_provision_clone_wo_listentry(provision);
     LIST_REMOVE(provision, entry);
     _tbcmh_provision_destroy(provision);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do timeout
     _tbcmh_provision_do_timeout(cache);
     // Free provision
     _tbcmh_provision_destroy(cache);

     return;// ESP_OK;
}
