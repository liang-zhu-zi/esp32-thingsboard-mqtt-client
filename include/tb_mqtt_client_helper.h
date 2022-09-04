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

// ThingsBoard MQTT Client high layer API

#ifndef _TB_MQTT_CLIENT_HELPER_H_
#define _TB_MQTT_CLIENT_HELPER_H_

#include <stdint.h>
#include <stdbool.h>

#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

//=====================================================================================================================
#define TBMCH_MALLOC   malloc
#define TBMCH_FREE     free

typedef int tbmch_err_t;

/**
 * ThingsBoard MQTT Client Helper value type, for example: cJSON_Number, cJSON_String, ...
 */
//typedef int tbmch_value_type_t;
/*
//cJSON Types:
#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7) // raw json
#define cJSON_IsReference 256
#define cJSON_StringIsConst 512
*/

/**
 * ThingsBoard MQTT Client Helper value, for example: data point, attribute
 */
typedef cJSON tbmch_value_t;

/**
 * ThingsBoard MQTT Client Helper rpc params
 */
typedef cJSON tbmch_rpc_params_t;

/**
 * ThingsBoard MQTT Client Helper rpc results
 */
typedef cJSON tbmch_rpc_results_t;

//====0.tbmc client====================================================================================================
/**
 * ThingsBoard MQTT Client Helper handle
 */
typedef tbmch_t *tbmch_handle_t;

typedef void (*tbmch_on_connected_t)(tbmch_handle_t client, void *context);    /*!< Callback of connected ThingsBoard MQTT */
typedef void (*tbmch_on_disconnected_t)(tbmch_handle_t client, void *context); /*!< Callback of disconnected ThingsBoard MQTT */

//====1.telemetry time-series data=====================================================================================
//Don't call TBMCH API in this callback!
typedef tbmch_value_t* (*tbmch_tsdata_on_get_t)(tbmch_handle_t client, void *context); /*!< Get tbmch_value from context */

//====2.client-side attribute==========================================================================================
//Don't call TBMCH API in these callback!
typedef tbmch_value_t* (*tbmch_clientattribute_on_get_t)(tbmch_handle_t client, void *context); /*!< Get tbmch_value from context */
typedef void (*tbmch_clientattribute_on_set_t)(tbmch_handle_t client, void *context, const tbmch_value_t *value); /*!< Set tbmch_value to context */

//====3.shared attribute===============================================================================================
//Don't call TBMCH API in this callback!
typedef tbmch_err_t (*tbmch_sharedattribute_on_set_t)(tbmch_handle_t client, void *context, const tbmch_value_t *value); /*!< Set tbmch_value to context */

//====4.attributes request for client-side_attribute & shared_attribute================================================
typedef void (*tbmch_attributesrequest_on_response_t)(tbmch_handle_t client, void *context, int request_id); //(none/resend/destroy/_destroy_all_attributes)?
typedef void (*tbmch_attributesrequest_on_timeout_t)(tbmch_handle_t client, void *context, int request_id); //(none/resend/destroy/_destroy_all_attributes)?

//====5.Server-side RPC================================================================================================
typedef tbmch_rpc_results_t *(*tbmch_serverrpc_on_request_t)(tbmch_handle_t client, void *context,
                                                             int request_id, const char *method, tbmch_rpc_params_t *params);

//====6.Client-side RPC================================================================================================
typedef void (*tbmch_clientrpc_on_response_t)(tbmch_handle_t client, void *context,
                                              int request_id, const char *method/*, tbmch_rpc_params_t *params*/, tbmch_rpc_results_t *results);
typedef void (*tbmch_clientrpc_on_timeout_t)(tbmch_handle_t client, void *context,
                                             int request_id, const char *method/*, tbmch_rpc_params_t *params*/);

//====7.Claiming device using device-side key scenario: Not implemented yet============================================

//====8.Device provisioning: Not implemented yet=======================================================================

//====9.Firmware update================================================================================================
//Don't call TBMCH API in these callback!
typedef bool (*tbmch_fwupdate_on_sharedattributes_t)(tbmch_handle_t client, void *context,
                                                     const char *fw_title, const char *fw_version, const char *fw_checksum, const char *fw_checksum_algorithm);
