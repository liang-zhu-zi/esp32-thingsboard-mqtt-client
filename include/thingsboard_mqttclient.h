// Copyright 2022 liangzhuzhi2020@gmail.com, https://github.com/liang-zhu-zi/thingsboard-mqttclient-basedon-espmqtt
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _THINGSBOARD_MQTTCLIENT_H_
#define _MQTT_CLIENT_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//Publish Telemetry data
#define TBMQTT_TOPIC_TELEMETRY_PUBLISH             "v1/devices/me/telemetry" //publish

//Publish client-side device attributes to the server
#define TBMQTT_TOPIC_CLIENT_ATTRIBUTES_PUBLISH     "v1/devices/me/attributes" //publish

//Request client-side or shared device attributes from the server
#define TBMQTT_TOPIC_CLIENT_SHARED_ATTRIBUTES_REQUEST_PATTERN     "v1/devices/me/attributes/request/%d"  //publish, $request_id
#define TBMQTT_TOPIC_CLIENT_SHARED_ATTRIBUTES_REQUEST_PREFIX      "v1/devices/me/attributes/request/"    //publish
#define TBMQTT_TOPIC_CLIENT_SHARED_ATTRIBUTES_RESPONSE_PATTERN    "v1/devices/me/attributes/response/%d" //receive, $request_id
#define TBMQTT_TOPIC_CLIENT_SHARED_ATTRIBUTES_RESPONSE_PREFIX     "v1/devices/me/attributes/response/"   //receive
#define TBMQTT_TOPIC_CLIENT_SHARED_ATTRIBUTES_RESPONSE_SUBSCRIRBE "v1/devices/me/attributes/response/+"  //subscribe

//Subscribe to shared device attribute updates from the server
#define TBMQTT_TOPIC_SHARED_ATTRIBUTES             "v1/devices/me/attributes"      //subscribe, receive

//Server-side RPC
#define TBMQTT_TOPIC_SERVER_RPC_REQUEST_PATTERN    "v1/devices/me/rpc/request/%d"  //receive, $request_id
#define TBMQTT_TOPIC_SERVER_RPC_REQUEST_PREFIX     "v1/devices/me/rpc/request/"    //receive
#define TBMQTT_TOPIC_SERVER_RPC_REQUEST_SUBSCRIBE  "v1/devices/me/rpc/request/+"   //subscribe
#define TBMQTT_TOPIC_SERVER_RPC_RESPONSE_PATTERN   "v1/devices/me/rpc/response/%d" //publish, $request_id
#define TBMQTT_TOPIC_SERVER_RPC_RESPONSE_PREFIX    "v1/devices/me/rpc/response/"   //publish

//Client-side RPC
#define TBMQTT_TOPIC_CLIENT_RPC_REQUEST_PATTERN    "v1/devices/me/rpc/request/%d"  //publish, $request_id
#define TBMQTT_TOPIC_CLIENT_RPC_REQUEST_PREFIX     "v1/devices/me/rpc/request/"    //publish
#define TBMQTT_TOPIC_CLIENT_RPC_RESPONSE_PATTERN   "v1/devices/me/rpc/response/%d" //receive, $request_id
#define TBMQTT_TOPIC_CLIENT_RPC_RESPONSE_PREFIX    "v1/devices/me/rpc/response/"   //receive
#define TBMQTT_TOPIC_CLIENT_RPC_RESPONSE_SUBSCRIBE "v1/devices/me/rpc/response/+"  //subscribe

//Claiming device using device-side key scenario: Not implemented yet
//#define TBMQTT_TOPIC_DEVICE_CLAIM         "v1/devices/me/claim" //publish

//Device provisioning: Not implemented yet
//#define TBMQTT_TOPIC_PROVISION_REQUESTC   "/provision/request"  //publish
//#define TBMQTT_TOPIC_PROVISION_RESPONSE   "/provision/response" //subscribe, receive

//Firmware update
//receive fw_title, fw_version, fw_checksum, fw_checksum_algorithm shared attributes after the device subscribes to "v1/devices/me/attributes/response/+".
#define TBMQTT_TOPIC_FW_REQUEST_PATTERN     "v2/fw/request/%d/chunk/%d"  //publish, ${requestId}, ${chunk}
#define TBMQTT_TOPIC_FW_RESPONSE_PATTERN    "v2/fw/response/%d/chunk/%d" //receive ${requestId}, ${chunk}
#define TBMQTT_TOPIC_FW_RESPONSE_SUBSCRIBE  "v2/fw/response/+/chunk/+"   //subsribe

