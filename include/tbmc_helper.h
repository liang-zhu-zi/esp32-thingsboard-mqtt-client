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

#ifndef _TBMC_HELPER_H_
#define _TBMC_HELPER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

//=====================================================================================================================
/**
 * ThingsBoard MQTT Client value, for example: data point, attribute
 */
typedef cJSON tbmc_value_t;

/**
 * ThingsBoard MQTT Client rpc params
 */
typedef cJSON tbmc_rpc_params_t;

/**
 * ThingsBoard MQTT Client rpc results
 */
typedef cJSON tbmc_rpc_results_t;

//====0.tbmc client====================================================================================================
/**
 * ThingsBoard MQTT Client handle
 */
typedef tbmc_client_t *tbmc_client_handle_t;

typedef void (*tbmc_on_connected_t)(tbmc_client_handle_t client, void *context);    /*!< Callback of connected ThingsBoard MQTT */
typedef void (*tbmc_on_disconnected_t)(tbmc_client_handle_t client, void *context); /*!< Callback of disconnected ThingsBoard MQTT */

//====1.telemetry_datapoint============================================================================================
typedef tbmc_err_t (*tbmc_datapoint_get_callback_t)(tbmc_client_handle_t client, void *context, tbmc_value_t *value); /*!< Get tbmc_value from context */
typedef tbmc_err_t (*tbmc_datapoint_set_callback_t)(tbmc_client_handle_t client, void *context, const tbmc_value_t *value); /*!< Set tbmc_value to context */

//====2.client-side attribute==========================================================================================
typedef tbmc_err_t (*tbmc_clientattribute_get_callback_t)(tbmc_client_handle_t client, void *context, tbmc_value_t *value); /*!< Get tbmc_value from context */
typedef tbmc_err_t (*tbmc_clientattribute_set_callback_t)(tbmc_client_handle_t client, void *context, const tbmc_value_t *value); /*!< Set tbmc_value to context */

//====3.shared attribute===============================================================================================
typedef tbmc_err_t (*tbmc_sharedattribute_set_callback_t)(tbmc_client_handle_t client, void *context, const tbmc_value_t *value); /*!< Set tbmc_value to context */

//====4.attributes request for client-side_attribute & shared_attribute================================================
typedef void (*tbmc_attributes_request_success_callback_t)(tbmc_client_handle_t client, void *context); //(none/resend/destroy/_destroy_all_attributes)?
typedef void (*tbmc_attributes_request_timeout_callback_t)(tbmc_client_handle_t client, void *context); //(none/resend/destroy/_destroy_all_attributes)?

//====5.Server-side RPC================================================================================================
typedef tbmc_rpc_results_t *(*tbmc_serverrpc_request_callback_t)(tbmc_handle_client_t client,
                                                                 void *context, int request_id, const char *method, tbmc_rpc_params_t *params);

//====6.Client-side RPC================================================================================================
typedef void (*tbmc_clientrpc_success_callback_t)(tbmc_handle_client_t client,
                                                  void *context, int request_id, const char *method, tbmc_rpc_params_t *params, tbmc_rpc_results_t *results);
typedef void (*tbmc_clientrpc_timeout_callback_t)(tbmc_handle_client_t client,
                                                  void *context, int request_id, const char *method, tbmc_rpc_params_t *params);

//====7.Claiming device using device-side key scenario: Not implemented yet============================================

//====8.Device provisioning: Not implemented yet=======================================================================

//====9.Firmware update================================================================================================
typedef bool (*tbmc_fw_shared_attributes_callback_t)(tbmc_handle_client_t client,
                                                     void *context, const char *fw_title, const char *fw_version, const char *fw_checksum, const char *fw_checksum_algorithm);
typedef tbmc_err_t (*tbmc_fw_response_chunk_callback_t)(tbmc_handle_client_t client,
                                                        void *context, int request_id, int chunk /*current chunk*/, const void *fw_data, int data_size);
typedef void (*tbmc_fw_response_success_callback_t)(tbmc_handle_client_t client,
                                                    void *context, int request_id, int chunk /*total_size*/);
typedef void (*tbmc_fw_response_timeout_callback_t)(tbmc_handle_client_t client,
                                                    void *context, int request_id, int chunk /*current chunk*/);

//====0.tbmc client====================================================================================================
tbmc_client_handle_t tbmc_init(void);
tbmc_err_t tbmc_destroy(tbmc_client_handle_t client);
//~~tbmc_config(); //move to tbmc_init()
//~~tbmc_set_ConnectedEvent(evtConnected); //move to tbmc_init()
//~~tbmc_set_DisconnectedEvent(evtDisconnected); //move to tbmc_init()
void tbmc_connect(tbmc_client_handle_t client,
                  const char *uri, const char *token, void *context,
                  tbmc_on_connected_t on_connected, tbmc_on_disconnected_t on_disconnected); //_begin();
