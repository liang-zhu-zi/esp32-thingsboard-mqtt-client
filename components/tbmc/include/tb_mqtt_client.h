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

//send provison requeset:
//Topic:  '/provision/request'
//Data:   '{
//            "deviceName": "DEVICE_NAME",
//            "provisionDeviceKey": "PUT_PROVISION_KEY_HERE",
//            "provisionDeviceSecret": "PUT_PROVISION_SECRET_HERE",
//            "credentialsType": "ACCESS_TOKEN",
//            "token": "DEVICE_ACCESS_TOKEN"
//          }'
//+
//receive provison Response:
//Topic:  '/provision/response'
//Data:   '{
//            "credentialsType":"ACCESS_TOKEN",
//            "credentialsValue":"DEVICE_ACCESS_TOKEN",
//            "status":"SUCCESS"
//         }'

//receive server-side RPC request:
//topic:  'v1/devices/me/rpc/request/$request_id'
//Data:   '{"method":"remoteSetChangeOverTempHeating", "params":25.5}'
//+
//send server-side RPC response:
//Topic:  'v1/devices/me/rpc/response/$request_id'
//Data:   '{"example_response":23.1}' ???

// ======== ThingsBoard MQTT topic===========================================================
// ======== Publish Telemetry data===========================================================
#define TB_MQTT_TOPIC_TELEMETRY_PUBLISH               "v1/devices/me/telemetry"   //publish

// ======== Publish client-side device attributes to the server==============================
#define TB_MQTT_TOPIC_CLIENT_ATTRIBUTES_PUBLISH       "v1/devices/me/attributes"  //publish

// ======== Request client-side or shared device attributes from the server==================
#define TB_MQTT_TOPIC_ATTRIBUTES_REQUEST_PATTERN      "v1/devices/me/attributes/request/%d"  //publish, $request_id
#define TB_MQTT_TOPIC_ATTRIBUTES_REQUEST_PREFIX       "v1/devices/me/attributes/request/"    //publish
#define TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PATTERN     "v1/devices/me/attributes/response/%d" //receive, $request_id
#define TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PREFIX      "v1/devices/me/attributes/response/"   //receive
#define TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_SUBSCRIRBE  "v1/devices/me/attributes/response/+"  //subscribe

#define TB_MQTT_TEXT_ATTRIBUTES_REQUEST_CLIENTKEYS    "clientKeys"
#define TB_MQTT_TEXT_ATTRIBUTES_REQUEST_SHAREDKEYS    "sharedKeys"
#define TB_MQTT_TEXT_ATTRIBUTES_REQUEST_CLIENT        "client"
#define TB_MQTT_TEXT_ATTRIBUTES_REQUEST_SHARED        "shared"

// ======== Subscribe to shared device attribute updates from the server=====================
#define TB_MQTT_TOPIC_SHARED_ATTRIBUTES               "v1/devices/me/attributes"      //subscribe, receive

// ======== Server-side RPC==================================================================
#define TB_MQTT_TOPIC_SERVERRPC_REQUEST_PATTERN       "v1/devices/me/rpc/request/%d"  //receive, $request_id
#define TB_MQTT_TOPIC_SERVERRPC_REQUEST_PREFIX        "v1/devices/me/rpc/request/"    //receive
#define TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE     "v1/devices/me/rpc/request/+"   //subscribe
#define TB_MQTT_TOPIC_SERVERRPC_RESPONSE_PATTERN      "v1/devices/me/rpc/response/%d" //publish, $request_id
#define TB_MQTT_TOPIC_SERVERRPC_RESPONSE_PREFIX       "v1/devices/me/rpc/response/"   //publish

// ======== Client-side RPC==================================================================
#define TB_MQTT_TOPIC_CLIENTRPC_REQUEST_PATTERN       "v1/devices/me/rpc/request/%d"  //publish, $request_id
#define TB_MQTT_TOPIC_CLIENTRPC_REQUEST_PREFIX        "v1/devices/me/rpc/request/"    //publish
#define TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PATTERN      "v1/devices/me/rpc/response/%d" //receive, $request_id
#define TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PREFIX       "v1/devices/me/rpc/response/"   //receive
#define TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE    "v1/devices/me/rpc/response/+"  //subscribe

#define TB_MQTT_TEXT_RPC_METHOD     "method"
#define TB_MQTT_TEXT_RPC_PARAMS     "params"
#define TB_MQTT_TEXT_RPC_RESULTS    "results"

// ======== Claiming device using device-side key scenario===================================
#define TB_MQTT_TOPIC_CLAIMING_DEVICE       "v1/devices/me/claim" //publish

