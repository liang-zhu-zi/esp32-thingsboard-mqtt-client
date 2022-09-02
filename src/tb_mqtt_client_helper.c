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
#include <stdarg.h>

#include "tb_mqtt_client_helper.h"
#include "timeseries_data_helper.h"
#include "client_attribute_helper.h"
#include "shared_attribute_observer.h"
#include "attributes_request_observer.h"
#include "server_rpc_observer.h"
#include "client_rpc_observer.h"
#include "fw_update_observer.h"

//#define TBMCHLOG_LONG

#define TBMCHLOG_LEVLE (4)               //ERR:1, WARN:2, INFO:3, DBG:4, OBSERVES:5

#if (TBMCHLOG_LEVLE>=1)
#define TBMCHLOG_E(format, ...)  printf("[TBMC][E] " format "\r\n", ##__VA_ARGS__)
#else 
#define TBMCHLOG_E
#endif
#if (TBMCHLOG_LEVLE>=2)
#define TBMCHLOG_W(format, ...)  printf("[TBMC][W] " format "\r\n", ##__VA_ARGS__)
#else 
#define TBMCHLOG_W
#endif
#if (TBMCHLOG_LEVLE>=3)
#define TBMCHLOG_I(format, ...)  printf("[TBMC][I] " format "\r\n", ##__VA_ARGS__)
#else 
#define TBMCHLOG_I
#endif
#if (TBMCHLOG_LEVLE>=4)
#define TBMCHLOG_D(format, ...)  printf("[TBMC][D] " format "\r\n", ##__VA_ARGS__)
#else 
#define TBMCHLOG_D
#endif
#if (TBMCHLOG_LEVLE>=5)
#define TBMCHLOG_V(format, ...)  printf("[TBMC][V] " format "\r\n", ##__VA_ARGS__)
#else 
#define TBMCHLOG_V
#endif

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


//====0.tbmc client====================================================================================================
tbmch_handle_t tbmch_init(void)
{
     // TODO:
}
void tbmch_destroy(tbmch_handle_t client_)
{
     // TODO:
}
//~~tbmch_config(); //move to tbmch_connect()
//~~tbmch_set_ConnectedEvent(evtConnected); //move to tbmch_init()
//~~tbmch_set_DisconnectedEvent(evtDisconnected); //move to tbmch_init()
bool tbmch_connect(tbmch_handle_t client_, const char *uri, const char *token,
                   void *context,
                   tbmch_on_connected_t on_connected,
                   tbmch_on_disconnected_t on_disconnected) //_begin();
{
     // TODO:
}
void tbmch_disconnect(tbmch_handle_t client_)               //_end();
{
     // TODO:
}
bool tbmch_is_connected(tbmch_handle_t client_)
{
     // TODO:
}
bool tbmch_has_events(tbmch_handle_t client_) // new function
{
     // TODO:
}
void tbmch_run(tbmch_handle_t client_)        //_recv()=>recvFromLink()=>parse() //tb_mqtt_client_loop()/checkTimeout(), recv/parse/sendqueue/ack...
{
     // TODO:
}

//====1.Publish Telemetry time-series data==============================================================================
tbmch_err_t tbmch_telemetry_append(tbmch_handle_t client_, const char *key, void *context, tbmch_tsdata_on_get_t on_get)
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCHLOG_E("client is NULL");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCHLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Create tsdata
     tbmch_tsdata_t *tsdata = _tbmch_tsdata_init(key/*, type*/, context, on_get/*, on_set*/);
     if (!tsdata) {
          // Give semaphore
          xSemaphoreGive(client->lock);
          TBMCHLOG_E("Init tsdata failure! key=%s", key);
          return ESP_FAIL;
     }

     // Insert tsdata to list
     tbmch_tsdata_t *it, last = NULL;
     if (LIST_FIRST(&client->tsdata_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->tsdata_list, tsdata, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->tsdata_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, tsdata, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->lock);
     return ESP_OK;
}
tbmch_err_t tbmch_telemetry_clear(tbmch_handle_t client_, const char *key)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !key) {
          TBMCLOG_E("client or key is NULL!");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Search item
     tbmch_tsdata_t *tsdata = NULL;
     LIST_FOREACH(tsdata, &client->tsdata_list, entry) {
          if (tsdata && strcmp(_tbmch_tsdata_get_key(tsdata), key)==0) {
               break;
          }
     }

     // Remove form list
     if (tsdata) {
          LIST_REMOVE(tsdata, entry);
          _tbmch_tsdata_destroy(tsdata);
     } else {
          TBMCLOG_W("Unable to remove time-series data:%s!", key);
     }

     // Give semaphore
     xSemaphoreGive(client->lock);
     return ESP_OK;
}

