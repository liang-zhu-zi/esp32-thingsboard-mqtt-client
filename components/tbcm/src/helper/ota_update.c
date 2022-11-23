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
#include <unistd.h>

#include "esp32/rom/crc.h"
#include "esp_err.h"

#include "tbc_utils.h"

//#include "ota_update.h"
#include "tbc_mqtt_helper_internal.h"

const static char *TAG = "ota_update";

/*!< Initialize ota_update_t */
static ota_update_t *_ota_update_create(tbcmh_handle_t client,
                        const char *ota_description,
                        const tbcmh_otaupdate_config_t *config)
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

    ota_update_t *otaupdate = TBC_MALLOC(sizeof(ota_update_t));
    if (!otaupdate) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }
    memset(otaupdate, 0x00, sizeof(ota_update_t));
    otaupdate->client = client;
    otaupdate->ota_description = TBC_MALLOC(strlen(ota_description)+1);
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

    // LIST_ENTRY(ota_update) entry;
    return otaupdate;
}

/*!< Destroys the ota_update_t */
static tbc_err_t _ota_update_destroy(ota_update_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL");
        return ESP_FAIL;
    }

    otaupdate->client = NULL;
    if (otaupdate->ota_description) {
        TBC_FREE(otaupdate->ota_description);
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
        TBC_FREE(otaupdate->attribute.ota_title);
        otaupdate->attribute.ota_title = NULL;
    }
    if (otaupdate->attribute.ota_version) {
        TBC_FREE(otaupdate->attribute.ota_version);
        otaupdate->attribute.ota_version = NULL;
    }
    otaupdate->attribute.ota_size = 0;
    if (otaupdate->attribute.ota_checksum) {
        TBC_FREE(otaupdate->attribute.ota_checksum);
        otaupdate->attribute.ota_checksum = NULL;
    }
    if (otaupdate->attribute.ota_checksum_algorithm) {
        TBC_FREE(otaupdate->attribute.ota_checksum_algorithm);
        otaupdate->attribute.ota_checksum_algorithm = NULL;
    }

    otaupdate->state.request_id = -1;
    otaupdate->state.chunk_id = 0;
    otaupdate->state.received_len = 0;
    otaupdate->state.checksum = 0;

    TBC_FREE(otaupdate);
    return ESP_OK;
}

static const char *_ota_update_get_current_title(ota_update_t *otaupdate)
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

static void _ota_update_reset(ota_update_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL");
        return;
    }
    
    if (otaupdate->attribute.ota_title) {
        TBC_FREE(otaupdate->attribute.ota_title);
        otaupdate->attribute.ota_title = NULL;
    }
    if (otaupdate->attribute.ota_version) {
        TBC_FREE(otaupdate->attribute.ota_version);
        otaupdate->attribute.ota_version = NULL;
    }
    otaupdate->attribute.ota_size = 0;
    if (otaupdate->attribute.ota_checksum) {
        TBC_FREE(otaupdate->attribute.ota_checksum);
        otaupdate->attribute.ota_checksum = NULL;
    }
    if (otaupdate->attribute.ota_checksum_algorithm) {
        TBC_FREE(otaupdate->attribute.ota_checksum_algorithm);
        otaupdate->attribute.ota_checksum_algorithm = NULL;
    }

    otaupdate->state.request_id = -1;
    otaupdate->state.chunk_id = 0;
    otaupdate->state.received_len = 0;
    otaupdate->state.checksum = 0;
}

static bool _ota_update_checksum_verification(ota_update_t *otaupdate)
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
static void _ota_update_publish_early_current_version(ota_update_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        return;
    }

    const char *current_ota_title_key = NULL;
    const char *current_ota_version_key = NULL;
    const char *current_ota_title_value = NULL;
    const char *current_ota_version_value = NULL;
    if (otaupdate->config.ota_type == TBCMH_OTAUPDATE_TYPE_FW) {
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
    tbcm_handle_t tbcm_handle = otaupdate->client->tbmqttclient;
    /*int result =*/ tbcm_telemetry_publish(tbcm_handle, pack, 1/*qos*/, 0/*retain*/);
    cJSON_free(pack); // free memory
    cJSON_Delete(object); // delete json object
}