#define TB_MQTT_CLAIMING_DEVICE_SECRETKEY   "secretKey"
#define TB_MQTT_CLAIMING_DEVICE_DURATIONMS  "durationMs"

// ======== Device provisioning==============================================================
#define TB_MQTT_TOPIC_PROVISION_REQUESTC    "/provision/request"  //publish
#define TB_MQTT_TOPIC_PROVISION_RESPONSE    "/provision/response" //subscribe, receive

//Key: request
#define TB_MQTT_KEY_PROVISION_DEVICE_NAME               "deviceName"            //Device name in ThingsBoard
#define TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY      "provisionDeviceKey"	//Provisioning device key, you should take it from configured device profile
#define TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET   "provisionDeviceSecret"	//Provisioning device secret, you should take it from configured device profile
#define TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE          "credentialsType"	    //Credentials type parameter.
//Key: request: ACCESS TOKEN
#define TB_MQTT_KEY_PROVISION_TOKEN                     "token"	    //Access token for device in ThingsBoard.
//Key: request: MQTT_BASIC
#define TB_MQTT_KEY_PROVISION_USERNAME                  "username"  //Username for device in ThingsBoard
#define TB_MQTT_KEY_PROVISION_USERNAME2                 "userName"  //In response of Devices supplies Basic MQTT Credentials
#define TB_MQTT_KEY_PROVISION_PASSWORD                  "password"  //Password for device in ThingsBoard
#define TB_MQTT_KEY_PROVISION_CLIENT_ID                 "clientId"  //Client id for device in ThingsBoard
//Key: request: X509_CERTIFICATE
#define TB_MQTT_KEY_PROVISION_HASH                      "hash"	    //Public key X509 hash for device in ThingsBoard.
//Key: response
#define TB_MQTT_KEY_PROVISION_DEVICE_ID                 "deviceId"
#define TB_MQTT_KEY_PROVISION_CREDENTIALS_ID            "credentialsId"
#define TB_MQTT_KEY_PROVISION_CREDENTIALS_VALUE         "credentialsValue"
#define TB_MQTT_KEY_PROVISION_STATUS                    "status"
#define TB_MQTT_KEY_PROVISION_DEVICE_STATUS             "provisionDeviceStatus"
//Value
#define TB_MQTT_VALUE_PROVISION_ACCESS_TOKEN        "ACCESS_TOKEN"      //TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE
#define TB_MQTT_VALUE_PROVISION_MQTT_BASIC          "MQTT_BASIC"        //TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE
#define TB_MQTT_VALUE_PROVISION_X509_CERTIFICATE    "X509_CERTIFICATE"  //TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE
#define TB_MQTT_VALUE_PROVISION_SUCCESS             "SUCCESS"           //TB_MQTT_KEY_PROVISION_STATUS

// ======== Firmware update =================================================================
// receive some shared attributes after the device subscribes to "v1/devices/me/attributes/response/+":
//         fw_title, fw_version, fw_size, fw_checksum, fw_checksum_algorithm,
//         sw_title, sw_version, sw_size, sw_checksum, sw_checksum_algorithm
#define TB_MQTT_TOPIC_FW_REQUEST_PATTERN        "v2/fw/request/%d/chunk/%d"   //publish, ${requestId}, ${chunkId}
#define TB_MQTT_TOPIC_FW_RESPONSE_PATTERN       "v2/fw/response/%d/chunk/"    //receive, ${requestId}
#define TB_MQTT_TOPIC_FW_RESPONSE_PREFIX        "v2/fw/response/"             //receive, ${requestId}, ${chunkId}
#define TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE     "v2/fw/response/+/chunk/+"    //subsribe

#define TB_MQTT_SHAREDATTRBUTE_FW_TITLE         "fw_title"                  //shared attribute
#define TB_MQTT_SHAREDATTRBUTE_FW_VERSION       "fw_version"                //shared attribute
#define TB_MQTT_SHAREDATTRBUTE_FW_SIZE          "fw_size"                   //shared attribute
#define TB_MQTT_SHAREDATTRBUTE_FW_CHECKSUM      "fw_checksum"               //shared attribute
#define TB_MQTT_SHAREDATTRBUTE_FW_CHECKSUM_ALG  "fw_checksum_algorithm"     //shared attribute

#define TB_MQTT_SHAREDATTRBUTE_SW_TITLE         "sw_title"                  //shared attribute
#define TB_MQTT_SHAREDATTRBUTE_SW_VERSION       "sw_version"                //shared attribute
#define TB_MQTT_SHAREDATTRBUTE_SW_SIZE          "sw_size"                   //shared attribute
#define TB_MQTT_SHAREDATTRBUTE_SW_CHECKSUM      "sw_checksum"               //shared attribute
#define TB_MQTT_SHAREDATTRBUTE_SW_CHECKSUM_ALG  "sw_checksum_algorithm"     //shared attribute

