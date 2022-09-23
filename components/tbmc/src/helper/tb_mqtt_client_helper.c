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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "sys/queue.h"
#include "esp_err.h"

#include "tb_mqtt_client.h"
#include "tb_mqtt_client_helper.h"

#include "tb_mqtt_client_helper_log.h"
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
{                                  //param1        param1    param3    param4
  TBMCH_MSGID_TIMER_TIMEOUT = 1,   //
  TBMCH_MSGID_CONNECTED,           //
  TBMCH_MSGID_DISCONNECTED,        //
  TBMCH_MSGID_SHAREDATTR_RECEIVED, //                        cJSON
  TBMCH_MSGID_ATTRREQUEST_RESPONSE,//request_id,             cJSON
  TBMCH_MSGID_ATTRREQUEST_TIMEOUT, //request_id
  TBMCH_MSGID_SERVERRPC_REQUSET,   //request_id,             cJSON
  TBMCH_MSGID_CLIENTRPC_RESPONSE,  //request_id,             cJSON
  TBMCH_MSGID_CLIENTRPC_TIMEOUT,   //request_id
  TBMCH_MSGID_FWUPDATE_RESPONSE,   //request_id,   chunk,    payload,  len
  TBMCH_MSGID_FWUPDATE_TIMEOUT     //request_id
} tbmch_msgid_t;

typedef struct tbmch_msg_easy {
     int reserved;
} tbmch_msg_easy_t;

typedef struct tbmch_msg_sharedattr_received {
     cJSON *object; /*!< received palyload. free memory by msg receiver */
} tbmch_msg_sharedattr_received_t;

typedef struct tbmch_msg_response {
     int32_t request_id;
     cJSON *object; /*!< received palyload. free memory by msg receiver */
} tbmch_msg_response_t;

typedef struct tbmch_msg_fwupdate_response {
     int32_t request_id;
     int chunk;
     char *payload; /*!< received palyload. free memory by msg receiver */
     int length;
} tbmch_msg_fwupdate_response_t;

typedef struct tbmch_msg_timeout {
     int32_t request_id;
} tbmch_msg_timeout_t;

typedef union tbmch_msgbody {
     tbmch_msg_easy_t timer_timeout; //

     tbmch_msg_easy_t connected;    //
     tbmch_msg_easy_t disconnected; //

     tbmch_msg_sharedattr_received_t sharedattr_received; // cJSON

     tbmch_msg_response_t attrrequest_response; // request_id, cJSON
     tbmch_msg_timeout_t attrrequest_timeout;   // request_id

     tbmch_msg_response_t serverrpc_request; // request_id, cJSON

     tbmch_msg_response_t clientrpc_response; // request_id, cJSON
     tbmch_msg_timeout_t clientrpc_timeout;   // request_id

     tbmch_msg_fwupdate_response_t fwupdate_response; // request_id, chunk, payload, len
     tbmch_msg_timeout_t fwupdate_timeout;            // request_id
} tbmch_msgbody_t;

typedef struct tbmch_msg {
	tbmch_msgid_t   id;
	tbmch_msgbody_t body;
} tbmch_msg_t;

/**
 * Reference tbmch_config_t
 */
typedef struct
{
    bool log_rxtx_package; /*!< print Rx/Tx MQTT package */
    
    char *uri;             /*!< Complete ThingsBoard MQTT broker URI */
    char *access_token;    /*!< ThingsBoard Access Token */
    char *cert_pem;        /*!< Reserved. Pointer to certificate data in PEM format for server verify (with SSL), default is NULL, not required to verify the server */
    char *client_cert_pem; /*!< Reserved. Pointer to certificate data in PEM format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_key_pem` has to be provided. */
    char *client_key_pem;  /*!< Reserved. Pointer to private key data in PEM format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_cert_pem` has to be provided. */
    
    void *context;                           /*!< Context parameter of the below two callbacks */
    tbmch_on_connected_t on_connected;       /*!< Callback of connected ThingsBoard MQTT */
    tbmch_on_disconnected_t on_disconnected; /*!< Callback of disconnected ThingsBoard MQTT */
} tbmch_config_storage_t;

/**
 * ThingsBoard MQTT Client Helper 
 */
typedef struct tbmch_client
{
     // TODO: add a lock???
     // create & destroy
     tbmc_handle_t tbmqttclient;
     QueueHandle_t _xQueue;
     esp_timer_handle_t respone_timer;   // /*!< timer for checking response timeout */

     // modify at connect & disconnect
     tbmch_config_storage_t config;

     // tx & rx msg
     SemaphoreHandle_t _lock;
     LIST_HEAD(tbmch_tsdata_list, tbmch_tsdata) tsdata_list;                              /*!< telemetry time-series data entries */
     LIST_HEAD(tbmch_clientattribute_list, tbmch_clientattribute) clientattribute_list;   /*!< client attributes entries */
     LIST_HEAD(tbmch_sharedattribute_list, tbmch_sharedattribute) sharedattribute_list;   /*!< shared attributes entries */
     LIST_HEAD(tbmch_attributesrequest_list, tbmch_attributesrequest) attributesrequest_list;  /*!< attributes request entries */
     LIST_HEAD(tbmch_serverrpc_list, tbmch_serverrpc) serverrpc_list;  /*!< server side RPC entries */
     LIST_HEAD(tbmch_clientrpc_list, tbmch_clientrpc) clientrpc_list;  /*!< client side RPC entries */
     LIST_HEAD(tbmch_fwupdate_list, tbmch_fwupdate) fwupdate_list;    /*!< A device may have multiple firmware */
} tbmch_t;

static void _tbmch_on_connected(void *context);
static void _tbmch_on_disonnected(void *context);
static void _tbmch_on_sharedattr_received(void *context, const char* payload, int length);
static void _tbmch_on_attrrequest_response(void *context, int request_id, const char* payload, int length);
static void _tbmch_on_attrrequest_timeout(void *context, int request_id);
static void _tbmch_on_serverrpc_request(void *context, int request_id, const char* payload, int length);
static void _tbmch_on_clientrpc_response(void *context, int request_id, const char* payload, int length);
static void _tbmch_on_clientrpc_timeout(void *context, int request_id);
static void _tbmch_on_fwupdate_response(void *context, int request_id, int chunk, const char* payload, int length);
static void _tbmch_on_fwupdate_timeout(void *context, int request_id);

static void _response_timer_create(tbmch_handle_t client_);
static void _response_timer_start(tbmch_handle_t client_);
static void _response_timer_stop(tbmch_handle_t client_);
static void _response_timer_destroy(tbmch_handle_t client_);

static tbmch_err_t _tbmch_clientattribute_xx_append(tbmch_handle_t client_, const char *key, void *context,
                                                  tbmch_clientattribute_on_get_t on_get,
                                                  tbmch_clientattribute_on_set_t on_set);
