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
#include "esp32/rom/crc.h"
#include <unistd.h>

#include "ota_update_observer.h"
#include "tbc_utils.h"

const static char *TAG = "ota_update";

extern tbmc_handle_t _tbmch_get_tbmc_handle(tbmch_handle_t client_);

/*!< Initialize tbmch_otaupdate_t */
tbmch_otaupdate_t *_tbmch_otaupdate_init(tbmch_handle_t client,  const char *ota_description,
                        const tbmch_otaupdate_config_t *config)
{
    if (!client) {
        TBC_LOGE("client is NULL");
        return NULL;
    }
    if (!ota_description) {
        TBC_LOGE("ota_description is NULL");
        return NULL;
    }
    if (!config) {
        TBC_LOGE("config is NULL");
        return NULL;
    }
    if (!config->on_get_current_ota_title) {
        TBC_LOGE("config->on_get_current_ota_title is NULL");
        return NULL;
    }
    if (!config->on_get_current_ota_version) {
        TBC_LOGE("config->on_get_current_ota_version is NULL");
        return NULL;
    }
    if (!config->on_ota_negotiate ) {
        TBC_LOGE("config->on_ota_negotiate is NULL");
        return NULL;
    }
    if (!config->on_ota_write) {
        TBC_LOGE("config->on_ota_write is NULL");
        return NULL;
    }
    if (!config->on_ota_end ) {
        TBC_LOGE("config->on_ota_end is NULL");
        return NULL;
    }
    if (!config->on_ota_abort ) {
        TBC_LOGE("config->on_ota_abort is NULL");
        return NULL;
    }

    tbmch_otaupdate_t *otaupdate = TBMCH_MALLOC(sizeof(tbmch_otaupdate_t));
    if (!otaupdate) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }
    memset(otaupdate, 0x00, sizeof(tbmch_otaupdate_t));
    otaupdate->client = client;
    otaupdate->ota_description = TBMCH_MALLOC(strlen(ota_description)+1);
    if (otaupdate->ota_description) {
        strcpy(otaupdate->ota_description, ota_description);
    }

    otaupdate->config.ota_type = config->ota_type;
    otaupdate->config.chunk_size = config->chunk_size;
    otaupdate->config.context = config->context;
    otaupdate->config.on_get_current_ota_title = config->on_get_current_ota_title;
    otaupdate->config.on_get_current_ota_version = config->on_get_current_ota_version;
    otaupdate->config.on_ota_negotiate = config->on_ota_negotiate;         /*!< callback of F/W or S/W OTA attributes */
    otaupdate->config.on_ota_write = config->on_ota_write;                 /*!< callback of F/W or S/W OTA doing */
    otaupdate->config.on_ota_end = config->on_ota_end;                     /*!< callback of F/W or S/W OTA success & end */
    otaupdate->config.on_ota_abort = config->on_ota_abort;                 /*!< callback of F/W or S/W OTA failure & abort */
    ////otaupdate->config.is_first_boot = config->is_first_boot;               /*!< whether first boot after ota update  */
  
    otaupdate->attribute.ota_title = NULL;
    otaupdate->attribute.ota_version = NULL;
    otaupdate->attribute.ota_size = 0;
    otaupdate->attribute.ota_checksum = NULL;
    otaupdate->attribute.ota_checksum_algorithm = NULL;

    otaupdate->state.request_id = -1;
    otaupdate->state.chunk_id = 0;
    otaupdate->state.received_len = 0;
    otaupdate->state.checksum = 0;

    // LIST_ENTRY(tbmch_otaupdate) entry;
    return otaupdate;
}
/*!< Destroys the tbmch_otaupdate_t */
tbmch_err_t _tbmch_otaupdate_destroy(tbmch_otaupdate_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL");
        return ESP_FAIL;
    }

    otaupdate->client = NULL;
    if (otaupdate->ota_description) {
        TBMCH_FREE(otaupdate->ota_description);
        otaupdate->ota_description = NULL;
    }

    otaupdate->config.ota_type = 0;
    otaupdate->config.chunk_size = 0;
    otaupdate->config.context = NULL;
    otaupdate->config.on_get_current_ota_title = NULL;
    otaupdate->config.on_get_current_ota_version = NULL;
    otaupdate->config.on_ota_negotiate = NULL;      /*!< callback of F/W or S/W OTA attributes */
    otaupdate->config.on_ota_write = NULL;          /*!< callback of F/W or S/W OTA doing */
    otaupdate->config.on_ota_end = NULL;            /*!< callback of F/W or S/W OTA success & end*/
    otaupdate->config.on_ota_abort = NULL;          /*!< callback of F/W or S/W OTA failure & abort */
    ////otaupdate->config.is_first_boot = false;
    
    if (otaupdate->attribute.ota_title) {
        TBMCH_FREE(otaupdate->attribute.ota_title);
        otaupdate->attribute.ota_title = NULL;
    }
    if (otaupdate->attribute.ota_version) {
        TBMCH_FREE(otaupdate->attribute.ota_version);
        otaupdate->attribute.ota_version = NULL;
    }
    otaupdate->attribute.ota_size = 0;
    if (otaupdate->attribute.ota_checksum) {
        TBMCH_FREE(otaupdate->attribute.ota_checksum);
        otaupdate->attribute.ota_checksum = NULL;
    }
    if (otaupdate->attribute.ota_checksum_algorithm) {
        TBMCH_FREE(otaupdate->attribute.ota_checksum_algorithm);
        otaupdate->attribute.ota_checksum_algorithm = NULL;
    }

    otaupdate->state.request_id = -1;
    otaupdate->state.chunk_id = 0;
    otaupdate->state.received_len = 0;
    otaupdate->state.checksum = 0;

    TBMCH_FREE(otaupdate);
    return ESP_OK;
}

