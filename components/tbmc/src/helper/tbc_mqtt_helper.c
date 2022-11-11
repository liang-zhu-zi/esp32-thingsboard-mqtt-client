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

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "sys/queue.h"
#include "esp_err.h"

/* using uri parser */
#include "http_parser.h"

#include "tbc_utils.h"
#include "tbc_transport_config.h"
#include "tbc_transport_storage.h"

#include "tbc_mqtt.h"
#include "tbc_mqtt_helper.h"

#include "tbc_utils.h"
#include "timeseries_data_helper.h"
#include "client_attribute_helper.h"
#include "shared_attribute_observer.h"
#include "attributes_request_observer.h"
#include "server_rpc_observer.h"
#include "client_rpc_observer.h"
#include "provision_observer.h"
#include "ota_update_observer.h"

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

/**
 * ThingsBoard MQTT Client Helper 
 */
typedef struct tbcmh_client
{
     // TODO: add a lock???
     // create & destroy
     tbcm_handle_t tbmqttclient;
     QueueHandle_t _xQueue;
     esp_timer_handle_t respone_timer;   // /*!< timer for checking response timeout */

     // modify at connect & disconnect
     tbc_transport_storage_t config;
     void *context;                          /*!< Context parameter of the below two callbacks */
     tbcmh_on_connected_t on_connected;      /*!< Callback of connected ThingsBoard MQTT */
     tbcmh_on_disconnected_t on_disconnected;/*!< Callback of disconnected ThingsBoard MQTT */

     // tx & rx msg
     SemaphoreHandle_t _lock;
     LIST_HEAD(tbcmh_tsdata_list, tbcmh_tsdata) tsdata_list;                              /*!< telemetry time-series data entries */
     LIST_HEAD(tbcmh_clientattribute_list, tbcmh_clientattribute) clientattribute_list;   /*!< client attributes entries */
     LIST_HEAD(tbcmh_sharedattribute_list, tbcmh_sharedattribute) sharedattribute_list;   /*!< shared attributes entries */
     LIST_HEAD(tbcmh_attributesrequest_list, tbcmh_attributesrequest) attributesrequest_list;  /*!< attributes request entries */
     LIST_HEAD(tbcmh_serverrpc_list, tbcmh_serverrpc) serverrpc_list;  /*!< server side RPC entries */
     LIST_HEAD(tbcmh_clientrpc_list, tbcmh_clientrpc) clientrpc_list;  /*!< client side RPC entries */
     LIST_HEAD(tbcmh_provision_list, tbcmh_provision) provision_list;  /*!< provision entries */
     LIST_HEAD(tbcmh_otaupdate_list, tbcmh_otaupdate) otaupdate_list;    /*!< A device may have multiple firmware */
} tbcmh_t;

static void _tbcmh_on_connected(void *context);
static void _tbcmh_on_disonnected(void *context);
static void _tbcmh_on_sharedattr_received(void *context, const char* payload, int length);
static void _tbcmh_on_attrrequest_response(void *context, int request_id, const char* payload, int length);
static void _tbcmh_on_attrrequest_timeout(void *context, int request_id);
static void _tbcmh_on_serverrpc_request(void *context, int request_id, const char* payload, int length);
static void _tbcmh_on_clientrpc_response(void *context, int request_id, const char* payload, int length);
static void _tbcmh_on_clientrpc_timeout(void *context, int request_id);
static void _tbcmh_on_provision_response(void *context, int request_id, const char* payload, int length);
static void _tbcmh_on_provision_timeout(void *context, int request_id);
static void _tbcmh_on_otaupdate_response(void *context, int request_id, int chunk_id, const char* payload, int length);
static void _tbcmh_on_otaupdate_timeout(void *context, int request_id);

static void _response_timer_create(tbcmh_handle_t client_);
static void _response_timer_start(tbcmh_handle_t client_);
static void _response_timer_stop(tbcmh_handle_t client_);
static void _response_timer_destroy(tbcmh_handle_t client_);

static tbcmh_err_t _tbcmh_clientattribute_xx_append(tbcmh_handle_t client_, const char *key, void *context,
                                                  tbcmh_clientattribute_on_get_t on_get,
                                                  tbcmh_clientattribute_on_set_t on_set);
static void _tbcmh_otaupdate_on_connected(tbcmh_handle_t client_);
static void _tbcmh_otaupdate_on_sharedattributes(tbcmh_handle_t client_, tbcmh_otaupdate_type_t ota_type,
                                                 const char *ota_title, const char *ota_version, int ota_size,
                                                 const char *ota_checksum, const char *ota_checksum_algorithm);

static tbcmh_err_t _tbcmh_telemetry_empty(tbcmh_handle_t client_);
static tbcmh_err_t _tbcmh_clientattribute_empty(tbcmh_handle_t client_);
static tbcmh_err_t _tbcmh_sharedattribute_empty(tbcmh_handle_t client_);
static tbcmh_err_t _tbcmh_attributesrequest_empty(tbcmh_handle_t client_);
static tbcmh_err_t _tbcmh_serverrpc_empty(tbcmh_handle_t client_);
static tbcmh_err_t _tbcmh_clientrpc_empty(tbcmh_handle_t client_);
static tbcmh_err_t _tbcmh_provision_empty(tbcmh_handle_t client_);
static tbcmh_err_t _tbcmh_otaupdate_empty(tbcmh_handle_t client_);

const static char *TAG = "tb_mqtt_client_helper";

//====0.tbmc client====================================================================================================
tbcmh_handle_t tbcmh_init(void)
{
     tbcmh_t *client = (tbcmh_t *)TBCMH_MALLOC(sizeof(tbcmh_t));
     if (!client) {
          TBC_LOGE("Unable to malloc memory! %s()", __FUNCTION__);
          return NULL;
     }
     memset(client, 0x00, sizeof(tbcmh_t));

     client->tbmqttclient = tbcm_init();
     // Create a queue capable of containing 20 tbcmh_msg_t structures. These should be passed by pointer as they contain a lot of data.
     client->_xQueue = xQueueCreate(40, sizeof(tbcmh_msg_t));
     if (client->_xQueue == NULL) {
          TBC_LOGE("failed to create the queue! %s()", __FUNCTION__);
     }
     _response_timer_create(client);

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

     _response_timer_stop(client);
     _response_timer_destroy(client);
     if (client->_xQueue) {
          vQueueDelete(client->_xQueue);
          client->_xQueue = NULL;
     }
     tbcm_destroy(client->tbmqttclient);

     TBCMH_FREE(client);
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


//~~tbcmh_config(); //move to tbcmh_connect()
//~~tbcmh_set_ConnectedEvent(evtConnected); //move to tbcmh_init()
//~~tbcmh_set_DisconnectedEvent(evtDisconnected); //move to tbcmh_init()
bool tbcmh_connect(tbcmh_handle_t client_, const tbc_transport_config_esay_t *config,
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
    result = tbcmh_connect_ex(client_, &transport, context, on_connected, on_disconnected);

fail_exit:
    TBC_FIELD_FREE(address.schema);
    TBC_FIELD_FREE(address.host);
    TBC_FIELD_FREE(address.path);
    TBC_FIELD_FREE(credentials.username);
    TBC_FIELD_FREE(credentials.password);
    return result;
}

bool tbcmh_connect_ex(tbcmh_handle_t client_, const tbc_transport_config_t* config,
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
    
    // connect
    TBC_LOGI("connecting to %s://%s:%d ...",
                config->address.schema, config->address.host, config->address.port);
    bool result = tbcm_connect(client->tbmqttclient, config,
                               client,
                               _tbcmh_on_connected,
                               _tbcmh_on_disonnected,
                               _tbcmh_on_sharedattr_received,
                               _tbcmh_on_serverrpc_request);
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
     _response_timer_start(client);

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

tbcmh_err_t tbcmh_subscribe(tbcmh_handle_t client_, const char *topic)
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


//====10.Publish Telemetry time-series data==============================================================================
tbcmh_err_t tbcmh_telemetry_append(tbcmh_handle_t client_, const char *key, void *context, tbcmh_tsdata_on_get_t on_get)
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

     // Create tsdata
     tbcmh_tsdata_t *tsdata = _tbcmh_tsdata_init(client_, key/*, type*/, context, on_get/*, on_set*/);
     if (!tsdata) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init tsdata failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert tsdata to list
     tbcmh_tsdata_t *it, *last = NULL;
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
tbcmh_err_t tbcmh_telemetry_clear(tbcmh_handle_t client_, const char *key)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !key) {
          TBC_LOGE("client or key is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Search item
     tbcmh_tsdata_t *tsdata = NULL;
     LIST_FOREACH(tsdata, &client->tsdata_list, entry) {
          if (tsdata && strcmp(_tbcmh_tsdata_get_key(tsdata), key)==0) {
               break;
          }
     }
     if (!tsdata) {
          TBC_LOGW("Unable to remove time-series data:%s! %s()", key, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(tsdata, entry);
     _tbcmh_tsdata_destroy(tsdata);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

static tbcmh_err_t _tbcmh_telemetry_empty(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in tsdata_list
     tbcmh_tsdata_t *tsdata = NULL, *next;
     LIST_FOREACH_SAFE(tsdata, &client->tsdata_list, entry, next) {
          // remove from tsdata list and destory
          LIST_REMOVE(tsdata, entry);
          _tbcmh_tsdata_destroy(tsdata);
     }
     memset(&client->tsdata_list, 0x00, sizeof(client->tsdata_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}
////tbmqttlink.h.tbcmh_sendTelemetry();
tbcmh_err_t tbcmh_telemetry_send(tbcmh_handle_t client_, int count, /*const char *key,*/ ...)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (count <= 0) {
          TBC_LOGE("count(%d) is error! %s()", count, __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     int i;
     va_list ap;
     va_start(ap, count);
     cJSON *object = cJSON_CreateObject(); // create json object
     for (i=0; i<count; i++) {
          const char *key = va_arg(ap, const char*);

          // Search item
          tbcmh_tsdata_t *tsdata = NULL;
          LIST_FOREACH(tsdata, &client->tsdata_list, entry) {
               if (tsdata && strcmp(_tbcmh_tsdata_get_key(tsdata), key)==0) {
                    break;
               }
          }

          /// Add tsdata to package
          if (tsdata) {
               _tbcmh_tsdata_go_get(tsdata, object); // add item to json object
          } else {
               TBC_LOGW("Unable to find&send time-series data:%s! %s()", key, __FUNCTION__);
          }
     }
     va_end(ap);

     // send package...
     char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
     int result = tbcm_telemetry_publish(client->tbmqttclient, pack, 1/*qos*/, 0/*retain*/);
     cJSON_free(pack); // free memory
     cJSON_Delete(object); // delete json object

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return (result > -1) ? ESP_OK : ESP_FAIL;
}

//====20.Publish client-side device attributes to the server============================================================
tbcmh_err_t tbcmh_clientattribute_append(tbcmh_handle_t client_, const char *key, void *context,
                                         tbcmh_clientattribute_on_get_t on_get)
{
     return _tbcmh_clientattribute_xx_append(client_, key, context, on_get, NULL);
}
tbcmh_err_t tbcmh_clientattribute_with_set_append(tbcmh_handle_t client_, const char *key, void *context,
                                                  tbcmh_clientattribute_on_get_t on_get,
                                                  tbcmh_clientattribute_on_set_t on_set)
{
     if (!on_set)  {
          TBC_LOGE("on_set is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     return _tbcmh_clientattribute_xx_append(client_, key, context, on_get, on_set);
}
// tbcmh_attribute_of_clientside_init()
static tbcmh_err_t _tbcmh_clientattribute_xx_append(tbcmh_handle_t client_, const char *key, void *context,
                                                  tbcmh_clientattribute_on_get_t on_get,
                                                  tbcmh_clientattribute_on_set_t on_set)
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

     // Create clientattribute
     tbcmh_clientattribute_t *clientattribute = _tbcmh_clientattribute_init(client_, key, context, on_get, on_set);
     if (!clientattribute) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init clientattribute failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert clientattribute to list
     tbcmh_clientattribute_t *it, *last = NULL;
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
tbcmh_err_t tbcmh_clientattribute_clear(tbcmh_handle_t client_, const char *key)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !key) {
          TBC_LOGE("client or key is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Search item
     tbcmh_clientattribute_t *clientattribute = NULL;
     LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
          if (clientattribute && strcmp(_tbcmh_clientattribute_get_key(clientattribute), key)==0) {
               break;
          }
     }
     if (!clientattribute) {
          TBC_LOGW("Unable to remove client attribute: %s! %s()", key, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(clientattribute, entry);
     _tbcmh_clientattribute_destroy(clientattribute);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}
static tbcmh_err_t _tbcmh_clientattribute_empty(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in clientattribute_list
     tbcmh_clientattribute_t *clientattribute = NULL, *next;
     LIST_FOREACH_SAFE(clientattribute, &client->clientattribute_list, entry, next) {
          // remove from clientattribute list and destory
          LIST_REMOVE(clientattribute, entry);
          _tbcmh_clientattribute_destroy(clientattribute);
     }
     memset(&client->clientattribute_list, 0x00, sizeof(client->clientattribute_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}
tbcmh_err_t tbcmh_clientattribute_send(tbcmh_handle_t client_, int count, /*const char *key,*/ ...) ////tbmqttlink.h.tbcmh_sendClientAttributes();
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (count <= 0) {
          TBC_LOGE("count(%d) is error! %s()", count, __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     int i;
     va_list ap;
     va_start(ap, count);
     cJSON *object = cJSON_CreateObject(); // create json object
     for (i=0; i<count; i++) {
          const char *key = va_arg(ap, const char*);

          // Search item
          tbcmh_clientattribute_t *clientattribute = NULL;
          LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
               if (clientattribute && strcmp(_tbcmh_clientattribute_get_key(clientattribute), key)==0) {
                    break;
               }
          }

          /// Add clientattribute to package
          if (clientattribute) {
               _tbcmh_clientattribute_do_get(clientattribute, object); // add item to json object
          } else {
               TBC_LOGW("Unable to find client-side attribute:%s! %s()", key, __FUNCTION__);
          }
     }
     va_end(ap);

     // send package...
     char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
     int result = tbcm_clientattributes_publish(client->tbmqttclient, pack, 1/*qos*/, 0/*retain*/);
     cJSON_free(pack); // free memory
     cJSON_Delete(object); // delete json object

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return (result > -1) ? ESP_OK : ESP_FAIL;
}

//unpack & deal
static void _tbcmh_clientattribute_on_received(tbcmh_handle_t client_, const cJSON *object)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !object) {
          TBC_LOGE("client or object is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // foreach item to set value of clientattribute in lock/unlodk.  Don't call tbmch's funciton in set value callback!
     tbcmh_clientattribute_t *clientattribute = NULL;
     const char* key = NULL;
     LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
          if (clientattribute) {
               key = _tbcmh_clientattribute_get_key(clientattribute);
               if (cJSON_HasObjectItem(object, key)) {
                    _tbcmh_clientattribute_do_set(clientattribute, cJSON_GetObjectItem(object, key));
               }
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return;// ESP_OK;
}

//====21.Subscribe to shared device attribute updates from the server===================================================
////tbmqttlink.h.tbcmh_addSubAttrEvent(); //Call it before connect() //tbcmh_shared_attribute_list_t
tbcmh_err_t tbcmh_sharedattribute_append(tbcmh_handle_t client_, const char *key, void *context,
                                         tbcmh_sharedattribute_on_set_t on_set)
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

     // Create sharedattribute
     tbcmh_sharedattribute_t *sharedattribute = _tbcmh_sharedattribute_init(client_, key, context, on_set);
     if (!sharedattribute) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init sharedattribute failure! key=%s. %s()", key, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert sharedattribute to list
     tbcmh_sharedattribute_t *it, *last = NULL;
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

// remove shared_attribute from tbcmh_shared_attribute_list_t
tbcmh_err_t tbcmh_sharedattribute_clear(tbcmh_handle_t client_, const char *key)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !key) {
          TBC_LOGE("client or key is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Search item
     tbcmh_sharedattribute_t *sharedattribute = NULL;
     LIST_FOREACH(sharedattribute, &client->sharedattribute_list, entry) {
          if (sharedattribute && strcmp(_tbcmh_sharedattribute_get_key(sharedattribute), key)==0) {
               break;
          }
     }
     if (!sharedattribute) {
          TBC_LOGW("Unable to remove shared-attribute:%s! %s()", key, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(sharedattribute, entry);
     _tbcmh_sharedattribute_destroy(sharedattribute);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}
static tbcmh_err_t _tbcmh_sharedattribute_empty(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in sharedattribute_list
     tbcmh_sharedattribute_t *sharedattribute = NULL, *next;
     LIST_FOREACH_SAFE(sharedattribute, &client->sharedattribute_list, entry, next) {
          // remove from sharedattribute list and destory
          LIST_REMOVE(sharedattribute, entry);
          _tbcmh_sharedattribute_destroy(sharedattribute);
     }
     memset(&client->sharedattribute_list, 0x00, sizeof(client->sharedattribute_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

//unpack & deal
//onAttrOfSubReply()
static void _tbcmh_sharedattribute_on_received(tbcmh_handle_t client_, const cJSON *object)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !object) {
          TBC_LOGE("client or object is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // foreach itme to set value of sharedattribute in lock/unlodk.  Don't call tbmch's funciton in set value callback!
     tbcmh_sharedattribute_t *sharedattribute = NULL;
     const char* key = NULL;
     LIST_FOREACH(sharedattribute, &client->sharedattribute_list, entry) {
          if (sharedattribute) {
               key = _tbcmh_sharedattribute_get_key(sharedattribute);
               if (cJSON_HasObjectItem(object, key)) {
                    _tbcmh_sharedattribute_do_set(sharedattribute, cJSON_GetObjectItem(object, key));
               }
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);

     // special process for otaupdate
     if (cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_TITLE) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_VERSION) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_SIZE) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_CHECKSUM) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_FW_CHECKSUM_ALG))
     {
          char *ota_title = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_TITLE));
          char *ota_version = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_VERSION));
          int ota_size = cJSON_GetNumberValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_SIZE));
          char *ota_checksum = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_CHECKSUM));
          char *ota_checksum_algorithm = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_FW_CHECKSUM_ALG));
          _tbcmh_otaupdate_on_sharedattributes(client_, TBCMH_OTAUPDATE_TYPE_FW, ota_title, ota_version, ota_size, ota_checksum, ota_checksum_algorithm);
     } else if (cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_TITLE) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_VERSION) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_SIZE) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_CHECKSUM) &&
         cJSON_HasObjectItem(object, TB_MQTT_KEY_SW_CHECKSUM_ALG))
     {
          char *sw_title = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_TITLE));
          char *sw_version = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_VERSION));
          int sw_size = cJSON_GetNumberValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_SIZE));
          char *sw_checksum = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_CHECKSUM));
          char *sw_checksum_algorithm = cJSON_GetStringValue(cJSON_GetObjectItem(object, TB_MQTT_KEY_SW_CHECKSUM_ALG));
          _tbcmh_otaupdate_on_sharedattributes(client_, TBCMH_OTAUPDATE_TYPE_SW, sw_title, sw_version, sw_size, sw_checksum, sw_checksum_algorithm);
     }

     return;// ESP_OK;
}

//====22.Request client-side or shared device attributes from the server================================================
static tbcmh_err_t _tbcmh_attributesrequest_empty(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in attributesrequest_list
     tbcmh_attributesrequest_t *attributesrequest = NULL, *next;
     LIST_FOREACH_SAFE(attributesrequest, &client->attributesrequest_list, entry, next) {
          // exec timeout callback
          _tbcmh_attributesrequest_do_timeout(attributesrequest);

          // remove from attributesrequest list and destory
          LIST_REMOVE(attributesrequest, entry);
          _tbcmh_attributesrequest_destroy(attributesrequest);
     }
     memset(&client->attributesrequest_list, 0x00, sizeof(client->attributesrequest_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

#define MAX_KEYS_LEN (256)
////tbmqttlink.h.tbcmh_sendAttributesRequest();
////return request_id on successful, otherwise return -1/ESP_FAIL
int tbcmh_attributesrequest_send(tbcmh_handle_t client_,
                                 void *context,
                                 tbcmh_attributesrequest_on_response_t on_response,
                                 tbcmh_attributesrequest_on_timeout_t on_timeout,
                                 int count, /*const char *key,*/...)
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (count <= 0) {
          TBC_LOGE("count(%d) is error! %s()", count, __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore, malloc client_keys & shared_keys
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     char *client_keys = TBCMH_MALLOC(MAX_KEYS_LEN);
     char *shared_keys = TBCMH_MALLOC(MAX_KEYS_LEN);
     if (!client_keys || !shared_keys) {
          goto attributesrequest_fail;
     }
     memset(client_keys, 0x00, MAX_KEYS_LEN);
     memset(shared_keys, 0x00, MAX_KEYS_LEN);

     // Get client_keys & shared_keys
     int i = 0;
     va_list ap;
     va_start(ap, count);
next_attribute_key:
     while (i<count) {
          i++;
          const char *key = va_arg(ap, const char*);

          // Search item in clientattribute
          tbcmh_clientattribute_t *clientattribute = NULL;
          LIST_FOREACH(clientattribute, &client->clientattribute_list, entry) {
               if (clientattribute && strcmp(_tbcmh_clientattribute_get_key(clientattribute), key)==0) {
                    // copy key to client_keys
                    if (strlen(client_keys)==0) {
                         strncpy(client_keys, key, MAX_KEYS_LEN-1);
                    } else {
                         strncat(client_keys, ",", MAX_KEYS_LEN-1);                         
                         strncat(client_keys, key, MAX_KEYS_LEN-1);
                    }
                    goto next_attribute_key;
               }
          }

          // Search item in sharedattribute
          tbcmh_sharedattribute_t *sharedattribute = NULL;
          LIST_FOREACH(sharedattribute, &client->sharedattribute_list, entry) {
               if (sharedattribute && strcmp(_tbcmh_sharedattribute_get_key(sharedattribute), key)==0) {
                    // copy key to shared_keys
                    if (strlen(shared_keys)==0) {
                         strncpy(shared_keys, key, MAX_KEYS_LEN-1);
                    } else {
                         strncat(shared_keys, ",", MAX_KEYS_LEN-1);                         
                         strncat(shared_keys, key, MAX_KEYS_LEN-1);
                    }
                    goto next_attribute_key;
               }
          }

          TBC_LOGW("Unable to find attribute in request:%s! %s()", key, __FUNCTION__);
     }
     va_end(ap);

     // Send msg to server
     int request_id = tbcm_attributes_request_ex(client->tbmqttclient, client_keys, shared_keys,
                               client,
                               _tbcmh_on_attrrequest_response,
                               _tbcmh_on_attrrequest_timeout,
                               1/*qos*/, 0/*retain*/);
     if (request_id<0) {
          TBC_LOGE("Init tbcm_attributes_request failure! %s()", __FUNCTION__);
          goto attributesrequest_fail;
     }

     // Create attributesrequest
     tbcmh_attributesrequest_t *attributesrequest = _tbcmh_attributesrequest_init(client_, request_id, context, on_response, on_timeout);
     if (!attributesrequest) {
          TBC_LOGE("Init attributesrequest failure! %s()", __FUNCTION__);
          goto attributesrequest_fail;
     }

     // Insert attributesrequest to list
     tbcmh_attributesrequest_t *it, *last = NULL;
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
     TBCMH_FREE(client_keys);
     TBCMH_FREE(shared_keys);
     return request_id;

attributesrequest_fail:
     xSemaphoreGive(client->_lock);
     if (!client_keys) {
          TBCMH_FREE(client_keys);
     }
     if (!shared_keys) {
          TBCMH_FREE(shared_keys);
     }
     return ESP_FAIL;
}

////return request_id on successful, otherwise return -1/ESP_FAIL
static int _tbcmh_attributesrequest_send_4_ota_sharedattributes(tbcmh_handle_t client_,
                                  void *context,
                                  tbcmh_attributesrequest_on_response_t on_response,
                                  tbcmh_attributesrequest_on_timeout_t on_timeout,
                                  int count, /*const char *key,*/...)
{
      // this funciton is in client->_lock !

      tbcmh_t *client = (tbcmh_t*)client_;
      if (!client) {
           TBC_LOGE("client is NULL! %s()", __FUNCTION__);
           return ESP_FAIL;
      }
      if (count <= 0) {
           TBC_LOGE("count(%d) is error! %s()", count, __FUNCTION__);
           return ESP_FAIL;
      }
 
      // Take semaphore, malloc client_keys & shared_keys
      //if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
      //     TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
      //     return ESP_FAIL;
      //}
      char *shared_keys = TBCMH_MALLOC(MAX_KEYS_LEN);
      if (!shared_keys) {
           goto attributesrequest_fail;
      }
      memset(shared_keys, 0x00, MAX_KEYS_LEN);
 
      // Get shared_keys
      int i = 0;
      va_list ap;
      va_start(ap, count);
      while (i<count) {
        i++;
        const char *key = va_arg(ap, const char*);
        if (strlen(key)>0) {
            // copy key to shared_keys
            if (strlen(shared_keys)==0) {
                strncpy(shared_keys, key, MAX_KEYS_LEN-1);
            } else {
                strncat(shared_keys, ",", MAX_KEYS_LEN-1);                         
                strncat(shared_keys, key, MAX_KEYS_LEN-1);
            }
        }
      }
      va_end(ap);
 
      // Send msg to server
      int request_id = tbcm_attributes_request_ex(client->tbmqttclient, NULL, shared_keys,
                                client,
                                _tbcmh_on_attrrequest_response,
                                _tbcmh_on_attrrequest_timeout,
                                1/*qos*/, 0/*retain*/);
      if (request_id<0) {
           TBC_LOGE("Init tbcm_attributes_request failure! %s()", __FUNCTION__);
           goto attributesrequest_fail;
      }
 
      // Create attributesrequest
      tbcmh_attributesrequest_t *attributesrequest = _tbcmh_attributesrequest_init(client_, request_id, context, on_response, on_timeout);
      if (!attributesrequest) {
           TBC_LOGE("Init attributesrequest failure! %s()", __FUNCTION__);
           goto attributesrequest_fail;
      }
 
      // Insert attributesrequest to list
      tbcmh_attributesrequest_t *it, *last = NULL;
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
      //xSemaphoreGive(client->_lock);
      TBCMH_FREE(shared_keys);
      return request_id;
 
 attributesrequest_fail:
      //xSemaphoreGive(client->_lock);
      if (!shared_keys) {
           TBCMH_FREE(shared_keys);
      }
      return ESP_FAIL;
 }


// onAttributesResponse()=>_attributesResponse()
static void _tbcmh_attributesrequest_on_response(tbcmh_handle_t client_, int request_id, const cJSON *object)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !object) {
          TBC_LOGE("client or object is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search attributesrequest
     tbcmh_attributesrequest_t *attributesrequest = NULL; 
     LIST_FOREACH(attributesrequest, &client->attributesrequest_list, entry) {
          if (attributesrequest && (_tbcmh_attributesrequest_get_request_id(attributesrequest)==request_id)) {
               break;
          }
     }
     if (!attributesrequest) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGW("Unable to find attribute request:%d! %s()", request_id, __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Cache and remove attributesrequest
     tbcmh_attributesrequest_t *cache = _tbcmh_attributesrequest_clone_wo_listentry(attributesrequest);
     LIST_REMOVE(attributesrequest, entry);
     _tbcmh_attributesrequest_destroy(attributesrequest);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // foreach item to set value of clientattribute in lock/unlodk.  Don't call tbmch's funciton in set value callback!
     if (cJSON_HasObjectItem(object, TB_MQTT_KEY_ATTRIBUTES_RESPONSE_CLIENT)) {
          _tbcmh_clientattribute_on_received(client_, cJSON_GetObjectItem(object, TB_MQTT_KEY_ATTRIBUTES_RESPONSE_CLIENT));
     }
     // foreach item to set value of sharedattribute in lock/unlodk.  Don't call tbmch's funciton in set value callback!
     if (cJSON_HasObjectItem(object, TB_MQTT_KEY_ATTRIBUTES_RESPONSE_SHARED)) {
          _tbcmh_sharedattribute_on_received(client_, cJSON_GetObjectItem(object, TB_MQTT_KEY_ATTRIBUTES_RESPONSE_SHARED));
     }

     // Do response
     _tbcmh_attributesrequest_do_response(cache);
     // Free cache
     _tbcmh_attributesrequest_destroy(cache);

     return;// ESP_OK;
}
// onAttributesResponseTimeout()
static void _tbcmh_attributesrequest_on_timeout(tbcmh_handle_t client_, int request_id)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search attributesrequest
     tbcmh_attributesrequest_t *attributesrequest = NULL;
     LIST_FOREACH(attributesrequest, &client->attributesrequest_list, entry) {
          if (attributesrequest && (_tbcmh_attributesrequest_get_request_id(attributesrequest)==request_id)) {
               break;
          }
     }
     if (!attributesrequest) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGW("Unable to find attribute request:%d! %s()", request_id, __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Cache and remove attributesrequest
     tbcmh_attributesrequest_t *cache = _tbcmh_attributesrequest_clone_wo_listentry(attributesrequest);
     LIST_REMOVE(attributesrequest, entry);
     _tbcmh_attributesrequest_destroy(attributesrequest);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do timeout
     _tbcmh_attributesrequest_do_timeout(cache);
     // Free attributesrequest
     _tbcmh_attributesrequest_destroy(cache);

     return;// ESP_OK;
}

//====30.Server-side RPC================================================================================================
////tbmqttlink.h.tbcmh_addServerRpcEvent(evtServerRpc); //Call it before connect()
tbcmh_err_t tbcmh_serverrpc_append(tbcmh_handle_t client_, const char *method,
                                   void *context,
                                   tbcmh_serverrpc_on_request_t on_request)
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

     // Create serverrpc
     tbcmh_serverrpc_t *serverrpc = _tbcmh_serverrpc_init(client, method, context, on_request);
     if (!serverrpc) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init serverrpc failure! method=%s. %s()", method, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert serverrpc to list
     tbcmh_serverrpc_t *it, *last = NULL;
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

// remove from LIST_ENTRY(tbcmh_serverrpc_) & delete
tbcmh_err_t tbcmh_serverrpc_clear(tbcmh_handle_t client_, const char *method)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !method) {
          TBC_LOGE("client or method is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Search item
     tbcmh_serverrpc_t *serverrpc = NULL;
     LIST_FOREACH(serverrpc, &client->serverrpc_list, entry) {
          if (serverrpc && strcmp(_tbcmh_serverrpc_get_method(serverrpc), method)==0) {
               break;
          }
     }
     if (!serverrpc)  {
          TBC_LOGW("Unable to remove server-rpc:%s! %s()", method, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(serverrpc, entry);
     _tbcmh_serverrpc_destroy(serverrpc);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}
static tbcmh_err_t _tbcmh_serverrpc_empty(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in serverrpc_list
     tbcmh_serverrpc_t *serverrpc = NULL, *next;
     LIST_FOREACH_SAFE(serverrpc, &client->serverrpc_list, entry, next) {
          // remove from serverrpc list and destory
          LIST_REMOVE(serverrpc, entry);
          _tbcmh_serverrpc_destroy(serverrpc);
     }
     memset(&client->serverrpc_list, 0x00, sizeof(client->serverrpc_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}


////parseServerSideRpc(msg)
static void _tbcmh_serverrpc_on_request(tbcmh_handle_t client_, int request_id, const cJSON *object)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !object) {
          TBC_LOGE("client or object is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     const char *method = NULL;
     if (cJSON_HasObjectItem(object, TB_MQTT_KEY_RPC_METHOD)) {
          cJSON *methodItem = cJSON_GetObjectItem(object, TB_MQTT_KEY_RPC_METHOD);
          method = cJSON_GetStringValue(methodItem);
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search item
     tbcmh_serverrpc_t *serverrpc = NULL;
     LIST_FOREACH(serverrpc, &client->serverrpc_list, entry) {
          if (serverrpc && strcmp(_tbcmh_serverrpc_get_method(serverrpc), method)==0) {
               break;
          }
     }
     if (!serverrpc) {
          TBC_LOGW("Unable to deal server-rpc:%s! %s()", method, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return;// ESP_OK;
     }

     // Clone serverrpc
     tbcmh_serverrpc_t *cache = _tbcmh_serverrpc_clone_wo_listentry(serverrpc);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do request
     tbcmh_rpc_results_t *result = _tbcmh_serverrpc_do_request(cache, request_id,
                                                               cJSON_GetObjectItem(object, TB_MQTT_KEY_RPC_PARAMS));
     // Send reply
     if (result) {
          #if 0
          cJSON* reply = cJSON_CreateObject();
          cJSON_AddStringToObject(reply, TB_MQTT_KEY_RPC_METHOD, method);
          cJSON_AddItemToObject(reply, TB_MQTT_KEY_RPC_RESULTS, result);
          const char *response = cJSON_PrintUnformatted(reply); //cJSON_Print()
          tbcm_serverrpc_response(client_->tbmqttclient, request_id, response, 1/*qos*/, 0/*retain*/);
          cJSON_free(response); // free memory
          cJSON_Delete(reply); // delete json object
          #else
          char *response = cJSON_PrintUnformatted(result); //cJSON_Print(result);
          tbcm_serverrpc_response(client_->tbmqttclient, request_id, response, 1/*qos*/, 0/*retain*/);
          cJSON_free(response); // free memory
          cJSON_Delete(result); // delete json object
          #endif
     }
     // Free serverrpc
     _tbcmh_serverrpc_destroy(cache);

     return;// ESP_OK;
}

//====31.Client-side RPC================================================================================================
static tbcmh_err_t _tbcmh_clientrpc_empty(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in clientrpc_list
     tbcmh_clientrpc_t *clientrpc = NULL, *next;
     LIST_FOREACH_SAFE(clientrpc, &client->clientrpc_list, entry, next) {
          // exec timeout callback
          _tbcmh_clientrpc_do_timeout(clientrpc);

          // remove from clientrpc list and destory
          LIST_REMOVE(clientrpc, entry);
          _tbcmh_clientrpc_destroy(clientrpc);
     }
     memset(&client->clientrpc_list, 0x00, sizeof(client->clientrpc_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

////tbmqttlink.h.tbcmh_sendClientRpcRequest(); //add list
int tbcmh_clientrpc_of_oneway_request(tbcmh_handle_t client_, const char *method,
                                                           /*const*/ tbcmh_rpc_params_t *params)
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (!method) {
          TBC_LOGE("method is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Send msg to server
     //cJSON *object = cJSON_CreateObject(); // create json object
     //cJSON_AddStringToObject(object, TB_MQTT_KEY_RPC_METHOD, method);
     //if (params) {
     //     cJSON_AddItemReferenceToObject(object, TB_MQTT_KEY_RPC_PARAMS, params);
     //} else  {
     //     cJSON_AddNullToObject(object, TB_MQTT_KEY_RPC_PARAMS);
     //}
     //char *params_str = cJSON_PrintUnformatted(object); //cJSON_Print(object);
     int request_id;
     if (params) {
         char *params_str = cJSON_PrintUnformatted(params); //cJSON_Print(object);
         request_id = tbcm_clientrpc_request_ex(client->tbmqttclient, method, params_str,
                          client,
                          NULL, //_tbcmh_on_clientrpc_response,
                          NULL, //_tbcmh_on_clientrpc_timeout,
                          1/*qos*/, 0/*retain*/);
         cJSON_free(params_str); // free memory
     } else {
         request_id = tbcm_clientrpc_request_ex(client->tbmqttclient, method, "{}",
                          client,
                          NULL, //_tbcmh_on_clientrpc_response,
                          NULL, //_tbcmh_on_clientrpc_timeout,
                          1/*qos*/, 0/*retain*/);     
     }
     //cJSON_Delete(object); // delete json object
     if (request_id<0) {
          TBC_LOGE("Init tbcm_clientrpc_request failure! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     return request_id;
}
////tbmqttlink.h.tbcmh_sendClientRpcRequest(); //create to add to LIST_ENTRY(tbcmh_clientrpc_)
int tbcmh_clientrpc_of_twoway_request(tbcmh_handle_t client_, const char *method, 
                                                           /*const*/ tbcmh_rpc_params_t *params,
                                                           void *context,
                                                           tbcmh_clientrpc_on_response_t on_response,
                                                           tbcmh_clientrpc_on_timeout_t on_timeout)
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (!method) {
          TBC_LOGE("method is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Send msg to server
     //cJSON *object = cJSON_CreateObject(); // create json object
     //cJSON_AddStringToObject(object, TB_MQTT_KEY_RPC_METHOD, method);
     //if (params)
     //     cJSON_AddItemReferenceToObject(object, TB_MQTT_KEY_RPC_PARAMS, params);
     //else 
     //     cJSON_AddNullToObject(object, TB_MQTT_KEY_RPC_PARAMS);
     //char *params_str = cJSON_PrintUnformatted(object); //cJSON_Print(object);
     int request_id;
     if (params) {
         char *params_str = cJSON_PrintUnformatted(params); //cJSON_Print(object);
         request_id = tbcm_clientrpc_request_ex(client->tbmqttclient, method, params_str,
                                  client,
                                  _tbcmh_on_clientrpc_response,
                                  _tbcmh_on_clientrpc_timeout,
                                   1/*qos*/, 0/*retain*/);
         cJSON_free(params_str); // free memory
     } else {
         request_id = tbcm_clientrpc_request_ex(client->tbmqttclient, method, "{}",
                                  client,
                                  _tbcmh_on_clientrpc_response,
                                  _tbcmh_on_clientrpc_timeout,
                                   1/*qos*/, 0/*retain*/);
     }
     //cJSON_Delete(object); // delete json object
     if (request_id<0) {
          TBC_LOGE("Init tbcm_clientrpc_request failure! %s()", __FUNCTION__);
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Create clientrpc
     tbcmh_clientrpc_t *clientrpc = _tbcmh_clientrpc_init(client, request_id, method, context, on_response, on_timeout);
     if (!clientrpc) {
          TBC_LOGE("Init clientrpc failure! %s()", __FUNCTION__);
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Insert clientrpc to list
     tbcmh_clientrpc_t *it, *last = NULL;
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
static void _tbcmh_clientrpc_on_response(tbcmh_handle_t client_, int request_id, const cJSON *object)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !object) {
          TBC_LOGE("client or object is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search clientrpc
     tbcmh_clientrpc_t *clientrpc = NULL;
     LIST_FOREACH(clientrpc, &client->clientrpc_list, entry) {
          if (clientrpc && (_tbcmh_clientrpc_get_request_id(clientrpc)==request_id)) {
               break;
          }
     }
     if (!clientrpc) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGW("Unable to find client-rpc:%d! %s()", request_id, __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Cache and remove clientrpc
     tbcmh_clientrpc_t *cache = _tbcmh_clientrpc_clone_wo_listentry(clientrpc);
     LIST_REMOVE(clientrpc, entry);
     _tbcmh_clientrpc_destroy(clientrpc);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do response
     _tbcmh_clientrpc_do_response(cache, cJSON_GetObjectItem(object, TB_MQTT_KEY_RPC_RESULTS));
     // Free cache
     _tbcmh_clientrpc_destroy(cache);

     return;// ESP_OK;
}
//onClientRpcResponseTimeout()
static void _tbcmh_clientrpc_on_timeout(tbcmh_handle_t client_, int request_id)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search clientrpc
     tbcmh_clientrpc_t *clientrpc = NULL;
     LIST_FOREACH(clientrpc, &client->clientrpc_list, entry) {
          if (clientrpc && (_tbcmh_clientrpc_get_request_id(clientrpc)==request_id)) {
               break;
          }
     }
     if (!clientrpc) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGW("Unable to find client-rpc:%d! %s()", request_id, __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Cache and remove clientrpc
     tbcmh_clientrpc_t *cache = _tbcmh_clientrpc_clone_wo_listentry(clientrpc);
     LIST_REMOVE(clientrpc, entry);
     _tbcmh_clientrpc_destroy(clientrpc);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do timeout
     _tbcmh_clientrpc_do_timeout(cache);
     // Free clientrpc
     _tbcmh_clientrpc_destroy(cache);

     return;// ESP_OK;
}

//====40.Claiming device using device-side key scenario============================================
tbcmh_err_t tbcmh_claiming_device_using_device_side_key(tbcmh_handle_t client_,
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

//====50.Device provisioning: Not implemented yet=======================================================================
static tbcmh_err_t _tbcmh_provision_empty(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in provision_list
     tbcmh_provision_t *provision = NULL, *next;
     LIST_FOREACH_SAFE(provision, &client->provision_list, entry, next) {
          // exec timeout callback
          _tbcmh_provision_do_timeout(provision);

          // remove from provision list and destory
          LIST_REMOVE(provision, entry);
          _tbcmh_provision_destroy(provision);
     }
     memset(&client->provision_list, 0x00, sizeof(client->provision_list));

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return ESP_OK;
}

// return ESP_OK on successful, ESP_FAIL on failure
static int _params_of_credentials_generated_by_server(tbcmh_provision_params_t *params, const tbc_provison_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    // TBC_CHECK_PTR_WITH_RETURN_VALUE(config->deviceName, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceKey, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceSecret, ESP_FAIL);

    if (config->deviceName) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_DEVICE_NAME, config->deviceName);
    }
    if (config->provisionDeviceKey) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY, config->provisionDeviceKey);
    }
    if (config->provisionDeviceSecret) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET, config->provisionDeviceSecret);
    }
    return ESP_OK;
}

// return ESP_OK on successful, ESP_FAIL on failure
static int _params_of_devices_supplies_access_token(tbcmh_provision_params_t *params, const tbc_provison_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    // TBC_CHECK_PTR_WITH_RETURN_VALUE(config->deviceName, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceKey, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceSecret, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->token, ESP_FAIL);

    if (config->deviceName) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_DEVICE_NAME, config->deviceName);
    }
    if (config->provisionDeviceKey) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY, config->provisionDeviceKey);
    }
    if (config->provisionDeviceSecret) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET, config->provisionDeviceSecret);
    }
    cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE, TB_MQTT_VALUE_PROVISION_ACCESS_TOKEN); //Credentials type parameter.
    if (config->token) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_TOKEN, config->token);
    }
    return ESP_OK;
}

// return ESP_OK on successful, ESP_FAIL on failure
static int _params_of_devices_supplies_basic_mqtt_credentials(tbcmh_provision_params_t *params, const tbc_provison_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    // TBC_CHECK_PTR_WITH_RETURN_VALUE(config->deviceName, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceKey, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceSecret, ESP_FAIL);
    if (!config->clientId && !config->username) {
         TBC_LOGE("config->clientId and config->username are NULL! %s()", __FUNCTION__);
         return ESP_FAIL;
    }

    if (config->deviceName) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_DEVICE_NAME, config->deviceName);
    }
    if (config->provisionDeviceKey) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY, config->provisionDeviceKey);
    }
    if (config->provisionDeviceSecret) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET, config->provisionDeviceSecret);
    }
    cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE, TB_MQTT_VALUE_PROVISION_MQTT_BASIC); //Credentials type parameter.
    if (config->clientId) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_CLIENT_ID, config->clientId);
    }
    if (config->username) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_USERNAME, config->username);
    }
    if (config->password) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PASSWORD, config->password);
    }
    return ESP_OK;
}

// hash - Public key X509 hash for device in ThingsBoard.
// return ESP_OK on successful, ESP_FAIL on failure
static int _params_of_devices_supplies_x509_certificate(tbcmh_provision_params_t *params, const tbc_provison_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    // TBC_CHECK_PTR_WITH_RETURN_VALUE(config->deviceName, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceKey, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceSecret, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->hash, ESP_FAIL);

    if (config->deviceName) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_DEVICE_NAME, config->deviceName);
    }
    if (config->provisionDeviceKey) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY, config->provisionDeviceKey);
    }
    if (config->provisionDeviceSecret) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET, config->provisionDeviceSecret);
    }
    cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE, TB_MQTT_VALUE_PROVISION_X509_CERTIFICATE); //Credentials type parameter.
    if (config->hash) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_HASH, config->hash);  ////Public key X509 hash for device in ThingsBoard.
    }
    return ESP_OK;
}

static int _tbcmh_provision_request(tbcmh_handle_t client_,
                                const tbcmh_provision_params_t *params,
                                void *context,
                                tbcmh_provision_on_response_t on_response,
                                tbcmh_provision_on_timeout_t on_timeout)
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (!params) {
          TBC_LOGE("params is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Send msg to server
     //cJSON *object = cJSON_CreateObject(); // create json object
     //cJSON_AddStringToObject(object, TB_MQTT_TEXT_PROVISION_METHOD, method);
     //if (params)
     //     cJSON_AddItemReferenceToObject(object, TB_MQTT_TEXT_PROVISION_PARAMS, params);
     //else 
     //     cJSON_AddNullToObject(object, TB_MQTT_TEXT_PROVISION_PARAMS);
     //char *params_str = cJSON_PrintUnformatted(object); //cJSON_Print(object);
     int request_id;
     char *params_str = cJSON_PrintUnformatted(params); //cJSON_Print(object);
     request_id = tbcm_provision_request(client->tbmqttclient, params_str,
                              client,
                              _tbcmh_on_provision_response,
                              _tbcmh_on_provision_timeout,
                               1/*qos*/, 0/*retain*/);
     cJSON_free(params_str); // free memory
     //cJSON_Delete(object); // delete json object
     if (request_id<0) {
          TBC_LOGE("Init tbcm_provision_request failure! %s()", __FUNCTION__);
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Create provision
     tbcmh_provision_t *provision = _tbcmh_provision_init(client, request_id, params, context, on_response, on_timeout);
     if (!provision) {
          TBC_LOGE("Init provision failure! %s()", __FUNCTION__);
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Insert provision to list
     tbcmh_provision_t *it, *last = NULL;
     if (LIST_FIRST(&client->provision_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->provision_list, provision, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->provision_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, provision, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return request_id;
}

// return request_id or ESP_FAIL
int tbcmh_provision_request(tbcmh_handle_t client_,
                                    const tbc_provison_config_t *config,
                                    void *context,
                                    tbcmh_provision_on_response_t on_response,
                                    tbcmh_provision_on_timeout_t on_timeout)
{
    //TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client_, ESP_FAIL);

    tbcmh_provision_params_t *params = cJSON_CreateObject();
    if (!params) {
         TBC_LOGE("create params is error(NULL)!");
         return ESP_FAIL;
    }
    int ret = ESP_FAIL;
    if (config->provisionType == TBC_PROVISION_TYPE_SERVER_GENERATES_CREDENTIALS) { // Credentials generated by the ThingsBoard server
         ret = _params_of_credentials_generated_by_server(params, config);
    } else if (config->provisionType == TBC_PROVISION_TYPE_DEVICE_SUPPLIES_ACCESS_TOKEN) { // Devices supplies Access Token
         ret = _params_of_devices_supplies_access_token(params, config);
    } else if (config->provisionType == TBC_PROVISION_TYPE_DEVICE_SUPPLIES_BASIC_MQTT_CREDENTIALS) { // Devices supplies Basic MQTT Credentials
         ret = _params_of_devices_supplies_basic_mqtt_credentials(params, config);
    } else if (config->provisionType == TBC_PROVISION_TYPE_DEVICE_SUPPLIES_X509_CREDENTIALS) { // Devices supplies X.509 Certificate)
         ret = _params_of_devices_supplies_x509_certificate(params, config);
    } else {
         TBC_LOGE("config->provisionType(%d) is error!", config->provisionType);
         ret = ESP_FAIL;
    }

    if (ret == ESP_OK) {
         // request_id
         ret = _tbcmh_provision_request(client_, params, context,
                                       on_response, on_timeout);
    } else {
         // TBC_LOGE("ret is error!", ret);
         ret = ESP_FAIL;
    }

    cJSON_Delete(params); // delete json object     
    return ret;
}


static void _tbcmh_provision_on_response(tbcmh_handle_t client_, int request_id, const cJSON *object)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !object) {
          TBC_LOGE("client or object is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search provision
     tbcmh_provision_t *provision = NULL;
     LIST_FOREACH(provision, &client->provision_list, entry) {
          if (provision && (_tbcmh_provision_get_request_id(provision)==request_id)) {
               break;
          }
     }
     if (!provision) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGW("Unable to find provision:%d! %s()", request_id, __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Cache and remove provision
     tbcmh_provision_t *cache = _tbcmh_provision_clone_wo_listentry(provision);
     LIST_REMOVE(provision, entry);
     _tbcmh_provision_destroy(provision);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do response
     _tbcmh_provision_do_response(cache, object);
     // Free cache
     _tbcmh_provision_destroy(cache);

     return;// ESP_OK;
}

static void _tbcmh_provision_on_timeout(tbcmh_handle_t client_, int request_id)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search provision
     tbcmh_provision_t *provision = NULL;
     LIST_FOREACH(provision, &client->provision_list, entry) {
          if (provision && (_tbcmh_provision_get_request_id(provision)==request_id)) {
               break;
          }
     }
     if (!provision) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGW("Unable to find provision:%d! %s()", request_id, __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Cache and remove provision
     tbcmh_provision_t *cache = _tbcmh_provision_clone_wo_listentry(provision);
     LIST_REMOVE(provision, entry);
     _tbcmh_provision_destroy(provision);
     // Give semaphore
     xSemaphoreGive(client->_lock);

     // Do timeout
     _tbcmh_provision_do_timeout(cache);
     // Free provision
     _tbcmh_provision_destroy(cache);

     return;// ESP_OK;
}


//====60.Firmware update================================================================================================
tbcmh_err_t tbcmh_otaupdate_append(tbcmh_handle_t client_, const char *ota_description, const tbcmh_otaupdate_config_t *config)
{
     tbcmh_t *client = (tbcmh_t*)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (!ota_description) {
          TBC_LOGE("ota_description is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (!config) {
          TBC_LOGE("config is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Create otaupdate
     tbcmh_otaupdate_t *otaupdate = _tbcmh_otaupdate_init(client, ota_description, config);
     if (!otaupdate) {
          // Give semaphore
          xSemaphoreGive(client->_lock);
          TBC_LOGE("Init otaupdate failure! ota_description=%s. %s()", ota_description, __FUNCTION__);
          return ESP_FAIL;
     }

     // Insert otaupdate to list
     tbcmh_otaupdate_t *it, *last = NULL;
     if (LIST_FIRST(&client->otaupdate_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->otaupdate_list, otaupdate, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->otaupdate_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, otaupdate, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}
tbcmh_err_t tbcmh_otaupdate_clear(tbcmh_handle_t client_, const char *ota_description)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !ota_description) {
          TBC_LOGE("client or ota_description is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Search item
     tbcmh_otaupdate_t *otaupdate = NULL;
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate && strcmp(_tbcmh_otaupdate_get_description(otaupdate), ota_description)==0) {
               break;
          }
     }
     if (!otaupdate) {
          TBC_LOGW("Unable to remove ota_update data:%s! %s()", ota_description, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Remove form list
     LIST_REMOVE(otaupdate, entry);
     _tbcmh_otaupdate_destroy(otaupdate);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}
static tbcmh_err_t _tbcmh_otaupdate_empty(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // TODO: How to add lock??
     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore!");
     //      return ESP_FAIL;
     // }

     // remove all item in otaupdate_list
     tbcmh_otaupdate_t *otaupdate = NULL, *next;
     LIST_FOREACH_SAFE(otaupdate, &client->otaupdate_list, entry, next) {
          // exec timeout callback
          _tbcmh_otaupdate_do_abort(otaupdate);
          _tbcmh_otaupdate_reset(otaupdate);

          // remove from otaupdate list and destory
          LIST_REMOVE(otaupdate, entry);
          _tbcmh_otaupdate_destroy(otaupdate);
     }
     // memset(&client->otaupdate_list, 0x00, sizeof(client->otaupdate_list));

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return ESP_OK;
}

static void __tbcmh_otaupdate_on_fw_attributesrequest_response(tbcmh_handle_t client,
                void *context, int request_id)
{
    //no code
}
static void __tbcmh_otaupdate_on_sw_attributesrequest_response(tbcmh_handle_t client,
                void *context, int request_id)
{
    //no code
}

static void _tbcmh_otaupdate_on_connected(tbcmh_handle_t client_)
{
    // This function is in semaphore/client->_lock!!!

    tbcmh_t *client = (tbcmh_t *)client_;
    if (!client) {
         TBC_LOGE("client is NULL! %s()", __FUNCTION__);
         return;// ESP_FAIL;
    }

     // Search item
     tbcmh_otaupdate_t *otaupdate = NULL;
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate && (_tbcmh_otaupdate_get_type(otaupdate)==TBCMH_OTAUPDATE_TYPE_FW) ) {
              // send current f/w info UPDATED telemetry
              ////_tbcmh_otaupdate_publish_updated_status(otaupdate); // only at otaupdate->config.is_first_boot

              // send init current f/w info telemetry
              _tbcmh_otaupdate_publish_early_current_version(otaupdate);
              // send f/w info attributes request
              _tbcmh_attributesrequest_send_4_ota_sharedattributes(client_,
                     NULL/*context*/,
                     __tbcmh_otaupdate_on_fw_attributesrequest_response/*on_response*/,
                     NULL/*on_timeout*/,
                     5/*count*/,
                     TB_MQTT_KEY_FW_TITLE,
                     TB_MQTT_KEY_FW_VERSION,
                     TB_MQTT_KEY_FW_SIZE,
                     TB_MQTT_KEY_FW_CHECKSUM,
                     TB_MQTT_KEY_FW_CHECKSUM_ALG);
              break;
          }
     }
     
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate && (_tbcmh_otaupdate_get_type(otaupdate)==TBCMH_OTAUPDATE_TYPE_SW) ) {
              // send current s/w info UPDATED telemetry
              ////_tbcmh_otaupdate_publish_updated_status(otaupdate); // only at otaupdate->config.is_first_boot

              // send init current s/w telemetry
              _tbcmh_otaupdate_publish_early_current_version(otaupdate);
              // send s/w info attributes request
              _tbcmh_attributesrequest_send_4_ota_sharedattributes(client_,
                     NULL/*context*/,
                     __tbcmh_otaupdate_on_sw_attributesrequest_response/*on_response*/,
                     NULL/*on_timeout*/,
                     5/*count*/,
                     TB_MQTT_KEY_SW_TITLE,
                     TB_MQTT_KEY_SW_VERSION,
                     TB_MQTT_KEY_SW_SIZE,
                     TB_MQTT_KEY_SW_CHECKSUM,
                     TB_MQTT_KEY_SW_CHECKSUM_ALG);
              break;
          }
     }

     
}

static void _tbcmh_otaupdate_on_sharedattributes(tbcmh_handle_t client_, tbcmh_otaupdate_type_t ota_type,
                                                 const char *ota_title, const char *ota_version, int ota_size,
                                                 const char *ota_checksum, const char *ota_checksum_algorithm)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client || !ota_title) {
          TBC_LOGE("client or ota_title is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }
     
     tbcm_handle_t tbcm_handle = client->tbmqttclient;

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          _tbcmh_otaupdate_publish_early_failed_status(tbcm_handle, ota_type, "Device code is error!");
          return;// ESP_FAIL;
     }

     // Search item
     tbcmh_otaupdate_t *otaupdate = NULL;
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate &&
               (strcmp(_tbcmh_otaupdate_get_current_title(otaupdate), ota_title)==0) &&
               (_tbcmh_otaupdate_get_type(otaupdate)==ota_type) ) {
               break;
          }
     }
     if (!otaupdate) {
          TBC_LOGW("Unable to find ota_update:%s! %s()", ota_title, __FUNCTION__);
          _tbcmh_otaupdate_publish_early_failed_status(tbcm_handle, ota_type, "Device code is error!");
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return;// ESP_FAIL;
     }

     // exec ota_update
     char ota_error[128] = {0};
     const char* ota_error_ = "Unknown error!";
     int result = _tbcmh_otaupdate_do_negotiate(otaupdate, ota_title, ota_version, ota_size,
                        ota_checksum, ota_checksum_algorithm, ota_error, sizeof(ota_error)-1);
     if (result == 1) { //negotiate successful(next to F/W OTA)
        _tbcmh_otaupdate_publish_going_status(otaupdate, TB_MQTT_VALUE_FW_SW_STATE_DOWNLOADING);
        result = _tbcmh_otaupdate_request_chunk(otaupdate, _tbcmh_on_otaupdate_response, _tbcmh_on_otaupdate_timeout);
        if (result != 0) { //failure to request chunk
            TBC_LOGW("Request first OTA chunk failure! %s()", __FUNCTION__);
            _tbcmh_otaupdate_publish_early_failed_status(tbcm_handle, ota_type, "Request OTA chunk failure!");
            _tbcmh_otaupdate_do_abort(otaupdate);
            _tbcmh_otaupdate_reset(otaupdate);
        }
     } else if (result==0) { //0/ESP_OK: already updated!
        //no code!
        //if (strlen(ota_error)>0) {
        //    ota_error_ = ota_error;
        //}
        //TBC_LOGE("ota_error (%s) of _tbcmh_otaupdate_do_negotiate()!", ota_error_);
        //_tbcmh_otaupdate_publish_early_failed_status(tbcm_handle, ota_type, ota_error_);
     }
     else { //-1/ESP_FAIL: negotiate failure
        if (strlen(ota_error)>0) {
            ota_error_ = ota_error;
        }
        TBC_LOGE("ota_error (%s) of _tbcmh_otaupdate_do_negotiate()!", ota_error_);
        _tbcmh_otaupdate_publish_early_failed_status(tbcm_handle, ota_type, ota_error_);
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return;// ESP_OK;
}

static void _tbcmh_otaupdate_on_response(tbcmh_handle_t client_, int request_id, int chunk_id, const char* payload, int length)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search item
     tbcmh_otaupdate_t *otaupdate = NULL;
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate && (_tbcmh_otaupdate_get_request_id(otaupdate)==request_id)) {
               break;
          }
     }
     if (!otaupdate) {
          TBC_LOGW("Unable to find ota_update:%d! %s()", request_id, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return;// ESP_FAIL;
     }

     // exec ota response
     char ota_error[128] = {0};
     const char* ota_error_ = "Unknown error!";
     int result = _tbcmh_otaupdate_do_write(otaupdate, chunk_id, payload, length, ota_error, sizeof(ota_error)-1);
     switch (result) {
     case 0: //return 0: success on response
         if (_tbcmh_otaupdate_is_received_all(otaupdate))  { //received all f/w or s/w
            _tbcmh_otaupdate_publish_going_status(otaupdate, TB_MQTT_VALUE_FW_SW_STATE_DOWNLOADED);

            if (_tbcmh_otaupdate_checksum_verification(otaupdate)) {
                _tbcmh_otaupdate_publish_going_status(otaupdate, TB_MQTT_VALUE_FW_SW_STATE_VERIFIED);
                _tbcmh_otaupdate_publish_going_status(otaupdate, TB_MQTT_VALUE_FW_SW_STATE_UPDATING);

                memset(ota_error, 0x00, sizeof(ota_error));
                result = _tbcmh_otaupdate_do_end(otaupdate, ota_error, sizeof(ota_error)-1);
                if (result==0) { // sussessful
                    _tbcmh_otaupdate_publish_updated_status(otaupdate); //UPDATED
                    _tbcmh_otaupdate_reset(otaupdate);
                } else {
                    if (strlen(ota_error)>0) {
                        ota_error_ = ota_error;
                    }
                    TBC_LOGE("Unknow result (%d, %s) of _tbcmh_otaupdate_do_write()!", result, ota_error_);
                    _tbcmh_otaupdate_publish_late_failed_status(otaupdate, ota_error_);
                    _tbcmh_otaupdate_do_abort(otaupdate);
                    _tbcmh_otaupdate_reset(otaupdate);
                }
            } else {
                _tbcmh_otaupdate_publish_late_failed_status(otaupdate, "Checksum verification failed!");
                _tbcmh_otaupdate_do_abort(otaupdate);
                _tbcmh_otaupdate_reset(otaupdate);
            }
         }else {  //un-receied all f/w or s/w: go on, get next package
            result = _tbcmh_otaupdate_request_chunk(otaupdate, _tbcmh_on_otaupdate_response, _tbcmh_on_otaupdate_timeout);
            if (result != 0) { //failure to request chunk
                _tbcmh_otaupdate_publish_late_failed_status(otaupdate, "Request OTA chunk failure!");
                _tbcmh_otaupdate_do_abort(otaupdate);
                _tbcmh_otaupdate_reset(otaupdate);
            }
         }
         break;
         
     case -1: //return -1: error
        if (strlen(ota_error)>0) {
            ota_error_ = ota_error;
        }
        TBC_LOGE("ota_error (%s) of _tbcmh_otaupdate_do_write()!", ota_error_);
        _tbcmh_otaupdate_publish_late_failed_status(otaupdate, ota_error_);
        _tbcmh_otaupdate_do_abort(otaupdate);
        _tbcmh_otaupdate_reset(otaupdate);
        break;
        
     default: //Unknow error
        TBC_LOGE("Unknow result (%d) of _tbcmh_otaupdate_do_write()!", result);
        _tbcmh_otaupdate_publish_late_failed_status(otaupdate, ota_error_);
        _tbcmh_otaupdate_do_abort(otaupdate);
        _tbcmh_otaupdate_reset(otaupdate);
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return;// ESP_OK;
}
static void _tbcmh_otaupdate_on_timeout(tbcmh_handle_t client_, int request_id)
{
      tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client  is NULL! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;// ESP_FAIL;
     }

     // Search item
     tbcmh_otaupdate_t *otaupdate = NULL;
     LIST_FOREACH(otaupdate, &client->otaupdate_list, entry) {
          if (otaupdate && (_tbcmh_otaupdate_get_request_id(otaupdate)==request_id)) {
               break;
          }
     }
     if (!otaupdate) {
          TBC_LOGW("Unable to find ota_update:%d! %s()", request_id, __FUNCTION__);
          // Give semaphore
          xSemaphoreGive(client->_lock);
          return;// ESP_FAIL;
     }

     // abort ota
     _tbcmh_otaupdate_publish_late_failed_status(otaupdate, "OTA response timeout!");
     _tbcmh_otaupdate_do_abort(otaupdate);
     _tbcmh_otaupdate_reset(otaupdate);

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return;// ESP_OK;
}

bool tbcmh_has_events(tbcmh_handle_t client_)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return false;
     }

     tbcmh_msg_t msg;
     if (client->_xQueue != 0)
     {
          // Peek a message on the created queue.  Block for 10 ticks if a
          // message is not immediately available.
          if (xQueuePeek(client->_xQueue, &(msg), (TickType_t)10))
          {
               // msg now container to the tbcmh_msg_t variable posted
               // by vATask, but the item still remains on the queue.
               return true;
          }
     }

     return false;
}

static int32_t _tbcmh_deal_msg(tbcmh_handle_t client_, tbcmh_msg_t *msg)
{
     tbcmh_t *client = (tbcmh_t *)client_;
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return ESP_FAIL;
     }
     if (!msg) {
          TBC_LOGE("msg is NULL!");
          return ESP_FAIL;
     }

     // deal msg
     switch (msg->id) {
     case TBCMH_MSGID_TIMER_TIMEOUT:       //
          tbcm_check_timeout(client_->tbmqttclient);
          break;

     case TBCMH_MSGID_CONNECTED:            //
          TBC_LOGI("Connected to thingsboard MQTT server!");
          _tbcmh_connected_on(client_);
          break;
     case TBCMH_MSGID_DISCONNECTED:         //
          TBC_LOGI("Disconnected to thingsboard MQTT server!");
          _tbcmh_disonnected_on(client_);
          break;

     case TBCMH_MSGID_SHAREDATTR_RECEIVED:  //                         cJSON
          _tbcmh_sharedattribute_on_received(client_, msg->body.sharedattr_received.object);
          cJSON_Delete(msg->body.sharedattr_received.object); //free cJSON
          break;

     case TBCMH_MSGID_ATTRREQUEST_RESPONSE: // request_id,             cJSON
          _tbcmh_attributesrequest_on_response(client_, msg->body.attrrequest_response.request_id,
                                               msg->body.attrrequest_response.object);
          cJSON_Delete(msg->body.attrrequest_response.object); // free cJSON
          break;
     case TBCMH_MSGID_ATTRREQUEST_TIMEOUT:  // request_id
          _tbcmh_attributesrequest_on_timeout(client_, msg->body.attrrequest_timeout.request_id);
          break;

     case TBCMH_MSGID_SERVERRPC_REQUSET:    // request_id,             cJSON
          _tbcmh_serverrpc_on_request(client_, msg->body.serverrpc_request.request_id, 
                                      msg->body.serverrpc_request.object);
          cJSON_Delete(msg->body.serverrpc_request.object); //free cJSON
          break;

     case TBCMH_MSGID_CLIENTRPC_RESPONSE:   // request_id,             cJSON
          _tbcmh_clientrpc_on_response(client_, msg->body.clientrpc_response.request_id, 
                                       msg->body.clientrpc_response.object);
          cJSON_Delete(msg->body.clientrpc_response.object); //free cJSON
          break;
     case TBCMH_MSGID_CLIENTRPC_TIMEOUT:    // request_id
          _tbcmh_clientrpc_on_timeout(client_, msg->body.clientrpc_response.request_id);
          break;

     case TBCMH_MSGID_PROVISION_RESPONSE:  //request_id,              payload,  len
          _tbcmh_provision_on_response(client_, msg->body.provision_response.request_id,
                                       msg->body.clientrpc_response.object);
          cJSON_Delete(msg->body.provision_response.object); //free cJSON
          break;
     case TBCMH_MSGID_PROVISION_TIMEOUT:   //request_id
          _tbcmh_provision_on_timeout(client_, msg->body.provision_timeout.request_id);
          break;

     case TBCMH_MSGID_FWUPDATE_RESPONSE:    // request_id,   chunk_id,    payload,  len
          _tbcmh_otaupdate_on_response(client_, msg->body.otaupdate_response.request_id, 
               msg->body.otaupdate_response.chunk_id, 
               msg->body.otaupdate_response.payload,
               msg->body.otaupdate_response.length);
          TBCMH_FREE(msg->body.otaupdate_response.payload); //free payload
          break;
     case TBCMH_MSGID_FWUPDATE_TIMEOUT:     // request_id
          _tbcmh_otaupdate_on_timeout(client_, msg->body.otaupdate_timeout.request_id);
          break;

     default:
          TBC_LOGE("msg->type(%d) is error!\r\n", msg->id);
          return ESP_FAIL;
     }

     return ESP_OK;
}

