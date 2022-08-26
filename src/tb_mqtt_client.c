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

#include "tb_mqtt_client.h"

typedef enum 
{
     TBMC_REQUEST_ATTR = 1, 
     TBMC_REQUEST_CLIENTRPC,
     TBMC_REQUEST_FWUPDATE
}tbmc_request_type_t;

typedef struct tbmc_request
{
     tbmc_request_type_t type;
     uint32_t request_id;

     uint32_t ts; /*!< time stamp at sending request */
     void *context;
     tbmc_on_request_success_t on_success;
     tbmc_on_request_timeout_t on_timeout;

     LIST_ENTRY(tbmc_request) entry;
} tbmc_request_t;

/**
 * ThingsBoard MQTT Client
 */
typedef struct tbmc_client_
{
     esp_mqtt_client_handle_t mqtt_handle;
     SemaphoreHandle_t lock;

     tbmc_state_t state;  // TBMQTT_STATE state;
     int next_request_id; 
     LIST_HEAD(tbmc_request_list, tbmc_request) request_list;   /*!< request list: attributes request, client side RPC & fw update request */ ////QueueHandle_t timeoutQueue;

     //char *uri;          /*!< ThingsBoard MQTT host uri */
     //char *access_token; /*!< ThingsBoard MQTT token */
     tbmc_config_t config; //esp_mqtt_client_config_t config;

     void *context;
     tbmc_on_connected_t on_connected;                     /*!< Callback of connected ThingsBoard MQTT */
     tbmc_on_disconnected_t on_disconnected;               /*!< Callback of disconnected ThingsBoard MQTT */
     tbmc_on_sharedattr_received_t on_sharedattr_received; /*!< Callback of receiving ThingsBoard MQTT shared-attribute T*/
     tbmc_on_serverrpc_request_t on_serverrpc_request;     /*!< Callback of receiving ThingsBoard MQTT server-RPC request */

} tbmc_t;

static bool _request_is_equal(const tbmc_request_t *a, const tbmc_request_t*b)
static void _request_list_lock(tbmc_handle_t client);
static void _request_list_unlock(tbmc_handle_t client);

static tbmc_err_t _tbmc_subscribe(tbmc_handle_t client, const char* topic, const char* postfix, int qos=0); //subscribe()
static tbmc_err_t _tbmc_publish(tbmc_handle_t client, const char *topic, const char *payload, int qos, int retain); //publish()

static void        _on_DataEventProcess(tbmc_handle_t client, esp_mqtt_event_handle_t event); //_onDataEventProcess(); //MQTT_EVENT_DATA
static tbmc_err_t  _on_MqttEventHandle(tbmc_handle_t client, esp_mqtt_event_handle_t event); //_onMqttEventHandle();  //MQTT_EVENT_...
static tbmc_err_t  _on_MqttEventCallback(tbmc_handle_t client, sp_mqtt_event_handle_t event); //_onMqttEventCallback();

tbmc_handle_t tbmc_init(void)
{
     // tbmc.request_list = LIST_HEAD_INITIALIZER(tbmc.request_list);
     // TODO: ...
}

tbmc_err_t tbmc_destroy(tbmc_handle_t client)
{
     // TODO: ...
} 

bool tbmc_connect(tbmc_handle_t client,
                  tbmc_config_t *config,
                  void *context,
                  tbmc_on_connected_t on_connected,
                  tbmc_on_disconnected_t on_disonnected,
                  tbmc_on_sharedattr_received_t on_sharedattributes_received,
                  // tbmc_on_attrrequest_success_t on_attributesrequest_success,
                  // tbmc_on_attrrequest_timeout_t on_attributesrequest_timeout,
                  // tbmc_on_clientrpc_success_t on_clientrpc_success,
                  // tbmc_on_clientrpc_timeout_t on_clientrpc_timeout,
                  tbmc_on_serverrpc_request_t on_serverrpc_request,
                  // tbmc_on_fwrequest_response_t on_fwrequest_response,
                  /* tbmc_on_fwrequest_timeout_t on_fwrequest_timeout */)// connect()//...start()
                                                                           // tbmqttclient_set_OnConnected(TBMQTT_ON_CONNECTED callback); //merge to tbmqttclient_connect()
                                                                           // tbmqttclient_set_OnDisconnected(TBMQTT_ON_DISCONNECTED callback); //merge to tbmqttclient_connect()
                                                                           // tbmqttclient_set_OnServerRpcRequest(TBMQTT_SERVER_RPC_CALLBACK callback); //merge to tbmqttclient_connect()
                                                                           // tbmqttclient_set_OnAttrSubReply(TBMQTT_ATTR_SUB_CALLBACK callback); //merge to tbmqttclient_connect()
                                                                           // tbmqttclient_get_OnConnected(void); //merge to tbmqttclient_connect()
                                                                           // tbmqttclient_get_OnDisconnected(void); //merge to tbmqttclient_connect()
                                                                           // tbmqttclient_get_OnServerRpcRequest(void); //merge to tbmqttclient_connect()
                                                                           // tbmqttclient_get_OnAttrSubReply(void); //merge to tbmqttclient_connect()

