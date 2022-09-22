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

#ifndef _TB_MQTT_CLIENT_H_
#define _TB_MQTT_CLIENT_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//====ThingsBoard MQTT message example=================================================================================
//send telemetry time-series data:
//Topic: 'v1/devices/me/telemetry'
//Data:  '{"key1":"value1", "key2":true, "key3": 3.0, "key4": 4}', '[{"key1":"value1"}, {"key2":true}]'

//send client-side attributes:
//Topic:  'v1/devices/me/attributes'
//Data:   '{"attribute1":"value1", "attribute2":true, "attribute3":42.0, "attribute4":73}'

//request attributes:
//Topic:  'v1/devices/me/attributes/request/$request_id'
//Data:   '{"clientKeys":"attribute1,attribute2", "sharedKeys":"shared1,shared2"}'
//+
//attributes response:
//Topic:  'v1/devices/me/attributes/response/$request_id'
//Data:   '{"client":{"controlMode":"On","floorTempLimited":27.5,"adaptiveControl":true},"shared":{"timezone":480,"syncTimeFreq":86400}}'

//subscirbed shared-attributes:
//Topic:  'v1/devices/me/attributes'
//Data:   '{"key1":"value1"}', '{"fwVersion":"0.1.0.0"}'

//send client-side RPC:
//Topic:  'v1/devices/me/rpc/request/$request_id'
//Data:   '{"method":"getTime","params":{}}'
//+
//receive client-side RPC Response:
//Topic:  'v1/devices/me/rpc/response/$request_id'
//Data:   '{"method":"getTime","results":{"utcDateime":"2020-05-29T08:02:30Z","utcTimestamp":1590739350}}'

//receive server-side RPC request:
//topic:  'v1/devices/me/rpc/request/$request_id'
//Data:   '{"method":"remoteSetChangeOverTempHeating", "params":25.5}'
//+
//send server-side RPC response:
//Topic:  'v1/devices/me/rpc/response/$request_id'
//Data:   '{"example_response":23.1}' ???

//====ThingsBoard MQTT topic===========================================================================================
// Publish Telemetry data
#define TB_MQTT_TOPIC_TELEMETRY_PUBLISH               "v1/devices/me/telemetry"   //publish
// Publish client-side device attributes to the server
#define TB_MQTT_TOPIC_CLIENT_ATTRIBUTES_PUBLISH       "v1/devices/me/attributes"  //publish
// Request client-side or shared device attributes from the server
#define TB_MQTT_TOPIC_ATTRIBUTES_REQUEST_PATTERN      "v1/devices/me/attributes/request/%d"  //publish, $request_id
#define TB_MQTT_TOPIC_ATTRIBUTES_REQUEST_PREFIX       "v1/devices/me/attributes/request/"    //publish
#define TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PATTERN     "v1/devices/me/attributes/response/%d" //receive, $request_id
#define TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PREFIX      "v1/devices/me/attributes/response/"   //receive
#define TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_SUBSCRIRBE  "v1/devices/me/attributes/response/+"  //subscribe
// Subscribe to shared device attribute updates from the server
#define TB_MQTT_TOPIC_SHARED_ATTRIBUTES               "v1/devices/me/attributes"      //subscribe, receive
// Server-side RPC
#define TB_MQTT_TOPIC_SERVERRPC_REQUEST_PATTERN       "v1/devices/me/rpc/request/%d"  //receive, $request_id
#define TB_MQTT_TOPIC_SERVERRPC_REQUEST_PREFIX        "v1/devices/me/rpc/request/"    //receive
#define TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE     "v1/devices/me/rpc/request/+"   //subscribe
#define TB_MQTT_TOPIC_SERVERRPC_RESPONSE_PATTERN      "v1/devices/me/rpc/response/%d" //publish, $request_id
#define TB_MQTT_TOPIC_SERVERRPC_RESPONSE_PREFIX       "v1/devices/me/rpc/response/"   //publish
// Client-side RPC
#define TB_MQTT_TOPIC_CLIENTRPC_REQUEST_PATTERN       "v1/devices/me/rpc/request/%d"  //publish, $request_id
#define TB_MQTT_TOPIC_CLIENTRPC_REQUEST_PREFIX        "v1/devices/me/rpc/request/"    //publish
#define TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PATTERN      "v1/devices/me/rpc/response/%d" //receive, $request_id
#define TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PREFIX       "v1/devices/me/rpc/response/"   //receive
#define TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE    "v1/devices/me/rpc/response/+"  //subscribe
// Claiming device using device-side key scenario: Not implemented yet
//#define TB_MQTT_TOPIC_DEVICE_CLAIM         "v1/devices/me/claim" //publish
// Device provisioning: Not implemented yet
//#define TB_MQTT_TOPIC_PROVISION_REQUESTC   "/provision/request"  //publish
//#define TB_MQTT_TOPIC_PROVISION_RESPONSE   "/provision/response" //subscribe, receive
// Firmware update
// receive fw_title, fw_version, fw_checksum, fw_checksum_algorithm shared attributes after the device subscribes to "v1/devices/me/attributes/response/+".
#define TB_MQTT_TOPIC_FW_REQUEST_PATTERN          "v2/fw/request/%d/chunk/%d"   //publish, ${requestId}, ${chunk}
#define TB_MQTT_TOPIC_FW_RESPONSE_PATTERN         "v2/fw/response/%d/chunk/%d"  //receive, ${requestId}, ${chunk}
#define TB_MQTT_TOPIC_FW_RESPONSE_PREFIX          "v2/fw/response/"             //receive, ${requestId}, ${chunk}
#define TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE       "v2/fw/response/+/chunk/+"    //subsribe

#define TB_MQTT_TEXT_ATTRIBUTES_REQUEST_CLIENTKEYS  "clientKeys"
#define TB_MQTT_TEXT_ATTRIBUTES_REQUEST_SHAREDKEYS  "sharedKeys"
#define TB_MQTT_TEXT_ATTRIBUTES_REQUEST_CLIENT      "client"
#define TB_MQTT_TEXT_ATTRIBUTES_REQUEST_SHARED      "shared"
#define TB_MQTT_TEXT_RPC_METHOD     "method"
#define TB_MQTT_TEXT_RPC_PARAMS     "params"
#define TB_MQTT_TEXT_RPC_RESULTS    "results"

#define TB_MQTT_SHAREDATTRBUTE_FW_TITLE "fw_title"
#define TB_MQTT_SHAREDATTRBUTE_FW_VERSION "fw_version"
#define TB_MQTT_SHAREDATTRBUTE_FW_CHECKSUM "fw_checksum"
#define TB_MQTT_SHAREDATTRBUTE_FW_CHECKSUM_ALG "fw_checksum_algorithm"

#define TB_MQTT_TIMEOUT (30) //second, Client-Side RPC timeout, Attributes Request timeout or fwupdate Request timeout

//====tbmc data========================================================================================================
/* Definitions for error constants. */
// #define TBMC_OK    0   /*!< tbmc_err_t value indicating success (no error) */
// #define TBMC_FAIL -1   /*!< Generic tbmc_err_t code indicating failure */

// #define TBMC_ERR_NO_MEM              0x101   /*!< Out of memory */
// #define TBMC_ERR_INVALID_ARG         0x102   /*!< Invalid argument */
// #define TBMC_ERR_INVALID_STATE       0x103   /*!< Invalid state */
// #define TBMC_ERR_INVALID_SIZE        0x104   /*!< Invalid size */
// #define TBMC_ERR_NOT_FOUND           0x105   /*!< Requested resource not found */
// #define TBMC_ERR_NOT_SUPPORTED       0x106   /*!< Operation or feature not supported */
// #define TBMC_ERR_TIMEOUT             0x107   /*!< Operation timed out */
// #define TBMC_ERR_INVALID_RESPONSE    0x108   /*!< Received response was invalid */
// #define TBMC_ERR_INVALID_CRC         0x109   /*!< CRC or checksum was invalid */
// #define TBMC_ERR_INVALID_VERSION     0x10A   /*!< Version was invalid */
// #define TBMC_ERR_INVALID_MAC         0x10B   /*!< MAC address was invalid */
// #define TBMC_ERR_NOT_FINISHED        0x10C   /*!< There are items remained to retrieve */