static void _tbmch_fwupdate_on_sharedattributes(tbmch_handle_t client_,
                                                 const char *fw_title, const char *fw_version,
                                                 const char *fw_checksum, const char *fw_checksum_algorithm);

static tbmch_err_t _tbmch_telemetry_empty(tbmch_handle_t client_);
static tbmch_err_t _tbmch_clientattribute_empty(tbmch_handle_t client_);
static tbmch_err_t _tbmch_sharedattribute_empty(tbmch_handle_t client_);
static tbmch_err_t _tbmch_attributesrequest_empty(tbmch_handle_t client_);
static tbmch_err_t _tbmch_serverrpc_empty(tbmch_handle_t client_);
static tbmch_err_t _tbmch_clientrpc_empty(tbmch_handle_t client_);
static tbmch_err_t _tbmch_fwupdate_empty(tbmch_handle_t client_);

const static char *TAG = "tb_mqtt_client_helper";

//====0.tbmc client====================================================================================================
tbmch_handle_t tbmch_init(void)
{
     tbmch_t *client = (tbmch_t *)TBMCH_MALLOC(sizeof(tbmch_t));
     if (!client) {
          TBMCH_LOGE("Unable to malloc memory!");
          return NULL;
     }
     memset(client, 0x00, sizeof(tbmch_t));

     client->tbmqttclient = tbmc_init();
     // Create a queue capable of containing 20 tbmch_msg_t structures. These should be passed by pointer as they contain a lot of data.
     client->_xQueue = xQueueCreate(40, sizeof(tbmch_msg_t));
     if (client->_xQueue == NULL) {
          TBMCH_LOGE("failed to create the queue!");
     }
     _response_timer_create(client);

     client->config.log_rxtx_package = false;
     client->config.uri = NULL;
     client->config.access_token = NULL;
     client->config.cert_pem = NULL;
     client->config.client_cert_pem = NULL;
     client->config.client_key_pem = NULL;
     client->config.context = NULL;
     client->config.on_connected = NULL; 
     client->config.on_disconnected = NULL;

     client->_lock = xSemaphoreCreateMutex();
     if (client->_lock == NULL)  {
          TBMCH_LOGE("failed to create the lock!");
     }
     
     memset(&client->tsdata_list, 0x00, sizeof(client->tsdata_list)); //client->tsdata_list = LIST_HEAD_INITIALIZER(client->tsdata_list);
     memset(&client->clientattribute_list, 0x00, sizeof(client->clientattribute_list)); //client->clientattribute_list = LIST_HEAD_INITIALIZER(client->clientattribute_list);
     memset(&client->sharedattribute_list, 0x00, sizeof(client->sharedattribute_list)); //client->sharedattribute_list = LIST_HEAD_INITIALIZER(client->sharedattribute_list);
     memset(&client->attributesrequest_list, 0x00, sizeof(client->attributesrequest_list)); //client->attributesrequest_list = LIST_HEAD_INITIALIZER(client->attributesrequest_list);
     memset(&client->serverrpc_list, 0x00, sizeof(client->serverrpc_list)); //client->serverrpc_list = LIST_HEAD_INITIALIZER(client->serverrpc_list);
     memset(&client->clientrpc_list, 0x00, sizeof(client->clientrpc_list)); //client->clientrpc_list = LIST_HEAD_INITIALIZER(client->clientrpc_list);
     memset(&client->fwupdate_list, 0x00, sizeof(client->fwupdate_list)); //client->fwupdate_list = LIST_HEAD_INITIALIZER(client->fwupdate_list);

     return client;
}
void tbmch_destroy(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;
     }

     // TODO: dead lock???
     tbmch_disconnect(client);

     // empty all 7 list!
     _tbmch_telemetry_empty(client_);
     _tbmch_clientattribute_empty(client_);
     _tbmch_sharedattribute_empty(client_);
     _tbmch_attributesrequest_empty(client_);
     _tbmch_serverrpc_empty(client_);
     _tbmch_clientrpc_empty(client_);
     _tbmch_fwupdate_empty(client_);

     if (client->_lock) {
          vSemaphoreDelete(client->_lock);
          client->_lock = NULL;
     }

     // config in tbmch_disconnect()

     _response_timer_stop(client);
     _response_timer_destroy(client);
     if (client->_xQueue) {
          vQueueDelete(client->_xQueue);
          client->_xQueue = NULL;
     }
     tbmc_destroy(client->tbmqttclient);

     TBMCH_FREE(client);
}

