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

// ThingsBoard MQTT Client high layer API

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* using uri parser */
#include "http_parser.h"

#include "tbc_utils.h"

#include "tbc_mqtt.h"
#include "tbc_mqtt_helper.h"

#include "tbc_mqtt_helper_internal.h"

#if 0
/**
 * ThingsBoard MQTT Client Helper message id
 */
typedef enum
{                                  //param1        param1    param3    param4
  TBCMH_MSGID_TIMER_TIMEOUT = 1,   //
  TBCMH_MSGID_CONNECTED,           //
  TBCMH_MSGID_DISCONNECTED,        //
  TBCMH_MSGID_SHAREDATTR_RECEIVED, //                        cJSON
  TBCMH_MSGID_ATTRREQUEST_RESPONSE,//request_id,             cJSON
  TBCMH_MSGID_ATTRREQUEST_TIMEOUT, //request_id
  TBCMH_MSGID_SERVERRPC_REQUSET,   //request_id,             cJSON
  TBCMH_MSGID_CLIENTRPC_RESPONSE,  //request_id,             cJSON
  TBCMH_MSGID_CLIENTRPC_TIMEOUT,   //request_id
  TBCMH_MSGID_PROVISION_RESPONSE,  //request_id,             cJSON
  TBCMH_MSGID_PROVISION_TIMEOUT,   //request_id
  TBCMH_MSGID_FWUPDATE_RESPONSE,   //request_id,   chunk_id, payload,  len
  TBCMH_MSGID_FWUPDATE_TIMEOUT,    //request_id
} tbcmh_msgid_t;

typedef struct tbcmh_msg_easy {
     int reserved;
} tbcmh_msg_easy_t;

typedef struct tbcmh_msg_sharedattr_received {
     cJSON *object; /*!< received palyload. free memory by msg receiver */
} tbcmh_msg_sharedattr_received_t;

typedef struct tbcmh_msg_response {
     int32_t request_id;
     cJSON *object; /*!< received palyload. free memory by msg receiver */
} tbcmh_msg_response_t;

typedef struct tbcmh_msg_provision_response {
     int32_t request_id;
     cJSON *object; /*!< received palyload. free memory by msg receiver */
} tbcmh_msg_provision_response_t;

typedef struct tbcmh_msg_otaupdate_response {
     int32_t request_id;
     int chunk_id;
     char *payload; /*!< received palyload. free memory by msg receiver */
     int length;
} tbcmh_msg_otaupdate_response_t;

typedef struct tbcmh_msg_timeout {
     int32_t request_id;
} tbcmh_msg_timeout_t;

typedef union tbcmh_msgbody {
     tbcmh_msg_easy_t timer_timeout; //

     tbcmh_msg_easy_t connected;    //
     tbcmh_msg_easy_t disconnected; //

     tbcmh_msg_sharedattr_received_t sharedattr_received; // cJSON

     tbcmh_msg_response_t attrrequest_response; // request_id, cJSON
     tbcmh_msg_timeout_t attrrequest_timeout;   // request_id

     tbcmh_msg_response_t serverrpc_request; // request_id, cJSON

     tbcmh_msg_response_t clientrpc_response; // request_id, cJSON
     tbcmh_msg_timeout_t clientrpc_timeout;   // request_id

     tbcmh_msg_provision_response_t provision_response; // request_id, payload, len
     tbcmh_msg_timeout_t provision_timeout;             // request_id

     tbcmh_msg_otaupdate_response_t otaupdate_response; // request_id, chunk_id, payload, len
     tbcmh_msg_timeout_t otaupdate_timeout;            // request_id
} tbcmh_msgbody_t;

typedef struct tbcmh_msg {
	tbcmh_msgid_t   id;
	tbcmh_msgbody_t body;
} tbcmh_msg_t;
#endif

static tbcmh_request_t *_request_create(tbcmh_request_type_t type, uint32_t request_id);
static void _request_destroy(tbcmh_request_t *tbcmh_request);

static void _tbcmh_bridge_event_send(tbcma_event_t *event);
//static void _tbcmh_on_event_handle(tbcma_event_t *event);

//static void _tbcmh_on_connected(void *context);
//static void _tbcmh_on_disonnected(void *context);
//static void _tbcmh_on_sharedattr_received(void *context, const char* payload, int length);
//static void _tbcmh_on_attrrequest_response(void *context, int request_id, const char* payload, int length);
//static void _tbcmh_on_attrrequest_timeout(void *context, int request_id);
//static void _tbcmh_on_serverrpc_request(void *context, int request_id, const char* payload, int length);
//static void _tbcmh_on_clientrpc_response(void *context, int request_id, const char* payload, int length);
//static void _tbcmh_on_clientrpc_timeout(void *context, int request_id);
//static void _tbcmh_on_provision_response(void *context, int request_id, const char* payload, int length);
//static void _tbcmh_on_provision_timeout(void *context, int request_id);
//static void _tbcmh_on_otaupdate_response(void *context, int request_id, int chunk_id, const char* payload, int length);
//static void _tbcmh_on_otaupdate_timeout(void *context, int request_id);   

//static void _response_timer_create(tbcmh_handle_t client_);
//static void _response_timer_start(tbcmh_handle_t client_);
//static void _response_timer_stop(tbcmh_handle_t client_);
//static void _response_timer_destroy(tbcmh_handle_t client_);


//static void _tbcmh_otaupdate_on_connected(tbcmh_handle_t client_);
// static void _tbcmh_otaupdate_on_sharedattributes(tbcmh_handle_t client_, tbcmh_otaupdate_type_t ota_type,
//                                                  const char *ota_title, const char *ota_version, int ota_size,
//                                                  const char *ota_checksum, const char *ota_checksum_algorithm);