//{"fw_state": "FAILED", "fw_error":  "the human readable message about the cause of the error"}
//{"sw_state": "FAILED", "sw_error":  "the human readable message about the cause of the error"}
//tbcm_handle_t tbcm_handle = otaupdate->client->tbmqttclient;
//_tbcmh_otaupdate_publish_fail_status(tbcm_handle, TBCMH_OTAUPDATE_TYPE_FW, "Device's code is error!")
static void _ota_update_publish_early_failed_status(tbcm_handle_t tbcm_handle, 
                                tbcmh_otaupdate_type_t ota_type, const char *ota_error)
{
    if (!tbcm_handle) {
        TBC_LOGE("tbcm_handle is NULL!");
        return;
    }

    const char *ota_state_key = NULL;
    const char *ota_error_key = NULL;
    if (ota_type == TBCMH_OTAUPDATE_TYPE_FW) {
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
    /*int result =*/ tbcm_telemetry_publish(tbcm_handle, pack, 1/*qos*/, 0/*retain*/);
    cJSON_free(pack); // free memory
    cJSON_Delete(object); // delete json object
}

//{"fw_state": "FAILED", "fw_error":  "the human readable message about the cause of the error"}
//{"sw_state": "FAILED", "sw_error":  "the human readable message about the cause of the error"}
//_tbcmh_otaupdate_publish_fail_status(tbcm_handle, TBCMH_OTAUPDATE_TYPE_FW, "Device's code is error!")
static void _ota_update_publish_late_failed_status(ota_update_t *otaupdate, const char *ota_error)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        return;
    }

    const char *ota_state_key = NULL;
    const char *ota_error_key = NULL;
    if (otaupdate->config.ota_type == TBCMH_OTAUPDATE_TYPE_FW) {
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
    tbcm_handle_t tbcm_handle = otaupdate->client->tbmqttclient;
    /*int result =*/ tbcm_telemetry_publish(tbcm_handle, pack, 1/*qos*/, 0/*retain*/);
    cJSON_free(pack); // free memory
    cJSON_Delete(object); // delete json object
}

//{"current_fw_title": "myFirmware", "current_fw_version": "1.2.3", "fw_state": TB_MQTT_VALUE_FW_SW_STATE_DOWNLOADING}
//{"current_fw_title": "myFirmware", "current_fw_version": "1.2.3", "fw_state": TB_MQTT_VALUE_FW_SW_STATE_DOWNLOADED}
//{"current_fw_title": "myFirmware", "current_fw_version": "1.2.3", "fw_state": TB_MQTT_VALUE_FW_SW_STATE_VERIFIED}
//{"current_fw_title": "myFirmware", "current_fw_version": "1.2.3", "fw_state": TB_MQTT_VALUE_FW_SW_STATE_UPDATING}
static void _ota_update_publish_going_status(ota_update_t *otaupdate, const char *ota_state)
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
    if (otaupdate->config.ota_type == TBCMH_OTAUPDATE_TYPE_FW) {
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
    tbcm_handle_t tbcm_handle = otaupdate->client->tbmqttclient;
    /*int result =*/ tbcm_telemetry_publish(tbcm_handle, pack, 1/*qos*/, 0/*retain*/);
    cJSON_free(pack); // free memory
    cJSON_Delete(object); // delete json object
}

//{"current_fw_title": "myNewFirmware", "current_fw_version": "1.5.2_new_version", "fw_state": "UPDATED"}
//{"current_sw_title": "myNewSoftware", "current_sw_version": "1.5.2_new_version", "sw_state": "UPDATED"}
static void _ota_update_publish_updated_status(ota_update_t *otaupdate)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        return;
    }

    /*if (!otaupdate->config.is_first_boot) {
        TBC_LOGI("otaupdate is NOT first boot! ota update type=%d(0:F/W, 1:S/W)",TBCMH_OTAUPDATE_TYPE_FW);
        return;
    }*/

    const char *current_ota_title_key = NULL;
    const char *current_ota_version_key = NULL;
    const char *ota_state_key = NULL;
    if (otaupdate->config.ota_type == TBCMH_OTAUPDATE_TYPE_FW) {
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
    tbcm_handle_t tbcm_handle = otaupdate->client->tbmqttclient;
    /*int result =*/ tbcm_telemetry_publish(tbcm_handle, pack, 1/*qos*/, 0/*retain*/);
    cJSON_free(pack); // free memory
    cJSON_Delete(object); // delete json object

    usleep(500000);
}