#define TB_MQTT_TELEMETRY_CURRENT_FW_TITLE      "current_fw_title"          //telemetry
#define TB_MQTT_TELEMETRY_CURRENT_FW_VERSION    "current_fw_version"        //telemetry
#define TB_MQTT_TELEMETRY_FW_STATE              "fw_state"                  //telemetry
#define TB_MQTT_TELEMETRY_FW_ERROR              "fw_error"                  //telemetry

#define TB_MQTT_TELEMETRY_CURRENT_SW_TITLE      "current_sw_title"          //telemetry
#define TB_MQTT_TELEMETRY_CURRENT_SW_VERSION    "current_sw_version"        //telemetry
#define TB_MQTT_TELEMETRY_SW_STATE              "sw_state"                  //telemetry
#define TB_MQTT_TELEMETRY_SW_ERROR              "sw_error"                  //telemetry

#define TB_MQTT_TELEMETRY_FW_SW_STATE_DOWNLOADING "DOWNLOADING" // telemetry value of fw/sw state - notification about new firmware/software update was received and device started downloading the update package.
#define TB_MQTT_TELEMETRY_FW_SW_STATE_DOWNLOADED  "DOWNLOADED"  // telemetry value of fw/sw state - device completed downloading of the update package.
#define TB_MQTT_TELEMETRY_FW_SW_STATE_VERIFIED    "VERIFIED"    // telemetry value of fw/sw state - device verified the checksum of the downloaded package.
#define TB_MQTT_TELEMETRY_FW_SW_STATE_UPDATING    "UPDATING"    // telemetry value of fw/sw state - device started the firmware/software update. Typically is sent before reboot of the device or restart of the service.
#define TB_MQTT_TELEMETRY_FW_SW_STATE_UPDATED     "UPDATED"     // telemetry value of fw/sw state - the firmware was successfully updated to the next version.
#define TB_MQTT_TELEMETRY_FW_SW_STATE_FAILED      "FAILED"      // telemetry value of fw/sw state - checksum wasn’t verified, or the device failed to update. See “Device failed” tab on the Firmware dashboard for more details.

// only support CRC32
//#define TB_MQTTT_FW_SW_CHECKSUM_ALG_SHA256      "sha256"
//#define TB_MQTTT_FW_SW_CHECKSUM_ALG_SHA384      "sha384"
//#define TB_MQTTT_FW_SW_CHECKSUM_ALG_SHA512      "sha512"
//#define TB_MQTTT_FW_SW_CHECKSUM_ALG_SHAMD5      "md5"
//#define TB_MQTTT_FW_SW_CHECKSUM_ALG_MURMUR3_32  "murmur3_32"
//#define TB_MQTTT_FW_SW_CHECKSUM_ALG_MURMUR3_128 "murmur3_128"
#define TB_MQTTT_FW_SW_CHECKSUM_ALG_CRC32       "crc32"         // TB_MQTT_SHAREDATTRBUTE_FW_CHECKSUM_ALG or TB_MQTT_SHAREDATTRBUTE_SW_CHECKSUM_ALG

//second, Client-Side RPC timeout, Attributes Request timeout or otaupdate Request timeout
#define TB_MQTT_TIMEOUT (30) 

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
typedef void (*tbmc_on_otaupdate_response_t)(void *context, int request_id, int chunk_id, const char *payload, int len); // First send
typedef tbmc_on_timeout_t tbmc_on_otaupdate_timeout_t;                                                        // First send
typedef void (*tbmc_on_provision_response_t)(void *context, int request_id, const char *payload, int len);
typedef tbmc_on_timeout_t tbmc_on_provision_timeout_t;

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

int tbmc_claiming_device_publish(tbmc_handle_t client_, const char *claiming,
                         int qos /*= 1*/, int retain /*= 0*/);

int tbmc_provision_request(tbmc_handle_t client_, const char *payload,
                          void *context,
                          tbmc_on_provision_response_t on_provision_response,
                          tbmc_on_provision_timeout_t on_provision_timeout,
                          int qos /*= 1*/, int retain /*= 0*/);

int tbmc_otaupdate_request(tbmc_handle_t client_,
                          int request_id_, int chunk_id, const char *payload, //?payload
                          void *context,
                          tbmc_on_otaupdate_response_t on_otaupdate_response,
                          tbmc_on_otaupdate_timeout_t on_otaupdate_timeout,
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
#define TBMC_OTA_REQUEST_SEND(client, request_id, chunk_id, payload) \
          tbmc_ota_request_send(client, request_id, chunk_id, payload, /*int qos =*/1, /*int retain =*/0)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