//~~tbmch_config(); //move to tbmch_connect()
//~~tbmch_set_ConnectedEvent(evtConnected); //move to tbmch_init()
//~~tbmch_set_DisconnectedEvent(evtDisconnected); //move to tbmch_init()
bool tbmch_connect(tbmch_handle_t client_, const tbmch_config_t *config) //_begin()
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return false;
     }
     if (!config) {
          TBMCH_LOGE("config is NULL!");
          return false;
     }
     if (!client->tbmqttclient) {
          TBMCH_LOGE("client->tbmqttclient is NULL!");
          return false;
     }
     if (!tbmc_is_disconnected(client->tbmqttclient)) {
          TBMCH_LOGI("It already connected to thingsboard MQTT server!");
          return false;
     }

     // connect
     TBMCH_LOGI("connecting to %s...", config->uri);
     tbmc_config_t tbmc_config = {
         .uri = config->uri,                            /*!< Complete MQTT broker URI */
         .access_token = config->access_token,          /*!< Access Token */
         .cert_pem = config->cert_pem,                  /*!< Reserved. Pointer to certificate data in PEM format for server verify (with SSL), default is NULL, not required to verify the server */
         .client_cert_pem = config->client_cert_pem,    /*!< Reserved. Pointer to certificate data in PEM format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_key_pem` has to be provided. */
         .client_key_pem = config->client_key_pem,      /*!< Reserved. Pointer to private key data in PEM format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_cert_pem` has to be provided. */

         .log_rxtx_package = config->log_rxtx_package   /*!< print Rx/Tx MQTT package */
     };
     bool result = tbmc_connect(client->tbmqttclient, &tbmc_config,
                                client,
                                _tbmch_on_connected,
                                _tbmch_on_disonnected,
                                _tbmch_on_sharedattr_received,
                                _tbmch_on_serverrpc_request);
     if (!result) {
          TBMCH_LOGW("client->tbmqttclient connect failure!");
          return false;
     }

     // cache config & callback
     client->config.log_rxtx_package = config->log_rxtx_package; /*!< print Rx/Tx MQTT package */
     if (client->config.uri) {
          free(client->config.uri);
          client->config.uri = NULL;
     }
     if (config->uri && strlen(config->uri)>0) {
          client->config.uri = strdup(config->uri); /*!< ThingsBoard MQTT host uri */
     }
     if (client->config.access_token) {
          free(client->config.access_token);
          client->config.access_token = NULL;
     }
     if (config->access_token && strlen(config->access_token)>0) {
          client->config.access_token = strdup(config->access_token); /*!< ThingsBoard MQTT token */
     }
     if (client->config.cert_pem) {
          free(client->config.cert_pem);
          client->config.cert_pem = NULL;
     }
     if (config->cert_pem && strlen(config->cert_pem)>0) {
          client->config.cert_pem = strdup(config->cert_pem); /*!< Reserved. Pointer to certificate data in PEM format for server verify (with SSL), default is NULL, not required to verify the server */
     }
     if (client->config.client_cert_pem) {
          free(client->config.client_cert_pem);
          client->config.client_cert_pem = NULL;
     }
     if (config->client_cert_pem && strlen(config->client_cert_pem)>0) {
          client->config.client_cert_pem = strdup(config->client_cert_pem); /*!< Reserved. Pointer to certificate data in PEM format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_key_pem` has to be provided. */
     }
     if (client->config.client_key_pem) {
          free(client->config.client_key_pem);
          client->config.client_key_pem = NULL;
     }
     if (config->client_key_pem && strlen(config->client_key_pem)>0) {
          client->config.client_key_pem = strdup(config->client_key_pem); /*!< ThingsBoard MQTT token */
     }
     client->config.context = config->context;
     client->config.on_connected = config->on_connected;       /*!< Callback of connected ThingsBoard MQTT */
     client->config.on_disconnected = config->on_disconnected; /*!< Callback of disconnected ThingsBoard MQTT */

     return true;
}
//_end()
void tbmch_disconnect(tbmch_handle_t client_)               
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;
     }
     if (!client->tbmqttclient) {
          TBMCH_LOGE("client->tbmqttclient is NULL!");
          return;
     }
     if (tbmc_is_disconnected(client->tbmqttclient)) {
          TBMCH_LOGI("It already disconnected from thingsboard MQTT server!");
          return;
     }

     TBMCH_LOGI("disconnecting from %s...", client->config.uri);

     // empty msg queue
     while (tbmch_has_events(client_)) {
          tbmch_run(client_);
     }
     // disconnect
     tbmc_disconnect(client->tbmqttclient);
     while (tbmch_has_events(client_)) {
          tbmch_run(client_);
     }

     // clear config & callback
     client->config.log_rxtx_package = false;
     if (client->config.uri) {
          free(client->config.uri);
     }
     client->config.uri = NULL;
     if (client->config.access_token) {
          free(client->config.access_token);
     }
     client->config.access_token = NULL;
     if (client->config.cert_pem) {
          free(client->config.cert_pem);
     }
     client->config.cert_pem = NULL;
     if (client->config.client_cert_pem) {
          free(client->config.client_cert_pem);
     }
     client->config.client_cert_pem = NULL;
     if (client->config.client_key_pem) {
          free(client->config.client_key_pem);
     }
     client->config.client_key_pem = NULL;
     client->config.context = NULL;
     client->config.on_connected = NULL;
     client->config.on_disconnected = NULL;

     // empty all request lists;
     //_tbmch_telemetry_empty(client_);
     //_tbmch_clientattribute_empty(client_);
     //_tbmch_sharedattribute_empty(client_);
     _tbmch_attributesrequest_empty(client_);
     //_tbmch_serverrpc_empty(client_);
     _tbmch_clientrpc_empty(client_);
     _tbmch_fwupdate_empty(client_);
}
bool tbmch_is_connected(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (client && client->tbmqttclient && tbmc_is_connected(client->tbmqttclient)) {
          return true;
     }
     return false;
}

static void _tbmch_connected_on(tbmch_handle_t client_) //onConnected() // First receive
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL");
          return;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return;
     }

     // clone parameter in lock/unlock
     void *context = client->config.context;
     tbmch_on_connected_t on_connected = client->config.on_connected; /*!< Callback of connected ThingsBoard MQTT */
     _response_timer_start(client);

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
          TBMCH_LOGE("client is NULL");
          return;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
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
          TBMCH_LOGE("client is NULL");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Create tsdata
     tbmch_tsdata_t *tsdata = _tbmch_tsdata_init(client_, key/*, type*/, context, on_get/*, on_set*/);
     if (!tsdata) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCH_LOGE("Init tsdata failure! key=%s", key);
          return ESP_FAIL;
     }

     // Insert tsdata to list
     tbmch_tsdata_t *it, *last = NULL;
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
          TBMCH_LOGE("client or key is NULL!");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Search item
     tbmch_tsdata_t *tsdata = NULL;
     LIST_FOREACH(tsdata, &client->tsdata_list, entry) {
          if (tsdata && strcmp(_tbmch_tsdata_get_key(tsdata), key)==0) {
               break;
          }
     }
     if (!tsdata) {
          TBMCH_LOGW("Unable to remove time-series data:%s!", key);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(tsdata, entry);
     _tbmch_tsdata_destroy(tsdata);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

static tbmch_err_t _tbmch_telemetry_empty(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBMCH_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in tsdata_list
     tbmch_tsdata_t *tsdata = NULL, *next;
     LIST_FOREACH_SAFE(tsdata, &client->tsdata_list, entry, next) {
          // remove from tsdata list and destory
          LIST_REMOVE(tsdata, entry);
          _tbmch_tsdata_destroy(tsdata);
     }
     memset(&client->tsdata_list, 0x00, sizeof(client->tsdata_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}
////tbmqttlink.h.tbmch_sendTelemetry();
tbmch_err_t tbmch_telemetry_send(tbmch_handle_t client_, int count, /*const char *key,*/ ...)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return ESP_FAIL;
     }
     if (count <= 0) {
          TBMCH_LOGE("count(%d) is error!", count);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     int i;
     va_list ap;
     va_start(ap, count);
     cJSON *object = cJSON_CreateObject(); // create json object
     for (i=0; i<count; i++) {
          const char *key = va_arg(ap, const char*);

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
               TBMCH_LOGW("Unable to find&send time-series data:%s!", key);
          }
     }
     va_end(ap);

     // send package...
     char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
     int result = tbmc_telemetry_publish(client->tbmqttclient, pack, 1/*qos*/, 0/*retain*/);
     cJSON_free(pack); // free memory
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
          TBMCH_LOGE("on_set is NULL!");
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
          TBMCH_LOGE("client is NULL");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Create clientattribute
     tbmch_clientattribute_t *clientattribute = _tbmch_clientattribute_init(client_, key, context, on_get, on_set);
     if (!clientattribute) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCH_LOGE("Init clientattribute failure! key=%s", key);
          return ESP_FAIL;
     }

     // Insert clientattribute to list
     tbmch_clientattribute_t *it, *last = NULL;
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
          TBMCH_LOGE("client or key is NULL!");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Search item
     tbmch_clientattribute_t *clientattribute = NULL;
     LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
          if (clientattribute && strcmp(_tbmch_clientattribute_get_key(clientattribute), key)==0) {
               break;
          }
     }
     if (!clientattribute) {
          TBMCH_LOGW("Unable to remove client attribute: %s!", key);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(clientattribute, entry);
     _tbmch_clientattribute_destroy(clientattribute);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}
static tbmch_err_t _tbmch_clientattribute_empty(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBMCH_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in clientattribute_list
     tbmch_clientattribute_t *clientattribute = NULL, *next;
     LIST_FOREACH_SAFE(clientattribute, &client->clientattribute_list, entry, next) {
          // remove from clientattribute list and destory
          LIST_REMOVE(clientattribute, entry);
          _tbmch_clientattribute_destroy(clientattribute);
     }
     memset(&client->clientattribute_list, 0x00, sizeof(client->clientattribute_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}
tbmch_err_t tbmch_clientattribute_send(tbmch_handle_t client_, int count, /*const char *key,*/ ...) ////tbmqttlink.h.tbmch_sendClientAttributes();
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return ESP_FAIL;
     }
     if (count <= 0) {
          TBMCH_LOGE("count(%d) is error!", count);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     int i;
     va_list ap;
     va_start(ap, count);
     cJSON *object = cJSON_CreateObject(); // create json object
     for (i=0; i<count; i++) {
          const char *key = va_arg(ap, const char*);

          // Search item
          tbmch_clientattribute_t *clientattribute = NULL;
          LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
               if (clientattribute && strcmp(_tbmch_clientattribute_get_key(clientattribute), key)==0) {
                    break;
               }
          }

          /// Add clientattribute to package
          if (clientattribute) {
               _tbmch_clientattribute_do_get(clientattribute, object); // add item to json object
          } else {
               TBMCH_LOGW("Unable to remove time-series data:%s!", key);
          }
     }
     va_end(ap);

     // send package...
     char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
     int result = tbmc_clientattributes_publish(client->tbmqttclient, pack, 1/*qos*/, 0/*retain*/);
     cJSON_free(pack); // free memory
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
          TBMCH_LOGE("client or object is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
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
          TBMCH_LOGE("client is NULL");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Create sharedattribute
     tbmch_sharedattribute_t *sharedattribute = _tbmch_sharedattribute_init(client_, key, context, on_set);
     if (!sharedattribute) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCH_LOGE("Init sharedattribute failure! key=%s", key);
          return ESP_FAIL;
     }

     // Insert sharedattribute to list
     tbmch_sharedattribute_t *it, *last = NULL;
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
          TBMCH_LOGE("client or key is NULL!");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Search item
     tbmch_sharedattribute_t *sharedattribute = NULL;
     LIST_FOREACH(sharedattribute, &client->sharedattribute_list, entry) {
          if (sharedattribute && strcmp(_tbmch_sharedattribute_get_key(sharedattribute), key)==0) {
               break;
          }
     }
     if (!sharedattribute) {
          TBMCH_LOGW("Unable to remove shared-attribute:%s!", key);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(sharedattribute, entry);
     _tbmch_sharedattribute_destroy(sharedattribute);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}
static tbmch_err_t _tbmch_sharedattribute_empty(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBMCH_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in sharedattribute_list
     tbmch_sharedattribute_t *sharedattribute = NULL, *next;
     LIST_FOREACH_SAFE(sharedattribute, &client->sharedattribute_list, entry, next) {
          // remove from sharedattribute list and destory
          LIST_REMOVE(sharedattribute, entry);
          _tbmch_sharedattribute_destroy(sharedattribute);
     }
     memset(&client->sharedattribute_list, 0x00, sizeof(client->sharedattribute_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

//unpack & deal
//onAttrOfSubReply()
static void _tbmch_sharedattribute_on_received(tbmch_handle_t client_, const cJSON *object)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !object) {
          TBMCH_LOGE("client or object is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
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
          char *fw_title = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_SHAREDATTRBUTE_FW_TITLE));
          char *fw_version = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_SHAREDATTRBUTE_FW_VERSION));
          char *fw_checksum = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_SHAREDATTRBUTE_FW_CHECKSUM));
          char *fw_checksum_algorithm = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_SHAREDATTRBUTE_FW_CHECKSUM_ALG));
          _tbmch_fwupdate_on_sharedattributes(client, fw_title, fw_version, fw_checksum, fw_checksum_algorithm);
     }

     return;// ESP_OK;
}

//====4.Request client-side or shared device attributes from the server================================================
static tbmch_err_t _tbmch_attributesrequest_empty(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBMCH_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in attributesrequest_list
     tbmch_attributesrequest_t *attributesrequest = NULL, *next;
     LIST_FOREACH_SAFE(attributesrequest, &client->attributesrequest_list, entry, next) {
          // exec timeout callback
          _tbmch_attributesrequest_do_timeout(attributesrequest);

          // remove from attributesrequest list and destory
          LIST_REMOVE(attributesrequest, entry);
          _tbmch_attributesrequest_destroy(attributesrequest);
     }
     memset(&client->attributesrequest_list, 0x00, sizeof(client->attributesrequest_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

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
          TBMCH_LOGE("client is NULL");
          return ESP_FAIL;
     }
     if (count <= 0) {
          TBMCH_LOGE("count(%d) is error!", count);
          return ESP_FAIL;
     }

     // Take semaphore, malloc client_keys & shared_keys
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
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
          const char *key = va_arg(ap, const char*);

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

          // Search item in sharedattribute
          tbmch_sharedattribute_t *sharedattribute = NULL;
          LIST_FOREACH(sharedattribute, &client->sharedattribute_list, entry) {
               if (sharedattribute && strcmp(_tbmch_sharedattribute_get_key(sharedattribute), key)==0) {
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

          TBMCH_LOGW("Unable to remove time-series data:%s!", key);
     }
     va_end(ap);

     // Send msg to server
     int request_id = tbmc_attributes_request_ex(client->tbmqttclient, client_keys, shared_keys,
                               client,
                               _tbmch_on_attrrequest_response,
                               _tbmch_on_attrrequest_timeout,
                               1/*qos*/, 0/*retain*/);
     if (request_id<0) {
          TBMCH_LOGE("Init tbmc_attributes_request failure!");
          goto attributesrequest_fail;
     }

     // Create attributesrequest
     tbmch_attributesrequest_t *attributesrequest = _tbmch_attributesrequest_init(client_, request_id, context, on_response, on_timeout);
     if (!attributesrequest) {
          TBMCH_LOGE("Init attributesrequest failure!");
          goto attributesrequest_fail;
     }

     // Insert attributesrequest to list
     tbmch_attributesrequest_t *it, *last = NULL;
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
          TBMCH_LOGE("client or object is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return;// ESP_FAIL;
     }

     // Search attributesrequest
     tbmch_attributesrequest_t *attributesrequest = NULL; 
     LIST_FOREACH(attributesrequest, &client->attributesrequest_list, entry) {
          if (attributesrequest && (_tbmch_attributesrequest_get_request_id(attributesrequest)==request_id)) {
               break;
          }
     }
     if (!attributesrequest) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCH_LOGW("Unable to find attribute request:%d!", request_id);
          return;// ESP_FAIL;
     }

     // Cache and remove attributesrequest
     tbmch_attributesrequest_t *cache = _tbmch_attributesrequest_clone_wo_listentry(attributesrequest);
     LIST_REMOVE(attributesrequest, entry);
     _tbmch_attributesrequest_destroy(attributesrequest);
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
     _tbmch_attributesrequest_destroy(cache);

     return;// ESP_OK;
}
// onAttributesResponseTimeout()
static void _tbmch_attributesrequest_on_timeout(tbmch_handle_t client_, int request_id)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return;// ESP_FAIL;
     }

     // Search attributesrequest
     tbmch_attributesrequest_t *attributesrequest = NULL;
     LIST_FOREACH(attributesrequest, &client->attributesrequest_list, entry) {
          if (attributesrequest && (_tbmch_attributesrequest_get_request_id(attributesrequest)==request_id)) {
               break;
          }
     }
     if (!attributesrequest) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCH_LOGW("Unable to find attribute request:%d!", request_id);
          return;// ESP_FAIL;
     }

     // Cache and remove attributesrequest
     tbmch_attributesrequest_t *cache = _tbmch_attributesrequest_clone_wo_listentry(attributesrequest);
     LIST_REMOVE(attributesrequest, entry);
     _tbmch_attributesrequest_destroy(attributesrequest);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do timeout
     _tbmch_attributesrequest_do_timeout(cache);
     // Free attributesrequest
     _tbmch_attributesrequest_destroy(cache);

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
          TBMCH_LOGE("client is NULL");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Create serverrpc
     tbmch_serverrpc_t *serverrpc = _tbmch_serverrpc_init(client, method, context, on_request);
     if (!serverrpc) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCH_LOGE("Init serverrpc failure! method=%s", method);
          return ESP_FAIL;
     }

     // Insert serverrpc to list
     tbmch_serverrpc_t *it, *last = NULL;
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
     if (!client || !method) {
          TBMCH_LOGE("client or method is NULL!");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Search item
     tbmch_serverrpc_t *serverrpc = NULL;
     LIST_FOREACH(serverrpc, &client->serverrpc_list, entry) {
          if (serverrpc && strcmp(_tbmch_serverrpc_get_method(serverrpc), method)==0) {
               break;
          }
     }
     if (!serverrpc)  {
          TBMCH_LOGW("Unable to remove server-rpc:%s!", method);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(serverrpc, entry);
     _tbmch_serverrpc_destroy(serverrpc);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}
static tbmch_err_t _tbmch_serverrpc_empty(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBMCH_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in serverrpc_list
     tbmch_serverrpc_t *serverrpc = NULL, *next;
     LIST_FOREACH_SAFE(serverrpc, &client->serverrpc_list, entry, next) {
          // remove from serverrpc list and destory
          LIST_REMOVE(serverrpc, entry);
          _tbmch_serverrpc_destroy(serverrpc);
     }
     memset(&client->serverrpc_list, 0x00, sizeof(client->serverrpc_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}


////parseServerSideRpc(msg)
static void _tbmch_serverrpc_on_request(tbmch_handle_t client_, int request_id, const cJSON *object)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !object) {
          TBMCH_LOGE("client or object is NULL!");
          return;// ESP_FAIL;
     }

     const char *method = NULL;
     if (cJSON_HasObjectItem(object, TB_MQTT_TEXT_RPC_METHOD)) {
          cJSON *methodItem = cJSON_GetObjectItem(object, TB_MQTT_TEXT_RPC_METHOD);
          method = cJSON_GetStringValue(methodItem);
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
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
          TBMCH_LOGW("Unable to deal server-rpc:%s!", method);
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
          const char *response = cJSON_PrintUnformatted(reply); //cJSON_Print()
          tbmc_serverrpc_response(client_->tbmqttclient, request_id, response, 1/*qos*/, 0/*retain*/);
          cJSON_free(response); // free memory
          cJSON_Delete(reply); // delete json object
          #else
          char *response = cJSON_PrintUnformatted(result); //cJSON_Print(result);
          tbmc_serverrpc_response(client_->tbmqttclient, request_id, response, 1/*qos*/, 0/*retain*/);
          cJSON_free(response); // free memory
          cJSON_Delete(result); // delete json object
          #endif
     }
     // Free serverrpc
     _tbmch_serverrpc_destroy(cache);

     return;// ESP_OK;
}

//====6.Client-side RPC================================================================================================
static tbmch_err_t _tbmch_clientrpc_empty(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBMCH_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in clientrpc_list
     tbmch_clientrpc_t *clientrpc = NULL, *next;
     LIST_FOREACH_SAFE(clientrpc, &client->clientrpc_list, entry, next) {
          // exec timeout callback
          _tbmch_clientrpc_do_timeout(clientrpc);

          // remove from clientrpc list and destory
          LIST_REMOVE(clientrpc, entry);
          _tbmch_clientrpc_destroy(clientrpc);
     }
     memset(&client->clientrpc_list, 0x00, sizeof(client->clientrpc_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

////tbmqttlink.h.tbmch_sendClientRpcRequest(); //add list
int tbmch_clientrpc_of_oneway_request(tbmch_handle_t client_, const char *method,
                                                           /*const*/ tbmch_rpc_params_t *params)
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL");
          return ESP_FAIL;
     }
     if (!method) {
          TBMCH_LOGE("method is NULL");
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
     char *params_str = cJSON_PrintUnformatted(object); //cJSON_Print(object);
     int request_id = tbmc_clientrpc_request_ex(client->tbmqttclient, method, params_str,
                              client,
                              _tbmch_on_clientrpc_response,
                              _tbmch_on_clientrpc_timeout,
                              1/*qos*/, 0/*retain*/);
     cJSON_free(params_str); // free memory
     cJSON_Delete(object); // delete json object
     if (request_id<0) {
          TBMCH_LOGE("Init tbmc_clientrpc_request failure!");
          return ESP_FAIL;
     }

     return request_id;
}
////tbmqttlink.h.tbmch_sendClientRpcRequest(); //create to add to LIST_ENTRY(tbmch_clientrpc_)
int tbmch_clientrpc_of_twoway_request(tbmch_handle_t client_, const char *method, 
                                                           /*const*/ tbmch_rpc_params_t *params,
                                                           void *context,
                                                           tbmch_clientrpc_on_response_t on_response,
                                                           tbmch_clientrpc_on_timeout_t on_timeout)
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL");
          return ESP_FAIL;
     }
     if (!method) {
          TBMCH_LOGE("method is NULL");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Send msg to server
     cJSON *object = cJSON_CreateObject(); // create json object
     cJSON_AddStringToObject(object, TB_MQTT_TEXT_RPC_METHOD, method);
     if (params)
          cJSON_AddItemReferenceToObject(object, TB_MQTT_TEXT_RPC_PARAMS, params);
     else 
          cJSON_AddNullToObject(object, TB_MQTT_TEXT_RPC_PARAMS);
     char *params_str = cJSON_PrintUnformatted(object); //cJSON_Print(object);
     int request_id = tbmc_clientrpc_request_ex(client->tbmqttclient, method, params_str,
                              client,
                              _tbmch_on_clientrpc_response,
                              _tbmch_on_clientrpc_timeout,
                               1/*qos*/, 0/*retain*/);
     cJSON_free(params_str); // free memory
     cJSON_Delete(object); // delete json object
     if (request_id<0) {
          TBMCH_LOGE("Init tbmc_clientrpc_request failure!");
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Create clientrpc
     tbmch_clientrpc_t *clientrpc = _tbmch_clientrpc_init(client, request_id, method, context, on_response, on_timeout);
     if (!clientrpc) {
          TBMCH_LOGE("Init clientrpc failure!");
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Insert clientrpc to list
     tbmch_clientrpc_t *it, *last = NULL;
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
          TBMCH_LOGE("client or object is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
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
          TBMCH_LOGW("Unable to find client-rpc:%d!", request_id);
          return;// ESP_FAIL;
     }

     // Cache and remove clientrpc
     tbmch_clientrpc_t *cache = _tbmch_clientrpc_clone_wo_listentry(clientrpc);
     LIST_REMOVE(clientrpc, entry);
     _tbmch_clientrpc_destroy(clientrpc);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do response
     _tbmch_clientrpc_do_response(cache, cJSON_GetObjectItem(object, TB_MQTT_TEXT_RPC_RESULTS));
     // Free cache
     _tbmch_clientrpc_destroy(cache);

     return;// ESP_OK;
}
//onClientRpcResponseTimeout()
static void _tbmch_clientrpc_on_timeout(tbmch_handle_t client_, int request_id)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
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
          TBMCH_LOGW("Unable to find client-rpc:%d!", request_id);
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
                                  tbmch_fwupdate_on_response_t on_fw_response,
                                  tbmch_fwupdate_on_success_t on_fw_success,
                                  tbmch_fwupdate_on_timeout_t on_fw_timeout)
{
     tbmch_t *client = (tbmch_t*)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Create fwupdate
     tbmch_fwupdate_t *fwupdate = _tbmch_fwupdate_init(client, fw_title, context, 
                                                       on_fw_attributes, on_fw_response,
                                                       on_fw_success, on_fw_timeout);
     if (!fwupdate) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBMCH_LOGE("Init fwupdate failure! fw_title=%s", fw_title);
          return ESP_FAIL;
     }

     // Insert fwupdate to list
     tbmch_fwupdate_t *it, *last = NULL;
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
     if (!client || !fw_title) {
          TBMCH_LOGE("client or fw_title is NULL!");
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return ESP_FAIL;
     }

     // Search item
     tbmch_fwupdate_t *fwupdate = NULL;
     LIST_FOREACH(fwupdate, &client->fwupdate_list, entry) {
          if (fwupdate && strcmp(_tbmch_fwupdate_get_title(fwupdate), fw_title)==0) {
               break;
          }
     }
     if (!fwupdate) {
          TBMCH_LOGW("Unable to remove fw_update data:%s!", fw_title);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(fwupdate, entry);
     _tbmch_fwupdate_destroy(fwupdate);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}
static tbmch_err_t _tbmch_fwupdate_empty(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBMCH_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in fwupdate_list
     tbmch_fwupdate_t *fwupdate = NULL, *next;
     LIST_FOREACH_SAFE(fwupdate, &client->fwupdate_list, entry, next) {
          // exec timeout callback
          _tbmch_fwupdate_do_timeout(fwupdate);

          // remove from fwupdate list and destory
          LIST_REMOVE(fwupdate, entry);
          _tbmch_fwupdate_destroy(fwupdate);
     }
     // memset(&client->fwupdate_list, 0x00, sizeof(client->fwupdate_list));

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}
static void _tbmch_fwupdate_on_sharedattributes(tbmch_handle_t client_,
                                                 const char *fw_title, const char *fw_version,
                                                 const char *fw_checksum, const char *fw_checksum_algorithm)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !fw_title) {
          TBMCH_LOGE("client or fw_title is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return;// ESP_FAIL;
     }

     // Search item
     tbmch_fwupdate_t *fwupdate = NULL;
     LIST_FOREACH(fwupdate, &client->fwupdate_list, entry) {
          if (fwupdate && strcmp(_tbmch_fwupdate_get_title(fwupdate), fw_title)==0) {
               break;
          }
     }
     if (!fwupdate) {
          TBMCH_LOGW("Unable to find fw_update:%s!", fw_title);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return;// ESP_FAIL;
     }

     // exec fw_update
     bool result = _tbmch_fwupdate_do_sharedattributes(fwupdate, fw_title, fw_version, fw_checksum, fw_checksum_algorithm);
     if (result) {
          int request_id = tbmc_fwupdate_request(client->tbmqttclient, -1 /*request_id*/, 0 /*chunk*/,
                                                 NULL /*payload*/,
                                                 client,
                                                 _tbmch_on_fwupdate_response,
                                                 _tbmch_on_fwupdate_timeout,
                                                 1/*qos*/, 0/*retain*/);
          if (request_id>0) {
               _tbmch_fwupdate_set_request_id(fwupdate, request_id);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return;// ESP_OK;
}

static void _tbmch_fwupdate_on_response(tbmch_handle_t client_, int request_id, int chunk, const char* payload, int length)
{
      tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return;// ESP_FAIL;
     }

     // Search item
     tbmch_fwupdate_t *fwupdate = NULL;
     LIST_FOREACH(fwupdate, &client->fwupdate_list, entry) {
          if (fwupdate && (_tbmch_fwupdate_get_request_id(fwupdate)==request_id)) {
               break;
          }
     }
     if (!fwupdate) {
          TBMCH_LOGW("Unable to find fw_update:%d!", request_id);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return;// ESP_FAIL;
     }

     // exec fw response
     int result = _tbmch_fwupdate_do_response(fwupdate, chunk/*current chunk*/, payload, length);
     // 1: go on, get next package
     if (result == 1) {
          tbmc_fwupdate_request(client->tbmqttclient, request_id, chunk + length/*chunk*/,
                                NULL /*payload*/,
                                client,
                                _tbmch_on_fwupdate_response,
                                _tbmch_on_fwupdate_timeout,
                                1/*qos*/, 0/*retain*/);
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return;// ESP_OK;
}
static void _tbmch_fwupdate_on_timeout(tbmch_handle_t client_, int request_id)
{
      tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client  is NULL!");
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCH_LOGE("Unable to take semaphore!");
          return;// ESP_FAIL;
     }

     // Search item
     tbmch_fwupdate_t *fwupdate = NULL;
     LIST_FOREACH(fwupdate, &client->fwupdate_list, entry) {
          if (fwupdate && (_tbmch_fwupdate_get_request_id(fwupdate)==request_id)) {
               break;
          }
     }
     if (!fwupdate) {
          TBMCH_LOGW("Unable to find fw_update:%d!", request_id);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return;// ESP_FAIL;
     }

     // exec fw timeout
     _tbmch_fwupdate_do_timeout(fwupdate);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return;// ESP_OK;
}

bool tbmch_has_events(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return false;
     }

     tbmch_msg_t msg;
     if (client->_xQueue != 0)
     {
          // Peek a message on the created queue.  Block for 10 ticks if a
          // message is not immediately available.
          if (xQueuePeek(client->_xQueue, &(msg), (TickType_t)10))
          {
               // msg now container to the tbmch_msg_t variable posted
               // by vATask, but the item still remains on the queue.
               return true;
          }
     }

     return false;
}

static int32_t _tbmch_deal_msg(tbmch_handle_t client_, tbmch_msg_t *msg)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return ESP_FAIL;
     }
     if (!msg) {
          TBMCH_LOGE("msg is NULL!");
          return ESP_FAIL;
     }

     // deal msg
     switch (msg->id) {
     case TBMCH_MSGID_TIMER_TIMEOUT:       //
          tbmc_check_timeout(client_->tbmqttclient);
          break;

     case TBMCH_MSGID_CONNECTED:            //
          TBMCH_LOGI("Connected to thingsboard MQTT server!");
          _tbmch_connected_on(client_);
          break;
     case TBMCH_MSGID_DISCONNECTED:         //
          TBMCH_LOGI("Disconnected to thingsboard MQTT server!");
          _tbmch_disonnected_on(client_);
          break;

     case TBMCH_MSGID_SHAREDATTR_RECEIVED:  //                         cJSON
          _tbmch_sharedattribute_on_received(client_, msg->body.sharedattr_received.object);
          cJSON_Delete(msg->body.sharedattr_received.object); //free cJSON
          break;

     case TBMCH_MSGID_ATTRREQUEST_RESPONSE: // request_id,             cJSON
          _tbmch_attributesrequest_on_response(client_, msg->body.attrrequest_response.request_id,
                                               msg->body.attrrequest_response.object);
          cJSON_Delete(msg->body.attrrequest_response.object); // free cJSON
          break;
     case TBMCH_MSGID_ATTRREQUEST_TIMEOUT:  // request_id
          _tbmch_attributesrequest_on_timeout(client_, msg->body.attrrequest_timeout.request_id);
          break;

     case TBMCH_MSGID_SERVERRPC_REQUSET:    // request_id,             cJSON
          _tbmch_serverrpc_on_request(client_, msg->body.serverrpc_request.request_id, 
                                      msg->body.serverrpc_request.object);
          cJSON_Delete(msg->body.serverrpc_request.object); //free cJSON
          break;

     case TBMCH_MSGID_CLIENTRPC_RESPONSE:   // request_id,             cJSON
          _tbmch_clientrpc_on_response(client_, msg->body.clientrpc_response.request_id, 
                                       msg->body.clientrpc_response.object);
          cJSON_Delete(msg->body.clientrpc_response.object); //free cJSON
          break;
     case TBMCH_MSGID_CLIENTRPC_TIMEOUT:    // request_id
          _tbmch_clientrpc_on_timeout(client_, msg->body.clientrpc_response.request_id);
          break;

     case TBMCH_MSGID_FWUPDATE_RESPONSE:    // request_id,   chunk,    payload,  len
          _tbmch_fwupdate_on_response(client_, msg->body.fwupdate_response.request_id, 
               msg->body.fwupdate_response.chunk, 
               msg->body.fwupdate_response.payload,
               msg->body.fwupdate_response.length);
          TBMCH_FREE(msg->body.fwupdate_response.payload); //free payload
          break;
     case TBMCH_MSGID_FWUPDATE_TIMEOUT:     // request_id
          _tbmch_fwupdate_on_timeout(client_, msg->body.fwupdate_timeout.request_id);
          break;

     default:
          TBMCH_LOGE("msg->type(%d) is error!\r\n", msg->id);
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
          TBMCH_LOGE("client is NULL!");
          return; // false;
     }

     // TODO: whether to insert lock?
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBMCH_LOGE("Unable to take semaphore!");
     //      return false;
     // }

     // read event from queue()
     tbmch_msg_t msg;
     int i = 0;
     if (client->_xQueue != 0) {
          // Receive a message on the created queue.  Block for 0(10) ticks if a message is not immediately available.
          while (i < 10 && xQueueReceive(client->_xQueue, &(msg), (TickType_t)0)) { // 10
               // pcRxedMessage now points to the struct AMessage variable posted by vATask.
               _tbmch_deal_msg(client, &msg);
               i++;
          }
     }

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return;// sendResult == pdTRUE ? true : false;
}

//=====================================================================================================================
//~~static int _tbmch_sendServerRpcReply(tbmch_handle_t client_, int request_id, const char* response, int qos=1, int retain=0); //sendServerRpcReply()

//send msg to queue
//true if the item was successfully posted, otherwise false. //pdTRUE if the item was successfully posted, otherwise errQUEUE_FULL.
//It runs in MQTT thread.
static bool _tbmch_sendTbmqttMsg2Queue(tbmch_handle_t client_, tbmch_msg_t *msg)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client || !client->_xQueue || !msg) {
          TBMCH_LOGE("client, client->_xQueue or msg is NULL!");
          return false;
     }

     // TODO: whether/how to insert lock?
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBMCH_LOGE("Unable to take semaphore!");
     //      return false;
     // }

     // Send a pointer to a struct AMessage object.  Don't block if the queue is already full.
     int i = 0;
     BaseType_t sendResult;
     do {
          sendResult = xQueueSend(client->_xQueue, (void *)msg, (TickType_t)10);
          i++;

          if (sendResult != pdTRUE) {
               TBMCH_LOGW("_xQueue is full!");
          }
     } while (i < 20 && sendResult != pdTRUE);
     if (i >= 20) {
          TBMCH_LOGW("send innermsg timeout!");
     }

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return sendResult == pdTRUE ? true : false;
}

