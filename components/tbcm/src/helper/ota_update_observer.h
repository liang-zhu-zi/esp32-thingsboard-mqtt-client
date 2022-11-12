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

#ifndef _OTA_OBSERBER_H_
#define _OTA_OBSERBER_H_

#include <stdint.h>
#include <stdbool.h>

#include "sys/queue.h"

#include "tbc_mqtt.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//====9.Firmware update================================================================================================
/**
 * ThingsBoard MQTT Client Helper F/W update OTA config for storage
 */
typedef struct tbcmh_otaupdate_storage_config
{
     tbcmh_otaupdate_type_t ota_type; /*!< FW/TBCMH_OTAUPDATE_TYPE_FW or SW/TBCMH_OTAUPDATE_TYPE_SW  */
     int chunk_size;                  /*!< chunk_size, eg: 2048. 0 to get all F/W or S/W by request  */

     void *context;
     tbcmh_otaupdate_on_get_current_ota_title_t   on_get_current_ota_title;     /*!< callback of getting current F/W or S/W OTA title */   // TODO: impl the field! //
     tbcmh_otaupdate_on_get_current_ota_version_t on_get_current_ota_version;   /*!< callback of getting current F/W or S/W OTA version */ // TODO: impl the field! //

     tbcmh_otaupdate_on_negotiate_t on_ota_negotiate;         /*!< callback of F/W or S/W OTA attributes */
     tbcmh_otaupdate_on_write_t on_ota_write;                 /*!< callback of F/W or S/W OTA doing */
     tbcmh_otaupdate_on_end_t on_ota_end;                     /*!< callback of F/W or S/W OTA success & end */
     tbcmh_otaupdate_on_abort_t on_ota_abort;                 /*!< callback of F/W or S/W OTA failure & abort */

     ////bool is_first_boot;            /*!< whether first boot after ota update  */
} tbcmh_otaupdate_config_storage_t;

/**
 * ThingsBoard MQTT Client Helper F/W update OTA attribute
 */
// TODO: it has a value after receiving a shared attributes msg!
typedef struct tbcmh_otaupdate_attribute
{
     char *ota_title;              /*!< fw_title or sw_title  */  // TODO: replace this field with otaupdate->config.on_get_current_ota_title
     char *ota_version;            /*!< fw_version or sw_version  */
     int   ota_size;               /*!< fw_size or sw_size  */
     char *ota_checksum;           /*!< fw_checksum or sw_checksum  */
     char *ota_checksum_algorithm; /*!< fw_checksum_algorithm or sw_checksum_algorithm. only support CRC32  */
} tbcmh_otaupdate_attribute_t;

/**
 * ThingsBoard MQTT Client Helper F/W update OTA state
 */
typedef struct tbcmh_otaupdate_state
{
     int request_id;    /*!< default is -1 */
     int chunk_id;      /*!< default is zero, from 0 to n */
     uint32_t received_len;  /*!< lenth of receiving ota data */
     uint32_t checksum;      /*!< only support CRC32  */          // TODO: support multi-ALG! 
} tbcmh_otaupdate_state_t;

/**
 * ThingsBoard MQTT Client Helper F/W update OTA
 */
typedef struct tbcmh_otaupdate
{
     tbcmh_handle_t client;           /*!< ThingsBoard MQTT Client Helper */
     char *ota_description;           /*!< F/W or S/W descripiton  */
     tbcmh_otaupdate_config_storage_t config;

     // reset these below fields.
     tbcmh_otaupdate_attribute_t attribute;
     tbcmh_otaupdate_state_t state;

     LIST_ENTRY(tbcmh_otaupdate) entry;
} tbcmh_otaupdate_t;

tbcmh_otaupdate_t *_tbcmh_otaupdate_init(tbcmh_handle_t client,  const char *ota_description, const tbcmh_otaupdate_config_t *config); /*!< Initialize tbcmh_otaupdate_t */
tbc_err_t        _tbcmh_otaupdate_destroy(tbcmh_otaupdate_t *otaupdate);                   /*!< Destroys the tbcmh_otaupdate_t */
void               _tbcmh_otaupdate_reset(tbcmh_otaupdate_t *otaupdate);

tbcmh_otaupdate_type_t _tbcmh_otaupdate_get_type(tbcmh_otaupdate_t *otaupdate);
const char*            _tbcmh_otaupdate_get_description(tbcmh_otaupdate_t *otaupdate);
const char*            _tbcmh_otaupdate_get_current_title(tbcmh_otaupdate_t *otaupdate);  
int _tbcmh_otaupdate_get_request_id(tbcmh_otaupdate_t *otaupdate);

void _tbcmh_otaupdate_publish_early_current_version(tbcmh_otaupdate_t *otaupdate);
void _tbcmh_otaupdate_publish_early_failed_status(tbcm_handle_t tbcm_handle, 
                                tbcmh_otaupdate_type_t ota_type, const char *ota_error);
void _tbcmh_otaupdate_publish_late_failed_status(tbcmh_otaupdate_t *otaupdate, const char *ota_error);
void _tbcmh_otaupdate_publish_going_status(tbcmh_otaupdate_t *otaupdate, const char *ota_state);
void _tbcmh_otaupdate_publish_updated_status(tbcmh_otaupdate_t *otaupdate);

tbc_err_t  _tbcmh_otaupdate_request_chunk(tbcmh_otaupdate_t *otaupdate,
                                            tbcm_on_otaupdate_response_t on_otaupdate_response,
                                            tbcm_on_otaupdate_timeout_t on_otaupdate_timeout);
bool _tbcmh_otaupdate_is_received_all(tbcmh_otaupdate_t *otaupdate);
bool _tbcmh_otaupdate_checksum_verification(tbcmh_otaupdate_t *otaupdate);

//return 0 on successful, -1 on failure
tbc_err_t _tbcmh_otaupdate_do_negotiate(tbcmh_otaupdate_t *otaupdate, const char *ota_title, const char *ota_version, int ota_size,
                                         const char *ota_checksum, const char *ota_checksum_algorithm,
                                         char *ota_error, int error_size);
//return 0 on successful, -1 on failure
tbc_err_t _tbcmh_otaupdate_do_write(tbcmh_otaupdate_t *otaupdate, int chunk_id, const void *ota_data, int data_size,
                                         char *ota_error, int error_size);
//return 0 on successful, -1 on failure
tbc_err_t _tbcmh_otaupdate_do_end(tbcmh_otaupdate_t *otaupdate, char *ota_error, int error_size);
void        _tbcmh_otaupdate_do_abort(tbcmh_otaupdate_t *otaupdate);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
