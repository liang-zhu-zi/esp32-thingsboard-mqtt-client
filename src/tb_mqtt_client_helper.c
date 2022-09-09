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

#include "tb_mqtt_client.h"
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
{                                  //context   param1        param1    param3    param4
  TBMCH_MSGID_TIMER_TIMEOUT = 1,   //context
  TBMCH_MSGID_CONNECTED,           //context
  TBMCH_MSGID_DISCONNECTED,        //context
  TBMCH_MSGID_SHAREDATTR_RECEIVED, //context,                         cJSON
  TBMCH_MSGID_ATTRREQUEST_RESPONSE,//context, request_id,             cJSON
  TBMCH_MSGID_ATTRREQUEST_TIMEOUT, //context, request_id
  TBMCH_MSGID_SERVERRPC_REQUSET,   //context, request_id,             cJSON
  TBMCH_MSGID_CLIENTRPC_RESPONSE,  //context, request_id,             cJSON
  TBMCH_MSGID_CLIENTRPC_TIMEOUT,   //context, request_id
  TBMCH_MSGID_FWUPDATE_RESPONSE,   //context, request_id,   chunk,    payload,  len
  TBMCH_MSGID_FWUPDATE_TIMEOUT     //context, request_id
} tbmch_msgid_t;

typedef struct tbmch_msg_easy {
     void *context; /*!< tbmch_handle_t */
} tbmch_msg_easy_t;

typedef struct tbmch_msg_sharedattr_received {
     void *context; /*!< tbmch_handle_t */
     cJSON *object; /*!< received palyload. free memory by msg receiver */
} tbmch_msg_sharedattr_received_t;

typedef struct tbmch_msg_response {
     void *context; /*!< tbmch_handle_t */
     int32_t request_id;
     cJSON *object; /*!< received palyload. free memory by msg receiver */
} tbmch_msg_response_t;

typedef struct tbmch_msg_fwupdate_response {
     void *context; /*!< tbmch_handle_t */
     int32_t request_id;
     int chunk;
     const char *payload; /*!< received palyload. free memory by msg receiver */
     int length;
} tbmch_msg_fwupdate_response_t;

typedef struct tbmch_msg_timeout {
     void *context; /*!< tbmch_handle_t */
     int32_t request_id;
} tbmch_msg_timeout_t;

typedef union tbmch_msgbody {
     tbmch_msg_easy_t timer_timeout; // context

     tbmch_msg_easy_t connected;    // context
     tbmch_msg_easy_t disconnected; // context

     tbmch_msg_sharedattr_received_t sharedattr_received; // context, cJSON

     tbmch_msg_response_t attrrequest_response; // context, request_id, cJSON
     tbmch_msg_timeout_t attrrequest_timeout;   // context, request_id

     tbmch_msg_response_t serverrpc_request; // context, request_id, cJSON

     tbmch_msg_response_t clientrpc_response; // context, request_id, cJSON
     tbmch_msg_timeout_t clientrpc_timeout;   // context, request_id

     tbmch_msg_fwupdate_response_t fwupdate_response; // context, request_id, chunk, payload, len
     tbmch_msg_timeout_t fwupdate_timeout;            // context, request_id
} tbmch_msgbody_t;

typedef struct tbmch_msg {
	tbmch_msgid_t   id;
	tbmch_msgbody_t body;
} tbmch_msg_t;

static void _tbmch_on_connected(tbmch_handle_t client_);
static void _tbmch_on_disonnected(tbmch_handle_t client_);
static void _tbmch_on_sharedattr_received(tbmch_handle_t client_, const char* payload, int length);
static void _tbmch_on_attrrequest_response(tbmch_handle_t client_, int request_id, const char* payload, int length);
static void _tbmch_on_attrrequest_timeout(tbmch_handle_t client_, int request_id);
static void _tbmch_on_serverrpc_request(tbmch_handle_t client_, int request_id, const char* payload, int length);
static void _tbmch_on_clientrpc_response(tbmch_handle_t client_, int request_id, const char* payload, int length);
static void _tbmch_on_clientrpc_timeout(tbmch_handle_t client_, int request_id);
static void _tbmch_on_fwupdate_response(tbmch_handle_t client_, int request_id, int chunk, const char* payload, int length);
static void _tbmch_on_fwupdate_timeout(tbmch_handle_t client_, int request_id);

/**
 * ThingsBoard MQTT Client Helper 
 */