//static tbc_err_t _tbcmh_telemetry_empty(tbcmh_handle_t client_);
//static tbc_err_t _tbcmh_clientattribute_empty(tbcmh_handle_t client_);
//static tbc_err_t _tbcmh_sharedattribute_empty(tbcmh_handle_t client_);
//static tbc_err_t _tbcmh_attributesrequest_empty(tbcmh_handle_t client_);
//static tbc_err_t _tbcmh_serverrpc_empty(tbcmh_handle_t client_);
//static tbc_err_t _tbcmh_clientrpc_empty(tbcmh_handle_t client_);
//static tbc_err_t _tbcmh_provision_empty(tbcmh_handle_t client_);
//static tbc_err_t _tbcmh_otaupdate_empty(tbcmh_handle_t client_);

const static char *TAG = "tb_mqtt_client_helper";

//====0.tbcm client====================================================================================================
tbcmh_handle_t tbcmh_init(void)
{
     tbcmh_t *client = (tbcmh_t *)TBC_MALLOC(sizeof(tbcmh_t));
     if (!client) {
          TBC_LOGE("Unable to malloc memory! %s()", __FUNCTION__);
          return NULL;
     }
     memset(client, 0x00, sizeof(tbcmh_t));

     client->tbmqttclient = tbcm_init();
     // Create a queue capable of containing 20 tbcma_event_t structures. These should be passed by pointer as they contain a lot of data.
     client->_xQueue = xQueueCreate(40, sizeof(tbcma_event_t));
     if (client->_xQueue == NULL) {
          TBC_LOGE("failed to create the queue! %s()", __FUNCTION__);
     }
     //_response_timer_create(client);

     //tbc_transport_storage_free_fields(&client->config);
     client->context = NULL;
     client->on_connected = NULL; 
     client->on_disconnected = NULL;

     client->_lock = xSemaphoreCreateMutex();
     if (client->_lock == NULL)  {
          TBC_LOGE("failed to create the lock!");
     }
     
     memset(&client->tsdata_list, 0x00, sizeof(client->tsdata_list)); //client->tsdata_list = LIST_HEAD_INITIALIZER(client->tsdata_list);
     memset(&client->clientattribute_list, 0x00, sizeof(client->clientattribute_list)); //client->clientattribute_list = LIST_HEAD_INITIALIZER(client->clientattribute_list);
     memset(&client->sharedattribute_list, 0x00, sizeof(client->sharedattribute_list)); //client->sharedattribute_list = LIST_HEAD_INITIALIZER(client->sharedattribute_list);
     memset(&client->attributesrequest_list, 0x00, sizeof(client->attributesrequest_list)); //client->attributesrequest_list = LIST_HEAD_INITIALIZER(client->attributesrequest_list);
     memset(&client->serverrpc_list, 0x00, sizeof(client->serverrpc_list)); //client->serverrpc_list = LIST_HEAD_INITIALIZER(client->serverrpc_list);
     memset(&client->clientrpc_list, 0x00, sizeof(client->clientrpc_list)); //client->clientrpc_list = LIST_HEAD_INITIALIZER(client->clientrpc_list);
     memset(&client->otaupdate_list, 0x00, sizeof(client->otaupdate_list)); //client->otaupdate_list = LIST_HEAD_INITIALIZER(client->otaupdate_list);

     client->next_request_id = 0;
     client->last_check_timestamp = (uint64_t)time(NULL);
     memset(&client->request_list, 0x00, sizeof(client->request_list));//client->request_list = LIST_HEAD_INITIALIZER(client->request_list);

     return client;
}
void tbcmh_destroy(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     // TODO: dead lock???
     tbcmh_disconnect(client);

     // empty all 7 list!
     _tbcmh_telemetry_empty(client_);
     _tbcmh_clientattribute_empty(client_);
     _tbcmh_sharedattribute_empty(client_);
     _tbcmh_attributesrequest_empty(client_);
     _tbcmh_serverrpc_empty(client_);
     _tbcmh_clientrpc_empty(client_);
     _tbcmh_provision_empty(client_);
     _tbcmh_otaupdate_empty(client_);

     if (client->_lock) {
          vSemaphoreDelete(client->_lock);
          client->_lock = NULL;
     }

     // config in tbcmh_disconnect()

     //_response_timer_stop(client);
     //_response_timer_destroy(client);
     if (client->_xQueue) {
          vQueueDelete(client->_xQueue);
          client->_xQueue = NULL;
     }
     tbcm_destroy(client->tbmqttclient);

     TBC_FREE(client);
}

tbcm_handle_t _tbcmh_get_tbcm_handle(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return NULL;
     }
     return client->tbmqttclient;    
}

bool tbcmh_connect(tbcmh_handle_t client_, const tbc_transport_config_t* config,
                            void *context,
                            tbcmh_on_connected_t on_connected,
                            tbcmh_on_disconnected_t on_disconnected)
{
    tbcmh_t *client = (tbcmh_t *)client_;
    if (!client) {
         TBC_LOGE("client is NULL! %s()", __FUNCTION__);
         return false;
    }
    if (!config) {
         TBC_LOGE("config is NULL! %s()", __FUNCTION__);
         return false;
    }
    if (!client->tbmqttclient) {
         TBC_LOGE("client->tbmqttclient is NULL! %s()", __FUNCTION__);
         return false;
    }
    if (!tbcm_is_disconnected(client->tbmqttclient)) {
         TBC_LOGI("It already connected to thingsboard MQTT server!");
         return false;
    }

    // SemaphoreHandle_t lock;
    // int next_request_id;
    // uint64_t last_check_timestamp;
    // tbcmh_request_list_t request_list; /*!< request list: attributes request, client side RPC & ota update request */ ////QueueHandle_t timeoutQueue;
    
    // connect
    TBC_LOGI("connecting to %s://%s:%d ...",
                config->address.schema, config->address.host, config->address.port);
    bool result = tbcm_connect(client->tbmqttclient, config,
                               client,
                               _tbcmh_bridge_event_send);
                               //_tbcmh_on_event_handle);
    if (!result) {
         TBC_LOGW("client->tbmqttclient connect failure! %s()", __FUNCTION__);
         return false;
    }
    
    // cache config & callback
    tbc_transport_storage_fill_from_config(&client->config, config);
    client->context = context;
    client->on_connected = on_connected;       /*!< Callback of connected ThingsBoard MQTT */
    client->on_disconnected = on_disconnected; /*!< Callback of disconnected ThingsBoard MQTT */

    return true;
}