typedef tbmch_err_t (*tbmch_fwupdate_on_response_t)(tbmch_handle_t client, void *context,
                                                    int request_id, int chunk /*current chunk*/, const void *fw_data, int data_size);
typedef void (*tbmch_fwupdate_on_success_t)(tbmch_handle_t client, void *context,
                                         int request_id, int chunk /*total_size*/);
typedef void (*tbmch_fwupdate_on_timeout_t)(tbmch_handle_t client, void *context,
                                            int request_id, int chunk /*current chunk*/);

//====0.tbmc client====================================================================================================
tbmch_handle_t tbmch_init(void);
void tbmch_destroy(tbmch_handle_t client_);
//~~tbmch_config(); //move to tbmch_connect()
//~~tbmch_set_ConnectedEvent(evtConnected); //move to tbmch_init()
//~~tbmch_set_DisconnectedEvent(evtDisconnected); //move to tbmch_init()
bool tbmch_connect(tbmch_handle_t client_, const char *uri, const char *token,
                   void *context,
                   tbmch_on_connected_t on_connected,
                   tbmch_on_disconnected_t on_disconnected); //_begin();
void tbmch_disconnect(tbmch_handle_t client_);               //_end();
bool tbmch_is_connected(tbmch_handle_t client_);
bool tbmch_has_events(tbmch_handle_t client_); // new function
void tbmch_run(tbmch_handle_t client_);        //_recv()=>recvFromLink()=>parse() //tb_mqtt_client_loop()/checkTimeout(), recv/parse/sendqueue/ack...

//====1.Publish Telemetry time-series data==============================================================================
tbmch_err_t tbmch_telemetry_append(tbmch_handle_t client_, const char *key, void *context, tbmch_tsdata_on_get_t on_get);
tbmch_err_t tbmch_telemetry_clear(tbmch_handle_t client_, const char *key);
tbmch_err_t tbmch_telemetry_send(tbmch_handle_t client_, int count, /*const char *key,*/ ...); ////tbmqttlink.h.tbmch_sendTelemetry();

//====2.Publish client-side device attributes to the server============================================================
tbmch_err_t tbmch_clientattribute_append(tbmch_handle_t client_, const char *key, void *context,
                                         tbmch_clientattribute_on_get_t on_get); // tbmch_attribute_of_clientside_init()
tbmch_err_t tbmch_clientattribute_with_set_append(tbmch_handle_t client_, const char *key, void *context,
                                                  tbmch_clientattribute_on_get_t on_get,
                                                  tbmch_clientattribute_on_set_t on_set); // tbmch_attribute_of_clientside_init()
tbmch_err_t tbmch_clientattribute_clear(tbmch_handle_t client_, const char *key);
tbmch_err_t tbmch_clientattribute_send(tbmch_handle_t client_, int count, /*const char *key,*/ ...); ////tbmqttlink.h.tbmch_sendClientAttributes();

//====3.Subscribe to shared device attribute updates from the server===================================================
tbmch_err_t tbmch_sharedattribute_append(tbmch_handle_t client_, const char *key, void *context,
                                         tbmch_sharedattribute_on_set_t on_set); ////tbmqttlink.h.tbmch_addSubAttrEvent(); //Call it before connect() //tbmch_shared_attribute_list_t
tbmch_err_t tbmch_sharedattribute_clear(tbmch_handle_t client_, const char *key); // remove shared_attribute from tbmch_shared_attribute_list_t

//====4.Request client-side or shared device attributes from the server================================================
int tbmch_attributesrequest_send(tbmch_handle_t client_,
                                 void *context,
                                 tbmch_attributesrequest_on_response_t on_response,
                                 tbmch_attributesrequest_on_timeout_t on_timeout,
                                 int count, /*const char *key,*/...); ////tbmqttlink.h.tbmch_sendAttributesRequest(); ////return request_id on successful, otherwise return -1

//====5.Server-side RPC================================================================================================
tbmch_err_t tbmch_serverrpc_append(tbmch_handle_t client_, const char *method,
                                   void *context,
                                   tbmch_serverrpc_on_request_t on_request);   ////tbmqttlink.h.tbmch_addServerRpcEvent(evtServerRpc); //Call it before connect()
tbmch_err_t tbmch_serverrpc_clear(tbmch_handle_t client_, const char *method); // remove from LIST_ENTRY(tbmch_serverrpc_) & delete

