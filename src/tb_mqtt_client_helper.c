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

#include <string.h>

#include "tb_mqtt_client_helper.h"
#include "timeseries_data_helper.h"
#include "client_attribute_helper.h"
#include "shared_attribute_observer.h"
#include "attributes_request_observer.h"
#include "server_rpc_observer.h"
#include "client_rpc_observer.h"
#include "fw_update_observer.h"


/**
 * ThingsBoard MQTT Client Helper message id
 */
typedef enum
{                                  //param1   param2        param3    param4
  TBMCH_MSGID_TIMEOUT = 0,         //context
  TBMCH_MSGID_DISCONNECTED,        //context
  TBMCH_MSGID_CONNECTED,           //context     
  TBMCH_MSGID_ATTRREQUEST_SUCCESS, //context, request_id,   cJSON
  TBMCH_MSGID_ATTRREQUEST_TIMEOUT, //context, request_id
  TBMCH_MSGID_SHAREDATTR,          //context, request_id,   cJSON
  TBMCH_MSGID_CLIENTRPC_SUCCESS,   //context, request_id,   cJSON
  TBMCH_MSGID_CLIENTRPC_TIMEOUT,   //context, request_id
  TBMCH_MSGID_SERVERRPC_REQUSET,   //context, request_id,   cJSON
  TBMCH_MSGID_FWREQUEST_SUCCESS,   //context, request_id,   payload,  len
  TBMCH_MSGID_FWREQUEST_TIMEOUT,   //context, request_id
} tbmch_msgid_t;

/**
 * ThingsBoard MQTT Client Helper 
 */
typedef struct tbmch_client_
{
     struct
     {
          char *uri;          /*!< ThingsBoard MQTT host uri */
          char *access_token; /*!< ThingsBoard MQTT token */

          void *context;
          tbmch_on_connected_t on_connected;       /*!< Callback of connected ThingsBoard MQTT */
          tbmch_on_disconnected_t on_disconnected; /*!< Callback of disconnected ThingsBoard MQTT */
     } config;

     tbmqttclient_handle_t tbmqttclient; // TODO:
     QueueHandle_t _xQueue;              // TODO:

     SemaphoreHandle_t _lock;            // TODO:
     LIST_HEAD(tbmch_tsdata_list, tbmch_tsdata) tsdata_list;                              /*!< telemetry time-series data entries */
     LIST_HEAD(tbmch_clientattribute_list, tbmch_clientattribute) clientattribute_list;   /*!< client attributes entries */
     LIST_HEAD(tbmch_sharedattribute_list, tbmch_sharedattribute) sharedattribute_list;   /*!< shared attributes entries */
     LIST_HEAD(tbmch_attributesrequest_list, tbmch_attributesrequest) attributesrequest_list;  /*!< attributes request entries */
     LIST_HEAD(tbmch_serverrpc_list, tbmch_serverrpc) serverrpc_list;  /*!< server side RPC entries */
     LIST_HEAD(tbmch_clientrpc_list, tbmch_clientrpc) clientrpc_list;  /*!< client side RPC entries */
     LIST_HEAD(tbmch_fwupdate_list, tbmch_fwupdate) fwupdate_list;    /*!< A device may have multiple firmware */
} tbmch_t;

//~~static int _tbmch_sendServerRpcReply(tbmch_handle_t client_, int request_id, const char* response, int qos=1, int retain=0); //sendServerRpcReply()

static bool _tbmch_sendTbmqttMsg2Queue(tbmch_handle_t client_, TbmqttInnerMsgType type, cJSON *payload); //_sendTbmqttInnerMsg2Queue()
//static bool _tbmch_tbDecodeAttributesJsonPayload(JsonObject& attr_kvs); //_tbDecodeAttributesJsonPayload()

static void _tbmch_on_connected(tbmch_handle_t client_); //onConnected()
static void _tbmch_on_disonnected(tbmch_handle_t client_); //onDisonnected()
static void _tbmch_on_sharedattributes_received(tbmch_handle_t client_, const char* payload, int length); //onAttrOfSubReply();
static void _tbmch_on_attributesrequest_success(tbmch_handle_t client_, int request_id, const char* payload, int length); //onAttributesResponse()=>_attributesResponse()
static void _tbmch_on_attributesrequest_timeout(tbmch_handle_t client_, int request_id); //onAttributesResponseTimeout()
//~~static bool _attributesResponse(int request_id, const char* payload, int length); //merge to _tbmch_on_attributesrequest_success()
static void _tbmch_on_clientrpc_success(tbmch_handle_t client_, int request_id, const char* payload, int length); //onClientRpcResponse()
static void _tbmch_on_clientrpc_timeout(tbmch_handle_t client_, int request_id); //onClientRpcResponseTimeout()
static void _tbmch_on_serverrpc_request(tbmch_handle_t client_, int request_id, const char* payload, int length); ////onServerRpcRequest()
static void _tbmch_on_fwrequest_response(tbmch_handle_t client_, int request_id, int chunk, const char* payload, int length);
static void _tbmch_on_fwrequest_timeout(tbmch_handle_t client_, int request_id);

static void _timer_start();
static void _timer_stop();
static void _timer_timerout(); //send msg to queue