void tbcmh_disconnect(tbcmh_handle_t client_)               
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }
     if (!client->tbmqttclient) {
          TBC_LOGE("client->tbmqttclient is NULL! %s()", __FUNCTION__);
          return;
     }
     if (tbcm_is_disconnected(client->tbmqttclient)) {
          TBC_LOGI("It already disconnected from thingsboard MQTT server!");
          return;
     }

     TBC_LOGI("disconnecting from %s://%s:%d ...",
                client->config.address.schema, client->config.address.host, client->config.address.port);
     // empty msg queue
     while (tbcmh_has_events(client_)) {
          tbcmh_run(client_);
     }
     // disconnect
     tbcm_disconnect(client->tbmqttclient);
     while (tbcmh_has_events(client_)) {
          tbcmh_run(client_);
     }

     // clear config & callback
     tbc_transport_storage_free_fields(&client->config);
     client->context = NULL;
     client->on_connected = NULL;
     client->on_disconnected = NULL;

     // empty all request lists;
     //_tbcmh_telemetry_empty(client_);
     //_tbcmh_clientattribute_empty(client_);
     //_tbcmh_sharedattribute_empty(client_);
     _tbcmh_attributesrequest_empty(client_);
     //_tbcmh_serverrpc_empty(client_);
     _tbcmh_clientrpc_empty(client_);
     _tbcmh_provision_empty(client_);
     _tbcmh_otaupdate_empty(client_);

     // SemaphoreHandle_t lock;
     // int next_request_id;
     // uint64_t last_check_timestamp;
     // remove all item in request_list
     tbcmh_request_t *tbcmh_request = NULL, *next;
     LIST_FOREACH_SAFE(tbcmh_request, &client->request_list, entry, next) {
          // exec timeout callback
          /*if (tbcmh_request->on_timeout) {
               tbcm_on_timeout_t on_timeout = tbcmh_request->on_timeout;
               on_timeout(context, tbcmh_request->request_id);
          }*/
          // deal timeout
          switch (tbcmh_request->type) {
          case TBCMH_REQUEST_ATTRIBUTES:
              _tbcmh_attributesrequest_on_timeout(client_, tbcmh_request->request_id);
              break;
          case TBCMH_REQUEST_CLIENTRPC:
              _tbcmh_clientrpc_on_timeout(client_, tbcmh_request->request_id);
              break;
          case TBCMH_REQUEST_PROVISION:
              _tbcmh_provision_on_timeout(client_, tbcmh_request->request_id);
              break;
          case TBCMH_REQUEST_FWUPDATE:
              _tbcmh_otaupdate_on_timeout(client_, tbcmh_request->request_id);
              break;
          default:
              TBC_LOGE("tbcmh_request->type(%d) is error!\r\n", tbcmh_request->type);
              break;
          }

          // remove from request list
          LIST_REMOVE(tbcmh_request, entry);
          _request_destroy(tbcmh_request);
     }
     memset(&client->request_list, 0x00, sizeof(client->request_list));
}

bool tbcmh_is_connected(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (client && client->tbmqttclient && tbcm_is_connected(client->tbmqttclient)) {
          return true;
     }
     return false;
}

static void _tbcmh_connected_on(tbcmh_handle_t client_) //onConnected() // First receive
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;
     }

     _tbcmh_otaupdate_on_connected(client_);

     // clone parameter in lock/unlock
     void *context = client->context;
     tbcmh_on_connected_t on_connected = client->on_connected; /*!< Callback of connected ThingsBoard MQTT */
     //_response_timer_start(client);

     // Give semaphore
     xSemaphoreGive(client->_lock);
  
     // do callback
     on_connected(client, context);
     return;
}
static void _tbcmh_disonnected_on(tbcmh_handle_t client_) //onDisonnected() // First receive
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;
     }

     // clone parameter in lock/unlock
     void *context = client->context;
     tbcmh_on_disconnected_t on_disconnected = client->on_disconnected; /*!< Callback of disconnected ThingsBoard MQTT */

     // Give semaphore
     xSemaphoreGive(client->_lock);
  
     // do callback
     on_disconnected(client, context);
     return;
}

void tbcmh_check_timeout(tbcmh_handle_t client_) // Executes an event loop for PubSub client. //loop()==>checkTimeout()
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL");
          return;
     }

     // Too early
     uint64_t timestamp = (uint64_t)time(NULL);
     if (timestamp < client->last_check_timestamp + TB_MQTT_TIMEOUT + 2) {
          return;
     }
     client->last_check_timestamp = timestamp;

     // move timeout item to timeout_list
     tbcmh_request_list_t timeout_list = LIST_HEAD_INITIALIZER(timeout_list);
     int count = _request_list_move_all_of_timeout(client, timestamp, &timeout_list);
     if (count <=0 ) {
          return;
     }

     tbcmh_request_t *tbcmh_request = NULL, *next;
     LIST_FOREACH_SAFE(tbcmh_request, &timeout_list, entry, next) {
          // exec timeout callback
          /*if (tbcmh_request->on_timeout) {
               tbcm_on_timeout_t on_timeout = tbcmh_request->on_timeout;
               on_timeout(client->context, tbcmh_request->request_id);
          }*/
          // deal timeout
          switch (tbcmh_request->type) {
          case TBCMH_REQUEST_ATTRIBUTES:
              _tbcmh_attributesrequest_on_timeout(client_, tbcmh_request->request_id);
              break;
          case TBCMH_REQUEST_CLIENTRPC:
              _tbcmh_clientrpc_on_timeout(client_, tbcmh_request->request_id);
              break;
          case TBCMH_REQUEST_PROVISION:
              _tbcmh_provision_on_timeout(client_, tbcmh_request->request_id);
              break;
          case TBCMH_REQUEST_FWUPDATE:
              _tbcmh_otaupdate_on_timeout(client_, tbcmh_request->request_id);
              break;
          default:
              TBC_LOGE("tbcmh_request->type(%d) is error!\r\n", tbcmh_request->type);
              break;
          }

          // remove from request list
          LIST_REMOVE(tbcmh_request, entry);
          _request_destroy(tbcmh_request);
     }
}