typedef struct tbmch_client_
{
     struct {
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

static bool _tbmch_sendTbmqttMsg2Queue(tbmch_handle_t client_, tbmch_msg_t *msg); //_sendTbmqttInnerMsg2Queue()
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
     // INIT all of list headers

    // Create a queue capable of containing 40 tbmch_msg_t structures.
    // These should be passed by pointer as they contain a lot of data.
    ...->_xQueue = xQueueCreate(40, sizeof(tbmch_msg_t));
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
                   tbmch_on_disconnected_t on_disconnected) //_begin()
{
     // TODO:
}
//_end()
void tbmch_disconnect(tbmch_handle_t client_)               
{
     // TODO:
}
bool tbmch_is_connected(tbmch_handle_t client_)
{
     // TODO:
}
// new function
bool tbmch_has_event(tbmch_handle_t client_)
{
     // TODO:
}

static int32_t _tbmch_deal_msg(tbmch_handle_t client_, tbmch_msg_t *msg)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return ESP_FAIL;
     }
     if (!msg) {
          TBMCLOG_E("msg is NULL!");
          return ESP_FAIL;
     }

     // deal msg
     switch (msg->id) {
     case TBMCH_MSGID_TIMER_TIMEOUT:       // context
          tbmc_check_timeout(client_);
          break;

     case TBMCH_MSGID_CONNECTED:            // context
          TBMCHLOG_I("TBMCH_RECV: Connected to TBMQTT Server!");
          _tbmch_connected_on(client_);
          break;
     case TBMCH_MSGID_DISCONNECTED:         // context
          TBMCHLOG_I("TBMCH_RECV: Disconnected from TBMQTT Server!");
          _tbmch_disonnected_on(client_);
          break;

     case TBMCH_MSGID_SHAREDATTR_RECEIVED:  // context,                         cJSON
          _tbmch_sharedattribute_on_received(client_, msg->body.sharedattr_received.object);
          cJSON_Delete(msg->body.sharedattr_received.object); //free cJSON
          break;

     case TBMCH_MSGID_ATTRREQUEST_RESPONSE: // context, request_id,             cJSON
          _tbmch_attributesrequest_on_response(client_, msg->body.attrrequest_response.request_id,
                                               msg->body.attrrequest_response.object);
          cJSON_Delete(msg->body.attrrequest_response.object); // free cJSON
          break;
     case TBMCH_MSGID_ATTRREQUEST_TIMEOUT:  // context, request_id
          _tbmch_attributesrequest_on_timeout(client_, msg->body.attrrequest_timeout.request_id);
          break;

     case TBMCH_MSGID_SERVERRPC_REQUSET:    // context, request_id,             cJSON
          _tbmch_serverrpc_on_request(client_, msg->body.serverrpc_request.request_id, 
                                      msg->body.serverrpc_request.object);
          cJSON_Delete(msg->body.serverrpc_request.object); //free cJSON
          break;

     case TBMCH_MSGID_CLIENTRPC_RESPONSE:   // context, request_id,             cJSON
          _tbmch_clientrpc_on_response(client_, msg->body.clientrpc_response.request_id, 
                                       msg->body.clientrpc_response.object);
          cJSON_Delete(msg->body.clientrpc_response.object); //free cJSON
          break;
     case TBMCH_MSGID_CLIENTRPC_TIMEOUT:    // context, request_id
          _tbmch_clientrpc_on_timeout(client_, msg->body.clientrpc_response.request_id);
          break;

     case TBMCH_MSGID_FWUPDATE_RESPONSE:    // context, request_id,   chunk,    payload,  len
          _tbmch_fwupdate_on_response(client_, msg->body.fwupdate_response.request_id, 
               msg->body.fwupdate_response.chunk, 
               msg->body.fwupdate_response.payload,
               msg->body.fwupdate_response.length);
          TBMCH_FREE(msg->body.fwupdate_response.payload); //free payload
          break;
     case TBMCH_MSGID_FWUPDATE_TIMEOUT:     // context, request_id
          _tbmch_fwupdate_on_timeout(client_, msg->body.fwupdate_timeout.request_id);
          break;

     default:
          TBMCHLOG_D("msg->type(%d) is error!\r\n", msg->type);
          return ESP_FAIL;
     }

     return ESP_OK;
}

//recv & deal msg from queue
//_recv()=>recvFromLink()=>parse() //tb_mqtt_client_loop()/checkTimeout(), recv/parse/sendqueue/ack...
void tbmch_run(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCHLOG_E("client is NULL!");
          return; // false;
     }

     // TODO: whether to insert lock?
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBMCLOG_E("Unable to take semaphore!");
     //      return false;
     // }

     // read event from queue()
     tbmch_msg_t msg;
     int i = 0;
     if (client->_xQueue != 0) {
          // Receive a message on the created queue.  Block for 0(10) ticks if a message is not immediately available.
          while (i < 10 && xQueueReceive(client->_xQueue, &(msg), (TickType_t)0)) { // 10
               // pcRxedMessage now points to the struct AMessage variable posted by vATask.
               _tbmch_deal_msg(&msg);
               i++;
          }
     }

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return;// sendResult == pdTRUE ? true : false;
}

static void _tbmch_connected_on(tbmch_handle_t client_) //onConnected() // First receive
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCHLOG_E("client is NULL");
          return;
     }

     // TODO: ???
     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCHLOG_E("Unable to take semaphore!");
          return;
     }

     // clone parameter in lock/unlock
     void *context = client->config.context;
     tbmch_on_connected_t on_connected = client->config.on_connected; /*!< Callback of connected ThingsBoard MQTT */

     // Give semaphore
     xSemaphoreGive(client->_lock);
  
     // do callback
     on_connected(client, context);
     return;
}
static void _tbmch_disonnected_on(tbmch_handle_t client_) //onDisonnected() // First receive
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCHLOG_E("client is NULL");
          return;
     }

     // TODO: ???
     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCHLOG_E("Unable to take semaphore!");
          return;
     }

     // clone parameter in lock/unlock
     void *context = client->config.context;
     tbmch_on_disconnected_t on_disconnected = client->config.on_disconnected; /*!< Callback of disconnected ThingsBoard MQTT */

     // Give semaphore
     xSemaphoreGive(client->_lock);
  
     // do callback
     on_disconnected(client, context);
     return;
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
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCHLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Create tsdata
     tbmch_tsdata_t *tsdata = _tbmch_tsdata_init(client_, key/*, type*/, context, on_get/*, on_set*/);
     if (!tsdata) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
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
     xSemaphoreGive(client->_lock);
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
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
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
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

