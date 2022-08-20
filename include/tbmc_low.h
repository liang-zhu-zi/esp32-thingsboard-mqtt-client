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

#ifndef _TBMC_LOW_H_
#define _TBMC_LOW_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//====ThingsBoard MQTT message example=================================================================================
//send telemetry datapoints:
//Topic: 'v1/devices/me/telemetry'
//Data:  '{"key1":"value1", "key2":true, "key3": 3.0, "key4": 4}', '[{"key1":"value1"}, {"key2":true}]'

//send client-side attributes:
//Topic: 'v1/devices/me/attributes'
//Data:  '{"attribute1":"value1", "attribute2":true, "attribute3":42.0, "attribute4":73}'

//request attributes:
//Topic: 'v1/devices/me/attributes/request/$request_id'
//Data:  '{"clientKeys":"attribute1,attribute2", "sharedKeys":"shared1,shared2"}'
//+
//attributes response:
//Topic:
//Data:  '{"client":{"controlMode":"On","floorTempLimited":27.5,"adaptiveControl":true},"shared":{"timezone":480,"syncTimeFreq":86400}}'

//send client-side RPC:
//Topic: 'v1/devices/me/rpc/request/$request_id'
//Data:  '{"method":"getTime","params":{}}'
//+
//receive client-side RPC Response:
//Topic: 
//Data:  '{"method":"getTime","results":{"utcDateime":"2020-05-29T08:02:30Z","utcTimestamp":1590739350}}'

//receive server-side RPC request:
//topic:    "v1/devices/me/rpc/request/$request_id" 
//payload:  {"method":"remoteSetChangeOverTempHeating", "params":25.5}
//+
//send server-side RPC response:
//Topic: 'v1/devices/me/rpc/response/$request_id'
//Data:  '{"example_response":23.1}' ???

//====tbmc data========================================================================================================
/* Definitions for error constants. */
#define TBMC_OK    0 /*!< tbmc_err_t value indicating success (no error) */
#define TBMC_FAIL -1 /*!< Generic tbmc_err_t code indicating failure */

#define TBMC_ERR_NO_MEM                 0x101   /*!< Out of memory */
#define TBMC_ERR_INVALID_ARG            0x102   /*!< Invalid argument */
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

typedef int tbmc_err_t;

//====ThingsBoard MQTT topic===========================================================================================
// Publish Telemetry data
#define TBMQTT_TOPIC_TELEMETRY_PUBLISH             "v1/devices/me/telemetry" //publish
// Publish client-side device attributes to the server
#define TBMQTT_TOPIC_CLIENT_ATTRIBUTES_PUBLISH     "v1/devices/me/attributes" //publish
// Request client-side or shared device attributes from the server
#define TBMQTT_TOPIC_CLIENT_SHARED_ATTRIBUTES_REQUEST_PATTERN     "v1/devices/me/attributes/request/%d"  //publish, $request_id
#define TBMQTT_TOPIC_CLIENT_SHARED_ATTRIBUTES_REQUEST_PREFIX      "v1/devices/me/attributes/request/"    //publish
#define TBMQTT_TOPIC_CLIENT_SHARED_ATTRIBUTES_RESPONSE_PATTERN    "v1/devices/me/attributes/response/%d" //receive, $request_id
#define TBMQTT_TOPIC_CLIENT_SHARED_ATTRIBUTES_RESPONSE_PREFIX     "v1/devices/me/attributes/response/"   //receive
#define TBMQTT_TOPIC_CLIENT_SHARED_ATTRIBUTES_RESPONSE_SUBSCRIRBE "v1/devices/me/attributes/response/+"  //subscribe
// Subscribe to shared device attribute updates from the server
#define TBMQTT_TOPIC_SHARED_ATTRIBUTES             "v1/devices/me/attributes"      //subscribe, receive
// Server-side RPC
#define TBMQTT_TOPIC_SERVER_RPC_REQUEST_PATTERN    "v1/devices/me/rpc/request/%d"  //receive, $request_id
#define TBMQTT_TOPIC_SERVER_RPC_REQUEST_PREFIX     "v1/devices/me/rpc/request/"    //receive
#define TBMQTT_TOPIC_SERVER_RPC_REQUEST_SUBSCRIBE  "v1/devices/me/rpc/request/+"   //subscribe
#define TBMQTT_TOPIC_SERVER_RPC_RESPONSE_PATTERN   "v1/devices/me/rpc/response/%d" //publish, $request_id
#define TBMQTT_TOPIC_SERVER_RPC_RESPONSE_PREFIX    "v1/devices/me/rpc/response/"   //publish
// Client-side RPC
#define TBMQTT_TOPIC_CLIENT_RPC_REQUEST_PATTERN    "v1/devices/me/rpc/request/%d"  //publish, $request_id
#define TBMQTT_TOPIC_CLIENT_RPC_REQUEST_PREFIX     "v1/devices/me/rpc/request/"    //publish
#define TBMQTT_TOPIC_CLIENT_RPC_RESPONSE_PATTERN   "v1/devices/me/rpc/response/%d" //receive, $request_id
#define TBMQTT_TOPIC_CLIENT_RPC_RESPONSE_PREFIX    "v1/devices/me/rpc/response/"   //receive
#define TBMQTT_TOPIC_CLIENT_RPC_RESPONSE_SUBSCRIBE "v1/devices/me/rpc/response/+"  //subscribe
// Claiming device using device-side key scenario: Not implemented yet
//#define TBMQTT_TOPIC_DEVICE_CLAIM         "v1/devices/me/claim" //publish
// Device provisioning: Not implemented yet
//#define TBMQTT_TOPIC_PROVISION_REQUESTC   "/provision/request"  //publish
//#define TBMQTT_TOPIC_PROVISION_RESPONSE   "/provision/response" //subscribe, receive
// Firmware update
// receive fw_title, fw_version, fw_checksum, fw_checksum_algorithm shared attributes after the device subscribes to "v1/devices/me/attributes/response/+".
#define TBMQTT_TOPIC_FW_REQUEST_PATTERN     "v2/fw/request/%d/chunk/%d"  //publish, ${requestId}, ${chunk}
#define TBMQTT_TOPIC_FW_RESPONSE_PATTERN    "v2/fw/response/%d/chunk/%d" //receive ${requestId}, ${chunk}
#define TBMQTT_TOPIC_FW_RESPONSE_SUBSCRIBE  "v2/fw/response/+/chunk/+"   //subsribe