tbc_err_t tbcmh_subscribe(tbcmh_handle_t client_, const char *topic)
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     int result = _tbcm_subscribe(client->tbmqttclient, topic, 1/*qos*/);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return result==ESP_OK ? ESP_OK : ESP_FAIL;
}

//====40.Claiming device using device-side key scenario============================================
tbc_err_t tbcmh_claiming_device_using_device_side_key(tbcmh_handle_t client_,
                    const char *secret_key, uint32_t *duration_ms)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // send package...
     cJSON *object = cJSON_CreateObject(); // create json object
     if (secret_key) {
        cJSON_AddStringToObject(object, TB_MQTT_KEY_CLAIMING_DEVICE_SECRETKEY, secret_key);
     }
     if (duration_ms) {
        cJSON_AddNumberToObject(object, TB_MQTT_KEY_CLAIMING_DEVICE_DURATIONMS, *duration_ms);
     }
     char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
     int result = tbcm_claiming_device_publish(client->tbmqttclient, pack, 1/*qos*/, 0/*retain*/);
     cJSON_free(pack); // free memory
     cJSON_Delete(object); // delete json object

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return (result > -1) ? ESP_OK : ESP_FAIL;
}

                    
/*static*/ bool _request_is_equal(const tbcmh_request_t *a, const tbcmh_request_t *b)
{
     if (!a & !b) {
          return true;
     }
     if (!a || !b) {
          return false;
     }
     if (a->type != b->type) {
          return false;
     } else if (a->request_id != b->request_id) {
          return false;
     } else { // if (a->timestamp != b->timestamp)
          return a->timestamp == b->timestamp;
     }
}

//return request_id on successful, otherwise return -1
/*static*/ int _request_list_create_and_append(tbcmh_handle_t client_, tbcmh_request_type_t type, int request_id)
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL!");
          return -1;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore!");
          return -1;
     }

     // Get request ID
     if (request_id <= 0) {
          do {
               if (client->next_request_id <= 0)
                    client->next_request_id = 1;
               else
                    client->next_request_id++;
          } while (client->next_request_id <= 0);

          request_id = client->next_request_id;
     }

     // Create request
     tbcmh_request_t *tbcmh_request = _request_create(type, request_id);
     if (!tbcmh_request) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Unable to create request: No memory!");
          return -1;
     }

     // Insert request to list
     tbcmh_request_t *it, *last = NULL;
     if (LIST_FIRST(&client->request_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->request_list, tbcmh_request, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->request_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, tbcmh_request, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return request_id;
}

/*static*/ void _request_list_search_and_remove(tbcmh_handle_t client_, int request_id)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL!");
          return;// NULL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore!");
          return;// NULL;
     }

     // Search item
     tbcmh_request_t *tbcmh_request = NULL;
     LIST_FOREACH(tbcmh_request, &client->request_list, entry) {
          if (tbcmh_request && tbcmh_request->request_id == request_id) {
               break;
          }
     }

     /// Remove form list
     if (tbcmh_request) {
          LIST_REMOVE(tbcmh_request, entry);
     } else {
          TBC_LOGW("Unable to remove request:%d!", request_id);
     }

     _request_destroy(tbcmh_request);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return; // tbcmh_request;
}


/*static*/ void _request_list_search_and_remove_by_type(tbcmh_handle_t client_, tbcmh_request_type_t type)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL!");
          return; // NULL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore!");
          return; // NULL;
     }

     // Search item
     tbcmh_request_t *tbcmh_request = NULL;
     LIST_FOREACH(tbcmh_request, &client->request_list, entry) {
          if (tbcmh_request && tbcmh_request->type == type) {
               break;
          }
     }

     /// Remove form list
     if (tbcmh_request) {
          LIST_REMOVE(tbcmh_request, entry);
     } else {
          TBC_LOGW("Unable to remove request: type=%d!", type);
     }

     _request_destroy(tbcmh_request);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     //return tbcmh_request;
}

