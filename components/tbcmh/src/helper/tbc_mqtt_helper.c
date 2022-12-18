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

// ThingsBoard MQTT Client helper (high layer) API

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* using uri parser */
#include "http_parser.h"

#include "tbc_utils.h"

#include "tbc_mqtt_wapper.h"
#include "tbc_mqtt_helper.h"

#include "tbc_mqtt_helper_internal.h"

static void _on_tbcm_event_handle(tbcm_event_t *event);
static void _on_tbcm_event_bridge_send(tbcm_event_t *event);

const static char *TAG = "tb_mqtt_client_helper";

uint32_t _tbcmh_get_request_id(tbcmh_handle_t client)
{
    // call it in a lock!
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);

    client->next_request_id++;
    if (client->next_request_id <= 0) {
        client->next_request_id = 1;
    }
    return client->next_request_id;
}

tbcmh_handle_t tbcmh_init(void)
{
     tbcmh_t *client = (tbcmh_t *)TBC_MALLOC(sizeof(tbcmh_t));
     if (!client) {
          TBC_LOGE("Unable to malloc memory! %s()", __FUNCTION__);
          return NULL;
     }
     memset(client, 0x00, sizeof(tbcmh_t));

     client->tbmqttclient = tbcm_init();
     // Create a queue capable of containing 20 tbcm_event_t structures.
     // These should be passed by pointer as they contain a lot of data.
     // client->is_running_in_mqtt_task = is_running_in_mqtt_task;
     // if (!client->is_running_in_mqtt_task) {
        client->_xQueue = xQueueCreate(40, sizeof(tbcm_event_t));
         if (client->_xQueue == NULL) {
              TBC_LOGE("failed to create the queue! %s()", __FUNCTION__);
         }
     // }

     //tbc_transport_storage_free_fields(&client->config);
     client->context = NULL;
     client->on_connected = NULL; 
     client->on_disconnected = NULL;

     client->_lock = xSemaphoreCreateMutex();
     if (client->_lock == NULL)  {
          TBC_LOGE("failed to create the lock!");
     }
     
     // create all 7/9 list!
     //_tbcmh_timeseriesdata_on_create(client);
     //tbce_clientattributes_create(client);
     _tbcmh_attributessubscribe_on_create(client);
     _tbcmh_attributesrequest_on_create(client);//req-resp
     _tbcmh_serverrpc_on_create(client);
     _tbcmh_clientrpc_on_create(client);        //req-resp
     _tbcmh_otaupdate_on_create(client);        //chunk: req-resp
     _tbcmh_claimingdevice_on_create(client);
     _tbcmh_provision_on_create(client);  //req-resp

     client->next_request_id = 0;
     client->last_check_timestamp = (uint64_t)time(NULL);

     return client;
}

void tbcmh_destroy(tbcmh_handle_t client)
{
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     // TODO: dead lock???
     tbcmh_disconnect(client);

     // empty all 7/9 list!
     //_tbcmh_timeseriesdata_on_destroy(client);
     //tbce_clientattributes_destroy(client);
     _tbcmh_attributessubscribe_on_destroy(client);
     _tbcmh_attributesrequest_on_destroy(client);
     _tbcmh_serverrpc_on_destroy(client);
     _tbcmh_clientrpc_on_destroy(client);
     _tbcmh_otaupdate_on_destroy(client);
     _tbcmh_claimingdevice_on_destroy(client);
     _tbcmh_provision_on_destroy(client);

     if (client->_lock) {
          vSemaphoreDelete(client->_lock);
          client->_lock = NULL;
     }

     // config in tbcmh_disconnect()
     if (client->_xQueue) {
          vQueueDelete(client->_xQueue);
          client->_xQueue = NULL;
     }
     // client->is_running_in_mqtt_task = false;
     tbcm_destroy(client->tbmqttclient);

     TBC_FREE(client);
}

/**
 * @brief Destroys the client handle
 *
 * Notes:
 *  - Cannot be called from the mqtt event handler
 *
 *
 * @return trure
 *         false  on wrong connection
 */
bool tbcmh_connect(tbcmh_handle_t client, const tbc_transport_config_t* config,
                        void *context,
                        tbcmh_on_connected_t on_connected,
                        tbcmh_on_disconnected_t on_disconnected)
{
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
    // uint16_t next_request_id;
    // uint64_t last_check_timestamp;
    
    // connect
    TBC_LOGI("connecting to %s://%s:%d ...",
                config->address.schema, config->address.host, config->address.port);
    bool result = false;
    // if (client->is_running_in_mqtt_task) {
    //     result = tbcm_connect(client->tbmqttclient, config, client, _on_tbcm_event_handle);
    // } else {
        result = tbcm_connect(client->tbmqttclient, config, client, _on_tbcm_event_bridge_send);
    // }
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

bool tbcmh_connect_using_url(tbcmh_handle_t client,
                                const tbc_transport_config_esay_t *config,
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
    if (transport.address.port==0){
        if (strcasecmp (transport.address.schema, "mqtt") == 0) {
            transport.address.port = 1883;
        }
        else if (strcasecmp (transport.address.schema, "mqtts") == 0) {
            transport.address.port = 8883;
        }
        else if (strcasecmp (transport.address.schema, "ws") == 0) {
            transport.address.port = 80;
        }
        else if (strcasecmp (transport.address.schema, "wss") == 0) {
            transport.address.port = 443;
        }
    }
    transport.address.path   = address.path;
    transport.credentials.username = credentials.username;
    transport.credentials.password = credentials.password;
    transport.credentials.token = config->access_token;
    transport.credentials.type = TBC_TRANSPORT_CREDENTIALS_TYPE_ACCESS_TOKEN;
    transport.log_rxtx_package = config->log_rxtx_package;
    result = tbcmh_connect(client, &transport, context, on_connected, on_disconnected);

fail_exit:
    TBC_FIELD_FREE(address.schema);
    TBC_FIELD_FREE(address.host);
    TBC_FIELD_FREE(address.path);
    TBC_FIELD_FREE(credentials.username);
    TBC_FIELD_FREE(credentials.password);
    return result;
}

void tbcmh_disconnect(tbcmh_handle_t client)               
{
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

     TBC_LOGI("disconnecting from %s://%s:%d ...", client->config.address.schema,
                client->config.address.host, client->config.address.port);
     // empty msg queue
     while (tbcmh_has_events(client)) {
          tbcmh_run(client);
     }
     // disconnect
     tbcm_disconnect(client->tbmqttclient);
     while (tbcmh_has_events(client)) {
          tbcmh_run(client);
     }

     // clear config & callback
     tbc_transport_storage_free_fields(&client->config);
     client->context = NULL;
     client->on_connected = NULL;
     client->on_disconnected = NULL;

     //_tbcmh_timeseriesdata_on_disconnected(client);
     _tbcmh_attributesrequest_on_disconnected(client); //empty all request
     //_tbcmh_clientattribute_on_disconnected(client); 
     _tbcmh_attributessubscribe_on_disconnected(client);
     _tbcmh_serverrpc_on_disconnected(client);
     _tbcmh_clientrpc_on_disconnected(client);         //empty all request
     _tbcmh_provision_on_disconnected(client);   //empty all request
     _tbcmh_otaupdate_on_disconnected(client);         //empty all request
     _tbcmh_claimingdevice_on_disconnected(client);

     // SemaphoreHandle_t lock;
     // uint16_t next_request_id;
     // uint64_t last_check_timestamp;
}

bool tbcmh_is_connected(tbcmh_handle_t client)
{
     if (client && client->tbmqttclient && tbcm_is_connected(client->tbmqttclient)) {
          return true;
     }
     return false;
}

static void __on_tbcm_connected(tbcmh_handle_t client)
{
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     // Take semaphore
     if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return;
     }

     //_tbcmh_telemetry_on_connected(client);
     _tbcmh_attributesrequest_on_connected(client);
     //_tbcmh_clientattribute_on_connected(client);
     _tbcmh_attributessubscribe_on_connected(client);
     _tbcmh_serverrpc_on_connected(client);
     _tbcmh_clientrpc_on_connected(client);
     _tbcmh_claimingdevice_on_connected(client);
     _tbcmh_otaupdate_on_connected(client);
     _tbcmh_provision_on_connected(client);

     // clone parameter in lock/unlock
     void *context = client->context;
     tbcmh_on_connected_t on_connected = client->on_connected;

     // Give semaphore
     xSemaphoreGiveRecursive(client->_lock);
  
     // do callback
     TBC_LOGI("before call on_connected()...");
     if (on_connected) {
         on_connected(client, context);
     }
     TBC_LOGI("after call on_connected()");
     return;
}

static void __on_tbcm_disonnected(tbcmh_handle_t client)
{
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return;
     }

     // Take semaphore
     // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
     //      return;
     // }

     //_tbcmh_timeseriesdata_on_disconnected(client);
     _tbcmh_attributesrequest_on_disconnected(client); //empty all request
     //_tbcmh_clientattribute_on_disconnected(client); 
     _tbcmh_attributessubscribe_on_disconnected(client);
     _tbcmh_serverrpc_on_disconnected(client);
     _tbcmh_clientrpc_on_disconnected(client);         //empty all request
     _tbcmh_provision_on_disconnected(client);   //empty all request
     _tbcmh_otaupdate_on_disconnected(client);         //empty all request
     _tbcmh_claimingdevice_on_disconnected(client);

     // clone parameter in lock/unlock
     void *context = client->context;
     tbcmh_on_disconnected_t on_disconnected = client->on_disconnected;

     // Give semaphore
     // xSemaphoreGiveRecursive(client->_lock);

     // do callback
     if (on_disconnected) {
        on_disconnected(client, context);
     }
     return;
}

static void __on_tbcm_data_handle(tbcm_event_t *event)
{
    TBC_CHECK_PTR(event);
    TBC_CHECK_PTR(event->user_context);
    
    tbcmh_t *client = (tbcmh_t *)event->user_context;
    cJSON *object = NULL;

    if (event->event_id != TBCM_EVENT_DATA) {
        TBC_LOGE("event->event_id(%d) is not TBCM_EVENT_DATA(%d)!", event->event_id, TBCM_EVENT_DATA);
        return;
    }
    
    switch (event->data.topic) {
    case TBCM_RX_TOPIC_ATTRIBUTES_RESPONSE:  /*!< request_id,           payload, payload_len */
         object = cJSON_ParseWithLength(event->data.payload, event->data.payload_len);
         _tbcmh_attributesrequest_on_data(client, event->data.request_id, object);
         cJSON_Delete(object);
         break;
    
    case TBCM_RX_TOPIC_SHARED_ATTRIBUTES:    /*!<                       payload, payload_len */
         object = cJSON_ParseWithLength(event->data.payload, event->data.payload_len);
         _tbcmh_attributessubscribe_on_data(client, object);
         cJSON_Delete(object);
         break;
    
    case TBCM_RX_TOPIC_SERVERRPC_REQUEST:    /*!< request_id,           payload, payload_len */
         object = cJSON_ParseWithLength(event->data.payload, event->data.payload_len);
         _tbcmh_serverrpc_on_data(client, event->data.request_id, object);
         cJSON_Delete(object);
         break;
        
    case TBCM_RX_TOPIC_CLIENTRPC_RESPONSE:   /*!< request_id,           payload, payload_len */
         object = cJSON_ParseWithLength(event->data.payload, event->data.payload_len);
         _tbcmh_clientrpc_on_data(client, event->data.request_id, object);
         cJSON_Delete(object);
         break;
    
    case TBCM_RX_TOPIC_FW_RESPONSE:          /*!< request_id, chunk_id, payload, payload_len */
         _tbcmh_otaupdate_on_chunk_data(client, event->data.request_id, 
              event->data.chunk_id, 
              event->data.payload,
              event->data.payload_len);
         break;
    
    case TBCM_RX_TOPIC_PROVISION_RESPONSE:   /*!< (no request_id)       payload, payload_len */
         object = cJSON_ParseWithLength(event->data.payload, event->data.payload_len);
         _tbcmh_provision_on_data(client, event->data.request_id, object);
         cJSON_Delete(object);
         break;

    case TBCM_RX_TOPIC_ERROR:
    default:
         TBC_LOGW("Other topic: event->data.topic=%d", event->data.topic);
         break;
    }
}

static void __on_tbcm_check_timeout(tbcmh_handle_t client)
{
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

     _tbcmh_attributesrequest_on_check_timeout(client, timestamp);
     _tbcmh_clientrpc_on_check_timeout(client, timestamp);
     _tbcmh_provision_on_check_timeout(client, timestamp);
     _tbcmh_otaupdate_on_chunk_check_timeout(client, timestamp);
}

// The callback for when a MQTT event is received.
static void _on_tbcm_event_handle(tbcm_event_t *event)
{
     TBC_CHECK_PTR(event);
     TBC_CHECK_PTR(event->user_context);

     tbcmh_t *client = (tbcmh_t *)event->user_context;

     switch (event->event_id) {
     case TBCM_EVENT_BEFORE_CONNECT:
          TBC_LOGI("TBCM_EVENT_BEFORE_CONNECT, msg_id=%d", event->msg_id);
          break;

     case TBCM_EVENT_CONNECTED:
          TBC_LOGI("TBCM_EVENT_CONNECTED");
          TBC_LOGI("client->tbmqttclient = %p", client->tbmqttclient);
          TBC_LOGI("Connected to thingsboard MQTT server!");
          __on_tbcm_connected(client);
          break;

     case TBCM_EVENT_DISCONNECTED:
          TBC_LOGI("TBCM_EVENT_DISCONNECTED");
          TBC_LOGI("Disconnected to thingsboard MQTT server!");
          __on_tbcm_disonnected(client);
          break;

     case TBCM_EVENT_SUBSCRIBED:
          TBC_LOGI("TBCM_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
          break;
     case TBCM_EVENT_UNSUBSCRIBED:
          TBC_LOGI("TBCM_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
          break;
     case TBCM_EVENT_PUBLISHED:
          TBC_LOGI("TBCM_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
          break;

     case TBCM_EVENT_DATA:
          TBC_LOGI("TBCM_EVENT_DATA");
          ////TBC_LOGI("DATA=%.*s", event->payload_len, event->payload);
          __on_tbcm_data_handle(event);
          break;

     case TBCM_EVENT_ERROR:
          TBC_LOGW("TBCM_EVENT_ERROR: esp_tls_last_esp_err=%d, esp_tls_stack_err=%d, "
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

     case TBCM_EVENT_DELETED:
          TBC_LOGW("TBCM_EVENT_DELETED: msg_id=%d", event->msg_id);
          break;

     case TBCM_EVENT_CHECK_TIMEOUT:
          TBC_LOGD("TBCM_EVENT_CHECK_TIMEOUT");
          __on_tbcm_check_timeout(client);
          break;

     default:
          TBC_LOGW("Other event: event_id=%d", event->event_id);
          break;
     }
}

//recv & deal msg from queue
//recv/parse/sendqueue/ack...
static void _on_tbcm_event_bridge_receive(tbcmh_handle_t client)
{
    if (!client) {
         TBC_LOGE("client is NULL! %s()", __FUNCTION__);
         return; // false;
    }
    
    // TODO: whether to insert lock?
    // Take semaphore
    // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return false;
    // }
    
    // read event from queue()
    tbcm_event_t event;
    int i = 0;
    if (client->_xQueue != 0) {
         // Receive a message on the created queue.  Block for 0(10) ticks if a message is not immediately available.
         while (i < 10 && xQueueReceive(client->_xQueue, &event, (TickType_t)0)) { // 10
              // pcRxedMessage now points to the struct event variable posted by vATask.
              _on_tbcm_event_handle(&event);
              
              if ((event.event_id==TBCM_EVENT_DATA) && event.data.payload && (event.data.payload_len>0)) {
                  TBC_FREE(event.data.payload);
                  event.data.payload = NULL;
              }
    
              i++;
         }
    }
    
    // Give semaphore
    // xSemaphoreGiveRecursive(client->_lock);
}

// call in user task, NOT mqtt task!
void tbcmh_run(tbcmh_handle_t client)
{
    _on_tbcm_event_bridge_receive(client);
}

// call in user task, NOT mqtt task!
bool tbcmh_has_events(tbcmh_handle_t client)
{
     if (!client) {
          TBC_LOGE("client is NULL! %s()", __FUNCTION__);
          return false;
     }

     tbcm_event_t event;
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

// NOTE: This function is running in the MQTT thread!
// send msg to queue with clone event & publish_data
static void _on_tbcm_event_bridge_send(tbcm_event_t *event)
{
    TBC_CHECK_PTR(event);
    TBC_CHECK_PTR(event->user_context);

    tbcmh_t *client = (tbcmh_t *)event->user_context;
    if (!client->_xQueue) {
         TBC_LOGE("client->_xQueue is NULL!");
         return;
    }

    char *payload = NULL;
    if ((event->event_id==TBCM_EVENT_DATA) && event->data.payload && (event->data.payload_len>0)) {
        payload = TBC_MALLOC(event->data.payload_len);
        if (!payload) {
            TBC_LOGE("malloc(%d) memory failure! %s()", event->data.payload_len, __FUNCTION__);
            return;            
        }
        memcpy(payload, event->data.payload, event->data.payload_len);
        event->data.payload = payload;
    }

    // TODO: whether/how to insert lock?
    // Take semaphore
    // if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
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
    if (sendResult != pdTRUE) {
        if (!payload) {
            TBC_FREE(payload);
            payload = NULL;
        }
        TBC_LOGW("send innermsg timeout! %s()", __FUNCTION__);
    }
    
    // Give semaphore
    // xSemaphoreGiveRecursive(client->_lock);
}