// typedef int tbmc_err_t;

#define TBMC_MALLOC   malloc
#define TBMC_FREE     free

//====tbmqttlientesp32.h-low===============================================================================================

typedef enum
{
  TBMC_STATE_DISCONNECTED = 0,
  TBMC_STATE_CONNECTING,
  TBMC_STATE_CONNECTED
} tbmc_state_t; //TBMQTT_STATE

typedef struct
{
  const bool log_rxtx_package; /*!< print Rx/Tx MQTT package */

  const char *uri;             /*!< Complete MQTT broker URI */
  const char *access_token;    /*!< Access Token */
  const char *cert_pem;        /*!< Reserved. Pointer to certificate data in PEM format for server verify (with SSL), default is NULL, not required to verify the server */
  const char *client_cert_pem; /*!< Reserved. Pointer to certificate data in PEM format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_key_pem` has to be provided. */
  const char *client_key_pem;  /*!< Reserved. Pointer to private key data in PEM format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_cert_pem` has to be provided. */
} tbmc_config_t;

/**
 * ThingsBoard MQTT Client handle
 */
//typedef tbmc_t *tbmc_handle_t;
typedef struct tbmc_client *tbmc_handle_t;

typedef void (*tbmc_on_connected_t)(void *context);                                                       // First receive
typedef void (*tbmc_on_disconnected_t)(void *context);                                                    // First receive
typedef void (*tbmc_on_sharedattr_received_t)(void *context, const char *payload, int len);               // First receive
typedef void (*tbmc_on_serverrpc_request_t)(void *context, int request_id, const char *payload, int len); // First receive

typedef void (*tbmc_on_response_t)(void *context, int request_id, const char *payload, int len); // First send
typedef void (*tbmc_on_timeout_t)(void *context, int request_id);                               // First send

typedef tbmc_on_response_t tbmc_on_attrrequest_response_t; // First send
typedef tbmc_on_timeout_t  tbmc_on_attrrequest_timeout_t; // First send
typedef tbmc_on_response_t tbmc_on_clientrpc_response_t; // First send
typedef tbmc_on_timeout_t  tbmc_on_clientrpc_timeout_t; // First send
typedef void (*tbmc_on_fwupdate_response_t)(void *context, int request_id, int chunk, const char *payload, int len); // First send
typedef tbmc_on_timeout_t tbmc_on_fwupdate_timeout_t;                                                        // First send

tbmc_handle_t tbmc_init(void);
void tbmc_destroy(tbmc_handle_t client_);
bool tbmc_connect(tbmc_handle_t client_, const tbmc_config_t *config,
                  void *context,
                  tbmc_on_connected_t on_connected,
                  tbmc_on_disconnected_t on_disconnected,
                  tbmc_on_sharedattr_received_t on_sharedattributes_received,
                  tbmc_on_serverrpc_request_t on_serverrpc_request);
void tbmc_disconnect(tbmc_handle_t client_);
bool tbmc_is_connected(tbmc_handle_t client_);
bool tbmc_is_connecting(tbmc_handle_t client_);
bool tbmc_is_disconnected(tbmc_handle_t client_);
tbmc_state_t tbmc_get_state(tbmc_handle_t client_);
void tbmc_check_timeout(tbmc_handle_t client_);

int tbmc_telemetry_publish(tbmc_handle_t client_, const char *telemetry,
                           int qos /*= 1*/, int retain /*= 0*/);
