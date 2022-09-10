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

#include <string.h>
#include <time.h>

#include "tb_mqtt_client.h"

//#define TBMCLOG_LONG

#define TBMCLOG_LEVLE (4)               //ERR:1, WARN:2, INFO:3, DBG:4, OBSERVES:5

#if (TBMCLOG_LEVLE>=1)
#define TBMCLOG_E(format, ...)  printf("[TBMC][E] " format "\r\n", ##__VA_ARGS__)
#else 
#define TBMCLOG_E
#endif
#if (TBMCLOG_LEVLE>=2)
#define TBMCLOG_W(format, ...)  printf("[TBMC][W] " format "\r\n", ##__VA_ARGS__)
#else 
#define TBMCLOG_W
#endif
#if (TBMCLOG_LEVLE>=3)
#define TBMCLOG_I(format, ...)  printf("[TBMC][I] " format "\r\n", ##__VA_ARGS__)
#else 
#define TBMCLOG_I
#endif
#if (TBMCLOG_LEVLE>=4)
#define TBMCLOG_D(format, ...)  printf("[TBMC][D] " format "\r\n", ##__VA_ARGS__)
#else 
#define TBMCLOG_D
#endif
#if (TBMCLOG_LEVLE>=5)
#define TBMCLOG_V(format, ...)  printf("[TBMC][V] " format "\r\n", ##__VA_ARGS__)
#else 
#define TBMCLOG_V
#endif

typedef enum
{
     TBMC_REQUEST_ATTR = 1,
     TBMC_REQUEST_CLIENTRPC,
     TBMC_REQUEST_FWUPDATE
} tbmc_request_type_t;

typedef struct tbmc_request
{
     tbmc_request_type_t type;
     int request_id;

     uint32_t timestamp; /*!< time stamp at sending request */
     void *context;
     tbmc_on_response_t on_response;
     tbmc_on_timeout_t on_timeout;

     LIST_ENTRY(tbmc_request) entry;
} tbmc_request_t;

/**
 * ThingsBoard MQTT Client
 */
typedef struct tbmc_client_
{
     esp_mqtt_client_handle_t mqtt_handle;

     tbmc_config_t config; /*!< ThingsBoard MQTT config */
     void *context;
     tbmc_on_connected_t on_connected;                     /*!< Callback of connected ThingsBoard MQTT */
     tbmc_on_disconnected_t on_disconnected;               /*!< Callback of disconnected ThingsBoard MQTT */
     tbmc_on_sharedattr_received_t on_sharedattr_received; /*!< Callback of receiving ThingsBoard MQTT shared-attribute T*/
     tbmc_on_serverrpc_request_t on_serverrpc_request;     /*!< Callback of receiving ThingsBoard MQTT server-RPC request */

     volatile tbmc_state_t state; // TBMQTT_STATE state;

     SemaphoreHandle_t lock;
     int next_request_id;
     uint32_t last_check_timestamp;
     LIST_HEAD(tbmc_request_list, tbmc_request) request_list; /*!< request list: attributes request, client side RPC & fw update request */ ////QueueHandle_t timeoutQueue;
} tbmc_t;

static int _tbmc_subscribe(tbmc_handle_t client_, const char *topic, int qos /*=0*/);
static int _tbmc_publish(tbmc_handle_t client_, const char *topic, const char *payload, int qos /*= 1*/, int retain /*= 0*/);
static esp_err_t _on_MqttEventCallback(/*tbmc_handle_t client_,*/ esp_mqtt_event_handle_t event);
static esp_err_t _on_MqttEventHandle(tbmc_handle_t client_, esp_mqtt_event_handle_t event);
static void _on_DataEventProcess(tbmc_handle_t client_, esp_mqtt_event_handle_t event);
static bool _request_is_equal(const tbmc_request_t *a, const tbmc_request_t *b);
static int _request_list_create_and_append(tbmc_handle_t client_, tbmc_request_type_t type, int request_id,
                                           void *context,
                                           tbmc_on_response_t on_response,
                                           tbmc_on_timeout_t on_timeout);
static tbmc_request_t *_request_list_search_and_remove(tbmc_handle_t client_, int request_id);
static int _request_list_move_all_of_timeout(tbmc_handle_t client_, uint32_t timestamp,
                                             LIST_HEAD(tbmc_request_list, tbmc_request) * timeout_request_list);
static tbmc_request_t *_request_create(tbmc_request_type_t type,
                                       uint32_t request_id,
                                       void *context,
                                       tbmc_on_response_t on_response,
                                       tbmc_on_timeout_t on_timeout);
static void _request_destroy(tbmc_request_t *tbmc_request);

// Initializes tbmc_handle_t with network client.
tbmc_handle_t tbmc_init(void)
{
     tbmc_t *client = TBMC_MALLOC(sizeof(tbmc_t));
     if (!client) {
          TBMCLOG_E("Unable to malloc memeory!");
          return NULL;
     }

     client->mqtt_handle = NULL;
     memset(&client->config, 0x00, sizeof(client->config));
     client->context = NULL;
     client->on_connected = NULL;           /*!< Callback of connected ThingsBoard MQTT */
     client->on_disconnected = NULL;        /*!< Callback of disconnected ThingsBoard MQTT */
     client->on_sharedattr_received = NULL; /*!< Callback of receiving ThingsBoard MQTT shared-attribute T*/
     client->on_serverrpc_request = NULL;   /*!< Callback of receiving ThingsBoard MQTT server-RPC request */

     client->state = TBMC_STATE_DISCONNECTED;

     client->lock = xSemaphoreCreateMutex();
     client->next_request_id = 0;
     client->last_check_timestamp = (uint32_t)time(NULL);
     client->request_list = LIST_HEAD_INITIALIZER(client->request_list);

     return client;
}