tb_mqtt_client_init();
tb_mqtt_client_destory();
tb_mqtt_client_connect(); //...start()
tb_mqtt_client_disconnect(); //...stop()
tb_mqtt_client_is_connected();
tb_mqtt_client_is_connecting();
tb_mqtt_client_is_disconnected();
tb_mqtt_client_get_state();
tb_mqtt_client_loop();  // Executes an event loop for PubSub client.
tb_mqtt_client_set_OnConnected(TBMQTT_ON_CONNECTED callback);
tb_mqtt_client_set_OnDisconnected(TBMQTT_ON_DISCONNECTED callback);
tb_mqtt_client_set_OnServerRpcRequest(TBMQTT_SERVER_RPC_CALLBACK callback);
tb_mqtt_client_set_OnAttrSubReply(TBMQTT_ATTR_SUB_CALLBACK callback);
tb_mqtt_client_get_OnConnected(void);
tb_mqtt_client_get_OnDisconnected(void);
tb_mqtt_client_get_OnServerRpcRequest(void);
tb_mqtt_client_get_OnAttrSubReply(void);
tb_mqtt_client_sendServerRpcReply(); //response server-side RPC
tb_mqtt_client_sendClientRpcCall(); //request client-side RPC
tb_mqtt_client_requestAttributes(); //request client and shared attributes
tb_mqtt_client_sendAttributes(); //publish client attributes
tb_mqtt_client_sendTelemetry();

_publish();
_subscribe();
_checkTimeout();
_onDataEventProcess(); //MQTT_EVENT_DATA
_onMqttEventHandle();  //MQTT_EVENT_...
_onMqttEventCallback();

tb_mqtt_link_init();
tb_mqtt_link_destroy();
tb_mqtt_link_config(); //=> set_config();
tb_mqtt_link_begin(); //connect();
tb_mqtt_link_end(); //disconnect();
tb_mqtt_link_run(); //tb_mqtt_client_loop(), recv/parse/sendqueue/ack...
tb_mqtt_link_isConnected();
_recv(); //??
tb_mqtt_link_set_ConnectedEvent(evtConnected); //??Call it before connect()
tb_mqtt_link_set_DisconnectedEvent(evtDisconnected); //??Call it before connect()
tb_mqtt_link_add_ServerRpcEvent(evtServerRpc); //Call it before connect()
tb_mqtt_link_add_SubAttrEvent(evtAttrbute); //Call it before connect()
tb_mqtt_link_send_AttributesRequest();
tb_mqtt_link_send_sendClientRpcRequest();
tb_mqtt_link_send_ServerRpcReply();
tb_mqtt_link_send_ClientAttributes();
tb_mqtt_link_send_Telemetry();
_sendTbmqttInnerMsg2Queue();
_onConnected();
_onDisonnected();
_onServerRpcRequest();
_onAttrOfSubReply();
_onClientRpcResponseTimeout();
_onClientRpcResponse();
_onAttributesResponseTimeout();
_onAttributesResponse(); //_attributesResponse()
//_tbDecodeAttributesJsonPayload()
//_isDeserializationError()

//===tbmc===========================================================
/* Definitions for error constants. */
#define TBMC_OK 0    /*!< tbmc_err_t value indicating success (no error) */
#define TBMC_FAIL -1 /*!< Generic tbmc_err_t code indicating failure */

#define TBMC_ERR_NO_MEM              0x101   /*!< Out of memory */
#define TBMC_ERR_INVALID_ARG         0x102   /*!< Invalid argument */
// #define ESP_ERR_INVALID_STATE       0x103   /*!< Invalid state */
// #define ESP_ERR_INVALID_SIZE        0x104   /*!< Invalid size */
// #define ESP_ERR_NOT_FOUND           0x105   /*!< Requested resource not found */
// #define ESP_ERR_NOT_SUPPORTED       0x106   /*!< Operation or feature not supported */
// #define ESP_ERR_TIMEOUT             0x107   /*!< Operation timed out */
// #define ESP_ERR_INVALID_RESPONSE    0x108   /*!< Received response was invalid */
// #define ESP_ERR_INVALID_CRC         0x109   /*!< CRC or checksum was invalid */
// #define ESP_ERR_INVALID_VERSION     0x10A   /*!< Version was invalid */
// #define ESP_ERR_INVALID_MAC         0x10B   /*!< MAC address was invalid */
// #define ESP_ERR_NOT_FINISHED        0x10C   /*!< There are items remained to retrieve */


// #define ESP_ERR_WIFI_BASE           0x3000  /*!< Starting number of WiFi error codes */
// #define ESP_ERR_MESH_BASE           0x4000  /*!< Starting number of MESH error codes */
// #define ESP_ERR_FLASH_BASE          0x6000  /*!< Starting number of flash error codes */
// #define ESP_ERR_HW_CRYPTO_BASE      0xc000  /*!< Starting number of HW cryptography module error codes */
// #define ESP_ERR_MEMPROT_BASE        0xd000  /*!< Starting number of Memory Protection API error codes */

typedef int tbmc_err_t;

//===value==========================================================
typedef char *TBMC_STRING;
typedef int TBMC_BOOLEAN;
typedef double TBMC_DOUBLE;
typedef long TBMC_LONG;
typedef char *TBMC_JSON;
//#define TBMC_STRING char*
//#define TBMC_BOOLEAN int
//#define TBMC_DOUBLE double
//#define TBMC_LONG long
//#define TBMC_JSON char*

/**
 * ThingsBoard MQTT Client value type
 */
typedef enum
{
     TBMC_VALUE_TYPE_INVALID = 0,
     TBMC_VALUE_TYPE_STRING,  /*!< string value */
     TBMC_VALUE_TYPE_BOOLEAN, /*!< boolean value */
     TBMC_VALUE_TYPE_DOUBLE,  /*!< double value */
     TBMC_VALUE_TYPE_LONG,    /*!< long value  */
     TBMC_VALUE_TYPE_JSON     /*!< JSON value */
} tbmc_value_type_t;

/**
 * ThingsBoard MQTT Client value
 */
typedef struct
{
     tbmc_value_type_t type; /*!< type of value */
     //int size;               /*!< size of value?? */
     union
     {
          TBMC_STRING stringV;
          TBMC_BOOLEAN boolV;
          TBMC_DOUBLE doubleV;
          TBMC_LONG longV;
          TBMC_JSON jsonV;
     } value;
} tbmc_value_t;

// typedef tbmc_err_t (*TBMC_GET_STRING_VALUE_CB)(void *context, TBMC_STRING *value, int value_size); /*!< Get TBMC_STRING value from context */
// typedef tbmc_err_t (*TBMC_GET_BOOLEAN_VALUE_CB)(void *context, TBMC_BOOLEAN *value, int);          /*!< Get TBMC_BOOLEAN value from context */
// typedef tbmc_err_t (*TBMC_GET_DOUBLE_VALUE_CB)(void *context, TBMC_DOUBLE *value, int);            /*!< Get TBMC_DOUBLE value from context */
// typedef tbmc_err_t (*TBMC_GET_LONG_VALUE_CB)(void *context, TBMC_LONG *value, int);                /*!< Get TBMC_LONG value from context */
// typedef tbmc_err_t (*TBMC_GET_JSON_VALUE_CB)(void *context, TBMC_JSON *value, int value_size);     /*!< Get TBMC_JSON value from context */

// typedef tbmc_err_t (*TBMC_SET_STRING_VALUE_CB)(void *context, TBMC_STRING value);   /*!< Set TBMC_STRING value to context */
// typedef tbmc_err_t (*TBMC_SET_BOOLEAN_VALUE_CB)(void *context, TBMC_BOOLEAN value); /*!< Set TBMC_BOOLEAN value to context */
// typedef tbmc_err_t (*TBMC_SET_DOUBLE_VALUE_CB)(void *context, TBMC_DOUBLE value);   /*!< Set TBMC_DOUBLE value to context */
// typedef tbmc_err_t (*TBMC_SET_LONG_VALUE_CB)(void *context, TBMC_LONG value);       /*!< Set TBMC_LONG value to context */
// typedef tbmc_err_t (*TBMC_SET_JSON_VALUE_CB)(void *context, TBMC_JSON value);       /*!< Set TBMC_JSON value to context */

typedef tbmc_err_t (*tbmc_value_get_callback_t)(void *context, tbmc_value_t *value);       /*!< Get tbmc_value from context */
typedef tbmc_err_t (*tbmc_value_set_callback_t)(void *context, const tbmc_value_t *value); /*!< Set tbmc_value to context */

//===key-value======================================================
/**
 * ThingsBoard MQTT Client key-value
 */
typedef struct
{
     char *key;           /*!< Key */
     tbmc_value_t *value; /*!< Value */

     void *context;                          /*!< Context of getting/setting value*/
     tbmc_value_get_callback_t get_value_cb; /*!< Callback of getting value from context */
     tbmc_value_set_callback_t set_value_cb; /*!< Callback of setting value to context */
} tbmc_kv_t;

typedef tbmc_kv_t *tbmc_kv_handle_t;

/**
 * @brief Creates tbmc key-value handle
 *
 * @param key
 * @param type           context of getting/setting value
 * @param context        contex of value for callback
 * @param get_value_cb   callback of getting value from context
 * @param set_value_cb   callback of setting value to context
 *
 * @return tbmc_kv_handle_t if successfully created, NULL on error
 */
tbmc_kv_handle_t tbmc_kv_init(const char *key, tbmc_value_type_t type, void *context, tbmc_value_get_callback_t get_value_cb, tbmc_value_set_callback_t set_value_cb);

/**
 * @brief Destroys the tbmc key-value handle
 *
 * Notes:
 *  - Cannot be called from the tbmc event handler
 *
 * @param kv    tbmc key-value handle
 *
 * @return TBMC_OK
 *         TBMC_ERR_INVALID_ARG on wrong initialization
 */
tbmc_err_t tbmc_kv_destory(tbmc_kv_handle_t kv);

/**
 * @brief Get key of the tbmc key-value handle
 *
 * @param kv    tbmc key-value handle
 *
 * @return key of the tbmc key-value handle if successfully created, NULL on error
 */
const char *tbmc_kv_get_key(tbmc_kv_handle_t kv);