int tbmc_clientattributes_publish(tbmc_handle_t client_, const char *attributes,
                                  int qos /*= 1*/, int retain /*= 0*/);
int tbmc_attributes_request(tbmc_handle_t client_, const char *payload,
                            void *context,
                            tbmc_on_attrrequest_response_t on_attrrequest_response,
                            tbmc_on_attrrequest_timeout_t on_attrrequest_timeout,
                            int qos /*= 1*/, int retain /*= 0*/);
int tbmc_attributes_request_ex(tbmc_handle_t client_, const char *client_keys, const char *shared_keys,
                               void *context,
                               tbmc_on_attrrequest_response_t on_attrrequest_response,
                               tbmc_on_attrrequest_timeout_t on_attrrequest_timeout,
                               int qos /*= 1*/, int retain /*= 0*/);
int tbmc_serverrpc_response(tbmc_handle_t client_, int request_id, const char *response,
                            int qos /*= 1*/, int retain /*= 0*/);
int tbmc_clientrpc_request(tbmc_handle_t client_, const char *payload,
                           void *context,
                           tbmc_on_clientrpc_response_t on_clientrpc_response,
                           tbmc_on_clientrpc_timeout_t on_clientrpc_timeout,
                           int qos /*= 1*/, int retain /*= 0*/);
int tbmc_clientrpc_request_ex(tbmc_handle_t client_, const char *method, const char *params,
                              void *context,
                              tbmc_on_clientrpc_response_t on_clientrpc_response,
                              tbmc_on_clientrpc_timeout_t on_clientrpc_timeout,
                              int qos /*= 1*/, int retain /*= 0*/);
int tbmc_fwupdate_request(tbmc_handle_t client_, int request_id_, int chunk, const char *payload, //?payload
                          void *context,
                          tbmc_on_fwupdate_response_t on_fwupdate_response,
                          tbmc_on_fwupdate_timeout_t on_fwupdate_timeout,
                          int qos /*= 1*/, int retain /*= 0*/);

#define TBMC_TELEMETRY_PUBLISH(client, payload) \
          tbmc_telemetry_publish(client, payload, /*int qos =*/1, /*int retain =*/0)
#define TBMC_CLIENTATTRIBUTES_PUBLISH(client, payloady) \
          tbmc_clientattributes_publish(client, payload, /*int qos =*/1, /*int retain =*/0)
#define TBMC_ATTRIUTES_REQUEST(client, payload, context, on_attrrequest_response, on_attrrequest_timeout) \
          tbmc_attributes_request(client, payload, context, on_attrrequest_response, on_attrrequest_timeout, /*int qos =*/1, /*int retain =*/0)
#define TBMC_ATTRIUTES_REQUEST_EX(client, client_keys, shared_keys, context, on_attrrequest_response, on_attrrequest_timeout) \
          tbmc_attributes_request_ex(client, client_keys, shared_keys, context, on_attrrequest_response, on_attrrequest_timeout, /*int qos =*/1, /*int retain =*/0)
#define TBMC_SERVERRPC_RESPONSE(client, request_id, response) \
          tbmc_serverrpc_response(client, request_id, response, /*int qos =*/1, /*int retain =*/0)
#define TBMC_CLIENTRPC_REQUEST(client, payload, context, on_clientrpc_response, on_clientrpc_timeout) \
          tbmc_clientrpc_request(client, payload, context, on_clientrpc_response, on_clientrpc_timeout, /*int qos =*/1, /*int retain =*/0)
#define TBMC_CLIENTRPC_REQUEST_EX(client, method, params, context, on_clientrpc_response, on_clientrpc_timeout) \
          tbmc_clientrpc_request_ex(client, method, params, context, on_clientrpc_response, on_clientrpc_timeout, /*int qos =*/1, /*int retain =*/0)
#define TBMC_FW_REQUEST_SEND(client, request_id, chunk, payload) \
          tbmc_fw_request_send(client, request_id, chunk, payload, /*int qos =*/1, /*int retain =*/0)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