// Destroys tbmc_handle_t with network client.
void tbmc_destroy(tbmc_handle_t client_)
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return;
     }

     if (client->mqtt_client) {
          tbmc_disconnect(client);
          client->mqtt_client = NULL;
     }
     if (client->lock) {
          vSemaphoreDelete(client->lock);
          client->lock = NULL;
     }

     TBMC_FREE(client);
}

// Connects to the specified ThingsBoard server and port.
// Access token is used to authenticate a client.
// Returns true on success, false otherwise.
bool tbmc_connect(tbmc_handle_t client_,
                  tbmc_config_t *config,
                  void *context,
                  tbmc_on_connected_t on_connected,
                  tbmc_on_disconnected_t on_disonnected,
                  tbmc_on_sharedattr_received_t on_sharedattributes_received,
                  // tbmc_on_attrrequest_response_t on_attrrequest_response,
                  // tbmc_on_attrrequest_timeout_t on_attrrequest_timeout,
                  // tbmc_on_clientrpc_response_t on_clientrpc_response,
                  // tbmc_on_clientrpc_timeout_t on_clientrpc_timeout,
                  tbmc_on_serverrpc_request_t on_serverrpc_request //,
                  /* tbmc_on_fwupdate_response_t on_fwupdate_response, */
                  /* tbmc_on_fwupdate_timeout_t on_fwupdate_timeout */) // connect()//...start()
{
     /*const char *host, int port = 1883, */
     /*min_reconnect_delay=1, timeout=120, tls=False, ca_certs=None, cert_file=None, key_file=None*/
     if (!client_ || !config || !config->access_token || !config->uri) {
          TBMCLOG_W("one argument isn't NULL!");
          return false;
     }

     tbmc_t *client = (tbmc_t*)client_;
     if (client->mqtt_handle) {
          TBMCLOG_W("unable to re-connect mqtt client: client isn't NULL!");
          return false; //!!
     }

     free(client->config.uri);               client->config.uri = strdup(config->uri);
     free(client->config.access_token);      client->config.access_token = strdup(config->access_token);
     free(client->config.cert_pem);          client->config.cert_pem = strdup(config->cert_pem);
     free(client->config.client_cert_pem);   client->config.client_cert_pem = strdup(config->client_cert_pem);
     free(client->config.client_key_pem);    client->config.client_key_pem = strdup(config->client_key_pem);

     client->context = context;
     client->on_connected = on_connected;
     client->on_disonnected = on_disonnected;
     client->on_sharedattributes_received = on_sharedattributes_received;
     client->on_serverrpc_request = on_serverrpc_request;

     client->state = TBMC_STATE_DISCONNECTED;

     // SemaphoreHandle_t lock;
     // int next_request_id;
     // uint32_t last_check_timestamp;
     // LIST_HEAD(tbmc_request_list, tbmc_request) request_list; /*!< request list: attributes request, client side RPC & fw update request */ ////QueueHandle_t timeoutQueue;

     const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = config->uri,
        .client_id = NULL, ////"TbDev"
        .username = config->access_token,
        .client_cert_pem = config->client_cert_pem,
        .client_key_pem = config->client_key_pem,
        .cert_pem = config->server_cert_pem,
     };
     client->mqtt_handle = esp_mqtt_client_init(&mqtt_cfg);
     if (!client->mqtt_handle)
     {
          TBMCLOG_W("unable to init mqtt client");
          return false;
     }
     int32_t result = esp_mqtt_client_start(client->mqtt_handle);
     if (result != ESP_OK)
     {
          TBMCLOG_W("unable to start mqtt client");
          esp_mqtt_client_destroy(client->mqtt_handle);
          client->mqtt_handle = NULL;
          return false;
     }

     client->state = TBMC_STATE_CONNECTING;
     return true;
}

// Disconnects from ThingsBoard. Returns true on success.
void tbmc_disconnect(tbmc_handle_t client_) // disconnect()//...stop()
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return;
     }
     if (!client->mqtt_handle) {
          TBMCLOG_W("unable to disconnect mqtt client: mqtt client is NULL!");
          return;
     }

     int32_t result = esp_mqtt_client_stop(client->mqtt_handle);
     if (result != ESP_OK) {
          TBMCLOG_E("unable to stop mqtt client");
          return;
     }

     result = esp_mqtt_client_destroy(client->mqtt_handle);
     if (result != ESP_OK) {
          TBMCLOG_E("unable to stop mqtt client");
          return;
     }

     client->mqtt_handle = NULL;

     free(client->config.uri);               client->config.uri = NULL;
     free(client->config.access_toke);       client->config.access_token = NULL;
     //free(client->config.client_id);       client->config.client_id = NULL;
     //free(client->config.username);        client->config.username = NULL;
     free(client->config.cert_pem);          client->config.cert_pem = NULL;
     free(client->config.client_cert_pem);   client->config.client_cert_pem = NULL;
     free(client->config.client_key_pem);    client->config.client_key_pem = NULL;

     client->context = NULL;
     client->on_connected = NULL;
     client->on_disonnected = NULL;
     client->on_sharedattributes_received = NULL;
     client->on_serverrpc_request = NULL;

     client->state = TBMC_STATE_DISCONNECTED;

     // SemaphoreHandle_t lock;
     // int next_request_id;
     // uint32_t last_check_timestamp;
     // LIST_HEAD(tbmc_request_list, tbmc_request) request_list; /*!< request list: attributes request, client side RPC & fw update request */ ////QueueHandle_t timeoutQueue;
}

// Returns true if connected, false otherwise.
bool tbmc_is_connected(tbmc_handle_t client_) // isConnected
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return false;
     }
     return client->state == TBMC_STATE_CONNECTED; 
}

bool tbmc_is_connecting(tbmc_handle_t client)
{
     return client->state == TBMC_STATE_CONNECTING; 
}

bool tbmc_is_disconnected(tbmc_handle_t client_)
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return false;
     }
     return client->state == TBMC_STATE_DISCONNECTED; 
}