tbmch_otaupdate_type_t _tbmch_otaupdate_get_type(tbmch_otaupdate_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL");
        return TBMCH_OTAUPDATE_TYPE_FW;
    }

    return otaupdate->config.ota_type;
}
const char *_tbmch_otaupdate_get_description(tbmch_otaupdate_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL");
        return NULL;
    }

    return otaupdate->ota_description;
}

const char *_tbmch_otaupdate_get_current_title(tbmch_otaupdate_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL");
        return NULL;
    }
    if (!otaupdate->config.on_get_current_ota_title) {
        TBC_LOGE("otaupdate->config.on_get_current_ota_title is NULL");
        return NULL;
    }

    return otaupdate->config.on_get_current_ota_title(otaupdate->client, otaupdate->config.context);
}

int _tbmch_otaupdate_get_request_id(tbmch_otaupdate_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL");
        return -1;
    }

    return otaupdate->state.request_id;
}

void _tbmch_otaupdate_reset(tbmch_otaupdate_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL");
        return;
    }
    
    if (otaupdate->attribute.ota_title) {
        TBMCH_FREE(otaupdate->attribute.ota_title);
        otaupdate->attribute.ota_title = NULL;
    }
    if (otaupdate->attribute.ota_version) {
        TBMCH_FREE(otaupdate->attribute.ota_version);
        otaupdate->attribute.ota_version = NULL;
    }
    otaupdate->attribute.ota_size = 0;
    if (otaupdate->attribute.ota_checksum) {
        TBMCH_FREE(otaupdate->attribute.ota_checksum);
        otaupdate->attribute.ota_checksum = NULL;
    }
    if (otaupdate->attribute.ota_checksum_algorithm) {
        TBMCH_FREE(otaupdate->attribute.ota_checksum_algorithm);
        otaupdate->attribute.ota_checksum_algorithm = NULL;
    }

    otaupdate->state.request_id = -1;
    otaupdate->state.chunk_id = 0;
    otaupdate->state.received_len = 0;
    otaupdate->state.checksum = 0;
}