void tbmc_disconnect(tbmc_client_handle_t client);                                           //_end();
bool tbmc_is_connected(tbmc_client_handle_t client);
bool tbmc_has_events(tbmc_client_handle_t client); // new function
void tbmc_run(tbmc_client_handle_t client);        //_recv()=>recvFromLink()=>parse() //tb_mqtt_client_loop()/checkTimeout(), recv/parse/sendqueue/ack...

//====1.Publish Telemetry datapoints===================================================================================
tbmc_err_t tbmc_telemetry_helper_append(tbmc_client_handle_t client,
                                        const char *key, tbmc_value_type_t type, void *context,
                                        tbmc_datapoint_get_callback_t get_value_cb,
                                        tbmc_datapoint_set_callback_t set_value_cb);
tbmc_err_t tbmc_telemetry_helper_clear(tbmc_client_handle_t client, const char *key);
tbmc_err_t tbmc_telemetry_helper_send(tbmc_client_handle_t client, const char *key, ...); ////tbmqttlink.h.tbmc_sendTelemetry();

//====2.Publish client-side device attributes to the server============================================================
tbmc_err_t tbmc_clientattribute_helper_of_twoway_append(tbmc_client_handle_t client,
                                                        const char *key, tbmc_value_type_t type, void *context,
                                                        tbmc_clientattribute_get_callback_t get_value_cb,
                                                        tbmc_clientattribute_set_callback_t set_value_cb); // tbmc_attribute_of_clientside_init()
tbmc_err_t tbmc_clientattribute_helper_of_oneway_append(tbmc_client_handle_t client,
                                                        const char *key, tbmc_value_type_t type, void *context,
                                                        tbmc_clientattribute_get_callback_t get_value_cb); // tbmc_attribute_of_clientside_init()
tbmc_err_t tbmc_clientattribute_helper_clear(tbmc_client_handle_t client, const char *key, ...);
tbmc_err_t tbmc_clientattribute_helper_send(tbmc_client_handle_t client, const char *key, ...); ////tbmqttlink.h.tbmc_sendClientAttributes();

//====3.Subscribe to shared device attribute updates from the server===================================================
tbmc_err_t tbmc_sharedattribute_observer_append(tbmc_client_handle_t client,
                                                const char *key, tbmc_value_type_t type, void *context,
                                                tbmc_sharedattribute_set_callback_t set_value_cb); ////tbmqttlink.h.tbmc_addSubAttrEvent(); //Call it before connect() //tbmc_shared_attribute_list_t
tbmc_err_t tbmc_sharedattribute_observer_clear(tbmc_client_handle_t client, const char *key, ...); // remove shared_attribute from tbmc_shared_attribute_list_t

//====4.Request client-side or shared device attributes from the server================================================
tbmc_err_t tbmc_attributes_request(tbmc_client_handle_t client, void *context,
                                   tbmc_attributes_request_success_callback_t success_cb,
                                   tbmc_attributes_request_timeout_callback_t timeout_cb,
                                   const char *key, ...); ////tbmqttlink.h.tbmc_sendAttributesRequest(); ////return request_id on successful, otherwise return TBMC_FAIL

//====5.Server-side RPC================================================================================================
tbmc_err_t tbmc_serverrpc_observer_append(tbmc_client_handle_t client, const char *method, void *context,
                                          tbmc_serverrpc_request_callback_t on_request); ////tbmqttlink.h.tbmc_addServerRpcEvent(evtServerRpc); //Call it before connect()
tbmc_err_t tbmc_serverrpc_observer_clear(tbmc_client_handle_t client, const char *method, ...); // remove from LIST_ENTRY(tbmc_serverrpc_) & delete

//====6.Client-side RPC================================================================================================
tbmc_clientrpc_handle_t tbmc_clientrpc_of_oneway_request(tbmc_client_handle_t client, const char *method, tbmc_rpc_params_t *params); ////tbmqttlink.h.tbmc_sendClientRpcRequest(); //add list
tbmc_clientrpc_handle_t tbmc_clientrpc_of_twoway_request(tbmc_client_handle_t client, const char *method, tbmc_rpc_params_t *params, void *context,
                                                         tbmc_clientrpc_response_callback_t on_response,
                                                         tbmc_clientrpc_timeout_callback_t on_timeout); ////tbmqttlink.h.tbmc_sendClientRpcRequest(); //create to add to LIST_ENTRY(tbmc_clientrpc_)

//====7.Claiming device using device-side key scenario: Not implemented yet============================================