// return  count of timeout_request_list
/*static*/ int _request_list_move_all_of_timeout(tbcmh_handle_t client_, uint64_t timestamp,
                              tbcmh_request_list_t *timeout_request_list)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL!");
          return 0;
     }
     if (!timeout_request_list) {
          TBC_LOGE("timeout_request_list is NULL!");
          return 0;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore!");
          return 0;
     }

     // Search & move item
     int count = 0;
     tbcmh_request_t *tbcmh_request = NULL, *next;
     LIST_FOREACH_SAFE(tbcmh_request, &client->request_list, entry, next) {
          if (tbcmh_request && tbcmh_request->timestamp + TB_MQTT_TIMEOUT <= timestamp) {
               // remove from request list
               LIST_REMOVE(tbcmh_request, entry);

               // append to timeout list
               tbcmh_request_t *it, *last = NULL;
               if (LIST_FIRST(timeout_request_list) == NULL) {
                    LIST_INSERT_HEAD(timeout_request_list, tbcmh_request, entry);
                    count++;
               } else {
                    LIST_FOREACH(it, timeout_request_list, entry) {
                         last = it;
                    }
                    if (it == NULL) {
                         assert(last);
                         LIST_INSERT_AFTER(last, tbcmh_request, entry);
                         count++;
                    }
               }
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return count;
}

//return tbcmh_request on successful, otherwise return NULL.
static tbcmh_request_t *_request_create(tbcmh_request_type_t type,
                                       uint32_t request_id)
{
     tbcmh_request_t *tbcmh_request = TBC_MALLOC(sizeof(tbcmh_request_t));
     if (!tbcmh_request) {
          TBC_LOGE("Unable to malloc memory!");
          return NULL;
     }

     memset(tbcmh_request, 0x00, sizeof(tbcmh_request_t));
     tbcmh_request->type = type;
     tbcmh_request->request_id = request_id;
     tbcmh_request->timestamp = (uint64_t)time(NULL);

     return tbcmh_request;
}

static void _request_destroy(tbcmh_request_t *tbcmh_request)
{
     if (!tbcmh_request) {
          TBC_LOGE("Invalid argument!");
          return; 
     }

     tbcmh_request->type = 0;
     tbcmh_request->request_id = 0;
     tbcmh_request->timestamp = 0;
     TBC_FREE(tbcmh_request);
}

static void _tbmch_on_payload_handle(tbcma_event_t *event)
{
    TBC_CHECK_PTR(event);
    TBC_CHECK_PTR(event->user_context);
    
    tbcmh_t *client = (tbcmh_t *)event->user_context;
    cJSON *object = NULL;

    if (event->event_id != TBC_MQTT_EVENT_DATA) {
        TBC_LOGE("event->event_id(%d) is not TBC_MQTT_EVENT_DATA(%d)!", event->event_id, TBC_MQTT_EVENT_DATA);
        return;
    }
    
    switch (event->data.topic) {
    case TBCMA_RX_TOPIC_ATTRIBUTES_RESPONSE:  /*!< request_id,           payload, payload_len */
         object = cJSON_ParseWithLength(event->data.payload, event->data.payload_len);
         _tbcmh_attributesrequest_on_response(client, event->data.request_id, object);
         cJSON_Delete(object);
         break;
    
    case TBCMA_RX_TOPIC_SHARED_ATTRIBUTES:    /*!<                       payload, payload_len */
         object = cJSON_ParseWithLength(event->data.payload, event->data.payload_len);
         _tbcmh_sharedattribute_on_received(client, object);
         cJSON_Delete(object);
         break;
    
    case TBCMA_RX_TOPIC_SERVERRPC_REQUEST:    /*!< request_id,           payload, payload_len */
         object = cJSON_ParseWithLength(event->data.payload, event->data.payload_len);
         _tbcmh_serverrpc_on_request(client, event->data.request_id, object);
         cJSON_Delete(object);
         break;
        
    case TBCMA_RX_TOPIC_CLIENTRPC_RESPONSE:   /*!< request_id,           payload, payload_len */
         object = cJSON_ParseWithLength(event->data.payload, event->data.payload_len);
         _tbcmh_clientrpc_on_response(client, event->data.request_id, object);
         cJSON_Delete(object);
         break;
    
    case TBCMA_RX_TOPIC_FW_RESPONSE:          /*!< request_id, chunk_id, payload, payload_len */
         _tbcmh_otaupdate_on_response(client, event->data.request_id, 
              event->data.chunk_id, 
              event->data.payload,
              event->data.payload_len);
         break;
    
    case TBCMA_RX_TOPIC_PROVISION_RESPONSE:   /*!< (no request_id)       payload, payload_len */
         object = cJSON_ParseWithLength(event->data.payload, event->data.payload_len);
         _tbcmh_provision_on_response(client, event->data.request_id, object);
         cJSON_Delete(object);
         break;

    case TBCMA_RX_TOPIC_ERROR:
    default:
         TBC_LOGW("Other topic: event->data.topic=%d", event->data.topic);
         break;
    }
}

// The callback for when a MQTT event is received.
static void _tbcmh_on_event_handle(tbcma_event_t *event)
{
     TBC_CHECK_PTR(event);
     TBC_CHECK_PTR(event->user_context);

     tbcmh_t *client = (tbcmh_t *)event->user_context;
     int msg_id;

     switch (event->event_id) {
     case TBC_MQTT_EVENT_BEFORE_CONNECT:
          TBC_LOGI("TBC_MQTT_EVENT_BEFORE_CONNECT, msg_id=%d", event->msg_id);
          break;

     case TBC_MQTT_EVENT_CONNECTED:
          TBC_LOGI("TBC_MQTT_EVENT_CONNECTED");
          TBC_LOGI("client->tbmqttclient = %p", client->tbmqttclient);
          msg_id = _tbcm_subscribe(client->tbmqttclient, TB_MQTT_TOPIC_SHARED_ATTRIBUTES, 0);
          TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s", msg_id, TB_MQTT_TOPIC_SHARED_ATTRIBUTES);
          msg_id = _tbcm_subscribe(client->tbmqttclient, TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_SUBSCRIRBE, 0);
          TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s", msg_id, TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_SUBSCRIRBE);
          msg_id = _tbcm_subscribe(client->tbmqttclient, TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE, 0);
          TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s", msg_id, TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE);
          msg_id = _tbcm_subscribe(client->tbmqttclient, TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE, 0);
          TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s", msg_id, TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE);
          msg_id = _tbcm_subscribe(client->tbmqttclient, TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE, 0);
          TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s", msg_id, TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE);
          TBC_LOGI("before call on_connected()...");
          TBC_LOGI("Connected to thingsboard MQTT server!");
          _tbcmh_connected_on(client);
          TBC_LOGI("after call on_connected()");
          break;

     case TBC_MQTT_EVENT_DISCONNECTED:
          TBC_LOGI("TBC_MQTT_EVENT_DISCONNECTED");
          TBC_LOGI("Disconnected to thingsboard MQTT server!");
          _tbcmh_disonnected_on(client);
          break;

     case TBC_MQTT_EVENT_SUBSCRIBED:
          TBC_LOGI("TBC_MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
          break;
     case TBC_MQTT_EVENT_UNSUBSCRIBED:
          TBC_LOGI("TBC_MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
          break;
     case TBC_MQTT_EVENT_PUBLISHED:
          TBC_LOGI("TBC_MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
          break;

     case TBC_MQTT_EVENT_DATA:
          TBC_LOGI("TBC_MQTT_EVENT_DATA");
          ////TBC_LOGI("DATA=%.*s", event->payload_len, event->payload);
          _tbmch_on_payload_handle(event);
          break;

     case TBC_MQTT_EVENT_ERROR:
          TBC_LOGW("TBC_MQTT_EVENT_ERROR: esp_tls_last_esp_err=%d, esp_tls_stack_err=%d, "
             "esp_tls_cert_verify_flags=%d, error_type=%d, "
             "connect_return_code=%d, esp_transport_sock_errno=%d",
             event->error_handle.esp_tls_last_esp_err,              /*!< last esp_err code reported from esp-tls component */
             event->error_handle.esp_tls_stack_err,                 /*!< tls specific error code reported from underlying tls stack */
             event->error_handle.esp_tls_cert_verify_flags,         /*!< tls flags reported from underlying tls stack during certificate verification */
                                                            /* esp-mqtt specific structure extension */
             event->error_handle.error_type,                /*!< error type referring to the source of the error */
             event->error_handle.connect_return_code,       /*!< connection refused error code reported from MQTT broker on connection */
                                                            /* tcp_transport extension */
             event->error_handle.esp_transport_sock_errno); /*!< errno from the underlying socket */
          break;

     case TBC_MQTT_EVENT_DELETED:
          TBC_LOGW("TBC_MQTT_EVENT_DELETED: msg_id=%d", event->msg_id);
          break;

     case TBC_MQTT_EVENT_CHECK_TIMEOUT:
          TBC_LOGD("TBC_MQTT_EVENT_CHECK_TIMEOUT");
          tbcmh_check_timeout(client);
          break;

     default:
          TBC_LOGW("Other event: event_id=%d", event->event_id);
          break;
     }
}

//recv & deal msg from queue
//recv/parse/sendqueue/ack...
static void _tbcmh_bridge_event_receive(tbcmh_handle_t client_)
{
    tbcmh_t *client = (tbcmh_t *)client_;
    if (!client) {
         TBC_LOGE("client is NULL! %s()", __FUNCTION__);
         return; // false;
    }
    
    // TODO: whether to insert lock?
    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return false;
    // }
    
    // read event from queue()
    tbcma_event_t event;
    int i = 0;
    if (client->_xQueue != 0) {
         // Receive a message on the created queue.  Block for 0(10) ticks if a message is not immediately available.
         while (i < 10 && xQueueReceive(client->_xQueue, &event, (TickType_t)0)) { // 10
              // pcRxedMessage now points to the struct event variable posted by vATask.
              _tbcmh_on_event_handle(&event);
              
              if ((event.event_id==TBC_MQTT_EVENT_DATA) && event.data.payload && (event.data.payload_len>0)) {
                  TBC_FREE(event.data.payload);
                  event.data.payload = NULL;
              }
    
              i++;
         }
    }
    
    // Give semaphore
    // xSemaphoreGive(client->_lock);
    return;// sendResult == pdTRUE ? true : false;
}

void tbcmh_run(tbcmh_handle_t client_)
{
    _tbcmh_bridge_event_receive(client_);
}

bool tbcmh_has_events(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return false;
     }

     tbcma_event_t event;
     if (client->_xQueue != 0)
     {
          // Peek a message on the created queue.  Block for 10 ticks if a
          // event is not immediately available.
          if (xQueuePeek(client->_xQueue, &event, (TickType_t)10))
          {
               // msg now container to the tbcmh_msg_t variable posted
               // by vATask, but the item still remains on the queue.
               return true;
          }
     }

     return false;
}


//=====================================================================================================================

#if 1 //move from MOVE_TO_MQTT_ADADTER

//send event to queue
//true if the item was successfully posted, otherwise false. //pdTRUE if the item was successfully posted, otherwise errQUEUE_FULL.
//It runs in MQTT thread.
static bool _tbcmh_sendTbmqttEvent2Queue(tbcmh_handle_t client_, tbcma_event_t *event)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !client->_xQueue || !event) {
          TBC_LOGE("client, client->_xQueue or event is NULL!");
          return false;
     }

     // TODO: whether/how to insert lock?
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore!");
     //      return false;
     // }

     // Send a pointer to a struct event object.  Don't block if the queue is already full.
     int i = 0;
     BaseType_t sendResult;
     do {
          sendResult = xQueueSend(client->_xQueue, (void *)event, (TickType_t)10);
          i++;

          if (sendResult != pdTRUE) {
               TBC_LOGW("_xQueue is full!");
          }
     } while (i < 20 && sendResult != pdTRUE);
     if (i >= 20) {
          TBC_LOGW("send innermsg timeout! %s()", __FUNCTION__);
     }

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return sendResult == pdTRUE ? true : false;
}

// send msg to queue with clone event & publish_data
static void _tbcmh_bridge_event_send(tbcma_event_t *event)
{
    TBC_CHECK_PTR(event);
    TBC_CHECK_PTR(event->user_context);

    tbcmh_t *client = (tbcmh_t *)event->user_context;

    if ((event->event_id==TBC_MQTT_EVENT_DATA) && event->data.payload && (event->data.payload_len>0)) {
        char *payload = TBC_MALLOC(event->data.payload_len);
        if (!payload) {
            TBC_LOGE("malloc(%d) memory failure! %s()", event->data.payload_len, __FUNCTION__);
            return;            
        }
        memcpy(payload, event->data.payload, event->data.payload_len);
        event->data.payload = payload;
    }

    _tbcmh_sendTbmqttEvent2Queue(client, event);
}

#else 

//send msg to queue
//true if the item was successfully posted, otherwise false. //pdTRUE if the item was successfully posted, otherwise errQUEUE_FULL.
//It runs in MQTT thread.
static bool _tbcmh_sendTbmqttMsg2Queue(tbcmh_handle_t client_, tbcmh_msg_t *msg)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !client->_xQueue || !msg) {
          TBC_LOGE("client, client->_xQueue or msg is NULL!");
          return false;
     }

     // TODO: whether/how to insert lock?
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore!");
     //      return false;
     // }

     // Send a pointer to a struct AMessage object.  Don't block if the queue is already full.
     int i = 0;
     BaseType_t sendResult;
     do {
          sendResult = xQueueSend(client->_xQueue, (void *)msg, (TickType_t)10);
          i++;

          if (sendResult != pdTRUE) {
               TBC_LOGW("_xQueue is full!");
          }
     } while (i < 20 && sendResult != pdTRUE);
     if (i >= 20) {
          TBC_LOGW("send innermsg timeout! %s()", __FUNCTION__);
     }

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return sendResult == pdTRUE ? true : false;
}

