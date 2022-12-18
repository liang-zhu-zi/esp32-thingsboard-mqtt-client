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

#include "tbc_mqtt_helper_internal.h"
#include "ota_fwupdate.h"

static int _otaupdate_on_fw_attributesupdate(tbcmh_handle_t client,
                                      void *context, const cJSON *object);
static int _otaupdate_on_sw_attributesupdate(tbcmh_handle_t client,
                                    void *context, const cJSON *object);
static void _otaupdate_on_fw_attributesrequest_response(tbcmh_handle_t client,
                      void *context,
                      const cJSON *client_attributes,
                      const cJSON *shared_attributes);										 
static void _otaupdate_on_sw_attributesrequest_response(tbcmh_handle_t client,
                      void *context,
                      const cJSON *client_attributes,
                      const cJSON *shared_attributes);										 


const static char *TAG = "otaupdate";

/*!< Initialize otaupdate_t */
static otaupdate_t *_otaupdate_create(tbcmh_handle_t client,
                        const char *ota_description,
                        tbcmh_otaupdate_type_t ota_type,
                        void *context_user,
                        tbcmh_otaupdate_on_get_current_title_t on_get_current_title,
                        tbcmh_otaupdate_on_get_current_version_t on_get_current_version,
                        tbcmh_otaupdate_on_updated_t on_updated)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(ota_description, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_get_current_title, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_get_current_version, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_updated, NULL);

    void *context_xw = NULL;
    if (ota_type == TBCMH_OTAUPDATE_TYPE_FW) {
        context_xw = ota_fwupdate_init();
    } else if (ota_type == TBCMH_OTAUPDATE_TYPE_SW) {
        // TODO: support sw ota!
        // context_xw = _my_swupdate_init();
    }
    if (!context_xw) {
        TBC_LOGE("failure to call _my_xwupdate_init()!");
        return NULL;
    }

    otaupdate_t *otaupdate = TBC_MALLOC(sizeof(otaupdate_t));
    if (!otaupdate) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(otaupdate, 0x00, sizeof(otaupdate_t));
    otaupdate->client = client;
    otaupdate->ota_description = TBC_MALLOC(strlen(ota_description)+1);
    if (otaupdate->ota_description) {
        strcpy(otaupdate->ota_description, ota_description);
    }

    otaupdate->config.ota_type = ota_type;
    otaupdate->config.chunk_size = 16*1024;
    otaupdate->config.context_user = context_user;
    otaupdate->config.on_get_current_title = on_get_current_title;
    otaupdate->config.on_get_current_version = on_get_current_version;
    otaupdate->config.on_ota_updated = on_updated;

    otaupdate->config.context_xw = context_xw;
    if (ota_type == TBCMH_OTAUPDATE_TYPE_FW) {
        otaupdate->config.on_ota_negotiate = ota_fwupdate_negotiate,
        otaupdate->config.on_ota_write = ota_fwupdate_write;
        otaupdate->config.on_ota_end = ota_fwupdate_end;
        otaupdate->config.on_ota_abort = ota_fwupdate_abort;
        ////otaupdate->config.is_first_boot = config->is_first_boot;           /*!< whether first boot after ota update  */
    } else if (ota_type == TBCMH_OTAUPDATE_TYPE_SW) {
        // TODO: support sw ota!
        // otaupdate->config.on_ota_negotiate = config->on_ota_negotiate;      /*!< callback of F/W or S/W OTA attributes */
        // otaupdate->config.on_ota_write = config->on_ota_write;              /*!< callback of F/W or S/W OTA doing */
        // otaupdate->config.on_ota_end = config->on_ota_end;                  /*!< callback of F/W or S/W OTA success & end */
        // otaupdate->config.on_ota_abort = config->on_ota_abort;              /*!< callback of F/W or S/W OTA failure & abort */
        ////otaupdate->config.is_first_boot = config->is_first_boot;           /*!< whether first boot after ota update  */
    }
    otaupdate->attribute.ota_title = NULL;
    otaupdate->attribute.ota_version = NULL;
    otaupdate->attribute.ota_size = 0;
    otaupdate->attribute.ota_checksum = NULL;
    otaupdate->attribute.ota_checksum_algorithm = NULL;

    otaupdate->state.request_id = 0; //-1;
    otaupdate->state.chunk_id = 0;
    otaupdate->state.received_len = 0;
    otaupdate->state.checksum = 0;

    // LIST_ENTRY(otaupdate) entry;
    return otaupdate;
}