tbmch_err_t tbmch_telemetry_send(tbmch_handle_t client_, int count, /*const char *key,*/ ...) ////tbmqttlink.h.tbmch_sendTelemetry();
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return ESP_FAIL;
     }
     if (count <= 0) {
          TBMCLOG_E("count(%d) is error!", count);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     int i;
     va_list ap;
     va_start(ap, count);
     cJSON *object = cJSON_CreateObject(); // create json object
     for (i=0; i<count; i++) {
          const char *key = va_arg(ap, (const char*));

          // Search item
          tbmch_tsdata_t *tsdata = NULL;
          LIST_FOREACH(tsdata, &client->tsdata_list, entry) {
               if (tsdata && strcmp(_tbmch_tsdata_get_key(tsdata), key)==0) {
                    break;
               }
          }

          /// Add tsdata to package
          if (tsdata) {
               _tbmch_tsdata_value_to_pack(client, tsdata, object); // add item to json object
          } else {
               TBMCLOG_W("Unable to remove time-series data:%s!", key);
          }
     }
     va_end(ap);

     // send package...
     char *pack = cJSON_Print(object);
     int result = tbmc_telemetry_publish(client->tbmqttclient, pack, 1/*qos*/, 0/*retain*/);
     cJSON_Delete(object); // delete json object

     // Give semaphore
     xSemaphoreGive(client->lock);
     return (result > -1) ? ESP_OK : ESP_FAIL;
}

//====2.Publish client-side device attributes to the server============================================================
tbmch_err_t tbmch_clientattribute_append(tbmch_handle_t client_, const char *key, void *context,
                                         tbmch_clientattribute_on_get_t on_get)
{
     return _tbmch_clientattribute_xx_append(client_, key, context, on_get, NULL);
}
tbmch_err_t tbmch_clientattribute_with_set_append(tbmch_handle_t client_, const char *key, void *context,
                                                  tbmch_clientattribute_on_get_t on_get,
                                                  tbmch_clientattribute_on_set_t on_set)
{
     if (!on_set)  {
          TBMCLOG_E("on_set is NULL!");
          return ESP_FAIL;
     }
     return _tbmch_clientattribute_xx_append(client_, key, context, on_get, on_set);
}
// tbmch_attribute_of_clientside_init()
static tbmch_err_t _tbmch_clientattribute_xx_append(tbmch_handle_t client_, const char *key, void *context,
                                                  tbmch_clientattribute_on_get_t on_get,
                                                  tbmch_clientattribute_on_set_t on_set)
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCHLOG_E("client is NULL");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCHLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Create clientattribute
     tbmch_clientattribute_t *clientattribute = _tbmch_clientattribute_init(key, context, on_get, on_set);
     if (!clientattribute) {
          // Give semaphore
          xSemaphoreGive(client->lock);
          TBMCHLOG_E("Init clientattribute failure! key=%s", key);
          return ESP_FAIL;
     }

     // Insert clientattribute to list
     tbmch_clientattribute_t *it, last = NULL;
     if (LIST_FIRST(&client->clientattribute_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->clientattribute_list, clientattribute, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->clientattribute_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, clientattribute, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->lock);
     return ESP_OK;
}
tbmch_err_t tbmch_clientattribute_clear(tbmch_handle_t client_, const char *key)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !key) {
          TBMCLOG_E("client or key is NULL!");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Search item
     tbmch_clientattribute_t *clientattribute = NULL;
     LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
          if (clientattribute && strcmp(_tbmch_clientattribute_get_key(clientattribute), key)==0) {
               break;
          }
     }

     // Remove form list
     if (clientattribute) {
          LIST_REMOVE(clientattribute, entry);
          _tbmch_clientattribute_destroy(clientattribute);
     } else {
          TBMCLOG_W("Unable to remove time-series data:%s!", key);
     }

     // Give semaphore
     xSemaphoreGive(client->lock);
     return ESP_OK;
}
tbmch_err_t tbmch_clientattribute_send(tbmch_handle_t client_, int count, /*const char *key,*/ ...) ////tbmqttlink.h.tbmch_sendClientAttributes();
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return ESP_FAIL;
     }
     if (count <= 0) {
          TBMCLOG_E("count(%d) is error!", count);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     int i;
     va_list ap;
     va_start(ap, count);
     cJSON *object = cJSON_CreateObject(); // create json object
     for (i=0; i<count; i++) {
          const char *key = va_arg(ap, (const char*));

          // Search item
          tbmch_clientattribute_t *clientattribute = NULL;
          LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
               if (clientattribute && strcmp(_tbmch_clientattribute_get_key(clientattribute), key)==0) {
                    break;
               }
          }

          /// Add clientattribute to package
          if (clientattribute) {
               _tbmch_clientattribute_value_to_pack(client, clientattribute, object); // add item to json object
          } else {
               TBMCLOG_W("Unable to remove time-series data:%s!", key);
          }
     }
     va_end(ap);

     // send package...
     char *pack = cJSON_Print(object);
     int result = tbmc_clientattributes_publish(client->tbmqttclient, pack, 1/*qos*/, 0/*retain*/);
     cJSON_Delete(object); // delete json object

     // Give semaphore
     xSemaphoreGive(client->lock);
     return (result > -1) ? ESP_OK : ESP_FAIL;
}

//====3.Subscribe to shared device attribute updates from the server===================================================
tbmch_err_t tbmch_sharedattribute_append(tbmch_handle_t client_,
                                         const char *key, tbmch_value_type_t type, void *context,
                                         tbmch_sharedattribute_on_set_t on_set) ////tbmqttlink.h.tbmch_addSubAttrEvent(); //Call it before connect() //tbmch_shared_attribute_list_t
{
     // TODO:
}
tbmch_err_t tbmch_sharedattribute_clear(tbmch_handle_t client_, const char *key, ...) // remove shared_attribute from tbmch_shared_attribute_list_t

{
     // TODO:
}

//====4.Request client-side or shared device attributes from the server================================================
tbmch_err_t tbmch_attributes_request(tbmch_handle_t client_,
                                     void *context,
                                     tbmch_attributesrequest_on_response_t on_response,
                                     tbmch_attributesrequest_on_timeout_t on_timeout,
                                     const char *key, ...) ////tbmqttlink.h.tbmch_sendAttributesRequest(); ////return request_id on successful, otherwise return -1
{
     // TODO:
}

//====5.Server-side RPC================================================================================================
tbmch_err_t tbmch_serverrpc_append(tbmch_handle_t client_, const char *method,
                                   void *context,
                                   tbmch_serverrpc_on_request_t on_request)        ////tbmqttlink.h.tbmch_addServerRpcEvent(evtServerRpc); //Call it before connect()
{
     // TODO:
}
tbmch_err_t tbmch_serverrpc_clear(tbmch_handle_t client_, const char *method, ...) // remove from LIST_ENTRY(tbmch_serverrpc_) & delete
{
     // TODO:
}

//====6.Client-side RPC================================================================================================
tbmch_clientrpc_handle_t tbmch_clientrpc_of_oneway_request(tbmch_handle_t client_, const char *method, tbmch_rpc_params_t *params) ////tbmqttlink.h.tbmch_sendClientRpcRequest(); //add list
{
     // TODO:
}
tbmch_clientrpc_handle_t tbmch_clientrpc_of_twoway_request(tbmch_handle_t client_, const char *method, tbmch_rpc_params_t *params,
                                                           void *context,
                                                           tbmch_clientrpc_on_response_t on_response,
                                                           tbmch_clientrpc_on_timeout_t on_timeout) ////tbmqttlink.h.tbmch_sendClientRpcRequest(); //create to add to LIST_ENTRY(tbmch_clientrpc_)
{
     // TODO:
}

//====7.Claiming device using device-side key scenario: Not implemented yet============================================

//====8.Device provisioning: Not implemented yet=======================================================================

//====9.Firmware update================================================================================================
tbmch_err_t tbmch_fwupdate_append(tbmch_handle_client_t client_, const char *fw_title,
                                  void *context,
                                  tbmch_fwupdate_on_sharedattributes_t on_fw_attributes,
                                  tbmch_fwupdate_on_response_t on_fw_chunk,
                                  tbmch_fwupdate_on_done_t on_fw_success,
                                  tbmch_fwupdate_on_timeout_t on_fw_timeout)
{
     // TODO:
}
tbmch_err_t tbmch_fwupdate_clear(tbmch_handle_client_t client_,
                                 const char *fw_title, ...)
{
     // TODO:
}

=======================================================================================================================
//~~static int _tbmch_sendServerRpcReply(tbmch_handle_t client_, int request_id, const char* response, int qos=1, int retain=0); //sendServerRpcReply()

static bool _tbmch_sendTbmqttMsg2Queue(tbmch_handle_t client_, TbmqttInnerMsgType type, cJSON *payload) //_sendTbmqttInnerMsg2Queue()
{
     // TODO:
}
//static bool _tbmch_tbDecodeAttributesJsonPayload(JsonObject& attr_kvs); //_tbDecodeAttributesJsonPayload()

static void _tbmch_on_connected(tbmch_handle_t client_) //onConnected()
{
     // TODO:
}
static void _tbmch_on_disonnected(tbmch_handle_t client_) //onDisonnected()
{
     // TODO:
}
static void _tbmch_on_sharedattributes_received(tbmch_handle_t client_, const char* payload, int length) //onAttrOfSubReply();
{
     // TODO:
}
static void _tbmch_on_attributesrequest_success(tbmch_handle_t client_, int request_id, const char* payload, int length) //onAttributesResponse()=>_attributesResponse()
{
     // TODO:
}
static void _tbmch_on_attributesrequest_timeout(tbmch_handle_t client_, int request_id) //onAttributesResponseTimeout()
{
     // TODO:
}
//~~static bool _attributesResponse(int request_id, const char* payload, int length); //merge to _tbmch_on_attributesrequest_success()
static void _tbmch_on_clientrpc_success(tbmch_handle_t client_, int request_id, const char* payload, int length) //onClientRpcResponse()
{
     // TODO:
}
static void _tbmch_on_clientrpc_timeout(tbmch_handle_t client_, int request_id) //onClientRpcResponseTimeout()
{
     // TODO:
}
static void _tbmch_on_serverrpc_request(tbmch_handle_t client_, int request_id, const char* payload, int length) ////onServerRpcRequest()
{
     // TODO:
}
static void _tbmch_on_fwrequest_response(tbmch_handle_t client_, int request_id, int chunk, const char* payload, int length)
{
     // TODO:
}
static void _tbmch_on_fwrequest_timeout(tbmch_handle_t client_, int request_id)
{
     // TODO:
}

static void _timer_start()
{
     // TODO:
}
static void _timer_stop()
{
     // TODO:
}
static void _timer_timerout() //send msg to queue
{
     // TODO:
}