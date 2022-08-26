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

// ThingsBoard MQTT Client low layer API

#include "tb_mqtt_client.h"

/**
 * ThingsBoard MQTT Client
 */
typedef struct tbmc_client_
{
     //char *uri;          /*!< ThingsBoard MQTT host uri */
     //char *access_token; /*!< ThingsBoard MQTT token */
     tbmc_config_t config; //esp_mqtt_client_config_t config;

     void *context;
     tbmc_on_connected_t on_connected;                     /*!< Callback of connected ThingsBoard MQTT */
     tbmc_on_disconnected_t on_disconnected;               /*!< Callback of disconnected ThingsBoard MQTT */
     tbmc_on_sharedattr_received_t on_sharedattr_received; /*!< Callback of receiving hingsBoard MQTT shared-attribute T*/
     tbmc_on_serverrpc_request_t on_serverrpc_request;     /*!< Callback of receiving ThingsBoard MQTT server-RPC request */

     esp_mqtt_client_handle_t client;
     tbmc_state_t state;  // TBMQTT_STATE state;
     SemaphoreHandle_t lock;

     struct
     {
          LIST_ENTRY(tbmc_attributesrequest_) entries; /*!< attributes request entries */
          int next_request_id;
     } attributes_requests;
     struct
     {
          LIST_ENTRY(tbmc_clientrpcrequest_) entries; /*!< client side RPC entries */
          int next_request_id;
     } clientrpc_requests;
     struct
     {
          LIST_ENTRY(tbmc_fwrequest_) entries; /*!< A device may have multiple firmware */
          int next_request_id;
     } firmware_requests;
     ////QueueHandle_t timeoutQueue;
} tbmc_t;

static int _tbmc_subscribe(tbmc_handle_t client, const char* topic, const char* postfix, int qos=0); //subscribe()
static int _tbmc_publish(tbmc_handle_t client, const char *topic, const char *payload, int qos, int retain); //publish()

// TODO:
static void _tbmc_on_DataEventProcess(tbmc_handle_t client, esp_mqtt_event_handle_t event); //_onDataEventProcess(); //MQTT_EVENT_DATA
static esp_err_t  _tbmc_on_MqttEventHandle(tbmc_handle_t client, esp_mqtt_event_handle_t event); //_onMqttEventHandle();  //MQTT_EVENT_...
static esp_err_t  _tbmc_on_MqttEventCallback(tbmc_handle_t client, sp_mqtt_event_handle_t event); //_onMqttEventCallback();