// return 0 on success, -1 on failure
tbmch_err_t _tbmch_otaupdate_request_chunk(tbmch_otaupdate_t *otaupdate,
                                            tbmc_on_otaupdate_response_t on_otaupdate_response,
                                            tbmc_on_otaupdate_timeout_t on_otaupdate_timeout)
{
    char payload[20] = {0};
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        return -1;
    }

    tbmc_handle_t tbmc_handle = _tbmch_get_tbmc_handle(otaupdate->client);
    int chunk_size;
    if (otaupdate->attribute.ota_size < otaupdate->config.chunk_size) {
        chunk_size = 0; // full f/w or s/w
    } else {
        chunk_size = otaupdate->config.chunk_size;
    }
    sprintf(payload, "%d", chunk_size);
    int request_id = tbmc_otaupdate_request(tbmc_handle, otaupdate->state.request_id/*default -1*/, otaupdate->state.chunk_id/*default 0*/,
                          payload, //chunk_size
                          otaupdate->client, //tbmch_handle
                          on_otaupdate_response,
                          on_otaupdate_timeout,
                          1/*qos*/, 0/*retain*/);
    // First OTA request
    if ((otaupdate->state.request_id<0) && (request_id>0)) {
         otaupdate->state.request_id = request_id;
    }

    if (request_id<0){
        TBC_LOGW("Request OTA chunk(%d) failure! request_id=%d %s()", otaupdate->state.chunk_id, request_id, __FUNCTION__);
    }

    return (request_id<0)?-1:0;
}


bool _tbmch_otaupdate_is_received_all(tbmch_otaupdate_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        return false;
    }

    if (otaupdate->state.received_len < otaupdate->attribute.ota_size) {
        return false; //go on
    } else {
        return true; //received all
    }
}

bool _tbmch_otaupdate_checksum_verification(tbmch_otaupdate_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        return false;
    }

    // TODO: support multi-ALG! 

    //received all f/w or s/w
    char checksum_str[20] = {0};
    uint32_t checksum_int = otaupdate->state.checksum;
    uint32_t a, b, c, d;
    a = (checksum_int & 0xFF000000) >> 24;
    b = (checksum_int & 0x00FF0000) >> 16;
    c = (checksum_int & 0x0000FF00) >> 8;
    d = (checksum_int & 0x000000FF) >> 0;
    checksum_int = (d<<24) | (c<<16) | (b<<8) | a;
    sprintf(checksum_str, "%x", checksum_int);

    uint32_t ota_checksum = 0;
    if (otaupdate->attribute.ota_checksum) {
        sscanf(otaupdate->attribute.ota_checksum, "%x", &ota_checksum);
    }
    
    // CRC32 verify checksum
    //if (strcmp(checksum, otaupdate->attribute.ota_checksum)==0) {
    if (checksum_int == ota_checksum) {
        return true;
    } else {
        TBC_LOGW("checksum(%s, %#x) is NOT equal to otaupdate->attribute.ota_checksum(%s, %#x)!", 
            checksum_str, checksum_int, 
            otaupdate->attribute.ota_checksum, ota_checksum);
        return false;
    }
}

//{"current_fw_title": "myFirmware", "current_fw_version": "1.2.3"}
void _tbmch_otaupdate_publish_early_current_version(tbmch_otaupdate_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        return;
    }

    const char *current_ota_title_key = NULL;
    const char *current_ota_version_key = NULL;
    const char *current_ota_title_value = NULL;
    const char *current_ota_version_value = NULL;
    if (otaupdate->config.ota_type == TBMCH_OTAUPDATE_TYPE_FW) {
        current_ota_title_key   = TB_MQTT_KEY_CURRENT_FW_TITLE;   //"current_fw_title"
        current_ota_version_key = TB_MQTT_KEY_CURRENT_FW_VERSION; //"current_fw_version"
    } else {
        current_ota_title_key   = TB_MQTT_KEY_CURRENT_SW_TITLE;   //"current_sw_title"
        current_ota_version_key = TB_MQTT_KEY_CURRENT_SW_VERSION; //"current_sw_version"
    }
    current_ota_title_value = otaupdate->config.on_get_current_ota_title(otaupdate->client, otaupdate->config.context);
    current_ota_version_value = otaupdate->config.on_get_current_ota_version(otaupdate->client, otaupdate->config.context);

    // send package...    
    cJSON *object = cJSON_CreateObject(); // create json object
    cJSON_AddStringToObject(object, current_ota_title_key, current_ota_title_value);
    cJSON_AddStringToObject(object, current_ota_version_key, current_ota_version_value);
    char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
    tbmc_handle_t tbmc_handle = _tbmch_get_tbmc_handle(otaupdate->client);
    /*int result =*/ tbmc_telemetry_publish(tbmc_handle, pack, 1/*qos*/, 0/*retain*/);
    cJSON_free(pack); // free memory
    cJSON_Delete(object); // delete json object
}