////tbmqttlink.h.tbmch_sendTelemetry();
tbmch_err_t tbmch_telemetry_send(tbmch_handle_t client_, int count, /*const char *key,*/ ...)
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
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
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
               _tbmch_tsdata_go_get(tsdata, object); // add item to json object
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
     xSemaphoreGive(client->_lock);
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
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCHLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Create clientattribute
     tbmch_clientattribute_t *clientattribute = _tbmch_clientattribute_init(client_, key, context, on_get, on_set);
     if (!clientattribute) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
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
     xSemaphoreGive(client->_lock);
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
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
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
     xSemaphoreGive(client->_lock);
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
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
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
               _tbmch_clientattribute_do_get(client, clientattribute, object); // add item to json object
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
     xSemaphoreGive(client->_lock);
     return (result > -1) ? ESP_OK : ESP_FAIL;
}

//unpack & deal
static void _tbmch_clientattribute_on_received(tbmch_handle_t client_, const cJSON *object)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !object) {
          TBMCLOG_E("client or object is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return;// ESP_FAIL;
     }

     // foreach item to set value of clientattribute in lock/unlodk.  Don't call tbmch's funciton in set value callback!
     tbmch_clientattribute_t *clientattribute = NULL;
     const char* key = NULL;
     LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
          if (clientattribute) {
               key = _tbmch_clientattribute_get_key(clientattribute);
               if (cJSON_HasObjectItem(object, key)) {
                    _tbmch_clientattribute_do_set(clientattribute, cJSON_GetObjectItem(object, key));
               }
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return;// ESP_OK;
}

//====3.Subscribe to shared device attribute updates from the server===================================================
////tbmqttlink.h.tbmch_addSubAttrEvent(); //Call it before connect() //tbmch_shared_attribute_list_t
tbmch_err_t tbmch_sharedattribute_append(tbmch_handle_t client_, const char *key, void *context,
                                         tbmch_sharedattribute_on_set_t on_set)
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCHLOG_E("client is NULL");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCHLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Create sharedattribute
     tbmch_sharedattribute_t *sharedattribute = _tbmch_sharedattribute_init(key, context, on_set);
     if (!sharedattribute) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCHLOG_E("Init sharedattribute failure! key=%s", key);
          return ESP_FAIL;
     }

     // Insert sharedattribute to list
     tbmch_sharedattribute_t *it, last = NULL;
     if (LIST_FIRST(&client->sharedattribute_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->sharedattribute_list, sharedattribute, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->sharedattribute_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, sharedattribute, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

// remove shared_attribute from tbmch_shared_attribute_list_t
tbmch_err_t tbmch_sharedattribute_clear(tbmch_handle_t client_, const char *key)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !key) {
          TBMCLOG_E("client or key is NULL!");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Search item
     tbmch_sharedattribute_t *sharedattribute = NULL;
     LIST_FOREACH(sharedattribute, &client->sharedattribute_list, entry) {
          if (sharedattribute && strcmp(_tbmch_sharedattribute_get_key(sharedattribute), key)==0) {
               break;
          }
     }

     // Remove form list
     if (sharedattribute) {
          LIST_REMOVE(sharedattribute, entry);
          _tbmch_sharedattribute_destroy(sharedattribute);
     } else {
          TBMCLOG_W("Unable to remove time-series data:%s!", key);
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

//unpack & deal
//onAttrOfSubReply()
static void _tbmch_sharedattribute_on_received(tbmch_handle_t client_, const cJSON *object)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !object) {
          TBMCLOG_E("client or object is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return;// ESP_FAIL;
     }

     // foreach itme to set value of sharedattribute in lock/unlodk.  Don't call tbmch's funciton in set value callback!
     tbmch_sharedattribute_t *sharedattribute = NULL;
     const char* key = NULL;
     LIST_FOREACH(sharedattribute, &client->sharedattribute_list, entry) {
          if (sharedattribute) {
               key = _tbmch_sharedattribute_get_key(sharedattribute);
               if (cJSON_HasObjectItem(object, key)) {
                    _tbmch_sharedattribute_do_set(sharedattribute, cJSON_GetObjectItem(object, key));
               }
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);

     // special process for fwupdate
     if (cJSON_HasObjectItem(object, TB_MQTT_SHAREDATTRBUTE_FW_TITLE) &&
         cJSON_HasObjectItem(object, TB_MQTT_SHAREDATTRBUTE_FW_VERSION) &&
         cJSON_HasObjectItem(object, TB_MQTT_SHAREDATTRBUTE_FW_CHECKSUM) &&
         cJSON_HasObjectItem(object, TB_MQTT_SHAREDATTRBUTE_FW_CHECKSUM_ALG))
     {
          char *fw_title = cJSON_GetStringValue(cJSON_GetObjectItem(const object, TB_MQTT_SHAREDATTRBUTE_FW_TITLE));
          char *fw_version = cJSON_GetStringValue(cJSON_GetObjectItem(const object, TB_MQTT_SHAREDATTRBUTE_FW_VERSION));
          char *fw_checksum = cJSON_GetStringValue(cJSON_GetObjectItem(const object, TB_MQTT_SHAREDATTRBUTE_FW_CHECKSUM));
          char *fw_checksum_algorithm = cJSON_GetStringValue(cJSON_GetObjectItem(const object, TB_MQTT_SHAREDATTRBUTE_FW_CHECKSUM_ALG));
          _tbmch_fwupdate_on_sharedattributes(client, fw_title, fw_version, fw_checksum, fw_checksum_algorithm);
     }

     return;// ESP_OK;
}

//====4.Request client-side or shared device attributes from the server================================================
////tbmqttlink.h.tbmch_sendAttributesRequest();
////return request_id on successful, otherwise return -1/ESP_FAIL
int tbmch_attributesrequest_send(tbmch_handle_t client_,
                                 void *context,
                                 tbmch_attributesrequest_on_response_t on_response,
                                 tbmch_attributesrequest_on_timeout_t on_timeout,
                                 int count, /*const char *key,*/...)
{
#define MAX_KEYS_LEN (256)

     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCHLOG_E("client is NULL");
          return ESP_FAIL;
     }
     if (count <= 0) {
          TBMCLOG_E("count(%d) is error!", count);
          return ESP_FAIL;
     }

     // Take semaphore, malloc client_keys & shared_keys
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCHLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }
     char *client_keys = TBMCH_MALLOC(MAX_KEYS_LEN);
     char *shared_keys = TBMCH_MALLOC(MAX_KEYS_LEN);
     if (!client_keys || !shared_keys) {
          goto attributesrequest_fail;
     }
     memset(client_keys, 0x00, 256);
     memset(shared_keys, 0x00, 256);

     // Get client_keys & shared_keys
     int i;
     va_list ap;
     va_start(ap, count);
     for (i=0; i<count; i++) {
          const char *key = va_arg(ap, (const char*));

          // Search item in clientattribute
          tbmch_clientattribute_t *clientattribute = NULL;
          LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
               if (clientattribute && strcmp(_tbmch_clientattribute_get_key(clientattribute), key)==0) {
                    // copy key to client_keys
                    if (strlen(client_keys)==0) {
                         strncpy(client_keys, key, MAX_KEYS_LEN-1);
                    } else {
                         strncat(client_keys, ",", MAX_KEYS_LEN-1);                         
                         strncat(client_keys, key, MAX_KEYS_LEN-1);
                    }
                    continue;
               }
          }

          // Search item in sharedtattribute
          tbmch_sharedtattribute_t *sharedtattribute = NULL;
          LIST_FOREACH(sharedtattribute, &client->sharedtattribute_list, entry) {
               if (sharedtattribute && strcmp(_tbmch_sharedtattribute_get_key(sharedtattribute), key)==0) {
                    // copy key to client_keys
                    if (strlen(shared_keys)==0) {
                         strncpy(shared_keys, key, MAX_KEYS_LEN-1);
                    } else {
                         strncat(shared_keys, ",", MAX_KEYS_LEN-1);                         
                         strncat(shared_keys, key, MAX_KEYS_LEN-1);
                    }
                    continue;
               }
          }

          TBMCLOG_W("Unable to remove time-series data:%s!", key);
     }
     va_end(ap);

     // TODO: replace real parameter!
     // Send msg to server
     int request_id = tbmc_attributes_request_ex(client->tbmqttclient, client_keys, shared_keys,
                               void *context,
                               tbmc_on_attrrequest_response_t on_attrrequest_response,
                               tbmc_on_attrrequest_timeout_t on_attrrequest_timeout,
                               1/*qos*/, 0/*retain*/);
     if (request_id<0) {
          TBMCHLOG_E("Init tbmc_attributes_request failure!");
          goto attributesrequest_fail;
     }

     // Create attributesrequest
     tbmch_attributesrequest_t *attributesrequest = _tbmch_attributesrequest_init(request_id, context, on_response, on_timeout);
     if (!attributesrequest) {
          TBMCHLOG_E("Init attributesrequest failure!");
          goto attributesrequest_fail;
     }

     // Insert attributesrequest to list
     tbmch_attributesrequest_t *it, last = NULL;
     if (LIST_FIRST(&client->attributesrequest_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->attributesrequest_list, attributesrequest, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->attributesrequest_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, attributesrequest, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     TBMCH_FREE(client_keys);
     TBMCH_FREE(shared_keys);
     return request_id;

attributesrequest_fail:
     xSemaphoreGive(client->_lock);
     if (!client_keys) {
          TBMCH_FREE(client_keys);
     }
     if (!shared_keys) {
          TBMCH_FREE(shared_keys);
     }
     return ESP_FAIL;
}

// onAttributesResponse()=>_attributesResponse()
static void _tbmch_attributesrequest_on_response(tbmch_handle_t client_, int request_id, const cJSON *object)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !object) {
          TBMCLOG_E("client or object is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return;// ESP_FAIL;
     }

     // Search attributesrequest
     tbmch_attributesrequest_t *attributerequest = NULL;
     LIST_FOREACH(attributerequest, &client->attributerequest_list, entry) {
          if (attributerequest && (_tbmch_attributesrequest_get_request_id(attributerequest)==request_id)) {
               break;
          }
     }
     if (!attributerequest) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCLOG_W("Unable to find attribute request:%d!", request_id);
          return;// ESP_FAIL;
     }

     // Cache and remove attributesrequest
     tbmch_attributesrequest_t *cache = _tbmch_attributesrequest_clone_wo_listentry(attributerequest);
     LIST_REMOVE(attributerequest, entry);
     _tbmch_attributerequest_destroy(attributerequest);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // foreach item to set value of clientattribute in lock/unlodk.  Don't call tbmch's funciton in set value callback!
     if (cJSON_HasObjectItem(object, TB_MQTT_TEXT_ATTRIBUTES_REQUEST_CLIENT)) {
          _tbmch_clientattribute_on_received(client_, cJSON_GetObjectItem(object, TB_MQTT_TEXT_ATTRIBUTES_REQUEST_CLIENT));
     }
     // foreach item to set value of sharedattribute in lock/unlodk.  Don't call tbmch's funciton in set value callback!
     if (cJSON_HasObjectItem(object, TB_MQTT_TEXT_ATTRIBUTES_REQUEST_SHARED)) {
          _tbmch_sharedattribute_on_received(client_, cJSON_GetObjectItem(object, TB_MQTT_TEXT_ATTRIBUTES_REQUEST_SHARED));
     }

     // Do response
     _tbmch_attributesrequest_do_response(cache);
     // Free cache
     _tbmch_attributerequest_destroy(cache);

     return;// ESP_OK;
}
// onAttributesResponseTimeout()
static void _tbmch_attributesrequest_on_timeout(tbmch_handle_t client_, int request_id)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return;// ESP_FAIL;
     }

     // Search attributesrequest
     tbmch_attributesrequest_t *attributerequest = NULL;
     LIST_FOREACH(attributerequest, &client->attributerequest_list, entry) {
          if (attributerequest && (_tbmch_attributesrequest_get_request_id(attributerequest)==request_id)) {
               break;
          }
     }
     if (!attributerequest) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCLOG_W("Unable to find attribute request:%d!", request_id);
          return;// ESP_FAIL;
     }

     // Cache and remove attributesrequest
     tbmch_attributesrequest_t *cache = _tbmch_attributesrequest_clone_wo_listentry(attributerequest);
     LIST_REMOVE(attributerequest, entry);
     _tbmch_attributerequest_destroy(attributerequest);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do timeout
     _tbmch_attributesrequest_do_timeout(cache);
     // Free attributesrequest
     _tbmch_attributerequest_destroy(cache);

     return;// ESP_OK;
}

//====5.Server-side RPC================================================================================================
////tbmqttlink.h.tbmch_addServerRpcEvent(evtServerRpc); //Call it before connect()
tbmch_err_t tbmch_serverrpc_append(tbmch_handle_t client_, const char *method,
                                   void *context,
                                   tbmch_serverrpc_on_request_t on_request)
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCHLOG_E("client is NULL");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCHLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Create serverrpc
     tbmch_serverrpc_t *serverrpc = _tbmch_serverrpc_init(client, method, context, on_request);
     if (!serverrpc) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCHLOG_E("Init serverrpc failure! key=%s", key);
          return ESP_FAIL;
     }

     // Insert serverrpc to list
     tbmch_serverrpc_t *it, last = NULL;
     if (LIST_FIRST(&client->serverrpc_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->serverrpc_list, serverrpc, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->serverrpc_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, serverrpc, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

// remove from LIST_ENTRY(tbmch_serverrpc_) & delete
tbmch_err_t tbmch_serverrpc_clear(tbmch_handle_t client_, const char *method)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !key) {
          TBMCLOG_E("client or key is NULL!");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Search item
     tbmch_serverrpc_t *serverrpc = NULL;
     LIST_FOREACH(serverrpc, &client->serverrpc_list, entry) {
          if (serverrpc && strcmp(_tbmch_serverrpc_get_method(serverrpc), method)==0) {
               break;
          }
     }

     // Remove form list
     if (serverrpc) {
          LIST_REMOVE(serverrpc, entry);
          _tbmch_serverrpc_destroy(serverrpc);
     } else {
          TBMCLOG_W("Unable to remove time-series data:%s!", method);
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

////parseServerSideRpc(msg)
static void _tbmch_serverrpc_on_request(tbmch_handle_t client_, int request_id, const cJSON *object)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !object) {
          TBMCLOG_E("client or object is NULL!");
          return;// ESP_FAIL;
     }

     const char *method = NULL;
     if (cJSON_HasObjectItem(object, TB_MQTT_TEXT_RPC_METHOD)) {
          cJSON *methodItem = cJSON_GetObjectItem(object, TB_MQTT_TEXT_RPC_METHOD)
          method = cJSON_GetStringValue(methodItem);
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return;// ESP_FAIL;
     }

     // Search item
     tbmch_serverrpc_t *serverrpc = NULL;
     LIST_FOREACH(serverrpc, &client->serverrpc_list, entry) {
          if (serverrpc && strcmp(_tbmch_serverrpc_get_method(serverrpc), method)==0) {
               break;
          }
     }
     if (!serverrpc) {
          TBMCLOG_W("Unable to deal server-rpc:%s!", method);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return;// ESP_OK;
     }

     // Clone serverrpc
     tbmch_serverrpc_t *cache = _tbmch_serverrpc_clone_wo_listentry(serverrpc);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do request
     tbmch_rpc_results_t *result = _tbmch_serverrpc_do_request(cache, request_id,
                                                               cJSON_GetObjectItem(object, TB_MQTT_TEXT_RPC_PARAMS));
     // Send reply
     if (result) {
          #if 0
          cJSON* reply = cJSON_CreateObject();
          cJSON_AddStringToObject(reply, TB_MQTT_TEXT_RPC_METHOD, method);
          cJSON_AddItemToObject(reply, TB_MQTT_TEXT_RPC_RESULTS, result);
          const char *response = cJSON_Print(reply);
          tbmc_serverrpc_response(client_, request_id, response, 1/*qos*/, 0/*retain*/)
          cJSON_Delete(reply);
          #else
          const char *response = cJSON_Print(result);
          tbmc_serverrpc_response(client_, request_id, response, 1/*qos*/, 0/*retain*/)
          cJSON_Delete(result);
          #endif
     }
     // Free serverrpc
     _tbmch_serverrpc_destroy(cache);

     return;// ESP_OK;
}

//====6.Client-side RPC================================================================================================
////tbmqttlink.h.tbmch_sendClientRpcRequest(); //add list
tbmch_clientrpc_handle_t tbmch_clientrpc_of_oneway_request(tbmch_handle_t client_, const char *method, 
                                                       const tbmch_rpc_params_t *params)
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCHLOG_E("client is NULL");
          return ESP_FAIL;
     }
     if (!method) {
          TBMCHLOG_E("method is NULL");
          return ESP_FAIL;
     }

     // Send msg to server
     cJSON *object = cJSON_CreateObject(); // create json object
     cJSON_AddStringToObject(object, TB_MQTT_TEXT_RPC_METHOD, method);
     if (params) {
          cJSON_AddItemReferenceToObject(object, TB_MQTT_TEXT_RPC_PARAMS, params);
     } else  {
          cJSON_AddNullToObject(object, TB_MQTT_TEXT_RPC_PARAMS);
     }
     char *params_str = cJSON_Print(object);
     // TODO: replace real parameter!
     int request_id = tbmc_clientrpc_request_ex(client->tbmqttclient, method, params_str,
                              void *context,
                              tbmc_on_clientrpc_response_t on_clientrpc_response,
                              tbmc_on_clientrpc_timeout_t on_clientrpc_timeout,
                               1/*qos*/, 0/*retain*/);
     cJSON_Delete(object); // delete json object
     if (request_id<0) {
          TBMCHLOG_E("Init tbmc_clientrpc_request failure!");
          return ESP_FAIL;
     }

     return request_id;
}
////tbmqttlink.h.tbmch_sendClientRpcRequest(); //create to add to LIST_ENTRY(tbmch_clientrpc_)
tbmch_clientrpc_handle_t tbmch_clientrpc_of_twoway_request(tbmch_handle_t client_, const char *method, 
                                                            const tbmch_rpc_params_t *params,
                                                           void *context,
                                                           tbmch_clientrpc_on_response_t on_response,
                                                           tbmch_clientrpc_on_timeout_t on_timeout)
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCHLOG_E("client is NULL");
          return ESP_FAIL;
     }
     if (!method) {
          TBMCHLOG_E("method is NULL");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCHLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Send msg to server
     cJSON *object = cJSON_CreateObject(); // create json object
     cJSON_AddStringToObject(object, TB_MQTT_TEXT_RPC_METHOD, method);
     if (params)
          cJSON_AddItemReferenceToObject(object, TB_MQTT_TEXT_RPC_PARAMS, params);
     else 
          cJSON_AddNullToObject(object, TB_MQTT_TEXT_RPC_PARAMS);
     char *params_str = cJSON_Print(object);
     // TODO: replace real parameter!
     int request_id = tbmc_clientrpc_request_ex(client->tbmqttclient, method, params_str,
                              void *context,
                              tbmc_on_clientrpc_response_t on_clientrpc_response,
                              tbmc_on_clientrpc_timeout_t on_clientrpc_timeout,
                               1/*qos*/, 0/*retain*/);
     cJSON_Delete(object); // delete json object
     if (request_id<0) {
          TBMCHLOG_E("Init tbmc_clientrpc_request failure!");
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Create clientrpc
     tbmch_clientrpc_t *clientrpc = _tbmch_clientrpc_init(client, request_id, method, context, on_response, on_timeout);
     if (!clientrpc) {
          TBMCHLOG_E("Init clientrpc failure!");
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Insert clientrpc to list
     tbmch_clientrpc_t *it, last = NULL;
     if (LIST_FIRST(&client->clientrpc_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->clientrpc_list, clientrpc, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->clientrpc_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, clientrpc, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return request_id;
}

//onClientRpcResponse()
static void _tbmch_clientrpc_on_response(tbmch_handle_t client_, int request_id, const cJSON *object)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !object) {
          TBMCLOG_E("client or object is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return;// ESP_FAIL;
     }

     // Search clientrpc
     tbmch_clientrpc_t *clientrpc = NULL;
     LIST_FOREACH(clientrpc, &client->clientrpc_list, entry) {
          if (clientrpc && (_tbmch_clientrpc_get_request_id(clientrpc)==request_id)) {
               break;
          }
     }
     if (!clientrpc) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCLOG_W("Unable to find client-rpc:%d!", request_id);
          return;// ESP_FAIL;
     }

     // Cache and remove clientrpc
     tbmch_clientrpc_t *cache = _tbmch_clientrpc_clone_wo_listentry(clientrpc);
     LIST_REMOVE(clientrpc, entry);
     _tbmch_clientrpc_destroy(clientrpc);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do response
     _tbmch_clientrpc_do_response(cache);
     // Free cache
     _tbmch_clientrpc_destroy(cache);

     return;// ESP_OK;
}
//onClientRpcResponseTimeout()
static void _tbmch_clientrpc_on_timeout(tbmch_handle_t client_, int request_id)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return;// ESP_FAIL;
     }

     // Search clientrpc
     tbmch_clientrpc_t *clientrpc = NULL;
     LIST_FOREACH(clientrpc, &client->clientrpc_list, entry) {
          if (clientrpc && (_tbmch_clientrpc_get_request_id(clientrpc)==request_id)) {
               break;
          }
     }
     if (!clientrpc) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCLOG_W("Unable to find attribute request:%d!", request_id);
          return;// ESP_FAIL;
     }

     // Cache and remove clientrpc
     tbmch_clientrpc_t *cache = _tbmch_clientrpc_clone_wo_listentry(clientrpc);
     LIST_REMOVE(clientrpc, entry);
     _tbmch_clientrpc_destroy(clientrpc);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do timeout
     _tbmch_clientrpc_do_timeout(cache);
     // Free clientrpc
     _tbmch_clientrpc_destroy(cache);

     return;// ESP_OK;
}

//====7.Claiming device using device-side key scenario: Not implemented yet============================================

//====8.Device provisioning: Not implemented yet=======================================================================

//====9.Firmware update================================================================================================
tbmch_err_t tbmch_fwupdate_append(tbmch_handle_t client_, const char *fw_title,
                                  void *context,
                                  tbmch_fwupdate_on_sharedattributes_t on_fw_attributes,
                                  tbmch_fwupdate_on_response_t on_fw_chunk,
                                  tbmch_fwupdate_on_success_t on_fw_success,
                                  tbmch_fwupdate_on_timeout_t on_fw_timeout)
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCHLOG_E("client is NULL");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCHLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Create fwupdate
     tbmch_fwupdate_t *fwupdate = _tbmch_fwupdate_init(client, fw_title, context, 
                                                       on_fw_attributes, on_fw_response,
                                                       on_fw_success, on_fw_timeout);
     if (!fwupdate) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCHLOG_E("Init fwupdate failure! key=%s", key);
          return ESP_FAIL;
     }

     // Insert fwupdate to list
     tbmch_fwupdate_t *it, last = NULL;
     if (LIST_FIRST(&client->fwupdate_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->fwupdate_list, fwupdate, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->fwupdate_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, fwupdate, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}
tbmch_err_t tbmch_fwupdate_clear(tbmch_handle_t client_, const char *fw_title)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !key) {
          TBMCLOG_E("client or key is NULL!");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Search item
     tbmch_fwupdate_t *fwupdate = NULL;
     LIST_FOREACH(fwupdate, &client->fwupdate_list, entry) {
          if (fwupdate && strcmp(_tbmch_fwupdate_get_key(fwupdate), method)==0) {
               break;
          }
     }

     // Remove form list
     if (fwupdate) {
          LIST_REMOVE(fwupdate, entry);
          _tbmch_fwupdate_destroy(fwupdate);
     } else {
          TBMCLOG_W("Unable to remove time-series data:%s!", method);
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

static void !_tbmch_fwupdate_on_sharedattributes(tbmch_handle_t client, void *context,
                                                 const char *fw_title, const char *fw_version,
                                                 const char *fw_checksum, const char *fw_checksum_algorithm)
{
     // TODO: these is in lock/unlock
     // search _tbmch_fwupdate;
     // if (search ok) {}
     //   bool _tbmch_fwupdate_do_sharedattributes(tbmch_fwupdate_t *fwupdate, const char *fw_title, const char *fw_version,
     //                                    const char *fw_checksum, const char *fw_checksum_algorithm); //save fw_...
     //   exec fw_update: call tbmc_fwupdate_request()
     //   tbmch_err_t _tbmch_fwupdate_set_request_id(tbmch_fwupdate_t *fwupdate, int request_id);
     // }
}

static void !_tbmch_fwupdate_on_response(tbmch_handle_t client_, int request_id, int chunk, const char* payload, int length)
{
     // TODO: these is in lock/unlock
     //tbmch_err_t tbmch_fwupdate_do_response(tbmch_fwupdate_t *fwupdate, int chunk /*current chunk*/, const void *fw_data, int data_size); //update or done or failure
}
static void !_tbmch_fwupdate_on_timeout(tbmch_handle_t client_, int request_id)
{
     // TODO: these is in lock/unlock
     // _tbmch_fwupdate_do_timeout(tbmch_fwupdate_t *fwupdate, int chunk /*current chunk*/);
}

//=====================================================================================================================
//~~static int _tbmch_sendServerRpcReply(tbmch_handle_t client_, int request_id, const char* response, int qos=1, int retain=0); //sendServerRpcReply()

//send msg to queue
//true if the item was successfully posted, otherwise false. //pdTRUE if the item was successfully posted, otherwise errQUEUE_FULL.
//It runs in MQTT thread.
static bool _tbmch_sendTbmqttMsg2Queue(tbmch_handle_t client_, tbmch_msg_t *msg)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !msg) {
          TBMCLOG_E("client or msg is NULL!");
          return false;
     }

     // TODO: whether to insert lock?
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBMCLOG_E("Unable to take semaphore!");
     //      return false;
     // }

     // Send a pointer to a struct AMessage object.  Don't block if the queue is already full.
     int i = 0;
     BaseType_t sendResult;
     do {
          sendResult = xQueueSend(client->_xQueue, (void *)msg, (TickType_t)10);
          i++;

          if (sendResult != pdTRUE) {
               TBMCLOG_W("_xQueue is full!");
          }
     } while (i < 20 && sendResult != pdTRUE);
     if (i >= 20) {
          TBMCLOG_W("send innermsg timeout!");
     }

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return sendResult == pdTRUE ? true : false;
}