/**
 * @brief Get value type of tbmc_kv
 *
 * @param kv    tbmc key-value handle
 *
 * @return vale type of the tbmc key-value handle if successfully created, TBMC_VALUE_TYPE_INVALID on error
 */
tbmc_value_type_t tbmc_kv_get_value_type(tbmc_kv_handle_t kv);

/**
 * @brief Get tbmc_value of tbmc_kv
 *
 * @param kv    tbmc key-value handle
 * @param value result, fan-out parameter
 *
 * @return ESP_OK on success
 *         ESP_ERR_INVALID_ARG on wrong initialization
 *         ESP_FAIL if client is in invalid state
 */
tbmc_err_t tbmc_kv_get_value(tbmc_kv_handle_t kv, tbmc_value_t *value);

/**
 * @brief Set tbmc_value of tbmc_kv
 *
 * @param kv    tbmc key-value handle
 * @param value
 *
 * @return ESP_OK on success
 *         ESP_ERR_INVALID_ARG on wrong initialization
 *         ESP_FAIL if client is in invalid state
 */
tbmc_err_t tbmc_kv_set_value(tbmc_kv_handle_t kv, const tbmc_value_t *value);

//===telemetry_datapoint============================================
/**
 * ThingsBoard MQTT Client telemetry datapoint
 */
typedef tbmc_kv_t tbmc_dp_t;
typedef tbmc_dp_t *tbmc_dp_handle_t;

tbmc_dp_handle_t tbmc_dp_init(const char *key, tbmc_value_type_t type, void *context, tbmc_value_get_callback_t get_value_cb, tbmc_value_set_callback_t set_value_cb); /*!< Initialize tbmc_dp of TBMC_JSON */
tbmc_err_t tbmc_dp_destory(tbmc_dp_handle_t dp);                                                                                                                       /*!< Destroys the tbmc key-value handle */

const char *tbmc_dp_get_key(tbmc_dp_handle_t dp);              /*!< Get key of the tbmc datapoint handle */
tbmc_value_type_t tbmc_dp_get_value_type(tbmc_dp_handle_t dp); /*!< Get value type of tbmc_dp */

tbmc_err_t tbmc_dp_get_value(tbmc_dp_handle_t dp, tbmc_value_t *value);       /*!< Get tbmc_value of tbmc_dp */
tbmc_err_t tbmc_dp_set_value(tbmc_dp_handle_t dp, const tbmc_value_t *value); /*!< Set tbmc_value of tbmc_dp */
//_dps_pack()?/_dps_send()?

//===attribute: client-side_attribute, shared_attribute=========================================
/**
 * ThingsBoard MQTT Client telemetry attribute
 */
typedef tbmc_kv_t tbmc_attribute_t;
typedef tbmc_attribute_t *tbmc_attribute_handle_t;

#define tbmc_attribute_ tbmc_kv_

typedef struct tbmc_kv_
{
     char *key;           /*!< Key */
     tbmc_value_t *value; /*!< Value */

     void *context;                          /*!< Context of getting/setting value*/
     tbmc_value_get_callback_t get_value_cb; /*!< Callback of getting value from context */
     tbmc_value_set_callback_t set_value_cb; /*!< Callback of setting value to context */
} tbmc_kv_t;

/**
 * @brief Creates tbmc client-side attribute handle
 *
 * @param key
 * @param type           context of getting/setting value
 * @param context        contex of value for callback
 * @param get_value_cb   callback of getting value from context. It must not be NULL!
 * @param set_value_cb   callback of setting value to context. It must not be NULL 
 *                       if it needs to get initial values from the server
 *                       (request attribute values from the server), otherwise it should be NULL.
 *
 * @return tbmc_kv_handle_t if successfully created, NULL on error
 */
tbmc_attribute_handle_t tbmc_attribute_of_clientside_init(const char *key, tbmc_value_type_t type, void *context,
                                                          tbmc_value_get_callback_t get_value_cb,
                                                          tbmc_value_set_callback_t set_value_cb);
/**
 * @brief Creates tbmc shared attribute handle
 *
 * @param key
 * @param type           context of getting/setting value
 * @param context        contex of value for callback
 * @param set_value_cb  callback of setting value to context. It must not be NULL.
 *
 * @return tbmc_kv_handle_t if successfully created, NULL on error
 */
tbmc_attribute_handle_t tbmc_attribute_of_shared_init(const char *key, tbmc_value_type_t type, void *context, 
                                                       /*tbmc_value_get_callback_t get_value_cb,*/
                                                       tbmc_value_set_callback_t set_value_cb);

tbmc_err_t tbmc_attribute_destory(tbmc_attribute_handle_t attribute);               /*!< Destroys the tbmc key-value handle */

bool tbmc_attribute_is_shared(tbmc_attribute_handle_t attribute);                   /*!< Is it a shared attribute? */
bool tbmc_attribute_is_clientside(tbmc_attribute_handle_t attribute);               /*!< Is it a client-side attribute? */
bool tbmc_attribute_has_set_value_cb(tbmc_attribute_handle_t attribute);            /*!< Has it a set value callback? A shared attribute is always true;
                                                                                         a client-side attribute is true or false. */

