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

#ifndef _FW_OBSERBER_H_
#define _FW_OBSERBER_H_

#include <stdint.h>
#include <stdbool.h>

#include "sys/queue.h"
#include "tb_mqtt_client_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//====9.Firmware update================================================================================================
/**
 * ThingsBoard MQTT Client Helper F/W update OTA
 */
typedef struct tbmch_fwupdate
{
     tbmch_handle_t client; /*!< ThingsBoard MQTT Client Helper */
     char *fw_title;

     void *context;
     tbmch_fwupdate_on_sharedattributes_t on_fw_attributes; /*!< callback of F/W OTA attributes */
     tbmch_fwupdate_on_response_t on_fw_response;           /*!< callback of F/W OTA doing */
     tbmch_fwupdate_on_success_t on_fw_success;             /*!< callback of F/W OTA success */
     tbmch_fwupdate_on_timeout_t on_fw_timeout;             /*!< callback of F/W OTA timeout */

     // reset these below fields.
     char *fw_version;
     char *fw_checksum;
     char *fw_checksum_algorithm;

     int request_id; /*!< default is -1 */
     int chunk;      /*!< default is zero, from 0 to  */
     uint32_t checksum; 

     LIST_ENTRY(tbmch_fwupdate) entry;
} tbmch_fwupdate_t;

tbmch_fwupdate_t *_tbmch_fwupdate_init(tbmch_handle_t client, const char *fw_title,
                                       void *context,
                                       tbmch_fwupdate_on_sharedattributes_t on_fw_attributes,
                                       tbmch_fwupdate_on_response_t on_fw_response,
                                       tbmch_fwupdate_on_success_t on_fw_success,
                                       tbmch_fwupdate_on_timeout_t on_fw_timeout); /*!< Initialize tbmch_fwupdate_t */
tbmch_err_t _tbmch_fwupdate_destroy(tbmch_fwupdate_t *fwupdate);                   /*!< Destroys the tbmch_fwupdate_t */
////tbmch_err_t __tbmch_fwupdate_reset(tbmch_fwupdate_t *fwupdate);

const char *_tbmch_fwupdate_get_title(tbmch_fwupdate_t *fwupdate);  
int _tbmch_fwupdate_get_request_id(tbmch_fwupdate_t *fwupdate);

bool _tbmch_fwupdate_do_sharedattributes(tbmch_fwupdate_t *fwupdate, const char *fw_title, const char *fw_version,
                                         const char *fw_checksum, const char *fw_checksum_algorithm); //save fw_..., exec fw_update

tbmch_err_t _tbmch_fwupdate_set_request_id(tbmch_fwupdate_t *fwupdate, int request_id);

//return -1: end & failure;  0: end & success;  1: go on, get next package
tbmch_err_t _tbmch_fwupdate_do_response(tbmch_fwupdate_t *fwupdate, int chunk /*current chunk*/, const void *fw_data, int data_size); //on_response or on_success or on_timeout(failure), and reset()
////void _tbmch_fwupdate_do_success(tbmch_fwupdate_t *fwupdate, int chunk /*total_size*/);
void _tbmch_fwupdate_do_timeout(tbmch_fwupdate_t *fwupdate); // on_timeout() and reset()

//0.  Subscribe topic: shared attribute updates: fw_title, fw_version, fw_checksum, fw_checksum_algorithm 
//0.  Subscribe topic: f/w response: v2/fw/response/+/chunk/+

//1.   tbmch_fwupdate_append();
//1.1  if (fw_title is not in list fw_observer_entries) 
//     then tbmch_fwupdate_t *_tbmch_fwupdate_create(...);

//2.   if (all of fw shared attributes updated)
//2.1  if (on_fw_attributes() returns true)
//2.2  tbmc._tbmch_fw_observer(tbmch_handle_t client, tbmch_fwupdate_t *fw_request, ...); //tbmqttclient_fw_request(...)

//3.   response success: tbmc._tbmch_on_fw_response(...)
//3.1  if (chunk_size >0 ) on_fw_chunk(...): save fw chunk
//3.2  else if (chunk_size ==0 ) on_fw_success(...); _tbmch_fwupdate_reset(...);

//4.   response timeout
//4.1  on_fw_timeout(...); _tbmch_fwupdate_reset(...);

//5    tbmch_fwupdate_clear() / tbmch_client_destroy(...)
//5.x   tbmch_err_t _tbmch_fwupdate_destroy(tbmch_fwupdate_t *fw_request)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif