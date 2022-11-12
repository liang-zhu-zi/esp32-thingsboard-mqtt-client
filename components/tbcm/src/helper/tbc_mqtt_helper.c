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

static void _tbcmh_on_connected(void *context);
static void _tbcmh_on_disonnected(void *context);
static void _tbcmh_on_sharedattr_received(void *context, const char* payload, int length);
//static void _tbcmh_on_attrrequest_response(void *context, int request_id, const char* payload, int length);
//static void _tbcmh_on_attrrequest_timeout(void *context, int request_id);
static void _tbcmh_on_serverrpc_request(void *context, int request_id, const char* payload, int length);
//static void _tbcmh_on_clientrpc_response(void *context, int request_id, const char* payload, int length);
//static void _tbcmh_on_clientrpc_timeout(void *context, int request_id);
//static void _tbcmh_on_provision_response(void *context, int request_id, const char* payload, int length);
//static void _tbcmh_on_provision_timeout(void *context, int request_id);
//static void _tbcmh_on_otaupdate_response(void *context, int request_id, int chunk_id, const char* payload, int length);
//static void _tbcmh_on_otaupdate_timeout(void *context, int request_id);   

static void _response_timer_create(tbcmh_handle_t client_);
static void _response_timer_start(tbcmh_handle_t client_);
static void _response_timer_stop(tbcmh_handle_t client_);
static void _response_timer_destroy(tbcmh_handle_t client_);


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
          TBC_FREE(msg->body.otaupdate_response.payload); //free payload
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
//recv/parse/sendqueue/ack...
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