const char *tbmc_attribute_get_key(tbmc_attribute_handle_t attribute);              /*!< Get key of the tbmc tbmc_attribute handle */
tbmc_value_type_t tbmc_attribute_get_value_type(tbmc_attribute_handle_t attribute); /*!< Get value type of tbmc_attribute */

tbmc_err_t tbmc_attribute_of_clientside_get_value(tbmc_attribute_handle_t clientside_attribute, tbmc_value_t *value); /*!< Get tbmc_value of client-side attribute */
tbmc_err_t tbmc_attribute_set_value(tbmc_attribute_handle_t attribute, const tbmc_value_t *value);                    /*!< Set tbmc_value of tbmc_attribute */

//_csas_pack()/_csas_send()!, _as_unpack()/_as_deal()
//                            _as_unpack()/_as_deal()

//===attributes request for client-side_attribute or/and shared_attribute=========================================
typedef void (*tbmc_attributes_request_success_callback_t)(tbmc_client_handle_t client, tbmc_attributes_request_handle_t request, void* context, void* context); //(none/resend/destory/_destory_all_attributes)?
typedef void (*tbmc_attributes_request_timeout_callback_t)(tbmc_client_handle_t client, tbmc_attributes_request_handle_t request, void* context, void* context); //(none/resend/destory/_destory_all_attributes)?

//#include "sys/queue.h"
//struct track_; // Forward declaration
//typedef STAILQ_HEAD(track_list, track_) track_list_t;
//tbmc_clientside_attribute_list_t
//tbmc_shared_attribute_list_t

/**
 * ThingsBoard MQTT Client attributes request
 */
typedef struct tbmc_attributes_request_
{
     void *context;                                         /*!< Context of callback*/
     tbmc_attributes_request_success_callback_t success_cb; /*!< Callback of dealing successful */
     tbmc_attributes_request_timeout_callback_t timeout_cb; /*!< Callback of response timeout */

     tbmc_clientside_attribute_list_t clientside_attribute_list;  // TODO:~~~
     tbmc_shared_attribute_list_t shared_attribute_list;    // TODO:~~~

     int times;               /*!< times of sending. default value is 0 */
} tbmc_attributes_request_t;

typedef tbmc_attributes_request_t *tbmc_attributes_request_handle_t;

tbmc_attributes_request_handle_t tbmc_attributes_request_init(void *context,
                                                              tbmc_attributes_request_success_callback_t success_cb,
                                                              tbmc_attributes_request_timeout_callback_t timeout_cb); /*!< Initialize tbmc_attributes_request */
tbmc_err_t tbmc_attributes_request_destory(tbmc_attributes_request_handle_t request);                                 /*!< Destroys the tbmc_attributes_request */

tbmc_err_T tbmc_attributes_request_add(tbmc_attributes_request_handle_t request, tbmc_attribute_handle_t attribute, ...);

tbmc_err_T tbmc_attributes_request_get_client_keys(tbmc_attributes_request_handle_t request, char *buffer, int size);
tbmc_err_T tbmc_attributes_request_get_shared_keys(tbmc_attributes_request_handle_t request, char *buffer, int size);

tbmc_attribute_handle_t tbmc_attributes_request_search_clientside_attribute(tbmc_attributes_request_handle_t request, const char *clientside_attribute_name); /*!< Search the client-side attribute in request */
tbmc_attribute_handle_t tbmc_attributes_request_search_shared_attribute(tbmc_attributes_request_handle_t request, const char *clientside_attribute_name); /*!< Search the shared attribute in request */
//_pack()/_send()!, _unpack()/deal() //_onAttributesResponse()[unpack->queue], [queue->attribute deal], 
//???_destory_all_attributes(),

//===5.Server-side RPC: many-times deal========================================
/**
 * ThingsBoard MQTT Client rpc params
 */
typedef cJSON* tbmc_rpc_params_t;

/**
 * ThingsBoard MQTT Client rpc results
 */
typedef cJSON* tbmc_rpc_results_t;

typedef tbmc_rpc_results_t (*tbmc_serverrpc_request_callback_t)(tbmc_handle_client_t client, tbmc_serverrpc_handle_t serverrpc, void* context, int request_id, const char* method, tbmc_rpc_params_t params);

typedef tbmc_serverrpc_t* tbmc_serverrpc_handle_t;

/**
 * ThingsBoard MQTT Client server-RPC
 */
typedef struct tbmc_serverrpc_
{
     const char *method_value; /*!< method value */
     const char *method_key;   /*!< method key, default "method" */
     const char *params_key;   /*!< params key, default "params" */
     const char *results_key;  /*!< results key, default "results" */
     void *context;            /*!< Context of callback */

     tbmc_serverrpc_request_callback_t on_request; /*!< Callback of server-rpc request */
} tbmc_serverrpc_t;

//====================6.Client-side RPC: once-time======================================
typedef void (*tbmc_clientrpc_success_callback_t)(tbmc_handle_client_t client, tbmc_clientrpc_handle_t clientrpc, void* context, int request_id, const char* method, tbmc_rpc_params_t params, tbmc_rpc_results_t results);
typedef void (*tbmc_clientrpc_timeout_callback_t)(tbmc_handle_client_t client, tbmc_clientrpc_handle_t clientrpc, void* context, int request_id, const char* method, tbmc_rpc_params_t params);

typedef tbmc_clientrpc_t* tbmc_clientrpc_handle_t;

/**
 * ThingsBoard MQTT Client client-RPC
 */
typedef struct tbmc_clientrpc_
{
     const char *method_value; /*!< method value */
     const char *method_key;   /*!< method key, default "method" */
     const char *params_key;   /*!< params key, default "params" */
     const char *results_key;  /*!< results key, default "results" */
     void *context;            /*!< Context of callback */

     tbmc_rpc_params_t params;
     tbmc_clientrpc_success_callback_t on_success; /*!< Callback of client-rpc response success */
     tbmc_clientrpc_timeout_callback_t on_timeout; /*!< Callback of client-rpc response timeout */
} tbmc_clientrpc_t;

tbmc_clientrpc_handle_t tbmc_clientrpc_request(tbmc_client_handle_t client, const char* method, tbmc_rpc_params_t params, void *context, tbmc_clientrpc_response_callback_t on_response); //tbmqttclient_sendClientRpcRequest(); //list: struct clientside_rpc_request{}


//===============================================
tbmqttclient_init(config); //config: on_connected()+on_disconnected()
tbmqttclient_destory();
tbmqttclient_set_config(config); //config: on_connected()+on_disconnected()
tbmqttclient_start();
tbmqttclient_stop();
//on_connected(); //reqattrShared.send();
//on_disconnected();
tbmqttclient_disconnect();
tbmqttclient_reconnect();
tbmqttclient_run(); //tb_mqtt_client_loop(), recv/parse/sendqueue/ack...
tbmqttclient_isConnected();

//1.Publish Telemetry datapoints: once-time
tbmc_err_t tbmc_telemetry_datapoints_send(tbmc_client_handle_t client, tbmc_dp_handle_t dp, ...);
//tbmc_telemetry_datapoint_list_send(datapoint_list);   //telemetry_datapoint_list_init()/_destory(), _add(), _pack()/_send()!, _get_name()

//2.Publish client-side device attributes to the server: once-time
tbmc_err_t tbmc_clientside_attributes_send(tbmc_client_handle_t client, tbmc_csa_handle_t csa, ...);
//tbmc_clientside_attribute_list_send(clientside_attribute_list);//clientside_attribute_list_init()/_destory(), _add(), _pack()/_send()!,                    _get_name(), _get_keys()
                                                                             //shared_attribute_list_init()/_destory(), _add(),                   _unpack()/_deal()?,_get_name(), _get_keys()

//3.Request client-side or shared device attributes from the server: once-time
tbmc_err_t tbmc_attributes_request(tbmc_client_handle_t client, tbmc_attributes_request_handle_t request, ...); ////return request_id on successful, otherwise return TBMC_FAIL

//4.Subscribe to shared device attribute updates from the server: many-times deal. Call it before connect().
tbmc_err_t tbmc_shared_attributes_observer_append(tbmc_client_handle_t client, tbmc_sa_t shared_attribute, ...); //tbmqttclient_addSubAttrEvent(); //tbmc_shared_attribute_list_t
tbmc_err_t tbmc_shared_attributes_observer_clear(tbmc_client_handle_t client, tbmc_sa_t shared_attribute, ...); //remove shared_attribute from tbmc_shared_attribute_list_t
//tbmc_clientside_attribute_list_t

//======================5.Server-side RPC: many-times deal.Call it before connect()===========================
tbmc_err_t tbmc_serverrpc_observer_append(tbmc_client_handle_t client, const char* method, void *context, tbmc_serverrpc_request_callback_t on_request);
tbmc_err_t tbmc_serverrpc_observer_clear(tbmc_client_handle_t client, const char* method); //remove from LIST_ENTRY(tbmc_serverrpc_) & delete

//const char *_tbmc_serverrpc_get_method(tbmc_serverrpc_handle_t serverrpc);

//0.   Subscribe topic: server-side RPC request;

//1.   tbmc_serverrpc_observer_append(...);
//1.1  tbmc_serverrpc_handle_t _tbmc_serverrpc_init(const char* method, void *context, tbmc_serverrpc_request_callback_t on_request);
//1.2  create to add to LIST_ENTRY(tbmc_serverrpc_)
//1.3  tbmqttclient_addServerRpcEvent()???

//2.     _tbmc.on_serverrpc_request()
//2.1    _tbmc.on_serverrpc_request_unpack(): parse payload* to cJSON*, then push it to queue;
//2.2    _tbmc.on_serverrpc_request_deal(): call a server RPC's on_request callback by method name, then send a replay if on_request callback has a return value of tbmc_rpc_results_t.
//2.3   send serverrpc response, option:
//2.3.1  _tbmc.serverrpc_response_pack(...);
//2.3.2  _tbmc.serverrpc_response_send(...); //tbmc_err_t tbmc_serverrpc_response(tbmc_client_handle_t client, int request_id, const char* results); //tbmqttclient_sendServerRpcReply()

//3.    tbmc_client_destory(...)
//3.x   tbmc_err_t _tbmc_serverrpc_destory(tbmc_serverrpc_handle_t serverrpc);

//====================6.Client-side RPC: once-time==================================================
tbmc_clientrpc_handle_t tbmc_clientrpc_request(tbmc_client_handle_t client, const char* method, tbmc_rpc_params_t params, void *context,
                    tbmc_clientrpc_response_callback_t on_response, tbmc_clientrpc_timeout_callback_t on_timeout); //tbmqttclient_sendClientRpcRequest(); //create to add to LIST_ENTRY(tbmc_clientrpc_)

//const char *_tbmc_clientrpc_get_method(tbmc_clientrpc_handle_t clientrpc);

//0.   Subscribe topic: client-side RPC response;

//1.    tbmc_clientrpc_request(...)
//1.1   tbmc_clientrpc_handle_t _tbmc_clientrpc_init(tbmc_client_handle_t client, const char* method, tbmc_rpc_params_t params, void *context, tbmc_clientrpc_response_callback_t on_response);
//1.1  _tbmc.clientrpc_request_pack(...) 
//1.2  _tbmc.clientrpc_request_send(...); //tbmqttclient_sendClientRpcRequest()

//2    _tbmc.on_clientrpc_response()
//2.1  _tbmc.on_clientrpc_response_unpack(): parse payload* to cJSON*, then push it to queue;
//2.2  _tbmc.on_clientrpc_response_deal(on_response): call a server RPC's on_request callback by method name, then send a replay if on_request callback has a return value of tbmc_rpc_results_t.

//3.   _tbmc.on_clientrpc_timeout(on_timeout)
//3.1  _tbmc.on_clientrpc_response_timeout(on_timeout)

//2.f/3.f tbmc_err_t _tbmc_clientrpc_destory(tbmc_clientrpc_handle_t clientrpc)

//4     tbmc_client_destory(...)
//4.x   tbmc_err_t _tbmc_serverrpc_destory(tbmc_serverrpc_handle_t serverrpc)

//====================7.Claiming device using device-side key scenario: Not implemented yet=========

//====================8.Device provisioning: Not implemented yet====================================

//====================9.Firmware update=============================================================
typedef bool       (*tbmc_fw_shared_attributes_callback_t)(tbmc_handle_client_t client, void *context, /*const char *fw_title,*/ const char *fw_version, const char *fw_checksum, const char *fw_checksum_algorithm);
typedef tbmc_err_t (*tbmc_fw_response_chunk_callback_t)(tbmc_handle_client_t client, void *context, int request_id, int chunk/*current chunk*/, const void *fw_data, int data_size);
typedef void       (*tbmc_fw_response_success_callback_t)(tbmc_handle_client_t client, void *context, int request_id, int chunk/*total_size*/);
typedef void       (*tbmc_fw_response_timeout_callback_t)(tbmc_handle_client_t client, void *context, int request_id, int chunk/*current chunk*/);

/**
 * ThingsBoard MQTT Client F/W update OTA
 */
typedef struct tbmc_fw_observer_
{
     void *context;
     tbmc_fw_shared_attributes_callback_t on_fw_attributes; /*!< callback of F/W OTA attributes */
     tbmc_fw_response_chunk_callback_t on_fw_chunk;         /*!< callback of F/W OTA doing */
     tbmc_fw_response_success_callback_t on_fw_success;     /*!< callback of F/W OTA success */
     tbmc_fw_response_timeout_callback_t on_fw_timeout;     /*!< callback of F/W OTA timeout */

     // reset these below fields.
     const char *fw_title;                                  /*!< OS fw, App fw, ... */
     const char *fw_version;
     const char *fw_checksum;
     const char *fw_checksum_algorithm;

     int request_id; /*!< default is 0 */
     int chunk;      /*!< default is zero, from 0 to  */
} tbmc_fw_observer_t;
typedef tbmc_fw_observer_t *tbmc_fw_observer_handle_t;

tbmc_err_t tbmc_fw_observer_append(tbmc_handle_client_t client,
                                   const char *fw_title,
                                   void *context,
                                   tbmc_fw_shared_attributes_callback_t on_fw_attributes,
                                   tbmc_fw_response_chunk_callback_t on_fw_chunk,
                                   tbmc_fw_response_success_callback_t on_fw_success,
                                   tbmc_fw_response_timeout_callback_t on_fw_timeout);
tbmc_err_t tbmc_fw_observer_clear(tbmc_handle_client_t client,
                                  const char *fw_title);