//return 1 on negotiate successful(next to F/W OTA), -1/ESP_FAIL on negotiate failure, 0/ESP_OK on already updated!
static tbc_err_t _ota_update_do_negotiate(ota_update_t *otaupdate,
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

    _ota_update_reset(otaupdate);

    // TODO: if ota_title & ota_version == current_title & current_version, then return 0!
    
    int result = otaupdate->config.on_ota_negotiate(otaupdate->client, otaupdate->config.context,
                                             ota_title, ota_version, ota_size, ota_checksum, ota_checksum_algorithm,
                                             ota_error, error_size);
    if (result==1) { // negotiate successful(next to F/W OTA)
        // cache ota_title
        otaupdate->attribute.ota_title = TBC_MALLOC(strlen(ota_title)+1);
        if (otaupdate->attribute.ota_title) {
            strcpy(otaupdate->attribute.ota_title, ota_title);
        }
        // cache ota_version
        otaupdate->attribute.ota_version = TBC_MALLOC(strlen(ota_version)+1);
        if (otaupdate->attribute.ota_version) {
            strcpy(otaupdate->attribute.ota_version, ota_version);
        }
        otaupdate->attribute.ota_size = ota_size;
        // cache ota_checksum
        otaupdate->attribute.ota_checksum = TBC_MALLOC(strlen(ota_checksum)+1);
        if (otaupdate->attribute.ota_checksum) {
            strcpy(otaupdate->attribute.ota_checksum, ota_checksum);
        }
        // cache ota_checksum_algorithm
        otaupdate->attribute.ota_checksum_algorithm = TBC_MALLOC(strlen(ota_checksum_algorithm)+1);
        if (otaupdate->attribute.ota_checksum_algorithm) {
            strcpy(otaupdate->attribute.ota_checksum_algorithm, ota_checksum_algorithm);
        }
    }

    return result;
}

//return 0/ESP_OK on successful, -1/ESP_FAIL on failure
static tbc_err_t _ota_update_do_write(ota_update_t *otaupdate, int chunk_id, 
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

    tbc_err_t result = true;
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
static tbc_err_t _ota_update_do_end(ota_update_t *otaupdate, char *ota_error, int error_size)
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

static void _ota_update_do_abort(ota_update_t *otaupdate)
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

//============== F/W or S/W chunk request/response/timeout ================================================================================

// return 0 on success, -1 on failure
static tbc_err_t _tbcmh_otaupdate_chunk_request(ota_update_t *otaupdate)
{
    char payload[20] = {0};
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        return -1;
    }

    tbcm_handle_t tbcm_handle = otaupdate->client->tbmqttclient;
    int chunk_size;
    if (otaupdate->attribute.ota_size < otaupdate->config.chunk_size) {
        chunk_size = 0; // full f/w or s/w
    } else {
        chunk_size = otaupdate->config.chunk_size;
    }
    sprintf(payload, "%d", chunk_size);
    
    int request_id = _request_list_create_and_append(otaupdate->client, TBCMH_REQUEST_FWUPDATE,
                                          otaupdate->state.request_id/*default -1*/);
    if (request_id <= 0) {
         TBC_LOGE("Unable to create request! %s()", __FUNCTION__);
         return -1;
    }
    int msg_id = tbcm_otaupdate_chunk_request(tbcm_handle, request_id, otaupdate->state.chunk_id/*default 0*/,
                          payload, //chunk_size
                          1/*qos*/, 0/*retain*/);
    // First OTA request
    if ((otaupdate->state.request_id<0) && (request_id>0)) {
         otaupdate->state.request_id = request_id;
    }
    if (msg_id<0){
        TBC_LOGW("Request OTA chunk(%d) failure! request_id=%d, msg_id=%d %s()",
            otaupdate->state.chunk_id, request_id, msg_id, __FUNCTION__);
    }

    return (msg_id<0)?-1:0;
}