//{"fw_state": "FAILED", "fw_error":  "the human readable message about the cause of the error"}
//{"sw_state": "FAILED", "sw_error":  "the human readable message about the cause of the error"}
//tbmc_handle_t tbmc_handle = _tbmch_get_tbmc_handle(otaupdate->client);
//_tbmch_otaupdate_publish_fail_status(tbmc_handle, TBMCH_OTAUPDATE_TYPE_FW, "Device's code is error!")
void _tbmch_otaupdate_publish_early_failed_status(tbmc_handle_t tbmc_handle, 
                                tbmch_otaupdate_type_t ota_type, const char *ota_error)
{
    if (!tbmc_handle) {
        TBC_LOGE("tbmc_handle is NULL!");
        return;
    }

    const char *ota_state_key = NULL;
    const char *ota_error_key = NULL;
    if (ota_type == TBMCH_OTAUPDATE_TYPE_FW) {
        ota_state_key           = TB_MQTT_KEY_FW_STATE;           //"fw_state"
        ota_error_key           = TB_MQTT_KEY_FW_ERROR;           //"fw_error"  
    } else {
        ota_state_key           = TB_MQTT_KEY_SW_STATE;           //"sw_state"
        ota_error_key           = TB_MQTT_KEY_SW_ERROR;           //"sw_error"  
    }

    // send package...    
    cJSON *object = cJSON_CreateObject(); // create json object
    cJSON_AddStringToObject(object, ota_state_key, TB_MQTT_VALUE_FW_SW_STATE_FAILED);
    if (ota_error) {
        cJSON_AddStringToObject(object, ota_error_key, ota_error);
    }
    char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
    /*int result =*/ tbmc_telemetry_publish(tbmc_handle, pack, 1/*qos*/, 0/*retain*/);
    cJSON_free(pack); // free memory
    cJSON_Delete(object); // delete json object
}

//{"fw_state": "FAILED", "fw_error":  "the human readable message about the cause of the error"}
//{"sw_state": "FAILED", "sw_error":  "the human readable message about the cause of the error"}
//_tbmch_otaupdate_publish_fail_status(tbmc_handle, TBMCH_OTAUPDATE_TYPE_FW, "Device's code is error!")
void _tbmch_otaupdate_publish_late_failed_status(tbmch_otaupdate_t *otaupdate, const char *ota_error)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        return;
    }

    const char *ota_state_key = NULL;
    const char *ota_error_key = NULL;
    if (otaupdate->config.ota_type == TBMCH_OTAUPDATE_TYPE_FW) {
        ota_state_key           = TB_MQTT_KEY_FW_STATE;           //"fw_state"
        ota_error_key           = TB_MQTT_KEY_FW_ERROR;           //"fw_error"  
    } else {
        ota_state_key           = TB_MQTT_KEY_SW_STATE;           //"sw_state"
        ota_error_key           = TB_MQTT_KEY_SW_ERROR;           //"sw_error"  
    }

    // send package...    
    cJSON *object = cJSON_CreateObject(); // create json object
    cJSON_AddStringToObject(object, ota_state_key, TB_MQTT_VALUE_FW_SW_STATE_FAILED);
    if (ota_error) {
        cJSON_AddStringToObject(object, ota_error_key, ota_error);
    }
    char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
    tbmc_handle_t tbmc_handle = _tbmch_get_tbmc_handle(otaupdate->client);
    /*int result =*/ tbmc_telemetry_publish(tbmc_handle, pack, 1/*qos*/, 0/*retain*/);
    cJSON_free(pack); // free memory
    cJSON_Delete(object); // delete json object
}

