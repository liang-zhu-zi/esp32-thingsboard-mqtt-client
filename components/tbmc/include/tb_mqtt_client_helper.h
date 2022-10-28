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

/**
 * ThingsBoard MQTT Client Helper provision params
 */
typedef cJSON tbmch_provision_params_t;

/**
 * ThingsBoard MQTT Client Helper provision results
 */
typedef cJSON tbmch_provision_results_t;

//====0.tbmc client====================================================================================================
/**
 * ThingsBoard MQTT Client Helper handle
 */
typedef struct tbmch_client *tbmch_handle_t;

typedef void (*tbmch_on_connected_t)(tbmch_handle_t client, void *context);    /*!< Callback of connected ThingsBoard MQTT */
typedef void (*tbmch_on_disconnected_t)(tbmch_handle_t client, void *context); /*!< Callback of disconnected ThingsBoard MQTT */

//====1.telemetry time-series data=====================================================================================
//Don't call TBMCH API in this callback!
//Free return value by caller/(tbmch library)!
typedef tbmch_value_t* (*tbmch_tsdata_on_get_t)(tbmch_handle_t client, void *context); /*!< Get tbmch_value from context */

//====2.client-side attribute==========================================================================================
//Don't call TBMCH API in these callback!
//Free return value by caller/(tbmch library)!
typedef tbmch_value_t* (*tbmch_clientattribute_on_get_t)(tbmch_handle_t client, void *context); /*!< Get tbmch_value from context */
//Free value by caller/(tbmch library)!
typedef void (*tbmch_clientattribute_on_set_t)(tbmch_handle_t client, void *context, const tbmch_value_t *value); /*!< Set tbmch_value to context */

//====3.shared attribute===============================================================================================
//Don't call TBMCH API in this callback!
//Free value by caller/(tbmch library)!
typedef tbmch_err_t (*tbmch_sharedattribute_on_set_t)(tbmch_handle_t client, void *context, const tbmch_value_t *value); /*!< Set tbmch_value to context */

//====4.attributes request for client-side_attribute & shared_attribute================================================
typedef void (*tbmch_attributesrequest_on_response_t)(tbmch_handle_t client, void *context, int request_id); //(none/resend/destroy/_destroy_all_attributes)?
typedef void (*tbmch_attributesrequest_on_timeout_t)(tbmch_handle_t client, void *context, int request_id); //(none/resend/destroy/_destroy_all_attributes)?

//====5.Server-side RPC================================================================================================
// return NULL or cJSON* of object
// free return-value by caller/(tbmch library)!
// free params by caller/(tbmch library)!
typedef tbmch_rpc_results_t *(*tbmch_serverrpc_on_request_t)(tbmch_handle_t client, void *context,
                                                             int request_id, const char *method, const tbmch_rpc_params_t *params);

//====6.Client-side RPC================================================================================================
// free results by caller/(tbmch library)!
typedef void (*tbmch_clientrpc_on_response_t)(tbmch_handle_t client, void *context,
                                              int request_id, const char *method, const tbmch_rpc_results_t *results); /*, tbmch_rpc_params_t *params*/
typedef void (*tbmch_clientrpc_on_timeout_t)(tbmch_handle_t client, void *context,
                                             int request_id, const char *method); /*, tbmch_rpc_params_t *params*/

//====7.Claiming device using device-side key scenario============================================

//====8.Device provisioning=======================================================================
// free results by caller/(tbmch library)!
typedef void (*tbmch_provision_on_response_t)(tbmch_handle_t client, void *context,
                                              int request_id, const tbmch_provision_results_t *results); /*, tbmch_provision_params_t *params*/
typedef void (*tbmch_provision_on_timeout_t)(tbmch_handle_t client, void *context,
                                             int request_id); /*, tbmch_provision_params_t *params*/

//====9.Firmware/Software update=======================================================================================
/**
 * ThingsBoard MQTT Client Helper OTA update type
 */
typedef enum
{
     TBMCH_OTAUPDATE_TYPE_FW = 0,  /*!< F/W OTA update */
     TBMCH_OTAUPDATE_TYPE_SW       /*!< S/W OTA update */
} tbmch_otaupdate_type_t;

