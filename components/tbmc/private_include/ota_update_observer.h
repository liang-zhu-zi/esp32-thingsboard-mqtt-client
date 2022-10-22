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

#ifndef _OTA_OBSERBER_H_
#define _OTA_OBSERBER_H_

#include <stdint.h>
#include <stdbool.h>

#include "sys/queue.h"

#include "tb_mqtt_client.h"
#include "tb_mqtt_client_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//====9.Firmware update================================================================================================
/**
 * ThingsBoard MQTT Client Helper F/W update OTA config for storage
 */
typedef struct tbmch_otaupdate_storage_config
{
     tbmch_otaupdate_type_t ota_type; /*!< FW/TBMCH_OTAUPDATE_TYPE_FW or SW/TBMCH_OTAUPDATE_TYPE_SW  */
     int chunk_size;                  /*!< chunk_size, eg: 2048. 0 to get all F/W or S/W by request  */

     void *context;
     tbmch_otaupdate_on_get_current_ota_title_t   on_get_current_ota_title;     /*!< callback of getting current F/W or S/W OTA title */   // TODO: impl the field! //
     tbmch_otaupdate_on_get_current_ota_version_t on_get_current_ota_version;   /*!< callback of getting current F/W or S/W OTA version */ // TODO: impl the field! //

     tbmch_otaupdate_on_negotiate_t on_ota_negotiate;         /*!< callback of F/W or S/W OTA attributes */
     tbmch_otaupdate_on_write_t on_ota_write;                 /*!< callback of F/W or S/W OTA doing */
     tbmch_otaupdate_on_end_t on_ota_end;                     /*!< callback of F/W or S/W OTA success & end */
     tbmch_otaupdate_on_abort_t on_ota_abort;                 /*!< callback of F/W or S/W OTA failure & abort */

     ////bool is_first_boot;            /*!< whether first boot after ota update  */
} tbmch_otaupdate_config_storage_t;

/**
 * ThingsBoard MQTT Client Helper F/W update OTA attribute
 */
// TODO: it has a value after receiving a shared attributes msg!
typedef struct tbmch_otaupdate_attribute
{
     char *ota_title;              /*!< fw_title or sw_title  */  // TODO: replace this field with otaupdate->config.on_get_current_ota_title
     char *ota_version;            /*!< fw_version or sw_version  */
     int   ota_size;               /*!< fw_size or sw_size  */
     char *ota_checksum;           /*!< fw_checksum or sw_checksum  */
     char *ota_checksum_algorithm; /*!< fw_checksum_algorithm or sw_checksum_algorithm. only support CRC32  */
} tbmch_otaupdate_attribute_t;

/**
 * ThingsBoard MQTT Client Helper F/W update OTA state
 */
typedef struct tbmch_otaupdate_state
{
     int request_id;    /*!< default is -1 */
     int chunk_id;      /*!< default is zero, from 0 to n */
     uint32_t received_len;  /*!< lenth of receiving ota data */
     uint32_t checksum;      /*!< only support CRC32  */          // TODO: support multi-ALG! 
} tbmch_otaupdate_state_t;

/**
 * ThingsBoard MQTT Client Helper F/W update OTA
 */
typedef struct tbmch_otaupdate
{
     tbmch_handle_t client;           /*!< ThingsBoard MQTT Client Helper */
     char *ota_description;           /*!< F/W or S/W descripiton  */
     tbmch_otaupdate_config_storage_t config;

     // reset these below fields.
     tbmch_otaupdate_attribute_t attribute;
     tbmch_otaupdate_state_t state;

     LIST_ENTRY(tbmch_otaupdate) entry;
} tbmch_otaupdate_t;

tbmch_otaupdate_t *_tbmch_otaupdate_init(tbmch_handle_t client,  const char *ota_description, const tbmch_otaupdate_config_t *config); /*!< Initialize tbmch_otaupdate_t */
tbmch_err_t        _tbmch_otaupdate_destroy(tbmch_otaupdate_t *otaupdate);                   /*!< Destroys the tbmch_otaupdate_t */
void               _tbmch_otaupdate_reset(tbmch_otaupdate_t *otaupdate);

tbmch_otaupdate_type_t _tbmch_otaupdate_get_type(tbmch_otaupdate_t *otaupdate);
const char*            _tbmch_otaupdate_get_description(tbmch_otaupdate_t *otaupdate);
const char*            _tbmch_otaupdate_get_current_title(tbmch_otaupdate_t *otaupdate);  
int _tbmch_otaupdate_get_request_id(tbmch_otaupdate_t *otaupdate);

void _tbmch_otaupdate_publish_early_current_version(tbmch_otaupdate_t *otaupdate);
void _tbmch_otaupdate_publish_early_failed_status(tbmc_handle_t tbmc_handle, 
                                tbmch_otaupdate_type_t ota_type, const char *ota_error);
void _tbmch_otaupdate_publish_late_failed_status(tbmch_otaupdate_t *otaupdate, const char *ota_error);
void _tbmch_otaupdate_publish_going_status(tbmch_otaupdate_t *otaupdate, const char *ota_state);
void _tbmch_otaupdate_publish_updated_status(tbmch_otaupdate_t *otaupdate);

tbmch_err_t  _tbmch_otaupdate_request_chunk(tbmch_otaupdate_t *otaupdate,
                                            tbmc_on_otaupdate_response_t on_otaupdate_response,
                                            tbmc_on_otaupdate_timeout_t on_otaupdate_timeout);
bool _tbmch_otaupdate_is_received_all(tbmch_otaupdate_t *otaupdate);
bool _tbmch_otaupdate_checksum_verification(tbmch_otaupdate_t *otaupdate);

//return 0 on successful, -1 on failure
tbmch_err_t _tbmch_otaupdate_do_negotiate(tbmch_otaupdate_t *otaupdate, const char *ota_title, const char *ota_version, int ota_size,
                                         const char *ota_checksum, const char *ota_checksum_algorithm,
                                         char *ota_error, int error_size);
//return 0 on successful, -1 on failure
tbmch_err_t _tbmch_otaupdate_do_write(tbmch_otaupdate_t *otaupdate, int chunk_id, const void *ota_data, int data_size,
                                         char *ota_error, int error_size);
//return 0 on successful, -1 on failure
tbmch_err_t _tbmch_otaupdate_do_end(tbmch_otaupdate_t *otaupdate, char *ota_error, int error_size);
void        _tbmch_otaupdate_do_abort(tbmch_otaupdate_t *otaupdate);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
