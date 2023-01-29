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

// ThingsBoard Client MQTT Protocol API

#ifndef _TBC_MQTT_PROTOCOL_H_
#define _TBC_MQTT_PROTOCOL_H_

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

//send provision requeset:
//Topic:  '/provision/request'
//Data:   '{
//            "deviceName": "DEVICE_NAME",
//            "provisionDeviceKey": "PUT_PROVISION_KEY_HERE",
//            "provisionDeviceSecret": "PUT_PROVISION_SECRET_HERE",
//            "credentialsType": "ACCESS_TOKEN",
//            "token": "DEVICE_ACCESS_TOKEN"
//          }'
//+
//receive provision Response:
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
#define TB_MQTT_TOPIC_ATTRIBUTES_REQUEST_PATTERN      "v1/devices/me/attributes/request/%u"  //publish, $request_id
#define TB_MQTT_TOPIC_ATTRIBUTES_REQUEST_PREFIX       "v1/devices/me/attributes/request/"    //publish
#define TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PATTERN     "v1/devices/me/attributes/response/%u" //receive, $request_id
#define TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PREFIX      "v1/devices/me/attributes/response/"   //receive
#define TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_SUBSCRIBE   "v1/devices/me/attributes/response/+"  //subscribe

#define TB_MQTT_KEY_ATTRIBUTES_REQUEST_CLIENTKEYS     "clientKeys"
#define TB_MQTT_KEY_ATTRIBUTES_REQUEST_SHAREDKEYS     "sharedKeys"
#define TB_MQTT_KEY_ATTRIBUTES_RESPONSE_CLIENT        "client"
#define TB_MQTT_KEY_ATTRIBUTES_RESPONSE_SHARED        "shared"

// ======== Subscribe to shared device attribute updates from the server=====================
#define TB_MQTT_TOPIC_SHARED_ATTRIBUTES               "v1/devices/me/attributes"      //subscribe, receive

// ======== Server-side RPC==================================================================
#define TB_MQTT_TOPIC_SERVERRPC_REQUEST_PATTERN       "v1/devices/me/rpc/request/%u"  //receive, $request_id
#define TB_MQTT_TOPIC_SERVERRPC_REQUEST_PREFIX        "v1/devices/me/rpc/request/"    //receive
#define TB_MQTT_TOPIC_SERVERRPC_REQUEST_SUBSCRIBE     "v1/devices/me/rpc/request/+"   //subscribe
#define TB_MQTT_TOPIC_SERVERRPC_RESPONSE_PATTERN      "v1/devices/me/rpc/response/%u" //publish, $request_id
#define TB_MQTT_TOPIC_SERVERRPC_RESPONSE_PREFIX       "v1/devices/me/rpc/response/"   //publish

// ======== Client-side RPC==================================================================
#define TB_MQTT_TOPIC_CLIENTRPC_REQUEST_PATTERN       "v1/devices/me/rpc/request/%u"  //publish, $request_id
#define TB_MQTT_TOPIC_CLIENTRPC_REQUEST_PREFIX        "v1/devices/me/rpc/request/"    //publish
#define TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PATTERN      "v1/devices/me/rpc/response/%u" //receive, $request_id
#define TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PREFIX       "v1/devices/me/rpc/response/"   //receive
#define TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_SUBSCRIBE    "v1/devices/me/rpc/response/+"  //subscribe

#define TB_MQTT_KEY_RPC_METHOD     "method"
#define TB_MQTT_KEY_RPC_PARAMS     "params"
#define TB_MQTT_KEY_RPC_RESULTS    "results"

// ======== Claiming device using device-side key scenario===================================
#define TB_MQTT_TOPIC_CLAIMING_DEVICE           "v1/devices/me/claim" //publish

#define TB_MQTT_KEY_CLAIMING_DEVICE_SECRETKEY   "secretKey"
#define TB_MQTT_KEY_CLAIMING_DEVICE_DURATIONMS  "durationMs"

// ======== Device provisioning==============================================================
#define TB_MQTT_TOPIC_PROVISION_REQUESTC    "/provision/request"  //publish
#define TB_MQTT_TOPIC_PROVISION_RESPONSE    "/provision/response" //subscribe, receive

#define TB_MQTT_PARAM_PROVISION_USERNAME    "provision"           //MQTT username for provision

//Key: request
#define TB_MQTT_KEY_PROVISION_DEVICE_NAME               "deviceName"            //Device name in ThingsBoard
#define TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY      "provisionDeviceKey"	//Provisioning device key, you should take it from configured device profile
#define TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET   "provisionDeviceSecret"	//Provisioning device secret, you should take it from configured device profile
#define TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE          "credentialsType"	    //Credentials type parameter.
//Key: request: ACCESS TOKEN
#define TB_MQTT_KEY_PROVISION_TOKEN                     "token"	    //Access token for device in ThingsBoard.
//Key: request: MQTT_BASIC
#define TB_MQTT_KEY_PROVISION_CLIENT_ID                 "clientId"  //Client id for device in ThingsBoard
#define TB_MQTT_KEY_PROVISION_USERNAME                  "username"  //Username for device in ThingsBoard
#define TB_MQTT_KEY_PROVISION_USERNAME2                 "userName"  //In response of Devices supplies Basic MQTT Credentials
#define TB_MQTT_KEY_PROVISION_PASSWORD                  "password"  //Password for device in ThingsBoard
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
#define TB_MQTT_TOPIC_FW_REQUEST_PATTERN        "v2/fw/request/%u/chunk/%u"   //publish, ${requestId}, ${chunkId}
#define TB_MQTT_TOPIC_FW_RESPONSE_PATTERN       "v2/fw/response/%u/chunk/"    //receive, ${requestId}
#define TB_MQTT_TOPIC_FW_RESPONSE_PREFIX        "v2/fw/response/"             //receive, ${requestId}, ${chunkId}
#define TB_MQTT_TOPIC_FW_RESPONSE_SUBSCRIBE     "v2/fw/response/+/chunk/+"    //subsribe