void _tbcmh_otaupdate_chunk_on_response(tbcmh_handle_t client, int request_id, int chunk_id, const char* payload, int length)
{
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     // Remove it from request list
     _request_list_search_and_remove(client, request_id);

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;
     }

     // Search item
     ota_update_t *otaupdate = NULL;
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate && (otaupdate->state.request_id==request_id)) {
               break;
          }
     }
     if (!otaupdate) {
          TBC_LOGW("Unable to find ota_update:%d! %s()", request_id, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return;
     }

     // exec ota response
     char ota_error[128] = {0};
     const char* ota_error_ = "Unknown error!";
     int result = _ota_update_do_write(otaupdate, chunk_id, payload, length, ota_error, sizeof(ota_error)-1);
     switch (result) {
     case 0: //return 0: success on response
         if ((otaupdate->state.received_len >= otaupdate->attribute.ota_size))  { //Is it already received all f/w or s/w
            _ota_update_publish_going_status(otaupdate, TB_MQTT_VALUE_FW_SW_STATE_DOWNLOADED);

            if (_ota_update_checksum_verification(otaupdate)) {
                _ota_update_publish_going_status(otaupdate, TB_MQTT_VALUE_FW_SW_STATE_VERIFIED);
                _ota_update_publish_going_status(otaupdate, TB_MQTT_VALUE_FW_SW_STATE_UPDATING);

                memset(ota_error, 0x00, sizeof(ota_error));
                result = _ota_update_do_end(otaupdate, ota_error, sizeof(ota_error)-1);
                if (result==0) { // sussessful
                    _ota_update_publish_updated_status(otaupdate); //UPDATED
                    _ota_update_reset(otaupdate);
                } else {
                    if (strlen(ota_error)>0) {
                        ota_error_ = ota_error;
                    }
                    TBC_LOGE("Unknow result (%d, %s) of _ota_update_do_write()!", result, ota_error_);
                    _ota_update_publish_late_failed_status(otaupdate, ota_error_);
                    _ota_update_do_abort(otaupdate);
                    _ota_update_reset(otaupdate);
                }
            } else {
                _ota_update_publish_late_failed_status(otaupdate, "Checksum verification failed!");
                _ota_update_do_abort(otaupdate);
                _ota_update_reset(otaupdate);
            }
         }else {  //un-receied all f/w or s/w: go on, get next package
            result = _tbcmh_otaupdate_chunk_request(otaupdate);
            if (result != 0) { //failure to request chunk
                _ota_update_publish_late_failed_status(otaupdate, "Request OTA chunk failure!");
                _ota_update_do_abort(otaupdate);
                _ota_update_reset(otaupdate);
            }
         }
         break;
         
     case -1: //return -1: error
        if (strlen(ota_error)>0) {
            ota_error_ = ota_error;
        }
        TBC_LOGE("ota_error (%s) of _ota_update_do_write()!", ota_error_);
        _ota_update_publish_late_failed_status(otaupdate, ota_error_);
        _ota_update_do_abort(otaupdate);
        _ota_update_reset(otaupdate);
        break;
        
     default: //Unknow error
        TBC_LOGE("Unknow result (%d) of _ota_update_do_write()!", result);
        _ota_update_publish_late_failed_status(otaupdate, ota_error_);
        _ota_update_do_abort(otaupdate);
        _ota_update_reset(otaupdate);
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return;
}

void _tbcmh_otaupdate_chunk_on_timeout(tbcmh_handle_t client, int request_id)
{
     if (!client) {
          TBC_LOGE("client  is NULL! %s()", __FUNCTION__);
          return;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;
     }

     // Search item
     ota_update_t *otaupdate = NULL;
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate && (otaupdate->state.request_id==request_id)) {
               break;
          }
     }
     if (!otaupdate) {
          TBC_LOGW("Unable to find ota_update:%d! %s()", request_id, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return;
     }

     // abort ota
     _ota_update_publish_late_failed_status(otaupdate, "OTA response timeout!");
     _ota_update_do_abort(otaupdate);
     _ota_update_reset(otaupdate);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return;
}