static void _tbcmh_on_connected(void *context) // First receive
{
     tbcmh_t *client = (tbcmh_t *)context;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_CONNECTED;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}
static void _tbcmh_on_disonnected(void *context) // First receive
{
     tbcmh_t *client = (tbcmh_t *)context;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_DISCONNECTED;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}
// First receive
static void _tbcmh_on_sharedattr_received(void *context, const char* payload, int length)
{
     tbcmh_t *client = (tbcmh_t *)context;
     if (!client ) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }
     if (!payload) {
          TBC_LOGE("payload is NULL! %s()", __FUNCTION__);;
          return;
     }
     if (length<=0) {
          TBC_LOGE("payload length(%d) is error! %s()", length, __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_SHAREDATTR_RECEIVED;
     msg.body.sharedattr_received.object = cJSON_ParseWithLength(payload, length);  /*!< received palyload. free memory by msg receiver */
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}

// First send
/*static*/ void _tbcmh_on_attrrequest_response(void *context, int request_id, const char* payload, int length)
{
     tbcmh_t *client = (tbcmh_t *)context;
     if (!client ) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }
     if (!payload) {
          TBC_LOGE("payload is NULL! %s()", __FUNCTION__);
          return;
     }
     if (length<=0) {
          TBC_LOGE("payload length(%d) is error! %s()", length, __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_ATTRREQUEST_RESPONSE;
     msg.body.attrrequest_response.request_id = request_id;
     msg.body.attrrequest_response.object = cJSON_ParseWithLength(payload, length);  /*!< received palyload. free memory by msg receiver */
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
} 
// First send
/*static*/ void _tbcmh_on_attrrequest_timeout(void *context, int request_id)
{
     tbcmh_t *client = (tbcmh_t *)context;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_ATTRREQUEST_TIMEOUT;
     msg.body.attrrequest_timeout.request_id = request_id;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}

// First receive
static void _tbcmh_on_serverrpc_request(void *context, int request_id, const char* payload, int length)
{
     tbcmh_t *client = (tbcmh_t *)context;
     if (!client ) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }
     if (!payload) {
          TBC_LOGE("payload is NULL! %s()", __FUNCTION__);
          return;
     }
     if (length<=0) {
          TBC_LOGE("payload length(%d) is error! %s()", length, __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_SERVERRPC_REQUSET;
     msg.body.serverrpc_request.request_id = request_id;
     msg.body.serverrpc_request.object = cJSON_ParseWithLength(payload, length); /*!< received palyload. free memory by msg receiver */;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}

// First send
/*static*/ void _tbcmh_on_clientrpc_response(void *context, int request_id, const char* payload, int length)
{
     tbcmh_t *client = (tbcmh_t *)context;
     if (!client ) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }
     if (!payload) {
          TBC_LOGE("payload is NULL! %s()", __FUNCTION__);
          return;
     }
     if (length<=0) {
          TBC_LOGE("payload length(%d) is error! %s()", length, __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_CLIENTRPC_RESPONSE;
     msg.body.clientrpc_response.request_id = request_id;
     msg.body.clientrpc_response.object = cJSON_ParseWithLength(payload, length); /*!< received palyload. free memory by msg receiver */;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}
// First send
/*static*/ void _tbcmh_on_clientrpc_timeout(void *context, int request_id)
{
     tbcmh_t *client = (tbcmh_t *)context;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_CLIENTRPC_TIMEOUT;
     msg.body.clientrpc_timeout.request_id = request_id;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
} 

/*static*/ void _tbcmh_on_provision_response(void *context, int request_id, const char* payload, int length)
{
     tbcmh_t *client = (tbcmh_t *)context;
     if (!client ) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }
     if (!payload) {
          TBC_LOGE("payload is NULL! %s()", __FUNCTION__);
          return;
     }
     if (length<=0) {
          TBC_LOGE("payload length(%d) is error! %s()", length, __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_PROVISION_RESPONSE;
     msg.body.provision_response.request_id = request_id;
     msg.body.provision_response.object = cJSON_ParseWithLength(payload, length); /*!< received palyload. free memory by msg receiver */;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}

/*static*/ void _tbcmh_on_provision_timeout(void *context, int request_id)
{
     tbcmh_t *client = (tbcmh_t *)context;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_PROVISION_TIMEOUT;
     msg.body.provision_timeout.request_id = request_id;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}

// First send  
/*static*/ void _tbcmh_on_otaupdate_response(void *context, int request_id, int chunk_id, const char* payload, int length)
{
     tbcmh_t *client = (tbcmh_t *)context;
     if (!client ) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }
     if (!payload) {
          TBC_LOGE("payload is NULL! %s()", __FUNCTION__);
          return;
     }
     if (length<=0) {
          TBC_LOGE("payload length(%d) is error! %s()", length, __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_FWUPDATE_RESPONSE;
     msg.body.otaupdate_response.request_id = request_id;
     msg.body.otaupdate_response.chunk_id = chunk_id;
     msg.body.otaupdate_response.payload = TBC_MALLOC(length); /*!< received palyload. free memory by msg receiver */
     if (msg.body.otaupdate_response.payload) {
          memcpy(msg.body.otaupdate_response.payload, payload, length);
     }
     msg.body.otaupdate_response.length = length;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}
// First send
/*static*/ void _tbcmh_on_otaupdate_timeout(void *context, int request_id)
{
     tbcmh_t *client = (tbcmh_t *)context;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_FWUPDATE_TIMEOUT;
     msg.body.otaupdate_timeout.request_id = request_id;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}

// TODO: ......
static void __response_timer_timerout(void *client_/*timer_arg*/)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_TIMER_TIMEOUT;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}
#endif

#if 0 //move to MOVE_TO_MQTT_ADADTER
static void __response_timer_timerout(void *client_/*timer_arg*/)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     tbcmh_msg_t msg;
     msg.id = TBCMH_MSGID_TIMER_TIMEOUT;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}

static void _response_timer_create(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     esp_timer_create_args_t tmr_args = {
        .callback = &__response_timer_timerout,
        .arg = client_,
        .name = "response_timer",
     };
     esp_timer_create(&tmr_args, &client->respone_timer);
}
static void _response_timer_start(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     esp_timer_start_periodic(client->respone_timer, (uint64_t)TB_MQTT_TIMEOUT * 1000 * 1000);
}
static void _response_timer_stop(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     esp_timer_stop(client->respone_timer);
}
static void _response_timer_destroy(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     esp_timer_stop(client->respone_timer);
     esp_timer_delete(client->respone_timer);
     client->respone_timer = NULL;
}
#endif

#if 1  //extension party
static char *_create_string(const char *ptr, int len)
{
    char *ret;
    if (len <= 0) {
        return NULL;
    }
    ret = calloc(1, len + 1);
    if (!ret) {
        TBC_LOGE("%s(%d): %s",  __FUNCTION__, __LINE__, "Memory exhausted");
        return NULL;
    }

    memcpy(ret, ptr, len);
    return ret;
}

static esp_err_t _parse_uri(tbc_transport_address_storage_t *address,
                                tbc_transport_credentials_storage_t *credentials,
                                const char *uri)
{
    struct http_parser_url puri;
    http_parser_url_init(&puri);
    int parser_status = http_parser_parse_url(uri, strlen(uri), 0, &puri);
    if (parser_status != 0) {
        ESP_LOGE(TAG, "Error parse uri = %s", uri);
        return ESP_FAIL;
    }

    address->schema = _create_string(uri + puri.field_data[UF_SCHEMA].off, puri.field_data[UF_SCHEMA].len);
    address->host = _create_string(uri + puri.field_data[UF_HOST].off, puri.field_data[UF_HOST].len);
    address->path = NULL;

    if (puri.field_data[UF_PATH].len || puri.field_data[UF_QUERY].len) {
        if (puri.field_data[UF_QUERY].len == 0) {
            asprintf(&address->path, "%.*s", puri.field_data[UF_PATH].len, uri + puri.field_data[UF_PATH].off);
        } else if (puri.field_data[UF_PATH].len == 0)  {
            asprintf(&address->path, "/?%.*s", puri.field_data[UF_QUERY].len, uri + puri.field_data[UF_QUERY].off);
        } else {
            asprintf(&address->path, "%.*s?%.*s", puri.field_data[UF_PATH].len, uri + puri.field_data[UF_PATH].off,
                     puri.field_data[UF_QUERY].len, uri + puri.field_data[UF_QUERY].off);
        }

        if (!address->path) {
            TBC_LOGE("%s(%d): %s",  __FUNCTION__, __LINE__, "Memory exhausted");
            return ESP_ERR_NO_MEM;
        }
    }

    if (puri.field_data[UF_PORT].len) {
        address->port = strtol((const char *)(uri + puri.field_data[UF_PORT].off), NULL, 10);
    }

    char *user_info = _create_string(uri + puri.field_data[UF_USERINFO].off, puri.field_data[UF_USERINFO].len);
    if (user_info) {
        char *pass = strchr(user_info, ':');
        if (pass) {
            pass[0] = 0; //terminal username
            pass ++;
            credentials->password = strdup(pass);
        }
        credentials->username = strdup(user_info);

        free(user_info);
    }
   
    return ESP_OK;
}

bool tbcmh_connect_using_url(tbcmh_handle_t client_, const tbc_transport_config_esay_t *config,
                   void *context,
                   tbcmh_on_connected_t on_connected,
                   tbcmh_on_disconnected_t on_disconnected)
{
    if (!config) {
         TBC_LOGE("config is NULL! %s()", __FUNCTION__);
         return false;
    }
    if (!config->uri) {
         TBC_LOGE("config->uri is NULL! %s()", __FUNCTION__);
         return false;
    }
    if (!config->access_token) {
         TBC_LOGE("config->access_token is NULL! %s()", __FUNCTION__);
         return false;
    }

    bool result = false;
    tbc_transport_address_storage_t address = {0};
    tbc_transport_credentials_storage_t credentials = {0};
    tbc_transport_config_t transport = {0};
    if (_parse_uri(&address, &credentials, config->uri) != ESP_OK) {
        TBC_LOGE("parse config->uri(%s) failure! %s()", config->uri, __FUNCTION__);
        goto fail_exit;
    }
    transport.address.schema = address.schema;
    transport.address.host   = address.host;
    transport.address.port   = address.port;
    transport.address.path   = address.path;
    transport.credentials.username = credentials.username;
    transport.credentials.password = credentials.password;
    transport.credentials.token = config->access_token;
    transport.credentials.type = TBC_TRANSPORT_CREDENTIALS_TYPE_ACCESS_TOKEN;
    transport.log_rxtx_package = config->log_rxtx_package;
    result = tbcmh_connect(client_, &transport, context, on_connected, on_disconnected);

fail_exit:
    TBC_FIELD_FREE(address.schema);
    TBC_FIELD_FREE(address.host);
    TBC_FIELD_FREE(address.path);
    TBC_FIELD_FREE(credentials.username);
    TBC_FIELD_FREE(credentials.password);
    return result;
}
#endif

