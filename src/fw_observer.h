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

// This file is called by tbmc_help.c/.h.

#ifndef _FW_OBSERBER_H_
#define _FW_OBSERBER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//====9.Firmware update================================================================================================
/**
 * ThingsBoard MQTT Client F/W update OTA
 */
typedef struct tbmc_fw_observer_
{
     void *context;
     tbmc_fw_shared_attributes_callback_t on_fw_attributes; /*!< callback of F/W OTA attributes */
     tbmc_fw_response_chunk_callback_t on_fw_chunk;         /*!< callback of F/W OTA doing */
     tbmc_fw_response_success_callback_t on_fw_success;     /*!< callback of F/W OTA success */
     tbmc_fw_response_timeout_callback_t on_fw_timeout;     /*!< callback of F/W OTA timeout */

     // reset these below fields.
     tbmc_attribute_handle_t fw_title;
     tbmc_attribute_handle_t fw_version;
     tbmc_attribute_handle_t fw_checksum;
     tbmc_attribute_handle_t fw_checksum_algorithm;
     // const char *fw_title;                                  /*!< OS fw, App fw, ... */
     // const char *fw_version;
     // const char *fw_checksum;
     // const char *fw_checksum_algorithm;

     int request_id; /*!< default is 0 */
     int chunk;      /*!< default is zero, from 0 to  */
} tbmc_fw_observer_t;
typedef tbmc_fw_observer_t *tbmc_fw_observer_handle_t;

//0.  Subscribe topic: shared attribute updates: fw_title, fw_version, fw_checksum, fw_checksum_algorithm 
//0.  Subscribe topic: f/w response: v2/fw/response/+/chunk/+

//1.   tbmc_fw_observer_append();
//1.1  if (fw_title is not in list fw_observer_entries) 
//     then tbmc_fw_observer_handle_t _tbmc_fw_observer_create(...);

//2.   if (all of fw shared attributes updated)
//2.1  if (on_fw_attributes() returns true)
//2.2  tbmc._tbmc_fw_observer(tbmc_handle_client_t client, tbmc_fw_observer_handle_t fw_request, ...); //tbmqttclient_fw_request(...)

//3.   response success: tbmc._tbmc_on_fw_response(...)
//3.1  if (chunk_size >0 ) on_fw_chunk(...): save fw chunk
//3.2  else if (chunk_size ==0 ) on_fw_success(...); _tbmc_fw_observer_reset(...);

//4.   response timeout
//4.1  on_fw_timeout(...); _tbmc_fw_observer_reset(...);

//5    tbmc_fw_observer_clear() / tbmc_client_destory(...)
//5.x   esp_err_t _tbmc_fw_observer_destory(tbmc_fw_observer_handle_t fw_request)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif