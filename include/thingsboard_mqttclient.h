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
tbmqttclient_telemetry_datapoints_send(datapoint1, 2, 3, ...);       //telemetry_datapoint_init()/_destory(),         _pack()/_send()!, _get_name()
//tbmqttclient_telemetry_datapoint_list_send(datapoint_list);   //telemetry_datapoint_list_init()/_destory(), _add(), _pack()/_send()!, _get_name()

//2.Publish client-side device attributes to the server: once-time
tbmqttclient_clientside_attributes_send(attribute1, 2, 3, ...);               //clientside_attribute_init()/_destory(),         _pack()/_send()!, _unpack()/_deal(), _get_name(), _get_attribute_type()
//tbmqttclient_clientside_attribute_list_send(clientside_attribute_list);//clientside_attribute_list_init()/_destory(), _add(), _pack()/_send()!,                    _get_name(), _get_keys()
                                                                                  //shared_attribute_init()/_desotry(),                           _unpack()/_deal(), _get_name(), _get_attribute_type()
                                                                             //shared_attribute_list_init()/_destory(), _add(),                   _unpack()/_deal()?,_get_name(), _get_keys()

//3.Request client-side or shared device attributes from the server: once-time
                                        //attributes_request_init()/_destory(), _add(client/shared)/_get()/, ???_destory_all_attributes(), _pack()/_send()!
                                        //_getClientKeys(), _getSharedKeys(), 
                                        //_onAttributesResponse()[unpack->queue], [queue->attribute deal], 
                                        //(*none/resend/destory/_destory_all_attributes)_on_..._success(deal), (*none/resend/destory/_destory_all_attributes)_on..._timeout(deal)
                                        //sendTimes++
tbmqttclient_attributes_request(context, on_..._success, on_..._timeout, attribute1, 2, 3, ...); //list: struct attributes_request{clientside_attribute_list[], shared_attribute_list[], context, on_..._success, on_..._timeout}
//tbmqttclient_attribute_list_request(context, on_..._success, on_..._timeout, clientside_attribute_list, sharedside_attribute_list);

//4.Subscribe to shared device attribute updates from the server: many-times deal. Call it before connect().
tbmqttlcient_shared_attributes_subscribe(shared_attribute1, 2, 3, ...); //tbmqttclient_addSubAttrEvent(); //shared_attribute_list[]
tbmqttlcient_shared_attributes_unsubscribe(shared_attribute_name1, 2, 3, ...); //remove shared_attribute from shared_attribute_list[]

//5.Server-side RPC: many-times deal.Call it before connect()
                                                                   //serverside_rpc_init()/_destory(), _unpacket()/_deal(), _pack()/(reply)_send()!, _get_name()
                                                                   //_on_request()???, _on_replay()???
tbmqttclient_serverside_rpc_subscirbe(serverside_rpc1, 2, 3, ...); //tbmqttclient_addServerRpcEvent(); //list: struct serverside_rpc{}
tbmqttlcient_serverside_rpc_unsubscribe(serverside_rpc_name1, 2, 3, ...); //remove serverside_rpc from server_rpc_list[]
//tbmqttclient_serverside_rpc_reply(); //tbmqttclient_sendServerRpcReply

//6.Client-side RPC: once-time
                                       //clientside_rpc_init()/_destory(), _pack()/_send(), _unpack()/_deal(), _get_name()
                                       //_onClientRpc_response(), _on_..._success(), _on_ClientRpcResponse_timeout()
tbmqttclient_clientside_rpc_request(clientside_rpc); //tbmqttclient_sendClientRpcRequest(); //list: struct clientside_rpc_request{}

//7.Claiming device using device-side key scenario: Not implemented yet

//8.Device provisioning: Not implemented yet

//9.Firmware update
                                                //on_fw_shared_attributes_callback(context, fw_title, fw_version, fw_checksum, fw_checksum_algorithm)
tbmqttclient_set_on_fw_shared_attributes_callback(context, on_fw_shared_attributes_callback);
                                                //_on_fw_response_chunk(context, requestId, chunk, chunk_size);
                                                //_on_fw_response_chunk_success(context, requestId)
                                                //_on_fw_response_chunk_failure(context, requestId)
requestId tbmqttclient_fw_request(context, _on_fw_response_chunk, _on_fw_response_chunk_success, _on_fw_response_chunk_failure);

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

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