//static bool _tbmch_tbDecodeAttributesJsonPayload(JsonObject& attr_kvs); //_tbDecodeAttributesJsonPayload()

static void _tbmch_on_connected(tbmch_handle_t client_) //onConnected() // First receive
{
     // TODO:
     // _tbmch_sendTbmqttMsg2Queue({TBMCH_MSGID_CONNECTED, body.connected{context}});
}
static void _tbmch_on_disonnected(tbmch_handle_t client_) //onDisonnected() // First receive
{
     // TODO:
     // _tbmch_sendTbmqttMsg2Queue({TBMCH_MSGID_DISCONNECTED, body.disconnected{context}});
}
//onAttrOfSubReply(); // First receive
static void _tbmch_on_sharedattr_received(tbmch_handle_t client_, const char* payload, int length)
{
     // TODO:
     // _tbmch_sendTbmqttMsg2Queue({TBMCH_MSGID_SHAREDATTR_RECEIVED, body.sharedattr_received{context, cJSON}});
}

//onAttributesResponse()=>_attributesResponse() // First send
//~~static bool _attributesResponse(int request_id, const char* payload, int length); //merge to _tbmch_on_attributesrequest_success()
static void _tbmch_on_attrrequest_response(tbmch_handle_t client_, int request_id, const char* payload, int length)
{
     // TODO:
     // _tbmch_sendTbmqttMsg2Queue({TBMCH_MSGID_ATTRREQUEST_RESPONSE, body.attrrequest_response{context, request_id, cJSON}});
} 
//onAttributesResponseTimeout() // First send
static void _tbmch_on_attrrequest_timeout(tbmch_handle_t client_, int request_id)
{
     // TODO:
     // _tbmch_sendTbmqttMsg2Queue({TBMCH_MSGID_ATTRREQUEST_TIMEOUT, body.attrrequest_timeout{context, request_id}});
}
          