//{"current_fw_title": "myFirmware", "current_fw_version": "1.2.3", "fw_state": TB_MQTT_VALUE_FW_SW_STATE_DOWNLOADING}
//{"current_fw_title": "myFirmware", "current_fw_version": "1.2.3", "fw_state": TB_MQTT_VALUE_FW_SW_STATE_DOWNLOADED}
//{"current_fw_title": "myFirmware", "current_fw_version": "1.2.3", "fw_state": TB_MQTT_VALUE_FW_SW_STATE_VERIFIED}
//{"current_fw_title": "myFirmware", "current_fw_version": "1.2.3", "fw_state": TB_MQTT_VALUE_FW_SW_STATE_UPDATING}
void _tbmch_otaupdate_publish_going_status(tbmch_otaupdate_t *otaupdate, const char *ota_state)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        return;
    }
    if (!ota_state) {
        TBC_LOGE("ota_state is NULL!");
        return;
    }

    const char *current_ota_title_key = NULL;
    const char *current_ota_version_key = NULL;
    const char *ota_state_key = NULL;
    const char *current_ota_title_value = NULL;
    const char *current_ota_version_value = NULL;
    if (otaupdate->config.ota_type == TBMCH_OTAUPDATE_TYPE_FW) {
        current_ota_title_key   = TB_MQTT_KEY_CURRENT_FW_TITLE;   //"current_fw_title"
        current_ota_version_key = TB_MQTT_KEY_CURRENT_FW_VERSION; //"current_fw_version"
        ota_state_key           = TB_MQTT_KEY_FW_STATE;           //"fw_state"  
    } else {
        current_ota_title_key   = TB_MQTT_KEY_CURRENT_SW_TITLE;   //"current_sw_title"
        current_ota_version_key = TB_MQTT_KEY_CURRENT_SW_VERSION; //"current_sw_version"
        ota_state_key           = TB_MQTT_KEY_SW_STATE;           //"sw_state"  
    }
    current_ota_title_value = otaupdate->config.on_get_current_ota_title(otaupdate->client, otaupdate->config.context);
    current_ota_version_value = otaupdate->config.on_get_current_ota_version(otaupdate->client, otaupdate->config.context);

    // send package...    
    cJSON *object = cJSON_CreateObject(); // create json object
    cJSON_AddStringToObject(object, current_ota_title_key, current_ota_title_value);
    cJSON_AddStringToObject(object, current_ota_version_key, current_ota_version_value);
    cJSON_AddStringToObject(object, ota_state_key, ota_state);
    char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
    tbmc_handle_t tbmc_handle = _tbmch_get_tbmc_handle(otaupdate->client);
    /*int result =*/ tbmc_telemetry_publish(tbmc_handle, pack, 1/*qos*/, 0/*retain*/);
    cJSON_free(pack); // free memory
    cJSON_Delete(object); // delete json object
}

//{"current_fw_title": "myNewFirmware", "current_fw_version": "1.5.2_new_version", "fw_state": "UPDATED"}
//{"current_sw_title": "myNewSoftware", "current_sw_version": "1.5.2_new_version", "sw_state": "UPDATED"}
void _tbmch_otaupdate_publish_updated_status(tbmch_otaupdate_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        return;
    }

    /*if (!otaupdate->config.is_first_boot) {
        TBC_LOGI("otaupdate is NOT first boot! ota update type=%d(0:F/W, 1:S/W)",TBMCH_OTAUPDATE_TYPE_FW);
        return;
    }*/

    const char *current_ota_title_key = NULL;
    const char *current_ota_version_key = NULL;
    const char *ota_state_key = NULL;
    if (otaupdate->config.ota_type == TBMCH_OTAUPDATE_TYPE_FW) {
        current_ota_title_key   = TB_MQTT_KEY_CURRENT_FW_TITLE;   //"current_fw_title"
        current_ota_version_key = TB_MQTT_KEY_CURRENT_FW_VERSION; //"current_fw_version"
        ota_state_key           = TB_MQTT_KEY_FW_STATE;           //"fw_state"
    } else {
        current_ota_title_key   = TB_MQTT_KEY_CURRENT_SW_TITLE;   //"current_sw_title"
        current_ota_version_key = TB_MQTT_KEY_CURRENT_SW_VERSION; //"current_sw_version"
        ota_state_key           = TB_MQTT_KEY_SW_STATE;           //"sw_state"
    }

    // send package...    
    cJSON *object = cJSON_CreateObject(); // create json object
    cJSON_AddStringToObject(object, current_ota_title_key, otaupdate->attribute.ota_title);
    cJSON_AddStringToObject(object, current_ota_version_key, otaupdate->attribute.ota_version);
    cJSON_AddStringToObject(object, ota_state_key, TB_MQTT_VALUE_FW_SW_STATE_UPDATED);
    char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
    tbmc_handle_t tbmc_handle = _tbmch_get_tbmc_handle(otaupdate->client);
    /*int result =*/ tbmc_telemetry_publish(tbmc_handle, pack, 1/*qos*/, 0/*retain*/);
    cJSON_free(pack); // free memory
    cJSON_Delete(object); // delete json object

    usleep(500000);
}