//========== Firmware/Software update API ======================================================================
tbc_err_t tbcmh_otaupdate_append(tbcmh_handle_t client, 
                const char *ota_description, const tbcmh_otaupdate_config_t *config)
{
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (!ota_description) {
          TBC_LOGE("ota_description is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (!config) {
          TBC_LOGE("config is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Create otaupdate
     ota_update_t *otaupdate = _ota_update_create(client, ota_description, config);
     if (!otaupdate) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init otaupdate failure! ota_description=%s. %s()", ota_description, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert otaupdate to list
     ota_update_t *it, *last = NULL;
     if (LIST_FIRST(&client->otaupdate_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->otaupdate_list, otaupdate, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->otaupdate_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, otaupdate, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

tbc_err_t tbcmh_otaupdate_clear(tbcmh_handle_t client, const char *ota_description)
{
     if (!client || !ota_description) {
          TBC_LOGE("client or ota_description is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Search item
     ota_update_t *otaupdate = NULL;
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate && strcmp(otaupdate->ota_description, ota_description)==0) {
               break;
          }
     }
     if (!otaupdate) {
          TBC_LOGW("Unable to remove ota_update data:%s! %s()", ota_description, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(otaupdate, entry);
     _ota_update_destroy(otaupdate);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

tbc_err_t _tbcmh_otaupdate_empty(tbcmh_handle_t client)
{
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

     // remove all item in otaupdate_list
     ota_update_t *otaupdate = NULL, *next;
     LIST_FOREACH_SAFE(otaupdate, &client->otaupdate_list, entry, next) {
          // exec timeout callback
          _ota_update_do_abort(otaupdate);
          _ota_update_reset(otaupdate);

          // remove from otaupdate list and destory
          LIST_REMOVE(otaupdate, entry);
          _ota_update_destroy(otaupdate);
     }
     // memset(&client->otaupdate_list, 0x00, sizeof(client->otaupdate_list));

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

//========= shared attributes about F/W or S/W OTA update =========================

static void __on_fw_attributesrequest_response(tbcmh_handle_t client,
                void *context, int request_id)
{
    //no code
}
static void __on_sw_attributesrequest_response(tbcmh_handle_t client,
                void *context, int request_id)
{
    //no code
}

void _tbcmh_otaupdate_on_connected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!

    if (!client) {
         TBC_LOGE("client is NULL! %s()", __FUNCTION__);
         return;// ESP_FAIL;
    }

    int msg_id = tbcm_subscribe(client->tbmqttclient,
                                TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE, 0);
    TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s",
             msg_id, TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE);

     // Search item
     ota_update_t *otaupdate = NULL;
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate && (otaupdate->config.ota_type==TBCMH_OTAUPDATE_TYPE_FW) ) {
              // send current f/w info UPDATED telemetry
              ////_ota_update_publish_updated_status(otaupdate); // only at otaupdate->config.is_first_boot

              // send init current f/w info telemetry
              _ota_update_publish_early_current_version(otaupdate);
              // send f/w info attributes request
              _tbcmh_attributesrequest_send_4_ota_sharedattributes(client,
                     NULL/*context*/,
                     __on_fw_attributesrequest_response/*on_response*/,
                     NULL/*on_timeout*/,
                     5/*count*/,
                     TB_MQTT_KEY_FW_TITLE,
                     TB_MQTT_KEY_FW_VERSION,
                     TB_MQTT_KEY_FW_SIZE,
                     TB_MQTT_KEY_FW_CHECKSUM,
                     TB_MQTT_KEY_FW_CHECKSUM_ALG);
              break;
          }
     }
     
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate && (otaupdate->config.ota_type==TBCMH_OTAUPDATE_TYPE_SW) ) {
              // send current s/w info UPDATED telemetry
              ////_ota_update_publish_updated_status(otaupdate); // only at otaupdate->config.is_first_boot

              // send init current s/w telemetry
              _ota_update_publish_early_current_version(otaupdate);
              // send s/w info attributes request
              _tbcmh_attributesrequest_send_4_ota_sharedattributes(client,
                     NULL/*context*/,
                     __on_sw_attributesrequest_response/*on_response*/,
                     NULL/*on_timeout*/,
                     5/*count*/,
                     TB_MQTT_KEY_SW_TITLE,
                     TB_MQTT_KEY_SW_VERSION,
                     TB_MQTT_KEY_SW_SIZE,
                     TB_MQTT_KEY_SW_CHECKSUM,
                     TB_MQTT_KEY_SW_CHECKSUM_ALG);
              break;
          }
     }
}

void _tbcmh_otaupdate_on_sharedattributes(tbcmh_handle_t client, tbcmh_otaupdate_type_t ota_type,
                                         const char *ota_title, const char *ota_version, int ota_size,
                                         const char *ota_checksum, const char *ota_checksum_algorithm)
{
     if (!client || !ota_title) {
          TBC_LOGE("client or ota_title is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }
     
     tbcm_handle_t tbcm_handle = client->tbmqttclient;

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          _ota_update_publish_early_failed_status(tbcm_handle, ota_type, "Device code is error!");
          return;// ESP_FAIL;
     }

     // Search item
     ota_update_t *otaupdate = NULL;
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate &&
               (strcmp(_ota_update_get_current_title(otaupdate), ota_title)==0) &&
               (otaupdate->config.ota_type==ota_type) ) {
               break;
          }
     }
     if (!otaupdate) {
          TBC_LOGW("Unable to find ota_update:%s! %s()", ota_title, __FUNCTION__);
          _ota_update_publish_early_failed_status(tbcm_handle, ota_type, "Device code is error!");
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return;// ESP_FAIL;
     }

     // exec ota_update
     char ota_error[128] = {0};
     const char* ota_error_ = "Unknown error!";
     int result = _ota_update_do_negotiate(otaupdate, ota_title, ota_version, ota_size,
                        ota_checksum, ota_checksum_algorithm, ota_error, sizeof(ota_error)-1);
     if (result == 1) { //negotiate successful(next to F/W OTA)
        _ota_update_publish_going_status(otaupdate, TB_MQTT_VALUE_FW_SW_STATE_DOWNLOADING);
        result = _tbcmh_otaupdate_chunk_request(otaupdate);
        if (result != 0) { //failure to request chunk
            TBC_LOGW("Request first OTA chunk failure! %s()", __FUNCTION__);
            _ota_update_publish_early_failed_status(tbcm_handle, ota_type, "Request OTA chunk failure!");
            _ota_update_do_abort(otaupdate);
            _ota_update_reset(otaupdate);
        }
     } else if (result==0) { //0/ESP_OK: already updated!
        //no code!
        //if (strlen(ota_error)>0) {
        //    ota_error_ = ota_error;
        //}
        //TBC_LOGE("ota_error (%s) of _ota_update_do_negotiate()!", ota_error_);
        //_ota_update_publish_early_failed_status(tbcm_handle, ota_type, ota_error_);
     }
     else { //-1/ESP_FAIL: negotiate failure
        if (strlen(ota_error)>0) {
            ota_error_ = ota_error;
        }
        TBC_LOGE("ota_error (%s) of _ota_update_do_negotiate()!", ota_error_);
        _ota_update_publish_early_failed_status(tbcm_handle, ota_type, ota_error_);
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return;// ESP_OK;
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

/*static*/ void _tbcmh_otaupdate_publish_current_status(tbcmh_handle_t client)
{
    // TODO:
    //    1. Send telemetry: *current firmware info*
    //       * Topic: `v1/devices/me/telemetry`
    //       * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0"}`

    //    1. Send telemetry: *current firmware info*
    //       * Topic: `v1/devices/me/telemetry`
    //       * Payload: `{"current_sw_title":"Initial","current_sw_version":"v0"}`
}

/*static*/ void __tbcmh_otaupdate_attributesrequest_on_response(tbcmh_handle_t client, 
                    void *context, int request_id)
{
    // no code!
}
/*static*/ void __tbcmh_otaupdate_attributesrequest_on_timeout(tbcmh_handle_t client,
                    void *context, int request_id)
{
    // TODO: resend ???
}
/*static*/ void _tbcmh_otaupdate_send_attributes_request(tbcmh_handle_t client)
{
    // TODO:

    // tbc_err_t tbcmh_sharedattribute_append(tbcmh_handle_t client, const char *key, void *context,
    //                                     tbcmh_sharedattribute_on_set_t on_set);
    // key: fw_checksum,fw_checksum_algorithm,fw_size,fw_title,fw_version
    //      sw_checksum,sw_checksum_algorithm,sw_size,sw_title,sw_version

    //int tbcmh_attributesrequest_send(client,
    //                                 NULL,
    //                                 __tbcmh_otaupdate_attributesrequest_on_response,
    //                                 __tbcmh_otaupdate_attributesrequest_on_timeout,
    //                                int count, /*const char *key,*/...)
    //    Send attributes request: *request firmware info*
    //      * Topic: `v1/devices/me/attributes/request/{request_id}`
    //      * Payload: `{"sharedKeys": "fw_checksum,fw_checksum_algorithm,fw_size,fw_title,fw_version"}`

    //      * Topic: `v1/devices/me/attributes/request/{request_id}`
    //      * Payload: `{"sharedKeys": "sw_checksum,sw_checksum_algorithm,sw_size,sw_title,sw_version"}`

}
#endif