////onServerRpcRequest() // First receive
static void _tbmch_on_serverrpc_request(tbmch_handle_t client_, int request_id, const char* payload, int length)
{
     // TODO:
     // _tbmch_sendTbmqttMsg2Queue({TBMCH_MSGID_SERVERRPC_REQUSET, body.serverrpc_request{context, request_id, cJSON}});
}

//onClientRpcResponse() // First send
static void _tbmch_on_clientrpc_response(tbmch_handle_t client_, int request_id, const char* payload, int length)
{
     // TODO:
     // _tbmch_sendTbmqttMsg2Queue({TBMCH_MSGID_CLIENTRPC_RESPONSE, body.clientrpc_response{context, request_id, cJSON}});
}
//onClientRpcResponseTimeout() // First send
static void _tbmch_on_clientrpc_timeout(tbmch_handle_t client_, int request_id)
{
     // TODO:
     // _tbmch_sendTbmqttMsg2Queue({TBMCH_MSGID_CLIENTRPC_TIMEOUT, body.clientrpc_timeout{context, request_id}});
} 
// First send
static void _tbmch_on_fwupdate_response(tbmch_handle_t client_, int request_id, int chunk, const char* payload, int length)
{
     // TODO:
     // _tbmch_sendTbmqttMsg2Queue({TBMCH_MSGID_FWUPDATE_RESPONSE, body.fwupdate_response{context, request_id, chunk, payload, len}});
}
// First send
static void _tbmch_on_fwupdate_timeout(tbmch_handle_t client_, int request_id)
{
     // TODO:
     // _tbmch_sendTbmqttMsg2Queue({TBMCH_MSGID_FWUPDATE_TIMEOUT, body.fwupdate_timeout{context, request_id}});
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
     // _tbmch_sendTbmqttMsg2Queue({TBMCH_MSGID_TIMER_TIMEOUT, body.timer_timeout{context}});
}