//====tbmqttlientesp32.h-low===============================================================================================
typedef enum
{
  TBMQTT_CLIENT_STATE_DISCONNECTED = 0,
  TBMQTT_CLIENT_STATE_CONNECTING,
  TBMQTT_CLIENT_STATE_CONNECTED
} TBMQTT_CLIENT_STATE; //TBMQTT_STATE

/**
 * ThingsBoard MQTT Client handle
 */
typedef tbmqtt_client_t *tbmqtt_client_handle_t;

typedef void (*TBMQTT_CLIENT_ON_CONNECTED)(void* context);
typedef void (*TBMQTT_CLIENT_ON_DISCONNECTED)(void* context);
typedef void (*TBMQTT_CLIENT_ON_SHAREDATTR_RECEIVED)(void* context, const char *payload, int len);
typedef void (*TBMQTT_CLIENT_ON_ATTRREQUEST_SUCCESS)(void* context, int request_id, const char *payload, int len);
typedef void (*TBMQTT_CLIENT_ON_ATTRREQUEST_TIMEOUT)(void* context, int request_id);
typedef void (*TBMQTT_CLIENT_ON_CLIENTRPC_SUCCESS)(void* context, int request_id, const char *payload, int len);
typedef void (*TBMQTT_CLIENT_ON_CLIENTRPC_TIMEOUT)(void* context, int request_id);
typedef void (*TBMQTT_CLIENT_ON_SERVERRPC_REQUEST)(void* context, int request_id, const char *payload, int len);
typedef void (*TBMQTT_CLIENT_ON_FWREQUEST_RESPONSE)(void* context, int request_id, const char* payload, int len);
typedef void (*TBMQTT_CLIENT_ON_FWREQUEST_TIMEOUT)(void* context, int request_id);

typedef struct
{
    const char *uri;
    const char *access_token;
    const char *cert_pem;        // default is NULL
    const char *client_cert_pem; // default is NULL
    const char *client_key_pem;  // default is NULL

    TBMQTT_CLIENT_ON_CONNECTED on_connected;
    TBMQTT_CLIENT_ON_DISCONNECTED on_disonnected;
    TBMQTT_CLIENT_ON_SHAREDATTR_RECEIVED on_sharedattributes_received;
    //TBMQTT_CLIENT_ON_ATTRREQUEST_SUCCESS on_attributesrequest_success;
    //TBMQTT_CLIENT_ON_ATTRREQUEST_TIMEOUT on_attributesrequest_timeout;
    //TBMQTT_CLIENT_ON_CLIENTRPC_SUCCESS on_clientrpc_success;
    //TBMQTT_CLIENT_ON_CLIENTRPC_TIMEOUT on_clientrpc_timeout;
    TBMQTT_CLIENT_ON_SERVERRPC_REQUEST on_serverrpc_request;
    //TBMQTT_CLIENT_ON_FWREQUEST_RESPONSE on_fwrequest_response;
    //TBMQTT_CLIENT_ON_FWREQUEST_TIMEOUT on_fwrequest_timeout;
} tbmqtt_client_config_t;