//return 1 on negotiate successful(next to F/W OTA), -1/ESP_FAIL on negotiate failure, 0/ESP_OK on already updated!
tbmch_err_t _tbmch_otaupdate_do_negotiate(tbmch_otaupdate_t *otaupdate,
                                        const char *ota_title, const char *ota_version, int ota_size,
                                        const char *ota_checksum, const char *ota_checksum_algorithm,
                                        char *ota_error, int error_size)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL");
        strncpy(ota_error, "Device's code is error!", error_size);
        return -1;
    }

    // TODO: support multi-ALG! 
    // only support CRC32
    if (strcasecmp(ota_checksum_algorithm, TB_MQTTT_VALUE_FW_SW_CHECKSUM_ALG_CRC32)!=0) { // Not CRC32
        TBC_LOGW("Only support CRC32 for ota_update! Don't support %s! %s()", ota_checksum_algorithm, __FUNCTION__);
        strncpy(ota_error, "Only support CRC checksum!", error_size);
        return -1;
    }

    _tbmch_otaupdate_reset(otaupdate);

    // TODO: if ota_title & ota_version == current_title & current_version, then return 0!
    
    int result = otaupdate->config.on_ota_negotiate(otaupdate->client, otaupdate->config.context,
                                             ota_title, ota_version, ota_size, ota_checksum, ota_checksum_algorithm,
                                             ota_error, error_size);
    if (result==1) { // negotiate successful(next to F/W OTA)
        // cache ota_title
        otaupdate->attribute.ota_title = TBMCH_MALLOC(strlen(ota_title)+1);
        if (otaupdate->attribute.ota_title) {
            strcpy(otaupdate->attribute.ota_title, ota_title);
        }
        // cache ota_version
        otaupdate->attribute.ota_version = TBMCH_MALLOC(strlen(ota_version)+1);
        if (otaupdate->attribute.ota_version) {
            strcpy(otaupdate->attribute.ota_version, ota_version);
        }
        otaupdate->attribute.ota_size = ota_size;
        // cache ota_checksum
        otaupdate->attribute.ota_checksum = TBMCH_MALLOC(strlen(ota_checksum)+1);
        if (otaupdate->attribute.ota_checksum) {
            strcpy(otaupdate->attribute.ota_checksum, ota_checksum);
        }
        // cache ota_checksum_algorithm
        otaupdate->attribute.ota_checksum_algorithm = TBMCH_MALLOC(strlen(ota_checksum_algorithm)+1);
        if (otaupdate->attribute.ota_checksum_algorithm) {
            strcpy(otaupdate->attribute.ota_checksum_algorithm, ota_checksum_algorithm);
        }
    }

    return result;
}

//return 0/ESP_OK on successful, -1/ESP_FAIL on failure
tbmch_err_t _tbmch_otaupdate_do_write(tbmch_otaupdate_t *otaupdate, int chunk_id, 
                                            const void *ota_data, int data_size,
                                            char *ota_error, int error_size)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        strncpy(ota_error, "Device's code is error!", error_size);
        return -1; //code error
    }
    if (chunk_id != otaupdate->state.chunk_id) {
        TBC_LOGE("chunk_id(%d) is not equal to otaupdate->state.chunk_id(%d)!", chunk_id, otaupdate->state.chunk_id);
        strncpy(ota_error, "Chunk ID is error!", error_size);
        return -1; //chunk_id error
    }
    if (!ota_data || !data_size) {
        TBC_LOGE("ota_data(%p) or data_size(%d) is error!", ota_data, data_size);
        strncpy(ota_error, "OTA data is empty!", error_size);
        return -1; //ota_data is empty
    }

    tbmch_err_t result = true;
    result = otaupdate->config.on_ota_write(otaupdate->client, otaupdate->config.context,
                            otaupdate->state.request_id, chunk_id, ota_data, data_size,
                            ota_error, error_size);
    if (result != ESP_OK) {
        TBC_LOGW("fail to call on_ota_write()!");
        return -1; //payload error & end
    }
    
    otaupdate->state.chunk_id++; //next chunk id
    otaupdate->state.received_len += data_size;
    // TODO: support multi-ALG! 
    otaupdate->state.checksum = crc32_le(otaupdate->state.checksum, (uint8_t const*)ota_data, (uint32_t)data_size); //crc32_be(...)
    return 0;
}