//Don't call TBMCH API in these callback!
typedef const char* (*tbmch_otaupdate_on_get_current_ota_title_t)(tbmch_handle_t client, void *context);
typedef const char* (*tbmch_otaupdate_on_get_current_ota_version_t)(tbmch_handle_t client, void *context);
//return 1 on negotiate successful(next to F/W OTA), -1/ESP_FAIL on negotiate failure, 0/ESP_OK on already updated!
typedef tbmch_err_t (*tbmch_otaupdate_on_negotiate_t)(tbmch_handle_t client, void *context,
                  const char *ota_title, const char *ota_version, int ota_size, const char *ota_checksum, const char *ota_checksum_algorithm,
                  char *ota_error, int error_size);
//return 0/ESP_OK on successful, -1/ESP_FAIL on failure
typedef tbmch_err_t (*tbmch_otaupdate_on_write_t)(tbmch_handle_t client, void *context,
                  int request_id, int chunk_id/*current chunk_id*/, const void *ota_data, int data_size,
                  char *ota_error, int error_size);
//return 0/ESP_OK on successful, -1/ESP_FAIL on failure
typedef tbmch_err_t (*tbmch_otaupdate_on_end_t)(tbmch_handle_t client, void *context,
                                         int request_id, int chunk_id,
                                         char *ota_error, int error_size);
typedef void (*tbmch_otaupdate_on_abort_t)(tbmch_handle_t client, void *context,
                                            int request_id, int chunk_id/*current chunk_id*/);

//====0.tbmc client====================================================================================================
/**
 * ThingsBoard MQTT Client Helper config.
 *
 */
typedef struct
{
  bool log_rxtx_package;       /*!< print Rx/Tx MQTT package */

  const char *uri;             /*!< Complete ThingsBoard MQTT broker URI */
  const char *access_token;    /*!< ThingsBoard Access Token */
  const char *cert_pem;        /*!< Reserved. Pointer to certificate data in PEM format for server verify (with SSL), default is NULL, not required to verify the server */
  const char *client_cert_pem; /*!< Reserved. Pointer to certificate data in PEM format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_key_pem` has to be provided. */
  const char *client_key_pem;  /*!< Reserved. Pointer to private key data in PEM format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_cert_pem` has to be provided. */

  void *context;                           /*!< Context parameter of the below two callbacks */
  tbmch_on_connected_t on_connected;       /*!< Callback of connected ThingsBoard MQTT */
  tbmch_on_disconnected_t on_disconnected; /*!< Callback of disconnected ThingsBoard MQTT */
} tbmch_config_t;

tbmch_handle_t tbmch_init(void);
void tbmch_destroy(tbmch_handle_t client_);
//~~tbmch_config(); //move to tbmch_connect()
//~~tbmch_set_ConnectedEvent(evtConnected); //move to tbmch_init()
//~~tbmch_set_DisconnectedEvent(evtDisconnected); //move to tbmch_init()
bool tbmch_connect(tbmch_handle_t client_, const tbmch_config_t *config); //_begin();
void tbmch_disconnect(tbmch_handle_t client_);               //_end();
bool tbmch_is_connected(tbmch_handle_t client_);
bool tbmch_has_events(tbmch_handle_t client_); // new function
void tbmch_run(tbmch_handle_t client_);        //_recv()=>recvFromLink()=>parse() //tb_mqtt_client_loop()/checkTimeout(), recv/parse/sendqueue/ack...

//====10.Publish Telemetry time-series data==============================================================================
tbmch_err_t tbmch_telemetry_append(tbmch_handle_t client_, const char *key, void *context, tbmch_tsdata_on_get_t on_get);
tbmch_err_t tbmch_telemetry_clear(tbmch_handle_t client_, const char *key);
tbmch_err_t tbmch_telemetry_send(tbmch_handle_t client_, int count, /*const char *key,*/ ...); ////tbmqttlink.h.tbmch_sendTelemetry();

//====20.Publish client-side device attributes to the server============================================================
tbmch_err_t tbmch_clientattribute_append(tbmch_handle_t client_, const char *key, void *context,
                                         tbmch_clientattribute_on_get_t on_get); // tbmch_attribute_of_clientside_init()
tbmch_err_t tbmch_clientattribute_with_set_append(tbmch_handle_t client_, const char *key, void *context,
                                                  tbmch_clientattribute_on_get_t on_get,
                                                  tbmch_clientattribute_on_set_t on_set); // tbmch_attribute_of_clientside_init()
tbmch_err_t tbmch_clientattribute_clear(tbmch_handle_t client_, const char *key);
tbmch_err_t tbmch_clientattribute_send(tbmch_handle_t client_, int count, /*const char *key,*/ ...); ////tbmqttlink.h.tbmch_sendClientAttributes();