{
     // TODO: ...
} 

void tbmc_disconnect(tbmc_handle_t client) // disconnect()//...stop()
{
     // TODO: ...
} 

bool tbmc_is_connected(tbmc_handle_t client) //isConnected
{
     // TODO: ...
} 

bool tbmc_is_connecting(tbmc_handle_t client)
{
     // TODO: ...
} 

bool tbmc_is_disconnected(tbmc_handle_t client)
{
     // TODO: ...
} 

tbmc_state_t tbmc_get_state(tbmc_handle_t client)
{
     // TODO: ...
} 

void tbmc_check_timeout(tbmc_handle_t client)  // Executes an event loop for PubSub client. //loop()==>checkTimeout()
{
     // TODO: ...
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
tbmc_err_t tbmc_telemetry_publish(tbmc_handle_t client, const char *telemetry,
                                  int qos /*= 1*/, int retain /*= 0*/) // sendTelemetry()
{
     /*TBLOG_XD(String("[Telemetry] Tx Telemetry:")
          + "Length:" + strlen(telemetry)
          + ", QoS:" + qos
          + ", Retain:" + retain,
          telemetry, strlen(telemetry));*/
#ifdef TBLOG_LONG
     TBLOG_DD("[Telemetry][Tx] QoS=%d, Retain=%d, Timestamp=%d, Length=%d\r\n\t\t%.*s",
              qos, retain, millis(), strlen(telemetry),
              strlen(telemetry), telemetry);
#else
     TBLOG_DD("[Telemetry][Tx] %.*s",
              strlen(telemetry), telemetry);
#endif

     int32_t message_id = _tbmc_publish(client, TB_MQTT_TOPIC_TELEMETRY_PUBLISH, telemetry, qos, retain);
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
tbmc_err_t tbmc_clientattributes_publish(tbmc_handle_t client, const char *attributes,
                                         int qos /*= 1*/, int retain /*= 0*/) // sendAttributes() //publish client attributes
{
     /*TBLOG_XD(String("[Client-Side Attributes] Tx Attributes:")
          + " Length:" + strlen(attributes)
          + ", QoS:" + qos
          + ", Retain:" + retain,
          attributes, strlen(attributes));*/
#ifdef TBLOG_LONG
     TBLOG_DD("[Client-Side Attributes][Tx] QoS=%d, Retain=%d, Timestamp=%d, Length=%d\r\n\t\t%.*s",
              qos, retain, millis(), strlen(attributes),
              strlen(attributes), attributes);
#else
     TBLOG_DD("[Client-Side Attributes][Tx] %.*s",
              strlen(attributes), attributes);
#endif

     int32_t message_id = _tbmc_publish(client, TB_MQTT_TOPIC_CLIENT_ATTRIBUTES_PUBLISH, attributes, qos, retain);
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
 * @param on_attributesrequest_success     Attributes response callback
 * @param on_attributesrequest_timeout  Attributes response timeout callback
 * @param qos            qos of publish message
 * @param retain         ratain flag
 *
 * @return request_id of the subscribe message on success
 *        -1 if error
 */
tbmc_err_t tbmc_attributes_request(tbmc_handle_t client, const char *payload,
                            void *context,
                            tbmc_on_attrrequest_success_t on_attributesrequest_success,
                            tbmc_on_attrrequest_timeout_t on_attributesrequest_timeout,
                            int qos/*= 1*/, int retain/*= 0*/) // requestAttributes() //request client and shared attributes
{
     if (!client->mqtt_handle) {
          TBLOG_W("mqtt client is NULL");
          return -1;
     }
     if (!payload) {
          TBLOG_W("There are no payload to request");
          return -1;
     }

     int32_t request_id;
     if (xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) == pdTRUE) {
          client->next_request_id += 1;
          request_id = this->next_request_id;

          tbmc_err_t result = _request_list_append(client, TBMC_REQUEST_ATTR, request_id, context,
                                                   on_attributesrequest_success, on_attributesrequest_timeout);
          xSemaphoreGive(this->lock);

          if (result != TBMC_OK) {
               TBLOG_E("Unable to append to list in tbmc_attributes_request()");
               return -1;
          }
     } else {
          TBLOG_E("Unable to take semaphore in tbmc_attributes_request()");
          return -1;
     }

     int size = strlen(TB_MQTT_TOPIC_ATTRIBUTES_REQUEST_PREFIX) + 20;
     char* topic = TBMC_MALLOC(size);
     if (!topic) {
          return TBMC_ERR_NO_MEM;
     }
     memset(topic, 0x00, size);
     snprintf(topic, size-1, TB_MQTT_TOPIC_ATTRIBUTES_REQUEST_PATTERN, request_id);

     /*TBLOG_XD(String("[Request Attributes] Tx Request: ")
          + " Length:" + strlen(payload)
          + ", RequestID:" + request_id
          + ", Timestamp:" + millis() // + ", MsgID:" + message_id
          + ", QoS:" + qos
          + ", Retain:" + retain
          + ", onAttrReqMap.size=" + this->onAttrReqMap.size(),   // + ", timestamp=" + millis()
          payload, strlen(payload));*/
#ifdef TBLOG_LONG
     TBLOG_DD("[Request Attributes][Tx] RequestID=%d, QoS=%d, Retain=%d, Timestamp=%d, Length=%d, onAttrReqMap.size=%d\r\n\t\t%.*s",
              request_id,
              qos, retain, millis(), strlen(payload),
              this->onAttrReqMap.size(),
              strlen(payload), payload);
#else
     TBLOG_DD("[Request Attributes][Tx] RequestID=%d, onAttrReqMap.size:%d %.*s",
              request_id,
              this->onAttrReqMap.size(),
              strlen(payload), payload);
#endif

     int message_id = _tbmc_publish(client, topic, payload, qos, retain);
     TBMC_FREE(topic);
     return request_id;  /*return message_id;*/
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
 * @param on_attributesrequest_success     Attributes response callback
 * @param on_attributesrequest_timeout  Attributes response timeout callback
 * @param qos           qos of publish message
 * @param retain        ratain flag
 *
 * @return request_id of the subscribe message on success
 *        -1 if error
 */
tbmc_err_t tbmc_attributes_request_ex(tbmc_handle_t client, const char *client_keys, const char *shared_keys,
                                      void *context,
                                      tbmc_on_attrrequest_success_t on_attributesrequest_success,
                                      tbmc_on_attrrequest_timeout_t on_attributesrequest_timeout,
                                      int qos /*= 1*/, int retain /*= 0*/)
{
     if ((client_keys == NULL || strlen(client_keys) <= 0) && (shared_keys == NULL || strlen(shared_keys) <= 0)) {
          TBLOG_W("There are no keys to request");
          return -1;
     }

     int size = strlen(TB_MQTT_TEXT_ATTRIBUTES_REQUEST_CLIENTKEYS) + strlen(client_keys) 
          + strlen(TB_MQTT_TEXT_ATTRIBUTES_REQUEST_SHAREDKEYS) + strlen(shared_keys) + 20;
     char* payload = TBMC_MALLOC(size);
     if (!payload) {
          return TBMC_ERR_NO_MEM;
     }
     memset(payload, 0x00, size);
     snprintf(payload, size-1, "{\"clientKeys\":\"%s\", \"sharedKeys\":\"%s\"}", client_keys, shared_keys);
     tbmc_err_t retult = tbmc_attributes_request(client, payload,
                           context,
                           on_attributesrequest_success,
                           on_attributesrequest_timeout,
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
tbmc_err_t tbmc_serverrpc_response(tbmc_handle_t client, int request_id, const char *response,
                                   int qos /*= 1*/, int retain /*= 0*/) // sendServerRpcReply() //response server-side RPC
{
     if (!client->mqtt_handle)
     {
          TBLOG_W("mqtt client is NULL!");
          return -1;
     }

     int size = strlen(TB_MQTT_TOPIC_SERVERRPC_RESPONSE_PREFIX) + 20;
     char* topic = TBMC_MALLOC(size);
     if (!topic) {
          return TBMC_ERR_NO_MEM;
     }
     memset(topic, 0x00, size);
     snprintf(topic, size-1, TB_MQTT_TOPIC_SERVERRPC_RESPONSE_PATTERN, request_id);

     /*TBLOG_XD(String("[Server-Side  RPC] Tx Response: ")
          + " Length:" + strlen(response)
          + ", RequestID:" + request_id
          + ", Timestamp:" + millis() //+ ", MsgID:" + message_id
          + ", QoS:" + qos
          + ", Retain:" + retain,
          response, strlen(response));*/
#ifdef TBLOG_LONG
     TBLOG_DD("[Server-Side RPC][Tx] RequestID=%d, QoS=%d, Retain=%d, Timestamp=%d, Length=%d\r\n\t\t%.*s",
              request_id,
              qos, retain, millis(), strlen(response),
              strlen(response), response);
#else
     TBLOG_DD("[Server-Side  RPC][Tx] RequestID=%d %.*s",
              request_id,
              strlen(response), response);
#endif

     int message_id = _tbmc_publish(client, topic.c_str(), response, qos, retain);
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
 * @param on_clientrpc_success      Client-RPC response callback
 * @param on_clientrpc_timeout  Client-RPC response timeout callback
 * @param qos           qos of publish message
 * @param retain        ratain flag
 *
 * @return rpc_request_id of the subscribe message on success
 *        -1 if error
 */
tbmc_err_t tbmc_clientrpc_request(tbmc_handle_t client, const char *payload,
                           void *context,
                           tbmc_on_clientrpc_success_t on_clientrpc_success,
                           tbmc_on_clientrpc_timeout_t on_clientrpc_timeout,
                           int qos/*= 1*/, int retain/*= 0*/)                 // sendClientRpcCall() //request client-side RPC
{
     if (!client->mqtt_handle) {
          TBLOG_W("mqtt client is NULL");
          return -1;
     }
     if (!payload) {
          TBLOG_W("There are no payload to request");
          return -1;
     }

     int32_t request_id;
     if (xSemaphoreTake(client->lock, (TickType_t)0xFFFFF) == pdTRUE) {
          client->next_request_id += 1;
          request_id = this->next_request_id;

          tbmc_err_t result = _request_list_append(client, TBMC_REQUEST_ATTR, request_id, context,
                                                   on_clientrpc_success, on_clientrpc_timeout);
          xSemaphoreGive(this->lock);

          if (result != TBMC_OK) {
               TBLOG_E("Unable to append to list in tbmc_clientrpc_request()");
               return -1;
          }
     } else {
          TBLOG_E("Unable to take semaphore in tbmc_clientrpc_request()");
          return -1;
     }

     int size = strlen(TB_MQTT_TOPIC_CLIENTRPC_REQUEST_PREFIX) + 20;
     char* topic = TBMC_MALLOC(size);
     if (!topic) {
          return TBMC_ERR_NO_MEM;
     }
     memset(topic, 0x00, size);
     snprintf(topic, size-1, TB_MQTT_TOPIC_CLIENTRPC_REQUEST_PATTERN, request_id);

     /*TBLOG_XD(String("[Client-Side RPC] Tx Request:")
          + " Length:" + strlen(payload)
          + ", RequestID:" + request_id
          + ", Timestamp:" + millis() // + ", MsgID:" + message_id
          + ", QoS:" + qos
          + ", Retain:" + retain
          + ", onClientRpcRspMap.size=" + this->onClientRpcRspMap.size(),
          payload, strlen(payload));*/
#ifdef TBLOG_LONG
     TBLOG_DD("[Client-Side RPC][Tx] RequestID=%d, QoS=%d, Retain=%d, Timestamp=%d, Length=%d, onClientRpcRspMap.size=%d\r\n\t\t%.*s",
              request_id,
              qos, retain, millis(), strlen(payload), this->onClientRpcRspMap.size(),
              strlen(payload), payload);
#else
     TBLOG_DD("[Client-Side RPC][Tx] RequestID=%d, onClientRpcRspMap.size=%d %.*s",
              request_id,
              this->onClientRpcRspMap.size(),
              strlen(payload), payload);
#endif

     int message_id = _tbmc_publish(client, topic, payload, qos, retain);
     TBMC_FREE(topic);
     return request_id;  /*return message_id;*/
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
 * @param on_clientrpc_success      Client-RPC response callback
 * @param on_clientrpc_timeout  Client-RPC response timeout callback
 * @param qos           qos of publish message
 * @param retain        ratain flag
 *
 * @return rpc_request_id of the subscribe message on success
 *        -1 if error
 */
tbmc_err_t tbmc_clientrpc_request_ex(tbmc_handle_t client, const char *method, const char *params,
                              void *context,
                              tbmc_on_clientrpc_success_t on_clientrpc_success,
                              tbmc_on_clientrpc_timeout_t on_clientrpc_timeout,
                              int qos/*= 1*/, int retain/*= 0*/)
{
     int size = strlen(TB_MQTT_TEXT_RPC_METHOD) + strlen(method) + strlen(TB_MQTT_TEXT_RPC_PARAMS) + strlen(params) + 20;
     char* payload = TBMC_MALLOC(size);
     if (!payload) {
          return TBMC_ERR_NO_MEM;
     }
     memset(payload, 0x00, size);
     snprintf(payload, size-1, "{\"method\":\"%s\",\"params\":{%s}}", method, params);
     tbmc_err_t retult = tbmc_clientrpc_request(client, payload,
                           context,
                           on_clientrpc_success,
                           on_clientrpc_timeout,
                           qos, retain);
     TBMC_FREE(payload);
     return retult;
} 

tbmc_err_t tbmc_fw_request_init(tbmc_handle_t client,
                         void *context,
                         tbmc_on_fwrequest_response_t on_fwrequest_response,
                         tbmc_on_fwrequest_timeout_t on_fwrequest_timeout)
{
     // TODO: ...
     // return request_id;
}

tbmc_err_t tbmc_fw_request_destory(tbmc_handle_t client, int request_id)
{
     // TODO: ...
     // search then clear !
} 
tbmc_err_t tbmc_fw_request_send(tbmc_handle_t client, int request_id, int chunk, const char *payload, //?payload
                         int qos/*= 1*/, int retain/*= 0*/)
{
     // TODO: ...

     // refresh timestamp
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
static tbmc_err_t _tbmc_subscribe(tbmc_handle_t client, const char* topic, int qos/*=0*/) //subscribe()
{
     if (!client->mqtt_handle || !topic) {
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
static tbmc_err_t _tbmc_publish(tbmc_handle_t client, const char *topic, const char *payload, int qos/*= 1*/, int retain/*= 0*/) //publish()
{
     if (!client->mqtt_handle || !topic) {
          return -1;
     }

     int len = (payload==NULL) ? 0 : strlen(payload); //// +1
     return esp_mqtt_client_publish(client->mqtt_handle, topic, payload, len, qos, retain);
}

static void _on_DataEventProcess(tbmc_handle_t client, esp_mqtt_event_handle_t event) //_onDataEventProcess(); //MQTT_EVENT_DATA
{
     // TODO: ...
}
static tbmc_err_t  _on_MqttEventHandle(tbmc_handle_t client, esp_mqtt_event_handle_t event) //_onMqttEventHandle();  //MQTT_EVENT_...
{
     // TODO: ...
}

static tbmc_err_t  _on_MqttEventCallback(tbmc_handle_t client, sp_mqtt_event_handle_t event) //_onMqttEventCallback();
{
     // TODO: ...
}

static bool _request_is_equal(const tbmc_request_t *a, const tbmc_request_t*b)
{
     if (!a & !b) {
          return true;
     }
     if (!a || !b) {
          return false;
     }
     if (a->type != b->type) {
          return false;
     } else if(a->request_id != b->request_id) {
          return false;
     } else { //if (a->ts != b->ts) 
          return a->ts == b->ts;
     } else 
}
static void _request_list_lock(tbmc_handle_t client)
{
     // TODO: ...
}

static void _request_list_unlock(tbmc_handle_t client)
{
     // TODO: ...
}

static tbmc_err_t _request_list_append(tbmc_handle_t client, tbmc_request_type_t type, uint32_t request_id,
                                       void *context,
                                       tbmc_on_request_success_t on_success,
                                       tbmc_on_request_timeout_t on_timeout)
{
     tbmc_request_t *tbmc_request = TBMC_MALLOC(sizeof(tbmc_request_t));
     if (!tbmc_request) 
     {
          return TBMC_ERR_NO_MEM;
     }

     memset(tbmc_request, 0x00, sizeof(tbmc_request_t));
     tbmc_request->type = type;
     tbmc_request->request_id = request_id;
     tbmc_request->ts = (uint32_t)time(0); // TODO: //NULL
     tbmc_request->context = context;
     tbmc_request->on_success = on_success;
     tbmc_request->on_timeout = on_timeout;

     tbmc_request_t* it, last = NULL;
     if (LIST_FIRST(&client->request_list) == NULL) {
          // insert head
          LIST_INSERT_HEAD(&client->request_list, tbmc_request, entry);
     } else {
          // insert last
          LIST_FOREACH(it, &client->request_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, tbmc_request, entry);
          }
     }
     return TBMC_OK;
}

static tbmc_err_t _request_list_remove(tbmc_handle_t client, tbmc_request_t *tbmc_request)
{
     _request_list_lock();  // TODO:
     LIST_REMOVE(tbmc_request, entry);

     tbmc_request->type = 0;
     tbmc_request->request_id = 0;
     tbmc_request->ts = 0;
     tbmc_request->context = NULL;
     tbmc_request->on_success = NULL;
     tbmc_request->on_timeout = NULL;
     TBMC_FREE(tbmc_request);

     _request_list_unlock(); // TODO:
     return TBMC_OK;
}

#if 0 //// LIST_...
//_create()
//   tbmc.request_list = LIST_HEAD_INITIALIZER(tbmc.request_list);

// if (LIST_FIRST(&tbmc.request_list) == NULL) ...
// LIST_INSERT_HEAD(&tbmc.request_list, /* tbmc_request* */request, entry);
// struct tbmc_request *it, last = NULL;
//  LIST_FOREACH(it, &tbmc.request_list, entry) {
//        LIST_INSERT_BEFORE(it, /* tbmc_request* */request, entry);
//        LIST_INSERT_AFTER(last, /* tbmc_request* */request, entry);
//  LIST_REMOVE(/* tbmc_request* */request, entry);
// if (!LIST_EMPTY(&tbmc.request_list)) ...


typedef struct esp_timer* esp_timer_handle_t;
struct esp_timer {
    uint64_t alarm;
    uint64_t period;
    LIST_ENTRY(esp_timer) list_entry;
};

// list of currently armed timers
static LIST_HEAD(esp_timer_list, esp_timer) s_timers = LIST_HEAD_INITIALIZER(s_timers);

static IRAM_ATTR esp_err_t timer_insert(esp_timer_handle_t timer)
{
#if WITH_PROFILING
    timer_remove_inactive(timer);
#endif
    esp_timer_handle_t it, last = NULL;
    if (LIST_FIRST(&s_timers) == NULL) {
        LIST_INSERT_HEAD(&s_timers, timer, list_entry);
    } else {
        LIST_FOREACH(it, &s_timers, list_entry) {
            if (timer->alarm < it->alarm) {
                LIST_INSERT_BEFORE(it, timer, list_entry);
                break;
            }
            last = it;
        }
        if (it == NULL) {
            assert(last);
            LIST_INSERT_AFTER(last, timer, list_entry);
        }
    }
    if (timer == LIST_FIRST(&s_timers)) {
        esp_timer_impl_set_alarm(timer->alarm);
    }
    return ESP_OK;
}

static IRAM_ATTR esp_err_t timer_remove(esp_timer_handle_t timer)
{
    timer_list_lock();
    LIST_REMOVE(timer, list_entry);
    timer->alarm = 0;
    timer->period = 0;
#if WITH_PROFILING
    timer_insert_inactive(timer);
#endif
    timer_list_unlock();
    return ESP_OK;
}

esp_err_t esp_timer_deinit(void)
{
    if (!is_initialized()) {
        return ESP_ERR_INVALID_STATE;
    }

    /* Check if there are any active timers */
    if (!LIST_EMPTY(&s_timers)) {
        return ESP_ERR_INVALID_STATE;
    }
}

esp_err_t esp_timer_dump(FILE* stream)
{
    esp_timer_handle_t it;
    /* First count the number of timers */
    size_t timer_count = 0;
    timer_list_lock();
    LIST_FOREACH(it, &s_timers, list_entry) {
        ++timer_count;
    }
    timer_list_unlock();
}
#endif