//====6.Client-side RPC================================================================================================
tbmch_clientrpc_handle_t tbmch_clientrpc_of_oneway_request(tbmch_handle_t client_, const char *method, tbmch_rpc_params_t *params); ////tbmqttlink.h.tbmch_sendClientRpcRequest(); //add list
tbmch_clientrpc_handle_t tbmch_clientrpc_of_twoway_request(tbmch_handle_t client_, const char *method, tbmch_rpc_params_t *params,
                                                           void *context,
                                                           tbmch_clientrpc_on_response_t on_response,
                                                           tbmch_clientrpc_on_timeout_t on_timeout); ////tbmqttlink.h.tbmch_sendClientRpcRequest(); //create to add to LIST_ENTRY(tbmch_clientrpc_)

//====7.Claiming device using device-side key scenario: Not implemented yet============================================

//====8.Device provisioning: Not implemented yet=======================================================================

//====9.Firmware update================================================================================================
tbmch_err_t tbmch_fwupdate_append(tbmch_handle_t client_, const char *fw_title,
                                  void *context,
                                  tbmch_fwupdate_on_sharedattributes_t on_fw_attributes,
                                  tbmch_fwupdate_on_response_t on_fw_chunk,
                                  tbmch_fwupdate_on_success_t on_fw_success,
                                  tbmch_fwupdate_on_timeout_t on_fw_timeout);
tbmch_err_t tbmch_fwupdate_clear(tbmch_handle_t client_, const char *fw_title);

//====end==============================================================================================================

//LIST_ENTRY, LIST_HEAD

//===value==========================================================
// typedef char *TBMC_STRING;
// typedef bool TBMC_BOOLEAN;
// typedef double TBMC_DOUBLE;
// typedef long TBMC_LONG;
// // typedef char *TBMC_JSON; //object or array
// typedef char *TBMC_ARRAY;
// typedef char *TBMC_OBJECT;
// typedef char *TBMC_RAW;
// // typedef char *TBMC_NULL;

// /**
//  * ThingsBoard MQTT Client Helper value type
//  */
// typedef enum
// {
//      TBMC_VALUE_TYPE_INVALID = 0,
//      TBMC_VALUE_TYPE_STRING,  /*!< string value */
//      TBMC_VALUE_TYPE_BOOLEAN, /*!< boolean value */
//      TBMC_VALUE_TYPE_DOUBLE,  /*!< double value */
//      TBMC_VALUE_TYPE_LONG,    /*!< long value  */
//      // TBMC_VALUE_TYPE_JSON, /*!< JSON value */
//      TBMC_VALUE_TYPE_ARRAY,   /*!< array value */
//      TBMC_VALUE_TYPE_OBJECT,  /*!< object value */
//      TBMC_VALUE_TYPE_RAW,     /*!< raw value */
//      // TBMC_VALUE_TYPE_NULL, /*!< null value */
// } tbmch_value_type_t;

// /**
//  * ThingsBoard MQTT Client Helper value
//  */
// typedef struct
// {
//      tbmch_value_type_t type; /*!< type of value */
//      //int size;               /*!< size of value?? */
//      union
//      {
//           TBMC_STRING stringV;
//           TBMC_BOOLEAN boolV;
//           TBMC_DOUBLE doubleV;
//           TBMC_LONG longV;
//           TBMC_JSON jsonV;
//      } value;
// } tbmch_value_t;

// typedef tbmch_err_t (*TBMC_GET_STRING_VALUE_CB)(void *context, TBMC_STRING *value, int value_size); /*!< Get TBMC_STRING value from context */
// typedef tbmch_err_t (*TBMC_GET_BOOLEAN_VALUE_CB)(void *context, TBMC_BOOLEAN *value, int);          /*!< Get TBMC_BOOLEAN value from context */
// typedef tbmch_err_t (*TBMC_GET_DOUBLE_VALUE_CB)(void *context, TBMC_DOUBLE *value, int);            /*!< Get TBMC_DOUBLE value from context */
// typedef tbmch_err_t (*TBMC_GET_LONG_VALUE_CB)(void *context, TBMC_LONG *value, int);                /*!< Get TBMC_LONG value from context */
// typedef tbmch_err_t (*TBMC_GET_JSON_VALUE_CB)(void *context, TBMC_JSON *value, int value_size);     /*!< Get TBMC_JSON value from context */

