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

#include "tbc_mqtt.h"
// #include "tbc_mqtt_helper.h"

#include "timeseries_data.h"
#include "client_attribute.h"
#include "shared_attribute.h"
#include "attributes_request.h"
#include "server_rpc.h"
#include "client_rpc.h"
#include "device_provision.h"
#include "claiming_device.h"
#include "ota_update.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
     TBCMH_REQUEST_ATTRIBUTES = 1,
     TBCMH_REQUEST_CLIENTRPC,
     TBCMH_REQUEST_FWUPDATE,
     TBCMH_REQUEST_PROVISION
} tbcmh_request_type_t;

typedef struct tbcmh_request
{
     tbcmh_request_type_t type;
     int request_id;
     uint64_t timestamp; /*!< time stamp at sending request */
     LIST_ENTRY(tbcmh_request) entry;
} tbcmh_request_t;

typedef LIST_HEAD(tbcmh_request_list, tbcmh_request) tbcmh_request_list_t;

/**
 * ThingsBoard MQTT Client Helper 
 */
typedef struct tbcmh_client
{
     // TODO: add a lock???
     // create & destroy
     tbcm_handle_t tbmqttclient;
     bool is_running_in_mqtt_task;           /*!< is these code running in MQTT task? */
     QueueHandle_t _xQueue;

     // modify at connect & disconnect
     uint32_t function;                      /*!< function modules used. TBCMH_FUNCTION_FULL_GENERAL, ... */
     tbc_transport_storage_t config;         // TODO: remove it???
     void *context;                          /*!< Context parameter of the below two callbacks */
     tbcmh_on_connected_t on_connected;      /*!< Callback of connected ThingsBoard MQTT */
     tbcmh_on_disconnected_t on_disconnected;/*!< Callback of disconnected ThingsBoard MQTT */

     // tx & rx msg
     SemaphoreHandle_t _lock;
     LIST_HEAD(tbcmh_tsdata_list, timeseries_data) tsdata_list;                              /*!< telemetry time-series data entries */
     LIST_HEAD(tbcmh_clientattribute_list, client_attribute) clientattribute_list;   /*!< client attributes entries */
     LIST_HEAD(tbcmh_sharedattribute_list, shared_attribute) sharedattribute_list;   /*!< shared attributes entries */
     LIST_HEAD(tbcmh_attributesrequest_list, attributes_request) attributesrequest_list;  /*!< attributes request entries */
     LIST_HEAD(tbcmh_serverrpc_list, server_rpc) serverrpc_list;  /*!< server side RPC entries */
     LIST_HEAD(tbcmh_clientrpc_list, client_rpc) clientrpc_list;  /*!< client side RPC entries */
     LIST_HEAD(tbcmh_provision_list, device_provision) provision_list;  /*!< provision entries */
     LIST_HEAD(tbcmh_otaupdate_list, ota_update) otaupdate_list;    /*!< A device may have multiple firmware */

     //SemaphoreHandle_t lock;
     int next_request_id;
     uint64_t last_check_timestamp;
     tbcmh_request_list_t request_list;   /*!< request list: attributes request, client side RPC & ota update request */
} tbcmh_t;

bool _request_is_equal(const tbcmh_request_t *a, const tbcmh_request_t *b);
int  _request_list_create_and_append(tbcmh_handle_t client, tbcmh_request_type_t type, int request_id);
void _request_list_search_and_remove(tbcmh_handle_t client, int request_id);
void _request_list_search_and_remove_by_type(tbcmh_handle_t client, tbcmh_request_type_t type);
int  _request_list_move_all_of_timeout(tbcmh_handle_t client, uint64_t timestamp,
                                             tbcmh_request_list_t *timeout_request_list);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