tbmc_state_t tbmc_get_state(tbmc_handle_t client_)
{
     tbmc_t *client = (tbmc_t *)client_;
     if (!client) {
          TBMCLOG_E("client is NULL");
          return TBMC_STATE_DISCONNECTED;
     }
     return client->state;
}

void tbmc_check_timeout(tbmc_handle_t client_) // Executes an event loop for PubSub client. //loop()==>checkTimeout()
{
     tbmc_t *client = (tbmc_t *)client_;
     if (!client) {
          TBMCLOG_E("client is NULL");
          return;
     }

     // Too early
     uint32_t timestamp = (uint32_t)time(NULL);
     if (timestamp < client->last_check_timestamp + TB_MQTT_TIMEOUT + 2) {
          return;
     }
     client->last_check_timestamp = timestamp;

     // move timeout item to timeout_list
     LIST_HEAD(tbmc_request_list, tbmc_request) timeout_list = LIST_HEAD_INITIALIZER(timeout_list);
     int count = _request_list_move_all_of_timeout(client, timestamp, &timeout_list);
     if (count <=0 ) {
          return;
     }

     tbmc_request_t *tbmc_request = NULL, *next;
     LIST_FOREACH_SAFE(tbmc_request, &timeout_list, entry, next) {
          // exec timeout callback
          if (tbmc_request->on_timeout) {
               tbmc_request->on_timeout(client->context, tbmc_request->request_id);
          }

          // remove from request list
          LIST_REMOVE(tbmc_request, entry);
          _request_destroy(tbmc_request);
     }
}

/**
 * @brief Client to send a 'Telemetry' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/telemetry'
 *      Data:  '{"key1":"value1", "key2":true, "key3": 3.0, "key4": 4}', '[{"key1":"value1"}, {"key2":true}]'
 *
 * @param telemetry  telemetry. example: {"key1":"value1", "key2":true, "key3": 3.0, "key4": 4}, (字符串要符合 json 数据格式)
 * @param qos        qos of publish message
 * @param retain     ratain flag
 *
 * @return message_id of the subscribe message on success
 *         0 if cannot publish
 *        -1 if error
 */
int tbmc_telemetry_publish(tbmc_handle_t client_, const char *telemetry,
                           int qos /*= 1*/, int retain /*= 0*/) // sendTelemetry()
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return -1;
     }

#ifdef TBMCLOG_LONG
     TBMCLOG_D("[Telemetry][Tx] QoS=%d, Retain=%d, Timestamp=%u, Length=%d\r\n\t\t%.*s",
              qos, retain, (uint32_t)time(NULL), strlen(telemetry),
              strlen(telemetry), telemetry);
#else
     TBMCLOG_D("[Telemetry][Tx] %.*s",
              strlen(telemetry), telemetry);
#endif

     int message_id = _tbmc_publish(client, TB_MQTT_TOPIC_TELEMETRY_PUBLISH, telemetry, qos, retain);
     return message_id;
}

/**
 * @brief Client to send a 'Attributes' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/attributes'
 *      Data:  '{"attribute1":"value1", "attribute2":true, "attribute3":42.0, "attribute4":73}'
 *
 * @param attributes    attributes. example: {"attribute1":"value1", "attribute2":true, "attribute3":42.0, "attribute4":73} (字符串要符合 json 数据格式)
 * @param qos           qos of publish message
 * @param retain        ratain flag
 *
 * @return message_id of the subscribe message on success
 *         0 if cannot publish
 *        -1 if error
 */
int tbmc_clientattributes_publish(tbmc_handle_t client_, const char *attributes,
                                         int qos /*= 1*/, int retain /*= 0*/) // sendAttributes() //publish client attributes
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return -1;
     }

#ifdef TBMCLOG_LONG
     TBMCLOG_D("[Client-Side Attributes][Tx] QoS=%d, Retain=%d, Timestamp=%u, Length=%d\r\n\t\t%.*s",
              qos, retain, (uint32_t)time(NULL), strlen(attributes),
              strlen(attributes), attributes);
#else
     TBMCLOG_D("[Client-Side Attributes][Tx] %.*s",
              strlen(attributes), attributes);
#endif

     int message_id = _tbmc_publish(client, TB_MQTT_TOPIC_CLIENT_ATTRIBUTES_PUBLISH, attributes, qos, retain);
     return message_id;
}

/**
 * @brief Client to send a 'Attributes Request' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/attributes/request/$request_id'
 *      Data:  '{"clientKeys":"attribute1,attribute2", "sharedKeys":"shared1,shared2"}'
 *
 * @param payload        payload
 * @param on_attrrequest_response     Attributes response callback
 * @param on_attrrequest_timeout  Attributes response timeout callback
 * @param qos            qos of publish message
 * @param retain         ratain flag
 *
 * @return request_id of the subscribe message on success
 *        -1 if error
 */
int tbmc_attributes_request(tbmc_handle_t client_, const char *payload,
                            void *context,
                            tbmc_on_attrrequest_response_t on_attrrequest_response,
                            tbmc_on_attrrequest_timeout_t on_attrrequest_timeout,
                            int qos /*= 1*/, int retain /*= 0*/) // requestAttributes() //request client and shared attributes
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return -1;
     }

     if (!client->mqtt_handle) {
          TBMCLOG_E("mqtt client is NULL");
          return -1;
     }
     if (!payload) {
          TBMCLOG_W("There are no payload to request");
          return -1;
     }

     int request_id = _request_list_create_and_append(client, TBMC_REQUEST_ATTR, 0 /*request_id*/, context,
                                           on_attrrequest_response, on_attrrequest_timeout);
     if (request_id <= 0) {
          TBMCLOG_E("Unable to take semaphore");
          return -1;
     }

     int size = strlen(TB_MQTT_TOPIC_ATTRIBUTES_REQUEST_PREFIX) + 20;
     char *topic = TBMC_MALLOC(size);
     if (!topic) {
          TBMCLOG_E("Unable to malloc memory");
          return -1;
     }
     memset(topic, 0x00, size);
     snprintf(topic, size - 1, TB_MQTT_TOPIC_ATTRIBUTES_REQUEST_PATTERN, request_id);

#ifdef TBMCLOG_LONG
     TBMCLOG_D("[Attributes Request][Tx] RequestID=%d, QoS=%d, Retain=%d, Timestamp=%u, Length=%d\r\n\t\t%.*s",
              request_id,
              qos, retain, (uint32_t)time(NULL), strlen(payload),
              strlen(payload), payload);
#else
     TBMCLOG_D("[Attributes Request][Tx] RequestID=%d, %.*s",
              request_id,
              strlen(payload), payload);
#endif

     int message_id = _tbmc_publish(client, topic, payload, qos, retain);
     TBMC_FREE(topic);
     return request_id; /*return message_id;*/
}

/**
 * @brief Client to send a 'Attributes Request' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/attributes/request/$request_id'
 *      Data:  '{"clientKeys":"attribute1,attribute2", "sharedKeys":"shared1,shared2"}'
 *
 * @param client_keys   client attribute names. A ending char is '\0'. example: "attribute1,attribute2" (字符串要自带双引号,逗号分隔!!)
 * @param shared_keys   shared attribute names. A ending char is '\0'. example: "shared1,shared2"       (字符串要自带双引号,逗号分隔!!)
 * @param on_attrrequest_response     Attributes response callback
 * @param on_attrrequest_timeout  Attributes response timeout callback
 * @param qos           qos of publish message
 * @param retain        ratain flag
 *
 * @return request_id of the subscribe message on success
 *        -1 if error
 */
int tbmc_attributes_request_ex(tbmc_handle_t client_, const char *client_keys, const char *shared_keys,
                               void *context,
                               tbmc_on_attrrequest_response_t on_attrrequest_response,
                               tbmc_on_attrrequest_timeout_t on_attrrequest_timeout,
                               int qos /*= 1*/, int retain /*= 0*/)
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return -1;
     }

     if ((client_keys == NULL || strlen(client_keys) <= 0) && (shared_keys == NULL || strlen(shared_keys) <= 0)) {
          TBMCLOG_W("There are no keys to request");
          return -1;
     }

     int size = strlen(TB_MQTT_TEXT_ATTRIBUTES_REQUEST_CLIENTKEYS) + strlen(client_keys) 
               + strlen(TB_MQTT_TEXT_ATTRIBUTES_REQUEST_SHAREDKEYS) + strlen(shared_keys) + 20;
     char *payload = TBMC_MALLOC(size);
     if (!payload)
     {
          TBMCLOG_E("Unable to malloc memory");
          return -1;
     }
     memset(payload, 0x00, size);
     snprintf(payload, size - 1, "{\"clientKeys\":\"%s\", \"sharedKeys\":\"%s\"}", client_keys, shared_keys);
     int retult = tbmc_attributes_request(client, payload,
                                          context,
                                          on_attrrequest_response,
                                          on_attrrequest_timeout,
                                          qos, retain);
     TBMC_FREE(payload);
     return retult;
}

/**
 * @brief Client to send a 'Server-Side RPC Response' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/rpc/response/$request_id'
 *      Data:  '{"example_response":23.1}' ???
 *
 * @param request_id  Server-Side RPC Request ID
 * @param response    payload string (set to NULL, sending empty payload message). A ending char is '\0'.
 * @param qos         qos of publish message
 * @param retain      ratain flag
 *
 * @return message_id of the subscribe message on success
 *         0 if cannot publish
 *        -1 if error
 */
int tbmc_serverrpc_response(tbmc_handle_t client_, int request_id, const char *response,
                                   int qos /*= 1*/, int retain /*= 0*/) // sendServerRpcReply() //response server-side RPC
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return -1;
     }
     
     if (!client->mqtt_handle) {
          TBMCLOG_E("MQTT client is NULL!");
          return -1;
     }

     int size = strlen(TB_MQTT_TOPIC_SERVERRPC_RESPONSE_PREFIX) + 20;
     char *topic = TBMC_MALLOC(size);
     if (!topic) {
          TBMCLOG_E("Unable to malloc memory!");
          return -1;
     }
     memset(topic, 0x00, size);
     snprintf(topic, size - 1, TB_MQTT_TOPIC_SERVERRPC_RESPONSE_PATTERN, request_id);

#ifdef TBMCLOG_LONG
     TBMCLOG_D("[Server-Side RPC][Tx] RequestID=%d, QoS=%d, Retain=%d, Timestamp=%u, Length=%d\r\n\t\t%.*s",
              request_id,
              qos, retain, (uint32_t)time(NULL), strlen(response),
              strlen(response), response);
#else
     TBMCLOG_D("[Server-Side RPC][Tx] RequestID=%d %.*s",
              request_id,
              strlen(response), response);
#endif

     int message_id = _tbmc_publish(client, topic, response, qos, retain);
     TBMC_FREE(topic);
     return message_id;
}

/**
 * @brief Client to send a 'Client-Side RPC Request' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/rpc/request/$request_id'
 *      Data:  '{"method":"getTime","params":{}}'
 *
 * @param method        RPC method name.   A ending char is '\0'. example:　"getTime"   (字符串要自带双引号!!)
 * @param params        RPC method params. A ending char is '\0'. example:　{}          (字符串是 json 格式!!)
 * @param on_clientrpc_response      Client-RPC response callback
 * @param on_clientrpc_timeout  Client-RPC response timeout callback
 * @param qos           qos of publish message
 * @param retain        ratain flag
 *
 * @return rpc_request_id of the subscribe message on success
 *        -1 if error
 */
int tbmc_clientrpc_request(tbmc_handle_t client_, const char *payload,
                               void *context,
                               tbmc_on_clientrpc_response_t on_clientrpc_response,
                               tbmc_on_clientrpc_timeout_t on_clientrpc_timeout,
                               int qos /*= 1*/, int retain /*= 0*/) // sendClientRpcCall() //request client-side RPC
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return -1;
     }

     if (!client->mqtt_handle) {
          TBMCLOG_E("mqtt client is NULL");
          return -1;
     }
     if (!payload) {
          TBMCLOG_W("There are no payload to request");
          return -1;
     }

     int request_id = _request_list_create_and_append(client, TBMC_REQUEST_CLIENTRPC, 0/*request_id*/, context,
                                           on_clientrpc_response, on_clientrpc_timeout);
     if (request_id <= 0) {
          TBMCLOG_E("Unable to take semaphore");
          return -1;
     }

     int size = strlen(TB_MQTT_TOPIC_CLIENTRPC_REQUEST_PREFIX) + 20;
     char *topic = TBMC_MALLOC(size);
     if (!topic) {
          TBMCLOG_E("Unable to malloc memory");
          return -1;
     }
     memset(topic, 0x00, size);
     snprintf(topic, size - 1, TB_MQTT_TOPIC_CLIENTRPC_REQUEST_PATTERN, request_id);

#ifdef TBMCLOG_LONG
     TBMCLOG_D("[Client-Side RPC][Tx] RequestID=%d, QoS=%d, Retain=%d, Timestamp=%u, Length=%d\r\n\t\t%.*s",
              request_id,
              qos, retain, (uint32_t)time(NULL), strlen(payload),
              strlen(payload), payload);
#else
     TBMCLOG_D("[Client-Side RPC][Tx] RequestID=%d %.*s",
              request_id,
              strlen(payload), payload);
#endif

     int message_id = _tbmc_publish(client, topic, payload, qos, retain);
     TBMC_FREE(topic);
     return request_id; /*return message_id;*/
}

/**
 * @brief Client to send a 'Client-Side RPC Request' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/rpc/request/$request_id'
 *      Data:  '{"method":"getTime","params":{}}'
 *
 * @param method        RPC method name.   A ending char is '\0'. example:　"getTime"   (字符串要自带双引号!!)
 * @param params        RPC method params. A ending char is '\0'. example:　{}          (字符串是 json 格式!!)
 * @param on_clientrpc_response      Client-RPC response callback
 * @param on_clientrpc_timeout  Client-RPC response timeout callback
 * @param qos           qos of publish message
 * @param retain        ratain flag
 *
 * @return rpc_request_id of the subscribe message on success
 *        -1 if error
 */
int tbmc_clientrpc_request_ex(tbmc_handle_t client_, const char *method, const char *params,
                                     void *context,
                                     tbmc_on_clientrpc_response_t on_clientrpc_response,
                                     tbmc_on_clientrpc_timeout_t on_clientrpc_timeout,
                                     int qos /*= 1*/, int retain /*= 0*/)
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return -1;
     }

     int size = strlen(TB_MQTT_TEXT_RPC_METHOD) + strlen(method) + strlen(TB_MQTT_TEXT_RPC_PARAMS) + strlen(params) + 20;
     char *payload = TBMC_MALLOC(size);
     if (!payload) {
          TBMCLOG_E("Unable to malloc memory");
          return -1;
     }
     memset(payload, 0x00, size);
     snprintf(payload, size - 1, "{\"method\":\"%s\",\"params\":{%s}}", method, params);
     int retult = tbmc_clientrpc_request(client, payload,
                                         context,
                                         on_clientrpc_response,
                                         on_clientrpc_timeout,
                                         qos, retain);
     TBMC_FREE(payload);
     return retult;
}

/**
 * @brief Client to send a 'Client-Side RPC Request' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/rpc/request/$request_id'
 *      Data:  '{"method":"getTime","params":{}}'
 *
 * @param request_id_   0 on first f/w request(chunk is 0), otherwise if it is result of last tbmc_fwupdate_request()
 * @param chunk                    
 * @param on_fwupdate_response    f/w update response callback
 * @param on_fwupdate_timeout     f/w update response timeout callback
 * @param qos            qos of publish message
 * @param retain         ratain flag
 *
 * @return rpc_request_id of the subscribe message on success
 *        -1 if error
 */
int tbmc_fwupdate_request(tbmc_handle_t client_, int request_id_, int chunk, const char *payload, //?payload
                          void *context,
                          tbmc_on_fwupdate_response_t on_fwupdate_response,
                          tbmc_on_fwupdate_timeout_t on_fwupdate_timeout,
                          int qos /*= 1*/, int retain /*= 0*/)
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return -1;
     }

     if (!client->mqtt_handle) {
          TBMCLOG_E("mqtt client is NULL");
          return -1;
     }
     /*if (!payload) {
          TBMCLOG_W("There are no payload to request");
          return -1;
     }*/

     int request_id = _request_list_create_and_append(client, TBMC_REQUEST_FWUPDATE, request_id_, context,
                                           on_fwupdate_response, on_fwupdate_timeout);
     if (request_id <= 0) {
          TBMCLOG_E("Unable to take semaphore");
          return -1;
     }

     int size = strlen(TB_MQTT_TOPIC_FW_RESPONSE_PATTERN) + 20;
     char *topic = TBMC_MALLOC(size);
     if (!topic) {
          TBMCLOG_E("Unable to malloc memory");
          return -1;
     }
     memset(topic, 0x00, size);
     snprintf(topic, size - 1, TB_MQTT_TOPIC_FW_RESPONSE_PATTERN, request_id, chunk);

#ifdef TBMCLOG_LONG
     TBMCLOG_D("[FW update][Tx] RequestID=%d, QoS=%d, Retain=%d, Timestamp=%u, Length=%d\r\n\t\t%.*s",
              request_id,
              qos, retain, (uint32_t)time(NULL), strlen(payload),
              strlen(payload), payload);