//static bool _tbmch_tbDecodeAttributesJsonPayload(JsonObject& attr_kvs); //_tbDecodeAttributesJsonPayload()

static void _tbmch_on_connected(void *context) //onConnected() // First receive
{
     tbmch_t *client = (tbmch_t *)context;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;
     }

     tbmch_msg_t msg;
     msg.id = TBMCH_MSGID_CONNECTED;
     _tbmch_sendTbmqttMsg2Queue(client, &msg);
}
static void _tbmch_on_disonnected(void *context) //onDisonnected() // First receive
{
     tbmch_t *client = (tbmch_t *)context;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;
     }

     tbmch_msg_t msg;
     msg.id = TBMCH_MSGID_DISCONNECTED;
     _tbmch_sendTbmqttMsg2Queue(client, &msg);
}
//onAttrOfSubReply(); // First receive
static void _tbmch_on_sharedattr_received(void *context, const char* payload, int length)
{
     tbmch_t *client = (tbmch_t *)context;
     if (!client || !payload) {
          TBMCH_LOGE("client or payload is NULL!");
          return;
     }
     if (length<=0) {
          TBMCH_LOGE("payload length(%d) is error!", length);
          return;
     }

     tbmch_msg_t msg;
     msg.id = TBMCH_MSGID_SHAREDATTR_RECEIVED;
     msg.body.sharedattr_received.object = cJSON_ParseWithLength(payload, length);  /*!< received palyload. free memory by msg receiver */
     _tbmch_sendTbmqttMsg2Queue(client, &msg);
}