tbmqtt_client_handle_t tbmqtt_client_init(void);
tbmc_err_t tbmqtt_client_destroy(tbmqtt_client_handle_t client);
bool tbmqtt_client_connect(tbmqtt_client_handle_t client, tbmqtt_client_config_t *config); // connect()//...start()
                            // tbmqtt_client_set_OnConnected(TBMQTT_ON_CONNECTED callback); //merge to tbmqtt_client_connect()
                            // tbmqtt_client_set_OnDisconnected(TBMQTT_ON_DISCONNECTED callback); //merge to tbmqtt_client_connect()
                            // tbmqtt_client_set_OnServerRpcRequest(TBMQTT_SERVER_RPC_CALLBACK callback); //merge to tbmqtt_client_connect()
                            // tbmqtt_client_set_OnAttrSubReply(TBMQTT_ATTR_SUB_CALLBACK callback); //merge to tbmqtt_client_connect()
                            // tbmqtt_client_get_OnConnected(void); //merge to tbmqtt_client_connect()
                            // tbmqtt_client_get_OnDisconnected(void); //merge to tbmqtt_client_connect()
                            // tbmqtt_client_get_OnServerRpcRequest(void); //merge to tbmqtt_client_connect()
                            // tbmqtt_client_get_OnAttrSubReply(void); //merge to tbmqtt_client_connect()
void tbmqtt_client_disconnect(tbmqtt_client_handle_t client); // disconnect()//...stop()
bool tbmqtt_client_is_connected(tbmqtt_client_handle_t client); //isConnected
bool tbmqtt_client_is_connecting(tbmqtt_client_handle_t client);
bool tbmqtt_client_is_disconnected(tbmqtt_client_handle_t client);
TBMQTT_CLIENT_STATE tbmqtt_client_get_state(tbmqtt_client_handle_t client);
void tbmqtt_client_check_timeout(tbmqtt_client_handle_t client);  // Executes an event loop for PubSub client. //loop()==>checkTimeout()

int tbmqtt_client_telemetyr_publish(tbmqtt_client_handle_t client, const char* pyaload, int qos=1, int retain=0); //sendTelemetry()
int tbmqtt_client_attributes_publish(tbmqtt_client_handle_t client, const char* pyaload, int qos=1, int retain=0); //sendAttributes() //publish client attributes
int tbmqtt_client_attributes_request(tbmqtt_client_handle_t client, const char *payload, /*const char *client_keys = NULL, const char *shared_keys = NULL,*/
                                    void *context,
                                    TBMQTT_CLIENT_ON_ATTRREQUEST_SUCCESS on_attributesrequest_success,
                                    TBMQTT_CLIENT_ON_ATTRREQUEST_TIMEOUT on_attributesrequest_timeout,
                                    int qos = 1, int retain = 0); // requestAttributes() //request client and shared attributes
int tbmqtt_client_serverrpc_response(tbmqtt_client_handle_t client, int request_id, const char* response, int qos=1, int retain=0); //sendServerRpcReply() //response server-side RPC
int tbmqtt_client_clientrpc_request(tbmqtt_client_handle_t client, const char *payload, /*const char* method, const char* params,*/
                                    void *context,
                                    TBMQTT_CLIENT_ON_CLIENTRPC_SUCCESS on_clientrpc_success, // TODO???
                                    TBMQTT_CLIENT_ON_CLIENTRPC_TIMEOUT on_clientrpc_timeout, // TODO???
                                    int qos = 1, int retain = 0);                            // sendClientRpcCall() //request client-side RPC
int fw_request(tbmqtt_client_handle_t client, const char *payload,
                             void *context,
                             TBMQTT_CLIENT_ON_FWREQUEST_RESPONSE on_fwrequest_response,
                             TBMQTT_CLIENT_ON_FWREQUEST_TIMEOUT on_fwrequest_timeout,
                             int qos = 1, int retain = 0);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
