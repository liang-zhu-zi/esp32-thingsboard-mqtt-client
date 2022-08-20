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

// ThingsBoard MQTT Client high layer API

#include "tbmc_helper.h"

/**
 * ThingsBoard MQTT Client
 */
typedef struct tbmc_client_
{
     struct
     {
          char *uri;          /*!< ThingsBoard MQTT host uri */
          char *access_token; /*!< ThingsBoard MQTT token */

          void *context;
          tbmc_on_connected_t on_connected;       /*!< Callback of connected ThingsBoard MQTT */
          tbmc_on_disconnected_t on_disconnected; /*!< Callback of disconnected ThingsBoard MQTT */
     } config;

     tbmqttclient_handle_t tbmqttclient; // TODO:
     QueueHandle_t _xQueue;              // TODO:
     SemaphoreHandle_t _lock;            // TODO:

     struct
     {
          LIST_ENTRY(tbmc_datapoint_) helper_entries; /*!< telemetry data point entries */
     } datapoints;
     struct
     {
          LIST_ENTRY(tbmc_clientattribute_) helper_entries; /*!< client attributes entries */
     } clientattributes;
     struct
     {
          LIST_ENTRY(tbmc_sharedattribute_) observer_entries; /*!< shared attributes entries */
     } sharedattributes;
     struct
     {
          LIST_ENTRY(tbmc_attributesrequest_) observer_entries; /*!< attributes request entries */
          //int next_request_id;
     } attributesrequests;
     struct
     {
          LIST_ENTRY(tbmc_serverrpc_) observer_entries; /*!< server side RPC entries */
     } serverrpcs;
     struct
     {
          LIST_ENTRY(tbmc_clientrpc_) observer_entries; /*!< client side RPC entries */
          //int next_request_id;
     } clientrpcs;
     struct
     {
          LIST_ENTRY(tbmc_fw_observer_) observer_entries; /*!< A device may have multiple firmware */
          //int next_request_id;
     } firmwares;
} tbmc_client_t;

//~~static int _tbmc_sendServerRpcReply(tbmc_client_handle_t client, int request_id, const char* response, int qos=1, int retain=0); //sendServerRpcReply()

static bool _tbmc_sendTbmqttMsg2Queue(tbmc_client_handle_t client, TbmqttInnerMsgType type, cJSON *payload); //_sendTbmqttInnerMsg2Queue()
//static bool _tbmc_tbDecodeAttributesJsonPayload(JsonObject& attr_kvs); //_tbDecodeAttributesJsonPayload()

static void _tbmc_on_connected(tbmc_client_handle_t client); //onConnected()
static void _tbmc_on_disonnected(tbmc_client_handle_t client); //onDisonnected()
static void _tbmc_on_sharedattributes_received(tbmc_client_handle_t client, const char* payload, int length); //onAttrOfSubReply();
static void _tbmc_on_attributesrequest_success(tbmc_client_handle_t client, int request_id, const char* payload, int length); //onAttributesResponse()=>_attributesResponse()
static void _tbmc_on_attributesrequest_timeout(tbmc_client_handle_t client, int request_id); //onAttributesResponseTimeout()
//~~static bool _attributesResponse(int request_id, const char* payload, int length); //merge to _tbmc_on_attributesrequest_success()
static void _tbmc_on_clientrpc_success(tbmc_client_handle_t client, int request_id, const char* payload, int length); //onClientRpcResponse()
static void _tbmc_on_clientrpc_timeout(tbmc_client_handle_t client, int request_id); //onClientRpcResponseTimeout()
static void _tbmc_on_serverrpc_request(tbmc_client_handle_t client, int request_id,const char* payload, int length); ////onServerRpcRequest()
static void _tbmc_on_fwrequest_response(tbmc_client_handle_t client, int request_id, const char* payload, int length);
static void _tbmc_on_fwrequest_timeout(tbmc_client_handle_t client, int request_id);