//====8.Device provisioning: Not implemented yet=======================================================================

//====9.Firmware update================================================================================================
tbmc_err_t tbmc_fw_observer_append(tbmc_handle_client_t client, const char *fw_title, void *context,
                                   tbmc_fw_shared_attributes_callback_t on_fw_attributes,
                                   tbmc_fw_response_chunk_callback_t on_fw_chunk,
                                   tbmc_fw_response_success_callback_t on_fw_success,
                                   tbmc_fw_response_timeout_callback_t on_fw_timeout);
tbmc_err_t tbmc_fw_observer_clear(tbmc_handle_client_t client,
                                  const char *fw_title, ...);

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
//  * ThingsBoard MQTT Client value type
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
// } tbmc_value_type_t;

// /**
//  * ThingsBoard MQTT Client value
//  */
// typedef struct
// {
//      tbmc_value_type_t type; /*!< type of value */
//      //int size;               /*!< size of value?? */
//      union
//      {
//           TBMC_STRING stringV;
//           TBMC_BOOLEAN boolV;
//           TBMC_DOUBLE doubleV;
//           TBMC_LONG longV;
//           TBMC_JSON jsonV;
//      } value;
// } tbmc_value_t;

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

//typedef tbmc_err_t (*tbmc_value_get_callback_t)(void *context, tbmc_value_t *value);       /*!< Get tbmc_value from context */
//typedef tbmc_err_t (*tbmc_value_set_callback_t)(void *context, const tbmc_value_t *value); /*!< Set tbmc_value to context */

//===key-value======================================================
// /**
//  * ThingsBoard MQTT Client key-value
//  */
// typedef struct
// {
//      char *key;           /*!< Key */
//      tbmc_value_t *value; /*!< Value */

//      void *context;                          /*!< Context of getting/setting value*/
//      tbmc_value_get_callback_t get_value_cb; /*!< Callback of getting value from context */
//      tbmc_value_set_callback_t set_value_cb; /*!< Callback of setting value to context */
// } tbmc_kv_t;

// typedef tbmc_kv_t *tbmc_kv_handle_t;

// /**
//  * @brief Creates tbmc key-value handle
//  *
//  * @param key
//  * @param type           context of getting/setting value
//  * @param context        contex of value for callback
//  * @param get_value_cb   callback of getting value from context
//  * @param set_value_cb   callback of setting value to context
//  *
//  * @return tbmc_kv_handle_t if successfully created, NULL on error
//  */
// tbmc_kv_handle_t tbmc_kv_init(const char *key, tbmc_value_type_t type, void *context, tbmc_value_get_callback_t get_value_cb, tbmc_value_set_callback_t set_value_cb);

// /**
//  * @brief Destroys the tbmc key-value handle
//  *
//  * Notes:
//  *  - Cannot be called from the tbmc event handler
//  *
//  * @param kv    tbmc key-value handle
//  *
//  * @return TBMC_OK
//  *         TBMC_ERR_INVALID_ARG on wrong initialization
//  */
// tbmc_err_t tbmc_kv_destroy(tbmc_kv_handle_t kv);

// /**
//  * @brief Get key of the tbmc key-value handle
//  *
//  * @param kv    tbmc key-value handle
//  *
//  * @return key of the tbmc key-value handle if successfully created, NULL on error
//  */
// const char *tbmc_kv_get_key(tbmc_kv_handle_t kv);

// /**
//  * @brief Get value type of tbmc_kv
//  *
//  * @param kv    tbmc key-value handle
//  *
//  * @return vale type of the tbmc key-value handle if successfully created, TBMC_VALUE_TYPE_INVALID on error
//  */
// tbmc_value_type_t tbmc_kv_get_value_type(tbmc_kv_handle_t kv);

// /**
//  * @brief Get tbmc_value of tbmc_kv
//  *
//  * @param kv    tbmc key-value handle
//  * @param value result, fan-out parameter
//  *
//  * @return ESP_OK on success
//  *         ESP_ERR_INVALID_ARG on wrong initialization
//  *         ESP_FAIL if client is in invalid state
//  */
// tbmc_err_t tbmc_kv_get_value(tbmc_kv_handle_t kv, tbmc_value_t *value);

// /**
//  * @brief Set tbmc_value of tbmc_kv
//  *
//  * @param kv    tbmc key-value handle
//  * @param value
//  *
//  * @return ESP_OK on success
//  *         ESP_ERR_INVALID_ARG on wrong initialization
//  *         ESP_FAIL if client is in invalid state
//  */
// tbmc_err_t tbmc_kv_set_value(tbmc_kv_handle_t kv, const tbmc_value_t *value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