#define TB_MQTT_KEY_FW_TITLE         "fw_title"                  //shared attribute
#define TB_MQTT_KEY_FW_VERSION       "fw_version"                //shared attribute
#define TB_MQTT_KEY_FW_SIZE          "fw_size"                   //shared attribute
#define TB_MQTT_KEY_FW_CHECKSUM      "fw_checksum"               //shared attribute
#define TB_MQTT_KEY_FW_CHECKSUM_ALG  "fw_checksum_algorithm"     //shared attribute

#define TB_MQTT_KEY_SW_TITLE         "sw_title"                  //shared attribute
#define TB_MQTT_KEY_SW_VERSION       "sw_version"                //shared attribute
#define TB_MQTT_KEY_SW_SIZE          "sw_size"                   //shared attribute
#define TB_MQTT_KEY_SW_CHECKSUM      "sw_checksum"               //shared attribute
#define TB_MQTT_KEY_SW_CHECKSUM_ALG  "sw_checksum_algorithm"     //shared attribute

#define TB_MQTT_KEY_CURRENT_FW_TITLE      "current_fw_title"          //telemetry
#define TB_MQTT_KEY_CURRENT_FW_VERSION    "current_fw_version"        //telemetry
#define TB_MQTT_KEY_FW_STATE              "fw_state"                  //telemetry
#define TB_MQTT_KEY_FW_ERROR              "fw_error"                  //telemetry

#define TB_MQTT_KEY_CURRENT_SW_TITLE      "current_sw_title"          //telemetry
#define TB_MQTT_KEY_CURRENT_SW_VERSION    "current_sw_version"        //telemetry
#define TB_MQTT_KEY_SW_STATE              "sw_state"                  //telemetry
#define TB_MQTT_KEY_SW_ERROR              "sw_error"                  //telemetry

//TB_MQTT_KEY_FW_STATE or TB_MQTT_KEY_SW_STATE
#define TB_MQTT_VALUE_FW_SW_STATE_DOWNLOADING "DOWNLOADING" // telemetry value of fw/sw state - notification about new firmware/software update was received and device started downloading the update package.
#define TB_MQTT_VALUE_FW_SW_STATE_DOWNLOADED  "DOWNLOADED"  // telemetry value of fw/sw state - device completed downloading of the update package.
#define TB_MQTT_VALUE_FW_SW_STATE_VERIFIED    "VERIFIED"    // telemetry value of fw/sw state - device verified the checksum of the downloaded package.
#define TB_MQTT_VALUE_FW_SW_STATE_UPDATING    "UPDATING"    // telemetry value of fw/sw state - device started the firmware/software update. Typically is sent before reboot of the device or restart of the service.
#define TB_MQTT_VALUE_FW_SW_STATE_UPDATED     "UPDATED"     // telemetry value of fw/sw state - the firmware was successfully updated to the next version.
#define TB_MQTT_VALUE_FW_SW_STATE_FAILED      "FAILED"      // telemetry value of fw/sw state - checksum wasn’t verified, or the device failed to update. See “Device failed” tab on the Firmware dashboard for more details.

//TB_MQTT_KEY_FW_CHECKSUM_ALG or TB_MQTT_KEY_SW_CHECKSUM_ALG
// only support CRC32
//#define TB_MQTTT_VALUE_FW_SW_CHECKSUM_ALG_SHA256      "sha256"
//#define TB_MQTTT_VALUE_FW_SW_CHECKSUM_ALG_SHA384      "sha384"
//#define TB_MQTTT_VALUE_FW_SW_CHECKSUM_ALG_SHA512      "sha512"
//#define TB_MQTTT_VALUE_FW_SW_CHECKSUM_ALG_SHAMD5      "md5"
//#define TB_MQTTT_VALUE_FW_SW_CHECKSUM_ALG_MURMUR3_32  "murmur3_32"
//#define TB_MQTTT_VALUE_FW_SW_CHECKSUM_ALG_MURMUR3_128 "murmur3_128"
#define TB_MQTTT_VALUE_FW_SW_CHECKSUM_ALG_CRC32       "crc32"         // TB_MQTT_KEY_FW_CHECKSUM_ALG or TB_MQTT_KEY_SW_CHECKSUM_ALG

//second, Client-Side RPC timeout, Attributes Request timeout or otaupdate Request timeout
#define TB_MQTT_TIMEOUT (30) 

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