/*!< Destroys the otaupdate_t */
static tbc_err_t _otaupdate_destroy(otaupdate_t *otaupdate)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(otaupdate, ESP_FAIL);

    otaupdate->client = NULL;
    if (otaupdate->ota_description) {
        TBC_FREE(otaupdate->ota_description);
        otaupdate->ota_description = NULL;
    }

    otaupdate->config.ota_type = 0;
    otaupdate->config.chunk_size = 0;
    otaupdate->config.context_user = NULL;
    otaupdate->config.on_get_current_title = NULL;
    otaupdate->config.on_get_current_version = NULL;
    otaupdate->config.on_ota_updated = NULL;

    otaupdate->config.context_xw = NULL;
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

    otaupdate->state.request_id = 0; //-1;
    otaupdate->state.chunk_id = 0;
    otaupdate->state.received_len = 0;
    otaupdate->state.checksum = 0;

    TBC_FREE(otaupdate);
    return ESP_OK;
}

//{"current_fw_title": "myFirmware", "current_fw_version": "1.2.3"}
static void _otaupdate_publish_early_current_version(otaupdate_t *otaupdate)
{
    TBC_CHECK_PTR(otaupdate);

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
    current_ota_title_value = otaupdate->config.on_get_current_title(otaupdate->config.context_user);
    current_ota_version_value = otaupdate->config.on_get_current_version(otaupdate->config.context_user);

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
static void _otaupdate_publish_early_failed_status(tbcm_handle_t tbcm_handle, 
                                tbcmh_otaupdate_type_t ota_type, const char *ota_error)
{
    TBC_CHECK_PTR(tbcm_handle);

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
static void _otaupdate_publish_late_failed_status(otaupdate_t *otaupdate, const char *ota_error)
{
    TBC_CHECK_PTR(otaupdate);

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
static void _otaupdate_publish_going_status(otaupdate_t *otaupdate, const char *ota_state)
{
    TBC_CHECK_PTR(otaupdate);
    TBC_CHECK_PTR(ota_state);

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
    current_ota_title_value = otaupdate->config.on_get_current_title(otaupdate->config.context_user);
    current_ota_version_value = otaupdate->config.on_get_current_version(otaupdate->config.context_user);

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
static void _otaupdate_publish_updated_status(otaupdate_t *otaupdate)
{
    TBC_CHECK_PTR(otaupdate);

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

static bool _otaupdate_checksum_verification(otaupdate_t *otaupdate)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(otaupdate, false);

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

static void _otaupdate_reset(otaupdate_t *otaupdate)
{
    TBC_CHECK_PTR(otaupdate);

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

    otaupdate->state.request_id = 0; //-1;
    otaupdate->state.chunk_id = 0;
    otaupdate->state.received_len = 0;
    otaupdate->state.checksum = 0;
    otaupdate->state.timestamp = 0;
}

static void _otaupdate_do_updated(otaupdate_t *otaupdate, bool success)
{
    TBC_CHECK_PTR(otaupdate);

    // TODO: sw & fw are upgrading at same time!
    // Unsubscript topic <===  non-empty->empty
    if (tbcmh_is_connected(otaupdate->client)) {
        int msg_id = tbcm_unsubscribe(otaupdate->client->tbmqttclient,
                                TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE);
        TBC_LOGI("sent unsubscribe successful, msg_id=%d, topic=%s",
                                msg_id, TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE);
    }

    TBC_CHECK_PTR(otaupdate->config.on_ota_updated);
    otaupdate->config.on_ota_updated(otaupdate->config.context_user, success);
}

//return 1 on negotiate successful(next to F/W OTA), -1/ESP_FAIL on negotiate failure, 0/ESP_OK on already updated!
static tbc_err_t _otaupdate_do_negotiate(otaupdate_t *otaupdate,
                                        const char *ota_title, const char *ota_version, uint32_t ota_size,
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
        TBC_LOGW("Only support CRC32 for otaupdate! Don't support %s! %s()", ota_checksum_algorithm, __FUNCTION__);
        strncpy(ota_error, "Only support CRC checksum!", error_size);
        return -1;
    }

    _otaupdate_reset(otaupdate);

    // TODO: if ota_title & ota_version == current_title & current_version, then return 0!

    const char* current_fw_title = otaupdate->config.on_get_current_title(otaupdate->config.context_user);
    const char* current_fw_version = otaupdate->config.on_get_current_version(otaupdate->config.context_user);
    int result = otaupdate->config.on_ota_negotiate(otaupdate->config.context_xw,
                                        current_fw_title, current_fw_version,
                                        ota_title, ota_version, ota_size,
                                        ota_checksum, ota_checksum_algorithm,
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

// return 0 on success, -1 on failure
static tbc_err_t _otaupdate_chunk_request(otaupdate_t *otaupdate, bool isFirstChunk)
{
    char payload[20] = {0};
    TBC_CHECK_PTR_WITH_RETURN_VALUE(otaupdate, -1);

    tbcm_handle_t tbcm_handle = otaupdate->client->tbmqttclient;
    uint32_t chunk_size;
    if (otaupdate->attribute.ota_size < otaupdate->config.chunk_size) {
        chunk_size = 0; // full f/w or s/w
    } else {
        chunk_size = otaupdate->config.chunk_size;
    }
    sprintf(payload, "%u", chunk_size);

    // Send msg to server
    uint32_t request_id;
    if (isFirstChunk) {
        request_id = _tbcmh_get_request_id(otaupdate->client);
        otaupdate->state.request_id = request_id;
    } else {
        request_id = otaupdate->state.request_id;
    }
    // uint32_t request_id = otaupdate->state.request_id;
    // if (request_id <= 0) {
    //     request_id = _tbcmh_get_request_id(otaupdate->client);
    //     if (request_id <= 0) {
    //          TBC_LOGE("failure to getting request id!");
    //          return -1;
    //     }
    // }
    int msg_id = tbcm_otaupdate_chunk_request(tbcm_handle, request_id,
                          otaupdate->state.chunk_id/*default 0*/,
                          payload, //chunk_size
                          1/*qos*/, 0/*retain*/);
    if (msg_id<0){
        TBC_LOGW("Request OTA chunk(%u) failure! request_id=%u, msg_id=%d %s()",
            otaupdate->state.chunk_id, request_id, msg_id, __FUNCTION__);
    }
    // First OTA request
    // if ((otaupdate->state.request_id<=0) && (request_id>0)) {
    //      otaupdate->state.request_id = request_id;
    // }
    otaupdate->state.timestamp = (uint64_t)time(NULL);

    return (msg_id<0)?-1:0;
}


//return 0/ESP_OK on successful, -1/ESP_FAIL on failure
static tbc_err_t _otaupdate_do_write(otaupdate_t *otaupdate, uint32_t chunk_id, 
                                            const void *ota_data, uint32_t data_size,
                                            char *ota_error, int error_size)
{
    if (!otaupdate) {
        TBC_LOGE("otaupdate is NULL!");
        strncpy(ota_error, "Device's code is error!", error_size);
        return -1; //code error
    }
    if (chunk_id != otaupdate->state.chunk_id) {
        TBC_LOGE("chunk_id(%u) is not equal to otaupdate->state.chunk_id(%u)!", chunk_id, otaupdate->state.chunk_id);
        strncpy(ota_error, "Chunk ID is error!", error_size);
        return -1; //chunk_id error
    }
    if (!ota_data || !data_size) {
        TBC_LOGE("ota_data(%p) or data_size(%u) is error!", ota_data, data_size);
        strncpy(ota_error, "OTA data is empty!", error_size);
        return -1; //ota_data is empty
    }

    tbc_err_t result = true;
    result = otaupdate->config.on_ota_write(otaupdate->config.context_xw,
                            ota_data, data_size,
                            ota_error, error_size); //otaupdate->state.request_id, chunk_id,
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
static tbc_err_t _otaupdate_do_end(otaupdate_t *otaupdate, char *ota_error, int error_size)
{
    int ret = 0;
    TBC_CHECK_PTR_WITH_RETURN_VALUE(otaupdate, -1);

    // on_ota_end() and reset()
    if (otaupdate->config.on_ota_end && otaupdate->state.request_id>0 && otaupdate->state.received_len>0) {
        ret = otaupdate->config.on_ota_end(otaupdate->config.context_xw, 
                                         ota_error, error_size); //otaupdate->state.request_id, otaupdate->state.chunk_id,
    }
    return (ret==0)?0:-1;
}

static void _otaupdate_do_abort(otaupdate_t *otaupdate)
{
    TBC_CHECK_PTR(otaupdate);

    // on_ota_abort() and reset()
    if (otaupdate->config.on_ota_abort && otaupdate->state.request_id>0 && otaupdate->state.received_len>0) {
        otaupdate->config.on_ota_abort(otaupdate->config.context_xw);
                        //, otaupdate->state.request_id, otaupdate->state.chunk_id/*current chunk_id*/
    }
}

//========= shared attributes about F/W or S/W OTA update =========================
void _tbcmh_otaupdate_on_create(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // list create
    memset(&client->otaupdate_list, 0x00, sizeof(client->otaupdate_list)); //client->otaupdate_list = LIST_HEAD_INITIALIZER(client->otaupdate_list);

    // Give semaphore
    // xSemaphoreGiveRecursive(client->_lock);
}

void _tbcmh_otaupdate_on_destroy(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // items empty - remove all item in otaupdate_list
    otaupdate_t *otaupdate = NULL, *next;
    LIST_FOREACH_SAFE(otaupdate, &client->otaupdate_list, entry, next) {
         // exec timeout callback
         // _otaupdate_do_abort(otaupdate);
         // _otaupdate_reset(otaupdate);

         // remove from otaupdate list and destory
         LIST_REMOVE(otaupdate, entry);
         _otaupdate_destroy(otaupdate);
    }
    memset(&client->otaupdate_list, 0x00, sizeof(client->otaupdate_list));

    // Give semaphore
    // xSemaphoreGiveRecursive(client->_lock);
}

tbc_err_t tbcmh_otaupdate_subscribe(tbcmh_handle_t client, 
                const char *ota_description,  // TODO: remove it!
                tbcmh_otaupdate_type_t ota_type,
                void *context_user,
                tbcmh_otaupdate_on_get_current_title_t on_get_current_title,
                tbcmh_otaupdate_on_get_current_version_t on_get_current_version,
                tbcmh_otaupdate_on_updated_t on_updated)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(ota_description, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(on_get_current_title, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(on_get_current_version, ESP_FAIL);

     // Take semaphore
     if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Create otaupdate
     otaupdate_t *otaupdate = _otaupdate_create(client,
                                ota_description, ota_type,
                                context_user,
                                on_get_current_title,
                                on_get_current_version,
                                on_updated);
     if (!otaupdate) {
          // Give semaphore
          xSemaphoreGiveRecursive(client->_lock);
          TBC_LOGE("Init otaupdate failure! ota_description=%s. %s()", ota_description, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert otaupdate to list
     otaupdate_t *it, *last = NULL;
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
     xSemaphoreGiveRecursive(client->_lock);
     return ESP_OK;
}

tbc_err_t tbcmh_otaupdate_unsubscribe(tbcmh_handle_t client, const char *ota_description)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(ota_description, ESP_FAIL);

     // Take semaphore
     if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Search item
     otaupdate_t *otaupdate = NULL;
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate && strcmp(otaupdate->ota_description, ota_description)==0) {
             // Remove from list and destroy
             LIST_REMOVE(otaupdate, entry);
             _otaupdate_destroy(otaupdate);
             break;
          }
     }

     // Give semaphore
     xSemaphoreGiveRecursive(client->_lock);

     if (!otaupdate) {
          TBC_LOGW("Unable to remove otaupdate data:%s! %s()", ota_description, __FUNCTION__);
          return ESP_FAIL;
     }
     return ESP_OK;
}


void _tbcmh_otaupdate_on_connected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Search item
    otaupdate_t *otaupdate = NULL;
    LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
         if (otaupdate && (otaupdate->config.ota_type==TBCMH_OTAUPDATE_TYPE_FW) ) {
             // send current f/w info UPDATED telemetry
             ////_otaupdate_publish_updated_status(otaupdate); // only at otaupdate->config.is_first_boot

             // send init current f/w info telemetry
             _otaupdate_publish_early_current_version(otaupdate);
             tbcmh_attributes_subscribe(client,
                    otaupdate/*context*/,
                    _otaupdate_on_fw_attributesupdate,
                    5/*count*/,
                    TB_MQTT_KEY_FW_TITLE,
                    TB_MQTT_KEY_FW_VERSION,
                    TB_MQTT_KEY_FW_SIZE,
                    TB_MQTT_KEY_FW_CHECKSUM,
                    TB_MQTT_KEY_FW_CHECKSUM_ALG);
             // send f/w info attributes request
             tbcmh_sharedattributes_request(client,
                    otaupdate/*context*/,
                    _otaupdate_on_fw_attributesrequest_response/*on_response*/,
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
             ////_otaupdate_publish_updated_status(otaupdate); // only at otaupdate->config.is_first_boot

             // send init current s/w telemetry
             _otaupdate_publish_early_current_version(otaupdate);
             tbcmh_attributes_subscribe(client,
                    otaupdate/*context*/,
                    _otaupdate_on_sw_attributesupdate,
                    5/*count*/,
                    TB_MQTT_KEY_SW_TITLE,
                    TB_MQTT_KEY_SW_VERSION,
                    TB_MQTT_KEY_SW_SIZE,
                    TB_MQTT_KEY_SW_CHECKSUM,
                    TB_MQTT_KEY_SW_CHECKSUM_ALG);
             // send s/w info attributes request
             tbcmh_sharedattributes_request(client,
                    otaupdate/*context*/,
                    _otaupdate_on_sw_attributesrequest_response/*on_response*/,
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

void _tbcmh_otaupdate_on_disconnected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)

    // all chunk request timeout!!!!
    _tbcmh_otaupdate_on_chunk_check_timeout(client, (uint64_t)time(NULL)+2*TB_MQTT_TIMEOUT);
}

//on received shared attributes of fw/sw: unpack & deal
static void __otaupdate_on_sharedattributes(tbcmh_handle_t client,
                                         tbcmh_otaupdate_type_t ota_type,
                                         const char *ota_title,
                                         const char *ota_version, uint32_t ota_size,
                                         const char *ota_checksum, const char *ota_checksum_algorithm)
{
     TBC_CHECK_PTR(client);
     TBC_CHECK_PTR(ota_title);

     tbcm_handle_t tbcm_handle = client->tbmqttclient;

     // Take semaphore
     // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
     //      _otaupdate_publish_early_failed_status(tbcm_handle, ota_type, "Device code is error!");
     //      return;;
     // }

     // Search item
     otaupdate_t *otaupdate = NULL, *next;
     LIST_FOREACH_SAFE(otaupdate, &client->otaupdate_list, entry, next) {
        if (otaupdate &&
           (otaupdate->config.ota_type==ota_type) &&
            otaupdate->config.on_get_current_title)
        {
            const char * current_ota_title = 
                            otaupdate->config.on_get_current_title(otaupdate->config.context_user);
            if (current_ota_title && strcmp(current_ota_title, ota_title)==0) {
                break;
            }
        }
     }

     // Give semaphore
     // xSemaphoreGiveRecursive(client->_lock);

     if (!otaupdate) {
          TBC_LOGW("Unable to find otaupdate:%s! %s()", ota_title, __FUNCTION__);
          _otaupdate_publish_early_failed_status(tbcm_handle, ota_type, "Device code is error!");
          return;// ESP_FAIL;
     }

     // exec otaupdate
     char ota_error[128] = {0};
     const char* ota_error_ = "Unknown error!";
     int result = _otaupdate_do_negotiate(otaupdate, ota_title, ota_version, ota_size,
                        ota_checksum, ota_checksum_algorithm, ota_error, sizeof(ota_error)-1);
     if (result == 1) { //negotiate successful(next to F/W OTA)
        // TODO: sw & fw are upgrading at same time!
        // NOTE: It must subscribe response topic, then send request!
        // Subscript topic <===  empty->non-empty
        if (tbcmh_is_connected(client)) {
            int msg_id = tbcm_subscribe(client->tbmqttclient,
                                    TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE, 0);
            TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s",
                                    msg_id, TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE);
        }

        // chunk_request
        _otaupdate_publish_going_status(otaupdate, TB_MQTT_VALUE_FW_SW_STATE_DOWNLOADING);
        result = _otaupdate_chunk_request(otaupdate, true);
        if (result != 0) { //failure to request chunk
            TBC_LOGW("Request first OTA chunk failure! %s()", __FUNCTION__);
            _otaupdate_publish_early_failed_status(tbcm_handle, ota_type, "Request OTA chunk failure!");
            _otaupdate_do_abort(otaupdate);
            _otaupdate_reset(otaupdate);
            _otaupdate_do_updated(otaupdate, false);
        }
     } else if (result==0) { //0/ESP_OK: already updated!
        //no code!
        //if (strlen(ota_error)>0) {
        //    ota_error_ = ota_error;
        //}
        //TBC_LOGE("ota_error (%s) of _otaupdate_do_negotiate()!", ota_error_);
        //_otaupdate_publish_early_failed_status(tbcm_handle, ota_type, ota_error_);
     }
     else { //-1/ESP_FAIL: negotiate failure
        if (strlen(ota_error)>0) {
            ota_error_ = ota_error;
        }
        TBC_LOGE("ota_error (%s) of _otaupdate_do_negotiate()!", ota_error_);
        _otaupdate_publish_early_failed_status(tbcm_handle, ota_type, ota_error_);
     }
}

// return 2 if calling tbcmh_disconnect()/tbcmh_destroy() inside on_update()
// return 1 if calling tbcmh_sharedattribute_unregister()/tbcmh_attributes_unsubscribe inside on_update()
// return 0 otherwise
static int _otaupdate_on_fw_attributesupdate(tbcmh_handle_t client,
                                      void *context, const cJSON *object)
{

     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, 0);
     if (!object) { \
          TBC_LOGW("object is NULL! %s() %d", __FUNCTION__, __LINE__);
          return 0;
     }

     if (cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_TITLE) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_VERSION) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_SIZE) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_CHECKSUM) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_CHECKSUM_ALG))
     {
          char *ota_title = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_TITLE));
          char *ota_version = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_VERSION));
          int ota_size = cJSON_GetNumberValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_SIZE));
          char *ota_checksum = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_CHECKSUM));
          char *ota_checksum_algorithm = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_CHECKSUM_ALG));
          __otaupdate_on_sharedattributes(client, TBCMH_OTAUPDATE_TYPE_FW,
                    ota_title, ota_version, ota_size, ota_checksum, ota_checksum_algorithm);
     }

     return 0;
}