//0.  Subscribe topic: shared attribute updates: fw_title, fw_version, fw_checksum, fw_checksum_algorithm 
//0.  Subscribe topic: f/w response: v2/fw/response/+/chunk/+

//1.   tbmc_fw_observer_append();
//1.1  if (fw_title is not in list fw_observer_entries) 
//     then tbmc_fw_observer_handle_t _tbmc_fw_observer_create(...);

//2.   if (all of fw shared attributes updated)
//2.1  if (on_fw_attributes() returns true)
//2.2  tbmc._tbmc_fw_observer(tbmc_handle_client_t client, tbmc_fw_observer_handle_t fw_request, ...); //tbmqttclient_fw_request(...)

//3.   response success: tbmc._tbmc_on_fw_response(...)
//3.1  if (chunk_size >0 ) on_fw_chunk(...): save fw chunk
//3.2  else if (chunk_size ==0 ) on_fw_success(...); _tbmc_fw_observer_reset(...);

//4.   response timeout
//4.1  on_fw_timeout(...); _tbmc_fw_observer_reset(...);

//5    tbmc_fw_observer_clear() / tbmc_client_destory(...)
//5.x   tbmc_err_t _tbmc_fw_observer_destory(tbmc_fw_observer_handle_t fw_request)

/**
 * ThingsBoard MQTT Client
 */
typedef struct
{
     struct
     {
          LIST_ENTRY(tbmc_attribute_) entries; /*!< client attributes entries */
     } client_attributes;
     struct
     {
          LIST_ENTRY(tbmc_attribute_) entries; /*!< shared attributes entries */
     } shared_attributes;
     struct
     {
          LIST_ENTRY(tbmc_attributes_request_) entries; /*!< attributes request entries */
          int next_request_id;
     } attributes_request;
     struct
     {
          LIST_ENTRY(tbmc_serverrpc_) entries; /*!< server side RPC entries */
     } serverrpc;
     struct
     {
          LIST_ENTRY(tbmc_clientrpc_) entries; /*!< client side RPC entries */
          int next_request_id;
     } clientrpc;
     struct
     {
          LIST_ENTRY(tbmc_fw_observer_) entries; /*!< A device may have multiple firmware */
          int next_request_id;
          tbmc_attribute_handle_t fw_title;
          tbmc_attribute_handle_t fw_version;
          tbmc_attribute_handle_t fw_checksum;
          tbmc_attribute_handle_t fw_checksum_algorithm;
     } fw_observer;

     // tbmqttclient_handle_t tbmqttclient;
} tbmc_client_t;
typedef tbmc_client_t *tbmc_client_handle_t;

//=====================end===========================================================================

////tb_mqtt_link_addServerRpcEvent(evtServerRpc); //Call it before connect()
////tb_mqtt_link_addSubAttrEvent(evtAttrbute); //Call it before connect()
////tb_mqtt_link_sendAttributesRequest();
////tb_mqtt_link_sendClientRpcRequest();
////tb_mqtt_link_sendServerRpcReply();


_sendTbmqttInnerMsg2Queue();
_onConnected();
_onDisonnected();
_onServerRpcRequest();
_onAttrOfSubReply();
_onClientRpcResponseTimeout();
_onClientRpcResponse();
_onAttributesResponseTimeout();
_onAttributesResponse(); //_attributesResponse()
//_tbDecodeAttributesJsonPayload()
//_isDeserializationError()

_on_subscribed()???
_on_unsubscribed()???

//sendTelemetry
//Topic: 'v1/devices/me/telemetry'
//Data:  '{"key1":"value1", "key2":true, "key3": 3.0, "key4": 4}', '[{"key1":"value1"}, {"key2":true}]'

//sendAttributes
//Topic: 'v1/devices/me/attributes'
//Data:  '{"attribute1":"value1", "attribute2":true, "attribute3":42.0, "attribute4":73}'

//requestAttributes
//Topic: 'v1/devices/me/attributes/request/$request_id'
//Data:  '{"clientKeys":"attribute1,attribute2", "sharedKeys":"shared1,shared2"}'
//+
//_attributesResponse
//Topic:
//Data:  '{"client":{"controlMode":"On","floorTempLimited":27.5,"adaptiveControl":true},"shared":{"timezone":480,"syncTimeFreq":86400}}'

//sendClientRpcCall
//Topic: 'v1/devices/me/rpc/request/$request_id'
//Data:  '{"method":"getTime","params":{}}'
//+
//Client-Side RPC Response Callback. It runs in MQTT thread.
//Topic: 
//Data:  '{"method":"getTime","results":{"utcDateime":"2020-05-29T08:02:30Z","utcTimestamp":1590739350}}'

//receiveServerRpcRequest
//topic:    "v1/devices/me/rpc/request/$request_id" 
//payload:  {"method":"remoteSetChangeOverTempHeating", "params":25.5}
//+
//sendServerRpcReply
//Topic: 'v1/devices/me/rpc/response/$request_id'
//Data:  '{"example_response":23.1}' ???

//LIST_ENTRY, LIST_HEAD

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