#else
     TBMCLOG_D("[FW update][Tx] RequestID=%d %.*s",
              request_id,
              strlen(payload), payload);
#endif

     int message_id = _tbmc_publish(client, topic, payload, qos, retain);
     TBMC_FREE(topic);
     return request_id; /*return message_id;*/
}

/**
 * @brief Subscribe the client to defined topic with defined qos
 *
 * Notes:
 * - Client must be connected to send subscribe message
 * - This API is could be executed from a user task or
 * from a mqtt event callback i.e. internal mqtt task
 * (API is protected by internal mutex, so it might block
 * if a longer data receive operation is in progress.
 *
 * @param client    mqtt client handle
 * @param topic
 * @param qos
 *
 * @return message_id of the subscribe message on success
 *         -1 on failure
 */
static int _tbmc_subscribe(tbmc_handle_t client_, const char *topic, int qos /*=0*/) // subscribe()
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return -1;
     }

     if (!client->mqtt_handle || !topic)
     {
          return -1;
     }

     return esp_mqtt_client_subscribe(client->mqtt_handle, topic, qos);
}

/**
 * @brief Client to send a publish message to the broker
 *
 * Notes:
 * - Client doesn't have to be connected to send publish message
 *   (although it would drop all qos=0 messages, qos>1 messages would be enqueued)
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 *
 * @param topic     topic string
 * @param payload   payload string (set to NULL, sending empty payload message)
 * @param qos       qos of publish message
 * @param retain    ratain flag
 *
 * @return message_id of the subscribe message on success
 *         0 if cannot publish
 *        -1 if error
 */
static int _tbmc_publish(tbmc_handle_t client_, const char *topic, const char *payload, int qos /*= 1*/, int retain /*= 0*/) // publish()
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return -1;
     }

     if (!client->mqtt_handle || !topic)
     {
          return -1;
     }

     int len = (payload == NULL) ? 0 : strlen(payload); //// +1
     return esp_mqtt_client_publish(client->mqtt_handle, topic, payload, len, qos, retain);
}

// The callback for when a MQTT event is received.
static esp_err_t _on_MqttEventCallback(/*tbmc_handle_t client_,*/ esp_mqtt_event_handle_t event) //_onMqttEventCallback();
{
     if (!event) {
          return -1;
     }

     tbmc_handle_t *client = (tbmc_handle_t *)event->user_context;
     if (!client) {
          return -1;
     }

     return _onMqttEventHandle(client, event);
}
static esp_err_t _on_MqttEventHandle(tbmc_handle_t client_, esp_mqtt_event_handle_t event) //_onMqttEventHandle();  //MQTT_EVENT_...
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return -1;
     }
     if (!event) {
          TBMCLOG_E("event is NULL!");
          return -1;
     }

     int msg_id;
     switch (event->event_id) {
     case MQTT_EVENT_BEFORE_CONNECT:
          TBMCLOG_D("MQTT_EVENT_BEFORE_CONNECT, msg_id=%d, topic_len=%d, data_len=%d",
                  event->msg_id, event->topic_len, event->data_len);
          break;

     case MQTT_EVENT_CONNECTED:
          TBMCLOG_I("MQTT_EVENT_CONNECTED");
          msg_id = _tbmc_subscribe(client, TB_MQTT_TOPIC_SHARED_ATTRIBUTES, 0);
          TBMCLOG_V("sent subscribe successful, msg_id=%d, topic=%s", msg_id, TB_MQTT_TOPIC_SHARED_ATTRIBUTES);
          msg_id = _tbmc_subscribe(client, TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_SUBSCRIRBE, 0);
          TBMCLOG_V("sent subscribe successful, msg_id=%d, topic=%s", msg_id, TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_SUBSCRIRBE);
          msg_id = _tbmc_subscribe(client, TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE, 0);
          TBMCLOG_V("sent subscribe successful, msg_id=%d, topic=%s", msg_id, TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE);
          msg_id = _tbmc_subscribe(client, TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE, 0);
          TBMCLOG_V("sent subscribe successful, msg_id=%d, topic=%s", msg_id, TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE);
          msg_id = _tbmc_subscribe(client, TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE, 0);
          TBMCLOG_V("sent subscribe successful, msg_id=%d, topic=%s", msg_id, TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE);
          client->state = TBMC_STATE_CONNECTED;
          ////if(xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) == pdTRUE) {
          if (client->on_connected) {
               client->on_connected(client->context);
          }
          ////xSemaphoreGive(client->lock);
          ////}
          break;

     case MQTT_EVENT_DISCONNECTED:
          TBMCLOG_I("MQTT_EVENT_DISCONNECTED");
          client->state = TBMC_STATE_DISCONNECTED;
          ////if(xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) == pdTRUE) {
          if (client->on_disconnected) {
               client->on_disconnected(client->context);
          }
          ////xSemaphoreGive(client->lock);
          ////}
          break;

     case MQTT_EVENT_SUBSCRIBED:
          TBMCLOG_V("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
          break;
     case MQTT_EVENT_UNSUBSCRIBED:
          TBMCLOG_V("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
          break;
     case MQTT_EVENT_PUBLISHED:
          TBMCLOG_V("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
          break;

     case MQTT_EVENT_DATA:
          TBMCLOG_V("MQTT_EVENT_DATA");
          ////TBMCLOG_D("TOPIC=%.*s", event->topic_len, event->topic);
          ////TBMCLOG_D("DATA=%.*s", event->data_len, event->data);
          client->_on_DataEventProcess(client, event);
          break;

     case MQTT_EVENT_ERROR:
          TBMCLOG_W("MQTT_EVENT_ERROR");
          break;
     default:
          TBMCLOG_W("Other event id:%d", event->event_id);
          break;
     }
     return ESP_OK;
}
// Processes MQTT_EVENT_DATA message
static void _on_DataEventProcess(tbmc_handle_t client_, esp_mqtt_event_handle_t event) //_onDataEventProcess(); //MQTT_EVENT_DATA
{
     const char *topic = event->topic;
     const char *payload = event->data;
     const int topic_len = event->topic_len;
     const int payload_len = event->data_len;

     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return;
     }

     if (strncmp(topic, TB_MQTT_TOPIC_SHARED_ATTRIBUTES, strlen(TB_MQTT_TOPIC_SHARED_ATTRIBUTES)) == 0) {
          // 1.TB_MQTT_TOPIC_SHARED_ATTRIBUTES

#ifdef TBMCLOG_LONG
          TBMCLOG_D("[Subscribe Shared Attributes][Rx] Length=%d, Timestamp=%u\r\n\t\t%.*s",
                   payload_len, (uint32_t)time(NULL),
                   payload_len, payload);
#else
          TBMCLOG_D("[Subscribe Shared Attributes][Rx] %.*s",
                   payload_len, payload);
#endif

          if (client->on_sharedattr_received) {
               client->on_sharedattr_received(client->context, payload, payload_len);
          } else {
               TBMCLOG_W("Unable to find shared-attributes, (%.*s, %.*s)",
                       topic_len, topic, payload_len, payload);
          }

     } else if (strncmp(topic, TB_MQTT_TOPIC_SERVERRPC_REQUEST_PREFIX, strlen(TB_MQTT_TOPIC_SERVERRPC_REQUEST_PREFIX)) == 0) {
          // 2.TB_MQTT_TOPIC_SERVERRPC_REQUEST_PREFIX

          int request_id = atoi(topic + strlen(TB_MQTT_TOPIC_SERVERRPC_REQUEST_PREFIX));
#ifdef TBMCLOG_LONG
          TBMCLOG_D("[Server-Side RPC][Rx] RequestID=%d, Timestamp=%u, Length=%d\r\n\t\t%.*s",
                   request_id,
                   (uint32_t)time(NULL), payload_len,
                   payload_len, payload);
#else
          TBMCLOG_D("[Server-Side RPC][Rx] RequestID=%d %.*s",
                   request_id,
                   payload_len, payload);
#endif

          if (client->on_serverrpc_request) {
               client->on_serverrpc_request(client->context, request_id, payload, payload_len);
          } else {
               TBMCLOG_W("Unable to find server-rpc request(%d), (%.*s, %.*s)", request_id,
                       topic_len, topic, payload_len, payload);
          }

     } else if (strncmp(topic, TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PREFIX, strlen(TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PREFIX)) == 0) {
          // 3.TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PREFIX

          int request_id = atoi(topic + strlen(TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PREFIX));
#ifdef TBMCLOG_LONG
          TBMCLOG_D("[Attributes Request][Rx] RequestID=%d, Length=%d, Timestamp=%u\r\n\t\t%.*s",
                   request_id,
                   payload_len, (uint32_t)time(NULL),
                   payload_len, payload);
#else
          TBMCLOG_D("[Attributes Request][Rx] RequestID=%d %.*s",
                   request_id,
                   payload_len, payload);
#endif

          tbmc_request_t *tbmc_request = _request_list_search_and_remove(client, request_id);
          if (tbmc_request) {
               if (tbmc_request->on_response) {
                    tbmc_request->on_response(client->context, request_id, payload, payload_len);
               }
               _request_destroy(tbmc_request);
               tbmc_request = NULL;
          } else {
               TBMCLOG_E("Unable to find attributes requset(%d), (%.*s, %.*s)", request_id,
                       topic_len, topic, payload_len, payload);
               return; // -1;
          }

     } else if (strncmp(topic, TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PREFIX, strlen(TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PREFIX)) == 0) {
          // 4.TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PREFIX

          int request_id = atoi(topic + strlen(TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PREFIX));
#ifdef TBMCLOG_LONG
          TBMCLOG_D("[Client-Side RPC][Rx] RequestID=%d, Timestamp=%u, Length=%d\r\n\t\t%.*s",
                   request_id,
                   (uint32_t)time(NULL), payload_len,
                   payload_len, payload);
#else
          TBMCLOG_D("[Client-Side RPC][Rx] RequestID=%d %.*s",
                   request_id,
                   payload_len, payload);
#endif

          tbmc_request_t *tbmc_request = _request_list_search_and_remove(client, request_id);
          if (tbmc_request) {
               if (tbmc_request->on_response) {
                    tbmc_request->on_response(client->context, request_id, payload, payload_len);
               }
               _request_destroy(tbmc_request);
               tbmc_request = NULL;
          } else {
               TBMCLOG_E("Unable to find client-RPC requset(%d), (%.*s, %.*s)", request_id,
                       topic_len, topic, payload_len, payload);
               return; // -1;
          }

     } else if (strncmp(topic, TB_MQTT_TOPIC_FW_RESPONSE_PREFIX, strlen(TB_MQTT_TOPIC_FW_RESPONSE_PREFIX)) == 0) {
          // 5.TB_MQTT_TOPIC_FW_RESPONSE_PREFIX
          int request_id = 0;
          int chunk = 0;
          sscanf(topic, TB_MQTT_TOPIC_FW_RESPONSE_PATTERN, &request_id, &chunk);

#ifdef TBMCLOG_LONG
          TBMCLOG_D("[FW update][Rx] RequestID=%d, Timestamp=%u, Length=%d\r\n\t\t%.*s",
                    request_id,
                    (uint32_t)time(NULL), payload_len,
                    payload_len, payload);
#else
          TBMCLOG_D("[FW update][Rx] RequestID=%d %.*s",
                    request_id,
                    payload_len, payload);
#endif

          tbmc_request_t *tbmc_request = _request_list_search_and_remove(client, request_id);
          if (tbmc_request) {
               tbmc_on_fwupdate_response_t on_fwupdate_response = (tbmc_on_fwupdate_response_t)tbmc_request->on_response;
               if (on_fwupdate_response) {
                    on_fwupdate_response(client->context, request_id, chunk, payload, payload_len);
               }
               _request_destroy(tbmc_request);
               tbmc_request = NULL;
          } else {
               TBMCLOG_E("Unable to find FW update requset(%d), (%.*s, %.*s)",
                         request_id,
                         topic_len, topic, payload_len, payload);
               return; // -1;
          }

     }  else {
          // Payload is too long, then Serial.*/
#ifdef TBMCLOG_LONG
          TBMCLOG_W("[Unkown-Msg][Rx] Timestamp=%d, TopicLength=%d, PayloadLength=%d\r\n\t\t%.*s\r\n\t\t%.*s",
                    (uint32_t)time(NULL), topic_len, payload_len,
                    topic_len, topic, payload_len, payload);
#else
          TBMCLOG_W("[Unkown-Msg][Rx] topic=%.*s, payload=%.*s",
                    topic_len, topic, payload_len, payload);
#endif
     }
}

static bool _request_is_equal(const tbmc_request_t *a, const tbmc_request_t *b)
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
     } else { // if (a->ts != b->ts)
          return a->ts == b->ts;
     }
}

//return request_id on successful, otherwise return -1
static int _request_list_create_and_append(tbmc_handle_t client_, tbmc_request_type_t type, int request_id,
                                void *context,
                                tbmc_on_response_t on_response,
                                tbmc_on_timeout_t on_timeout)
{
     tbmc_t *client = (tbmc_t*)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return -1;
     }

     // Take semaphore
     if (xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
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

          request_id = this->next_request_id;
     }

     // Create request
     tbmc_request_t *tbmc_request = _request_create(type, request_id, context, on_response, on_timeout);
     if (!tbmc_request) {
          // Give semaphore
          xSemaphoreGive(client->lock);
          TBMCLOG_E("Unable to create request: No memory!");
          return -1;
     }

     // Insert request to list
     tbmc_request_t *it, last = NULL;
     if (LIST_FIRST(&client->request_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->request_list, tbmc_request, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->request_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, tbmc_request, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->lock);
     return request_id;
}

static tbmc_request_t *_request_list_search_and_remove(tbmc_handle_t client_, int request_id)
{
     tbmc_t *client = (tbmc_t *)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return NULL;
     }

     // Take semaphore
     if (xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return NULL;
     }

     // Search item
     tbmc_request_t *tbmc_request = NULL;
     LIST_FOREACH(tbmc_request, &client->request_list, entry) {
          if (tbmc_request && tbmc_request->request_id == request_id) {
               break;
          }
     }

     /// Remove form list
     if (tbmc_request) {
          LIST_REMOVE(tbmc_request, entry);
     } else {
          TBMCLOG_W("Unable to remove request:%d!", request_id);
     }

     // Give semaphore
     xSemaphoreGive(client->lock);
     return tbmc_request;
}

// return  count of timeout_request_list
static int _request_list_move_all_of_timeout(tbmc_handle_t client_, uint32_t timestamp,
                              LIST_HEAD(tbmc_request_list, tbmc_request) *timeout_request_list)
{
     tbmc_t *client = (tbmc_t *)client_;
     if (!client) {
          TBMCLOG_E("client is NULL!");
          return 0;
     }
     if (!timeout_request_list) {
          TBMCLOG_E("timeout_request_list is NULL!");
          return 0;
     }

     // Take semaphore
     if (xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBMCLOG_E("Unable to take semaphore!");
          return 0;
     }

     // Search & move item
     int count = 0;
     tbmc_request_t *tbmc_request = NULL, *next;
     LIST_FOREACH_SAFE(tbmc_request, &client->request_list, entry, next) {
          if (tbmc_request && tbmc_request->ts + TB_MQTT_TIMEOUT <= timestamp) {
               // remove from request list
               LIST_REMOVE(tbmc_request, entry);

               // append to timeout list
               tbmc_request_t *it, last = NULL;
               if (LIST_FIRST(timeout_request_list) == NULL) {
                    LIST_INSERT_HEAD(timeout_request_list, tbmc_request, entry);
                    count++;
               } else {
                    LIST_FOREACH(it, timeout_request_list, entry) {
                         last = it;
                    }
                    if (it == NULL) {
                         assert(last);
                         LIST_INSERT_AFTER(last, tbmc_request, entry);
                         count++;
                    }
               }
          }
     }

     // Give semaphore
     xSemaphoreGive(client->lock);
     return count;
}

//return tbmc_request on successful, otherwise return NULL.
static tbmc_request_t *_request_create(tbmc_request_type_t type,
                                       uint32_t request_id,
                                       void *context,
                                       tbmc_on_response_t on_response,
                                       tbmc_on_timeout_t on_timeout)
{
     tbmc_request_t *tbmc_request = TBMC_MALLOC(sizeof(tbmc_request_t));
     if (!tbmc_request) {
          TBMCLOG_E("Unable to malloc memory!");
          return NULL;
     }

     memset(tbmc_request, 0x00, sizeof(tbmc_request_t));
     tbmc_request->type = type;
     tbmc_request->request_id = request_id;
     tbmc_request->ts = (uint32_t)time(NULL);
     tbmc_request->context = context;
     tbmc_request->on_response = on_response;
     tbmc_request->on_timeout = on_timeout;

     return tbmc_request;
}

static void _request_destroy(tbmc_request_t *tbmc_request)
{
     if (!tbmc_request) {
          TBMCLOG_E("Invalid argument!");
          return; 
     }

     tbmc_request->type = 0;
     tbmc_request->request_id = 0;
     tbmc_request->ts = 0;
     tbmc_request->context = NULL;
     tbmc_request->on_response = NULL;
     tbmc_request->on_timeout = NULL;
     TBMC_FREE(tbmc_request);
}