// return 2 if calling tbcmh_disconnect()/tbcmh_destroy() inside on_update()
// return 1 if calling tbcmh_sharedattribute_unregister()/tbcmh_attributes_unsubscribe inside on_update()
// return 0 otherwise
static int _otaupdate_on_sw_attributesupdate(tbcmh_handle_t client,
                                    void *context, const cJSON *object)
{

    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, 0);
    if (!object) { \
         TBC_LOGW("object is NULL! %s() %d", __FUNCTION__, __LINE__);
         return 0;
    }

    if (cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_TITLE) &&
        cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_VERSION) &&
        cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_SIZE) &&
        cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_CHECKSUM) &&
        cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_CHECKSUM_ALG))
    {
         char *sw_title = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_TITLE));
         char *sw_version = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_VERSION));
         int sw_size = cJSON_GetNumberValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_SIZE));
         char *sw_checksum = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_CHECKSUM));
         char *sw_checksum_algorithm = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_CHECKSUM_ALG));
         __otaupdate_on_sharedattributes(client, TBCMH_OTAUPDATE_TYPE_SW,
                sw_title, sw_version, sw_size, sw_checksum, sw_checksum_algorithm);
    }
    return 0;
}

static void _otaupdate_on_fw_attributesrequest_response(tbcmh_handle_t client,
                      void *context,
                      const cJSON *client_attributes,
                      const cJSON *shared_attributes)//, uint32_t request_id
{
    TBC_CHECK_PTR(client);
    if (!shared_attributes) {
         TBC_LOGW("shared_attributes is NULL! %s() %d", __FUNCTION__, __LINE__);
         return;
    }

    _otaupdate_on_fw_attributesupdate(client, context, shared_attributes);
}

