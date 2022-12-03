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

#ifndef _OTA_UPDATE_OBSERBER_H_
#define _OTA_UPDATE_OBSERBER_H_

#include <stdint.h>
#include <stdbool.h>

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//==== Firmware/Software update ===================================================
// Don't call TBCMH API in this callback!
// return 1 on negotiate successful(next to F/W OTA), -1/ESP_FAIL on negotiate failure, 0/ESP_OK on already updated!
typedef tbc_err_t (*otaupdate_on_negotiate_t)(void *context_xw,
                                const char *current_fw_title, const char *current_fw_version,
                                const char *ota_title, const char *ota_version, uint32_t ota_size,
                                const char *ota_checksum, const char *ota_checksum_algorithm,
                                char *ota_error, int error_size);

// Don't call TBCMH API in this callback!
// return 0/ESP_OK on successful, -1/ESP_FAIL on failure
typedef tbc_err_t (*otaupdate_on_write_t)(void *context_xw,  //uint32_t request_id, uint32_t current_chunk_id,
                                const void *ota_data, uint32_t data_size,
                                char *ota_error, int error_size);

// Don't call TBCMH API in this callback!
// return 0/ESP_OK on successful, -1/ESP_FAIL on failure
typedef tbc_err_t (*otaupdate_on_end_t)(void *context_xw,
                                char *ota_error, int error_size); //uint32_t request_id, uint32_t chunk_id,

// Don't call TBCMH API in this callback!
typedef void (*otaupdate_on_abort_t)(void *context_xw); //,uint32_t request_id, uint32_t current_chunk_id

/**
 * ThingsBoard MQTT Client Helper F/W update OTA config
 */
typedef struct otaupdate_config
{
  tbcmh_otaupdate_type_t ota_type; /*!< FW/TBCMH_OTAUPDATE_TYPE_FW or SW/TBCMH_OTAUPDATE_TYPE_SW  */
  uint32_t chunk_size;             /*!< chunk_size, eg: 8192. 0 to get all F/W or S/W by request  */

  void *context_user;
  tbcmh_otaupdate_on_get_current_title_t on_get_current_title;     /*!< callback of getting current F/W or S/W OTA title */
  tbcmh_otaupdate_on_get_current_version_t on_get_current_version; /*!< callback of getting current F/W or S/W OTA version */
  tbcmh_otaupdate_on_updated_t on_ota_updated;     /*!< callback of F/W or S/W OTA updated/done */

  void *context_xw;                          /*!< context of F/W or S/W */
  otaupdate_on_negotiate_t on_ota_negotiate; /*!< callback of F/W or S/W OTA attributes */
  otaupdate_on_write_t on_ota_write;         /*!< callback of F/W or S/W OTA doing */
  otaupdate_on_end_t on_ota_end;             /*!< callback of F/W or S/W OTA success & end*/
  otaupdate_on_abort_t on_ota_abort;         /*!< callback of F/W or S/W OTA failure & abort */

  ////bool is_first_boot;            /*!< whether first boot after ota update  */
} otaupdate_config_t;

/**
 * ThingsBoard MQTT Client Helper F/W update OTA attribute
 */
// TODO: it has a value after receiving a shared attributes msg!
typedef struct tbcmh_otaupdate_attribute
{
     char *ota_title;              /*!< fw_title or sw_title  */  // TODO: replace this field with otaupdate->config.on_get_current_ota_title
     char *ota_version;            /*!< fw_version or sw_version  */
     char *ota_checksum;           /*!< fw_checksum or sw_checksum  */
     char *ota_checksum_algorithm; /*!< fw_checksum_algorithm or sw_checksum_algorithm. only support CRC32  */
     uint32_t ota_size;            /*!< fw_size or sw_size  */
} otaupdate_attribute_t;

/**
 * ThingsBoard MQTT Client Helper F/W update OTA state
 */
typedef struct otaupdate_state
{
     uint32_t request_id;    /*!< default is 0 */ //-1

     uint32_t received_len;  /*!< lenth of receiving ota data */
     uint32_t checksum;      /*!< only support CRC32  */          // TODO: support multi-ALG! 

     uint32_t chunk_id;      /*!< default is zero, from 0 to n */
     uint64_t timestamp;     /*!< time stamp at sending request */
} otaupdate_state_t;

/**
 * ThingsBoard MQTT Client Helper F/W update OTA
 */
typedef struct otaupdate
{
     tbcmh_handle_t client;           /*!< ThingsBoard MQTT Client Helper */
     char *ota_description;           /*!< F/W or S/W descripiton  */
     otaupdate_config_t config;

     // reset these below fields.
     otaupdate_attribute_t attribute;
     otaupdate_state_t state;

     LIST_ENTRY(otaupdate) entry;
} otaupdate_t;

typedef LIST_HEAD(tbcmh_otaupdate_list, otaupdate) otaupdate_list_t;

void _tbcmh_otaupdate_on_create(tbcmh_handle_t client);
void _tbcmh_otaupdate_on_destroy(tbcmh_handle_t client);
void _tbcmh_otaupdate_on_connected(tbcmh_handle_t client);
void _tbcmh_otaupdate_on_disconnected(tbcmh_handle_t client);
void _tbcmh_otaupdate_on_chunk_data(tbcmh_handle_t client, uint32_t request_id, uint32_t chunk_id, const char* payload, int length);
void _tbcmh_otaupdate_on_chunk_check_timeout(tbcmh_handle_t client, uint64_t timestamp);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