//recv & deal msg from queue
//_recv()=>recvFromLink()=>parse() //tb_mqtt_client_loop()/checkTimeout(), recv/parse/sendqueue/ack...
void tbcmh_run(tbcmh_handle_t client_)
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
     tbcmh_msg_t msg;
     int i = 0;
     if (client->_xQueue != 0) {
          // Receive a message on the created queue.  Block for 0(10) ticks if a message is not immediately available.
          while (i < 10 && xQueueReceive(client->_xQueue, &(msg), (TickType_t)0)) { // 10
               // pcRxedMessage now points to the struct AMessage variable posted by vATask.
               _tbcmh_deal_msg(client, &msg);
               i++;
          }
     }

     // Give semaphore
     // xSemaphoreGive(client->_lock);
     return;// sendResult == pdTRUE ? true : false;
}

//=====================================================================================================================
//~~static int _tbcmh_sendServerRpcReply(tbcmh_handle_t client_, int request_id, const char* response, int qos=1, int retain=0); //sendServerRpcReply()

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

//static bool _tbcmh_tbDecodeAttributesJsonPayload(JsonObject& attr_kvs); //_tbDecodeAttributesJsonPayload()

static void _tbcmh_on_connected(void *context) //onConnected() // First receive
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
static void _tbcmh_on_disonnected(void *context) //onDisonnected() // First receive
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
//onAttrOfSubReply(); // First receive
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

//onAttributesResponse()=>_attributesResponse() // First send
//~~static bool _attributesResponse(int request_id, const char* payload, int length); //merge to _tbcmh_on_attributesrequest_success()
static void _tbcmh_on_attrrequest_response(void *context, int request_id, const char* payload, int length)
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
//onAttributesResponseTimeout() // First send
static void _tbcmh_on_attrrequest_timeout(void *context, int request_id)
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
          
////onServerRpcRequest() // First receive
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

//onClientRpcResponse() // First send
static void _tbcmh_on_clientrpc_response(void *context, int request_id, const char* payload, int length)
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
//onClientRpcResponseTimeout() // First send
static void _tbcmh_on_clientrpc_timeout(void *context, int request_id)
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


static void _tbcmh_on_provision_response(void *context, int request_id, const char* payload, int length)
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

static void _tbcmh_on_provision_timeout(void *context, int request_id)
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
static void _tbcmh_on_otaupdate_response(void *context, int request_id, int chunk_id, const char* payload, int length)
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
     msg.body.otaupdate_response.payload = TBCMH_MALLOC(length); /*!< received palyload. free memory by msg receiver */
     if (msg.body.otaupdate_response.payload) {
          memcpy(msg.body.otaupdate_response.payload, payload, length);
     }
     msg.body.otaupdate_response.length = length;
     _tbcmh_sendTbmqttMsg2Queue(client, &msg);
}
// First send
static void _tbcmh_on_otaupdate_timeout(void *context, int request_id)
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