static void _otaupdate_on_sw_attributesrequest_response(tbcmh_handle_t client,
                      void *context,
                      const cJSON *client_attributes,
                      const cJSON *shared_attributes)//, uint32_t request_id
{
    TBC_CHECK_PTR(client);
    if (!shared_attributes) {
         TBC_LOGW("shared_attributes is NULL! %s() %d", __FUNCTION__, __LINE__);
         return;
    }

    _otaupdate_on_sw_attributesupdate(client, context, shared_attributes);
}

//on response
void _tbcmh_otaupdate_on_chunk_data(tbcmh_handle_t client,
                                         uint32_t request_id,
                                         uint32_t chunk_id,
                                         const char* payload, int length)
{
     TBC_CHECK_PTR(client);

     // Take semaphore
     // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
     //      return;
     // }

     // Search item
     otaupdate_t *otaupdate = NULL;
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate && (otaupdate->state.request_id==request_id)) {
               break;
          }
     }
     if (!otaupdate) {
          // Give semaphore
          // xSemaphoreGiveRecursive(client->_lock);
          TBC_LOGW("Unable to find otaupdate:%u! %s()", request_id, __FUNCTION__);
          return;
     }

     // exec ota response
     char ota_error[128] = {0};
     const char* ota_error_ = "Unknown error!";
     int result = _otaupdate_do_write(otaupdate, chunk_id, payload, length, ota_error, sizeof(ota_error)-1);
     switch (result) {
     case 0: //return 0: success on response
          if ((otaupdate->state.received_len >= otaupdate->attribute.ota_size))  { //Is it already received all f/w or s/w
             _otaupdate_publish_going_status(otaupdate, TB_MQTT_VALUE_FW_SW_STATE_DOWNLOADED);
 
             if (_otaupdate_checksum_verification(otaupdate)) {
                 _otaupdate_publish_going_status(otaupdate, TB_MQTT_VALUE_FW_SW_STATE_VERIFIED);
                 _otaupdate_publish_going_status(otaupdate, TB_MQTT_VALUE_FW_SW_STATE_UPDATING);
 
                 memset(ota_error, 0x00, sizeof(ota_error));
                 result = _otaupdate_do_end(otaupdate, ota_error, sizeof(ota_error)-1);
                 if (result==0) { // sussessful
                     _otaupdate_publish_updated_status(otaupdate); //UPDATED
                     _otaupdate_reset(otaupdate);
                     _otaupdate_do_updated(otaupdate, true);
                 } else {
                     if (strlen(ota_error)>0) {
                         ota_error_ = ota_error;
                     }
                     TBC_LOGE("Unknow result (%d, %s) of _otaupdate_do_write()!", result, ota_error_);
                     _otaupdate_publish_late_failed_status(otaupdate, ota_error_);
                     _otaupdate_do_abort(otaupdate);
                     _otaupdate_reset(otaupdate);
                     _otaupdate_do_updated(otaupdate, false);
                 }
             } else {
                 _otaupdate_publish_late_failed_status(otaupdate, "Checksum verification failed!");
                 _otaupdate_do_abort(otaupdate);
                 _otaupdate_reset(otaupdate);
                 _otaupdate_do_updated(otaupdate, false);
             }
          }else {  //un-receied all f/w or s/w: go on, get next package
             result = _otaupdate_chunk_request(otaupdate, false);
             if (result != 0) { //failure to request chunk
                 _otaupdate_publish_late_failed_status(otaupdate, "Request OTA chunk failure!");
                 _otaupdate_do_abort(otaupdate);
                 _otaupdate_reset(otaupdate);
                 _otaupdate_do_updated(otaupdate, false);
             }
          }
          break;
          
      case -1: //return -1: error
         if (strlen(ota_error)>0) {
             ota_error_ = ota_error;
         }
         TBC_LOGE("ota_error (%s) of _otaupdate_do_write()!", ota_error_);
         _otaupdate_publish_late_failed_status(otaupdate, ota_error_);
         _otaupdate_do_abort(otaupdate);
         _otaupdate_reset(otaupdate);
         _otaupdate_do_updated(otaupdate, false);
         break;
         
      default: //Unknow error
         TBC_LOGE("Unknow result (%d) of _otaupdate_do_write()!", result);
         _otaupdate_publish_late_failed_status(otaupdate, ota_error_);
         _otaupdate_do_abort(otaupdate);
         _otaupdate_reset(otaupdate);
         _otaupdate_do_updated(otaupdate, false);
      }
 
     // Give semaphore
     // xSemaphoreGiveRecursive(client->_lock);
}
 
void _tbcmh_otaupdate_on_chunk_check_timeout(tbcmh_handle_t client, uint64_t timestamp)
{
     TBC_CHECK_PTR(client);

     // Take semaphore
     // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
     //      return;
     // }

     // Search timeout item
     otaupdate_t *request = NULL, *next;
     LIST_FOREACH_SAFE(request, &client->otaupdate_list, entry, next) {
          if (request &&
             (request->state.request_id>0) &&
             (request->state.timestamp>0) &&
             (request->state.timestamp+TB_MQTT_TIMEOUT<=timestamp)) {
                // Deal timeout & abort ota
                _otaupdate_publish_late_failed_status(request, "OTA response timeout!");
                _otaupdate_do_abort(request);
                _otaupdate_reset(request);
                _otaupdate_do_updated(request, false);
          }
     }

     // Give semaphore
     // xSemaphoreGiveRecursive(client->_lock);
}