// typedef tbmch_err_t (*TBMC_SET_STRING_VALUE_CB)(void *context, TBMC_STRING value);   /*!< Set TBMC_STRING value to context */
// typedef tbmch_err_t (*TBMC_SET_BOOLEAN_VALUE_CB)(void *context, TBMC_BOOLEAN value); /*!< Set TBMC_BOOLEAN value to context */
// typedef tbmch_err_t (*TBMC_SET_DOUBLE_VALUE_CB)(void *context, TBMC_DOUBLE value);   /*!< Set TBMC_DOUBLE value to context */
// typedef tbmch_err_t (*TBMC_SET_LONG_VALUE_CB)(void *context, TBMC_LONG value);       /*!< Set TBMC_LONG value to context */
// typedef tbmch_err_t (*TBMC_SET_JSON_VALUE_CB)(void *context, TBMC_JSON value);       /*!< Set TBMC_JSON value to context */

//typedef tbmch_err_t (*tbmch_value_get_callback_t)(void *context, tbmch_value_t *value);       /*!< Get tbmch_value from context */
//typedef tbmch_err_t (*tbmch_value_set_callback_t)(void *context, const tbmch_value_t *value); /*!< Set tbmch_value to context */

//===key-value======================================================
// /**
//  * ThingsBoard MQTT Client Helper key-value
//  */
// typedef struct
// {
//      char *key;           /*!< Key */
//      tbmch_value_t *value; /*!< Value */

//      void *context;                          /*!< Context of getting/setting value*/
//      tbmch_value_get_callback_t on_get; /*!< Callback of getting value from context */
//      tbmch_value_set_callback_t on_set; /*!< Callback of setting value to context */
// } tbmch_kv_t;

// typedef tbmch_kv_t *tbmch_kv_handle_t;

// /**
//  * @brief Creates tbmc key-value handle
//  *
//  * @param key
//  * @param type           context of getting/setting value
//  * @param context        contex of value for callback
//  * @param on_get   callback of getting value from context
//  * @param on_set   callback of setting value to context
//  *
//  * @return tbmch_kv_handle_t if successfully created, NULL on error
//  */
// tbmch_kv_handle_t tbmch_kv_init(const char *key, tbmch_value_type_t type, void *context, tbmch_value_get_callback_t on_get, tbmch_value_set_callback_t on_set);

// /**
//  * @brief Destroys the tbmc key-value handle
//  *
//  * Notes:
//  *  - Cannot be called from the tbmc event handler
//  *
//  * @param kv    tbmc key-value handle
//  *
//  * @return 0
//  *         TBMC_ERR_INVALID_ARG on wrong initialization
//  */
// tbmch_err_t tbmch_kv_destroy(tbmch_kv_handle_t kv);

// /**
//  * @brief Get key of the tbmc key-value handle
//  *
//  * @param kv    tbmc key-value handle
//  *
//  * @return key of the tbmc key-value handle if successfully created, NULL on error
//  */
// const char *tbmch_kv_get_key(tbmch_kv_handle_t kv);

// /**
//  * @brief Get value type of tbmch_kv
//  *
//  * @param kv    tbmc key-value handle
//  *
//  * @return vale type of the tbmc key-value handle if successfully created, TBMC_VALUE_TYPE_INVALID on error
//  */
// tbmch_value_type_t tbmch_kv_get_value_type(tbmch_kv_handle_t kv);

// /**
//  * @brief Get tbmch_value of tbmch_kv
//  *
//  * @param kv    tbmc key-value handle
//  * @param value result, fan-out parameter
//  *
//  * @return ESP_OK on success
//  *         ESP_ERR_INVALID_ARG on wrong initialization
//  *         ESP_FAIL if client is in invalid state
//  */
// tbmch_err_t tbmch_kv_get_value(tbmch_kv_handle_t kv, tbmch_value_t *value);

// /**
//  * @brief Set tbmch_value of tbmch_kv
//  *
//  * @param kv    tbmc key-value handle
//  * @param value
//  *
//  * @return ESP_OK on success
//  *         ESP_ERR_INVALID_ARG on wrong initialization
//  *         ESP_FAIL if client is in invalid state
//  */
// tbmch_err_t tbmch_kv_set_value(tbmch_kv_handle_t kv, const tbmch_value_t *value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
