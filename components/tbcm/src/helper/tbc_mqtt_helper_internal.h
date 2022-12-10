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

#ifndef _TBC_MQTT_HETLPER_INTERNAL_H_
#define _TBC_MQTT_HETLPER_INTERNAL_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "sys/queue.h"
#include "esp_err.h"

// #include "tbc_utils.h"
#include "tbc_transport_config.h"
#include "tbc_transport_storage.h"

#include "tbc_mqtt_wapper.h"
// #include "tbc_mqtt_helper.h"

#include "telemetry_upload.h"
//#include "client_attribute.h"
//#include "shared_attribute.h"
#include "attributes_update.h"
#include "attributes_subscribe.h"
#include "attributes_request.h"
#include "server_rpc.h"
#include "client_rpc.h"
#include "provision_request.h"
#include "claiming_device.h"
#include "ota_update.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ThingsBoard MQTT Client Helper 
 */
typedef struct tbcmh_client
{
     // TODO: add a lock???
     // create & destroy
     tbcm_handle_t tbmqttclient;
     // bool is_running_in_mqtt_task;           /*!< is these code running in MQTT task? */
     QueueHandle_t _xQueue;

     // modify at connect & disconnect
     tbc_transport_storage_t config;         // TODO: remove it???
     void *context;                          /*!< Context parameter of the below two callbacks */
     tbcmh_on_connected_t on_connected;      /*!< Callback of connected ThingsBoard MQTT */
     tbcmh_on_disconnected_t on_disconnected;/*!< Callback of disconnected ThingsBoard MQTT */

     // tx & rx msg
     SemaphoreHandle_t _lock;
     // timeseriesaxis_list_t   timeseriesaxis_list;      /*!< telemetry time-series data entries */
     // clientattribute_list_t  clientattribute_list;     /*!< client attributes entries */
     attributessubscribe_list_t attributessubscribe_list; /*!< attributes subscreibe entries */
     attributesrequest_list_t   attributesrequest_list;   /*!< attributes request entries */
     serverrpc_list_t serverrpc_list; /*!< server side RPC entries */
     clientrpc_list_t clientrpc_list; /*!< client side RPC entries */
     otaupdate_list_t otaupdate_list; /*!< A device may have multiple firmware */
     provision_list_t deviceprovision_list;     /*!< device provision entries */

     //SemaphoreHandle_t lock;
     uint16_t next_request_id;
     uint64_t last_check_timestamp;
} tbcmh_t;

uint32_t _tbcmh_get_request_id(tbcmh_handle_t client);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