//onAttributesResponse()=>_attributesResponse() // First send
//~~static bool _attributesResponse(int request_id, const char* payload, int length); //merge to _tbmch_on_attributesrequest_success()
static void _tbmch_on_attrrequest_response(void *context, int request_id, const char* payload, int length)
{
     tbmch_t *client = (tbmch_t *)context;
     if (!client || !payload) {
          TBMCH_LOGE("client or payload is NULL!");
          return;
     }
     if (length<=0) {
          TBMCH_LOGE("payload length(%d) is error!", length);
          return;
     }

     tbmch_msg_t msg;
     msg.id = TBMCH_MSGID_ATTRREQUEST_RESPONSE;
     msg.body.attrrequest_response.request_id = request_id;
     msg.body.attrrequest_response.object = cJSON_ParseWithLength(payload, length);  /*!< received palyload. free memory by msg receiver */
     _tbmch_sendTbmqttMsg2Queue(client, &msg);
} 
//onAttributesResponseTimeout() // First send
static void _tbmch_on_attrrequest_timeout(void *context, int request_id)
{
     tbmch_t *client = (tbmch_t *)context;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;
     }

     tbmch_msg_t msg;
     msg.id = TBMCH_MSGID_ATTRREQUEST_TIMEOUT;
     msg.body.attrrequest_timeout.request_id = request_id;
     _tbmch_sendTbmqttMsg2Queue(client, &msg);
}
          
////onServerRpcRequest() // First receive
static void _tbmch_on_serverrpc_request(void *context, int request_id, const char* payload, int length)
{
     tbmch_t *client = (tbmch_t *)context;
     if (!client || !payload) {
          TBMCH_LOGE("client or payload is NULL!");
          return;
     }
     if (length<=0) {
          TBMCH_LOGE("payload length(%d) is error!", length);
          return;
     }

     tbmch_msg_t msg;
     msg.id = TBMCH_MSGID_SERVERRPC_REQUSET;
     msg.body.serverrpc_request.request_id = request_id;
     msg.body.serverrpc_request.object = cJSON_ParseWithLength(payload, length); /*!< received palyload. free memory by msg receiver */;
     _tbmch_sendTbmqttMsg2Queue(client, &msg);
}

//onClientRpcResponse() // First send
static void _tbmch_on_clientrpc_response(void *context, int request_id, const char* payload, int length)
{
     tbmch_t *client = (tbmch_t *)context;
     if (!client || !payload) {
          TBMCH_LOGE("client or payload is NULL!");
          return;
     }
     if (length<=0) {
          TBMCH_LOGE("payload length(%d) is error!", length);
          return;
     }

     tbmch_msg_t msg;
     msg.id = TBMCH_MSGID_CLIENTRPC_RESPONSE;
     msg.body.clientrpc_response.request_id = request_id;
     msg.body.clientrpc_response.object = cJSON_ParseWithLength(payload, length); /*!< received palyload. free memory by msg receiver */;
     _tbmch_sendTbmqttMsg2Queue(client, &msg);
}
//onClientRpcResponseTimeout() // First send
static void _tbmch_on_clientrpc_timeout(void *context, int request_id)
{
     tbmch_t *client = (tbmch_t *)context;
     if (!client) {
          TBMCH_LOGE("client or payload is NULL!");
          return;
     }

     tbmch_msg_t msg;
     msg.id = TBMCH_MSGID_CLIENTRPC_TIMEOUT;
     msg.body.clientrpc_timeout.request_id = request_id;
     _tbmch_sendTbmqttMsg2Queue(client, &msg);
} 
// First send
static void _tbmch_on_fwupdate_response(void *context, int request_id, int chunk, const char* payload, int length)
{
     tbmch_t *client = (tbmch_t *)context;
     if (!client || !payload) {
          TBMCH_LOGE("client or payload is NULL!");
          return;
     }
     if (length<=0) {
          TBMCH_LOGE("payload length(%d) is error!", length);
          return;
     }

     tbmch_msg_t msg;
     msg.id = TBMCH_MSGID_FWUPDATE_RESPONSE;
     msg.body.fwupdate_response.request_id = request_id;
     msg.body.fwupdate_response.chunk = chunk;
     msg.body.fwupdate_response.payload = TBMCH_MALLOC(length); /*!< received palyload. free memory by msg receiver */
     if (msg.body.fwupdate_response.payload) {
          memcpy(msg.body.fwupdate_response.payload, payload, length);
     }
     msg.body.fwupdate_response.length = length;
     _tbmch_sendTbmqttMsg2Queue(client, &msg);
}
// First send
static void _tbmch_on_fwupdate_timeout(void *context, int request_id)
{
     tbmch_t *client = (tbmch_t *)context;
     if (!client) {
          TBMCH_LOGE("client or payload is NULL!");
          return;
     }

     tbmch_msg_t msg;
     msg.id = TBMCH_MSGID_FWUPDATE_TIMEOUT;
     msg.body.fwupdate_timeout.request_id = request_id;
     _tbmch_sendTbmqttMsg2Queue(client, &msg);
}

static void __response_timer_timerout(void *client_/*timer_arg*/)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;
     }

     tbmch_msg_t msg;
     msg.id = TBMCH_MSGID_TIMER_TIMEOUT;
     _tbmch_sendTbmqttMsg2Queue(client, &msg);
}

static void _response_timer_create(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;
     }

     esp_timer_create_args_t tmr_args = {
        .callback = &__response_timer_timerout,
        .arg = client_,
        .name = "response_timer",
     };
     esp_timer_create(&tmr_args, &client->respone_timer);
}
static void _response_timer_start(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;
     }

     esp_timer_start_periodic(client->respone_timer, (uint64_t)TB_MQTT_TIMEOUT * 1000 * 1000);
}
static void _response_timer_stop(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;
     }

     esp_timer_stop(client->respone_timer);
}
static void _response_timer_destroy(tbmch_handle_t client_)
{
     tbmch_t *client = (tbmch_t *)client_;
     if (!client) {
          TBMCH_LOGE("client is NULL!");
          return;
     }

     esp_timer_stop(client->respone_timer);
     esp_timer_delete(client->respone_timer);
     client->respone_timer = NULL;
}