//return 0 on successful, -1 on failure
tbmch_err_t _tbmch_otaupdate_do_end(tbmch_otaupdate_t *otaupdate, char *ota_error, int error_size)
{
    int ret = 0;
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        return -1;
    }

    // on_ota_end() and reset()
    if (otaupdate->config.on_ota_end && otaupdate->state.request_id>0 && otaupdate->state.received_len>0) {
        ret = otaupdate->config.on_ota_end(otaupdate->client, otaupdate->config.context, 
                                         otaupdate->state.request_id, otaupdate->state.chunk_id,
                                         ota_error, error_size);
    }
    return (ret==0)?0:-1;
}

void _tbmch_otaupdate_do_abort(tbmch_otaupdate_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL");
        return;
    }

    // on_ota_abort() and reset()
    if (otaupdate->config.on_ota_abort && otaupdate->state.request_id>0 && otaupdate->state.received_len>0) {
        otaupdate->config.on_ota_abort(otaupdate->client, otaupdate->config.context, 
                                         otaupdate->state.request_id, otaupdate->state.chunk_id/*current chunk_id*/);
    }
}

#if 0
// TODO:
/* * On connected:
   1. Subscribe to `v1/devices/me/attributes/response/+`
   1. Subscribe to `v1/devices/me/attributes`
   1. Subscribe to `v2/fw/response/+`
   1. Send telemetry: *current firmware info*
      * Topic: `v1/devices/me/telemetry`
      * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0"}`

      Replace `Initial` and `v0` with your F/W title and version.

   1. Send attributes request: *request firmware info*
      * Topic: `v1/devices/me/attributes/request/{request_id}`
      * Payload: `{"sharedKeys": "fw_checksum,fw_checksum_algorithm,fw_size,fw_title,fw_version"}`
*/

/*static*/ void _tbmch_otaupdate_publish_current_status(tbmch_handle_t client_)
{
    // TODO:
    //    1. Send telemetry: *current firmware info*
    //       * Topic: `v1/devices/me/telemetry`
    //       * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0"}`

    //    1. Send telemetry: *current firmware info*
    //       * Topic: `v1/devices/me/telemetry`
    //       * Payload: `{"current_sw_title":"Initial","current_sw_version":"v0"}`
}

/*static*/ void __tbmch_otaupdate_attributesrequest_on_response(tbmch_handle_t client, 
                    void *context, int request_id)
{
    // no code!
}
/*static*/ void __tbmch_otaupdate_attributesrequest_on_timeout(tbmch_handle_t client,
                    void *context, int request_id)
{
    // TODO: resend ???
}
/*static*/ void _tbmch_otaupdate_send_attributes_request(tbmch_handle_t client_)
{
    // TODO:

    // tbmch_err_t tbmch_sharedattribute_append(tbmch_handle_t client_, const char *key, void *context,
    //                                     tbmch_sharedattribute_on_set_t on_set);
    // key: fw_checksum,fw_checksum_algorithm,fw_size,fw_title,fw_version
    //      sw_checksum,sw_checksum_algorithm,sw_size,sw_title,sw_version

    //int tbmch_attributesrequest_send(client_,
    //                                 NULL,
    //                                 __tbmch_otaupdate_attributesrequest_on_response,
    //                                 __tbmch_otaupdate_attributesrequest_on_timeout,
    //                                int count, /*const char *key,*/...)
    //    Send attributes request: *request firmware info*
    //      * Topic: `v1/devices/me/attributes/request/{request_id}`
    //      * Payload: `{"sharedKeys": "fw_checksum,fw_checksum_algorithm,fw_size,fw_title,fw_version"}`

    //      * Topic: `v1/devices/me/attributes/request/{request_id}`
    //      * Payload: `{"sharedKeys": "sw_checksum,sw_checksum_algorithm,sw_size,sw_title,sw_version"}`

}
#endif