//====21.Subscribe to shared device attribute updates from the server===================================================
tbmch_err_t tbmch_sharedattribute_append(tbmch_handle_t client_, const char *key, void *context,
                                         tbmch_sharedattribute_on_set_t on_set); ////tbmqttlink.h.tbmch_addSubAttrEvent(); //Call it before connect() //tbmch_shared_attribute_list_t
tbmch_err_t tbmch_sharedattribute_clear(tbmch_handle_t client_, const char *key); // remove shared_attribute from tbmch_shared_attribute_list_t

//====22.Request client-side or shared device attributes from the server================================================
int tbmch_attributesrequest_send(tbmch_handle_t client_,
                                 void *context,
                                 tbmch_attributesrequest_on_response_t on_response,
                                 tbmch_attributesrequest_on_timeout_t on_timeout,
                                 int count, /*const char *key,*/...); ////tbmqttlink.h.tbmch_sendAttributesRequest(); ////return request_id on successful, otherwise return -1

//====30.Server-side RPC================================================================================================
tbmch_err_t tbmch_serverrpc_append(tbmch_handle_t client_, const char *method,
                                   void *context,
                                   tbmch_serverrpc_on_request_t on_request);   ////tbmqttlink.h.tbmch_addServerRpcEvent(evtServerRpc); //Call it before connect()
tbmch_err_t tbmch_serverrpc_clear(tbmch_handle_t client_, const char *method); // remove from LIST_ENTRY(tbmch_serverrpc_) & delete

//====31.Client-side RPC================================================================================================
// free params by caller/(user code)!
int tbmch_clientrpc_of_oneway_request(tbmch_handle_t client_, const char *method, /*const*/ tbmch_rpc_params_t *params); ////tbmqttlink.h.tbmch_sendClientRpcRequest(); //add list
// free params by caller/(user code)!
int tbmch_clientrpc_of_twoway_request(tbmch_handle_t client_, const char *method, /*const*/ tbmch_rpc_params_t *params,
                                                           void *context,
                                                           tbmch_clientrpc_on_response_t on_response,
                                                           tbmch_clientrpc_on_timeout_t on_timeout); ////tbmqttlink.h.tbmch_sendClientRpcRequest(); //create to add to LIST_ENTRY(tbmch_clientrpc_)

//====40.Claiming device using device-side key scenario============================================
tbmch_err_t tbmch_claiming_device_using_device_side_key(tbmch_handle_t client_,
                    const char *secret_key, uint32_t *duration_ms);

//====50.Device provisioning: Not implemented yet=======================================================================
int tbmch_provision_request(tbmch_handle_t client_, /*const*/ tbmch_provision_params_t *params,
                                   void *context,
                                   tbmch_provision_on_response_t on_response,
                                   tbmch_provision_on_timeout_t on_timeout);

//====60.Firmware update================================================================================================
/**
 * ThingsBoard MQTT Client Helper F/W update OTA config
 */
typedef struct tbmch_otaupdate_config
{
     tbmch_otaupdate_type_t ota_type; /*!< FW/TBMCH_OTAUPDATE_TYPE_FW or SW/TBMCH_OTAUPDATE_TYPE_SW  */
     int chunk_size;                  /*!< chunk_size, eg: 8192. 0 to get all F/W or S/W by request  */

     void *context;
     tbmch_otaupdate_on_get_current_ota_title_t   on_get_current_ota_title;     /*!< callback of getting current F/W or S/W OTA title */
     tbmch_otaupdate_on_get_current_ota_version_t on_get_current_ota_version;   /*!< callback of getting current F/W or S/W OTA version */

     tbmch_otaupdate_on_negotiate_t on_ota_negotiate;         /*!< callback of F/W or S/W OTA attributes */
     tbmch_otaupdate_on_write_t on_ota_write;                 /*!< callback of F/W or S/W OTA doing */
     tbmch_otaupdate_on_end_t on_ota_end;                     /*!< callback of F/W or S/W OTA success & end*/
     tbmch_otaupdate_on_abort_t on_ota_abort;                 /*!< callback of F/W or S/W OTA failure & abort */

     ////bool is_first_boot;            /*!< whether first boot after ota update  */
} tbmch_otaupdate_config_t;

tbmch_err_t tbmch_otaupdate_append(tbmch_handle_t client_, const char *ota_description, const tbmch_otaupdate_config_t *config);
tbmch_err_t tbmch_otaupdate_clear(tbmch_handle_t client_, const char *ota_description);

//====end==============================================================================================================

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
