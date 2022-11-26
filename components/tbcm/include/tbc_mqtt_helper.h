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

// ThingsBoard MQTT Client helper (high layer) API

#ifndef _TBC_MQTT_HELPER_H_
#define _TBC_MQTT_HELPER_H_

#include <stdint.h>
#include <stdbool.h>

#include "cJSON.h"

#include "tbc_utils.h"
#include "tbc_transport_config.h"

#ifdef __cplusplus
extern "C" {
#endif

//=====================================================================================================================
/**
 * ThingsBoard MQTT Client Helper value type, for example: cJSON_Number, cJSON_String, ...
 */
//typedef int tbcmh_value_type_t;
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
typedef cJSON tbcmh_value_t;

/**
 * ThingsBoard MQTT Client Helper rpc params
 */
typedef cJSON tbcmh_rpc_params_t;

/**
 * ThingsBoard MQTT Client Helper rpc results
 */
typedef cJSON tbcmh_rpc_results_t;

/**
 * ThingsBoard MQTT Client Helper provision params
 */
typedef cJSON tbcmh_provision_params_t;

/**
 * ThingsBoard MQTT Client Helper provision results
 */
typedef cJSON tbcmh_provision_results_t;

//====0.tbcm client====================================================================================================
/**
 * ThingsBoard MQTT Client Helper handle
 */
typedef struct tbcmh_client *tbcmh_handle_t;

typedef void (*tbcmh_on_connected_t)(tbcmh_handle_t client, void *context);    /*!< Callback of connected ThingsBoard MQTT */
typedef void (*tbcmh_on_disconnected_t)(tbcmh_handle_t client, void *context); /*!< Callback of disconnected ThingsBoard MQTT */

//====1.telemetry time-series data=====================================================================================
//Don't call TBCMH API in this callback!
//Free return value by caller/(tbcmh library)!
typedef tbcmh_value_t* (*tbcmh_tsdata_on_get_t)(tbcmh_handle_t client, void *context); /*!< Get tbcmh_value from context */

//====2.client-side attribute==========================================================================================
//Don't call TBCMH API in these callback!
//Free return value by caller/(tbcmh library)!
typedef tbcmh_value_t* (*tbcmh_clientattribute_on_get_t)(tbcmh_handle_t client, void *context); /*!< Get tbcmh_value from context */
//Don't call TBCMH API in these callback!
//Free value by caller/(tbcmh library)!
typedef void (*tbcmh_clientattribute_on_set_t)(tbcmh_handle_t client, void *context, const tbcmh_value_t *value); /*!< Set tbcmh_value to context */

//====3.shared attribute===============================================================================================
//Don't call TBCMH API in this callback!
//Free value by caller/(tbcmh library)!
typedef tbc_err_t (*tbcmh_sharedattribute_on_set_t)(tbcmh_handle_t client, void *context, const tbcmh_value_t *value); /*!< Set tbcmh_value to context */

//====4.attributes request for client-side_attribute & shared_attribute================================================
typedef void (*tbcmh_attributesrequest_on_response_t)(tbcmh_handle_t client, void *context, int request_id); //(none/resend/destroy/_destroy_all_attributes)?
typedef void (*tbcmh_attributesrequest_on_timeout_t)(tbcmh_handle_t client, void *context, int request_id); //(none/resend/destroy/_destroy_all_attributes)?

//====5.Server-side RPC================================================================================================
// return NULL or cJSON* of object
// free return-value by caller/(tbcmh library)!
// free params by caller/(tbcmh library)!
typedef tbcmh_rpc_results_t *(*tbcmh_serverrpc_on_request_t)(tbcmh_handle_t client, void *context,
                                                             int request_id, const char *method, const tbcmh_rpc_params_t *params);

//====6.Client-side RPC================================================================================================
// free results by caller/(tbcmh library)!
typedef void (*tbcmh_clientrpc_on_response_t)(tbcmh_handle_t client, void *context,
                                              int request_id, const char *method, const tbcmh_rpc_results_t *results); /*, tbcmh_rpc_params_t *params*/
typedef void (*tbcmh_clientrpc_on_timeout_t)(tbcmh_handle_t client, void *context,
                                             int request_id, const char *method); /*, tbcmh_rpc_params_t *params*/

//====7.Claiming device using device-side key scenario============================================

//====8.Device provisioning=======================================================================
typedef enum tbc_provison_type
{
  TBC_PROVISION_TYPE_NONE = 0,
  TBC_PROVISION_TYPE_SERVER_GENERATES_CREDENTIALS,             // Credentials generated by the ThingsBoard server
  TBC_PROVISION_TYPE_DEVICE_SUPPLIES_ACCESS_TOKEN,             // Devices supplies Access Token
  TBC_PROVISION_TYPE_DEVICE_SUPPLIES_BASIC_MQTT_CREDENTIALS,   // Devices supplies Basic MQTT Credentials
  TBC_PROVISION_TYPE_DEVICE_SUPPLIES_X509_CREDENTIALS          // Devices supplies X.509 Certificate
} tbc_provison_type_t;

/*
| Provisioning request  | Parameter              | Description                                                                    | Credentials generated by <br/>the ThingsBoard server | Devices supplies<br/>Access Token | Devices supplies<br/>Basic MQTT Credentials | Devices supplies<br/>X.509 Certificate |
|-----------------------|------------------------|--------------------------------------------------------------------------------|------------------------------------------------------|-----------------------------------|---------------------------------------------|----------------------------------------|
|                       | deviceName             | Device name in ThingsBoard.                                                    | (O) DEVICE_NAME                                      | (O) DEVICE_NAME                   | (O) DEVICE_NAME                             | (O) DEVICE_NAME                        |
|                       | provisionDeviceKey     | Provisioning device key, you should take it from configured device profile.    | (M) PUT_PROVISION_KEY_HERE                           | (M) PUT_PROVISION_KEY_HERE        | (M) PUT_PROVISION_KEY_HERE                  | (M) PUT_PROVISION_KEY_HERE             |
|                       | provisionDeviceSecret  | Provisioning device secret, you should take it from configured device profile. | (M) PUT_PROVISION_SECRET_HERE                        | (M) PUT_PROVISION_SECRET_HERE     | (M) PUT_PROVISION_SECRET_HERE               | (M) PUT_PROVISION_SECRET_HERE          |
|                       | credentialsType        | Credentials type parameter.                                                    |                                                      | (M) ACCESS_TOKEN                  | (M) MQTT_BASIC                              | (M) X509_CERTIFICATE                   |
|                       | token                  | Access token for device in ThingsBoard.                                        |                                                      | (M) DEVICE_ACCESS_TOKEN           |                                             |                                        |
|                       | clientId               | Client id for device in ThingsBoard.                                           |                                                      |                                   | (M) DEVICE_CLIENT_ID_HERE                   |                                        |
|                       | username               | Username for device in ThingsBoard.                                            |                                                      |                                   | (M) DEVICE_USERNAME_HERE                    |                                        |
|                       | password               | Password for device in ThingsBoard.                                            |                                                      |                                   | (M) DEVICE_PASSWORD_HERE                    |                                        |
|                       | hash                   | Public key X509 hash for device in ThingsBoard.                                |                                                      |                                   |                                             | (M) MIIB……..AQAB                       |
|                       | (O) Optional, (M) Must |
*/
typedef struct tbc_provison_config
{
  tbc_provison_type_t provisionType; // Generates/Supplies credentials type. // Hardcode or device profile.
 
  const char *deviceName;           // Device name in TB        // Chip name + Chip id, e.g., "esp32-C8:D6:93:12:BC:01". Each device is different.
  const char *provisionDeviceKey;   // Provision device key     // Hardcode or device profile. Each model is different. 
  const char *provisionDeviceSecret;// Provision device secret  // Hardcode or device profile. Each model is different.

  const char *token;     // Access token for device             // Randomly generated. Each device is different.
  const char *clientId;  // Client id for device                // Randomly generated. Each device is different.
  const char *username;  // Username for device                 // Randomly generated. Each device is different.
  const char *password;  // Password for device                 // Randomly generated. Each device is different.
  const char *hash;      // Public key X509 hash for device     // Public key X509.    Each device is different.
} tbc_provison_config_t;

typedef void (*tbcmh_provision_on_response_t)(tbcmh_handle_t client, void *context,
                                              int request_id, const tbc_transport_credentials_config_t *credentials); /*, tbcmh_provision_params_t *params*/
typedef void (*tbcmh_provision_on_timeout_t)(tbcmh_handle_t client, void *context,
                                             int request_id); /*, tbcmh_provision_params_t *params*/

//====9.Firmware/Software update=======================================================================================
/**
 * ThingsBoard MQTT Client Helper OTA update type
 */
typedef enum
{
  TBCMH_OTAUPDATE_TYPE_FW = 0, /*!< F/W OTA update */
  TBCMH_OTAUPDATE_TYPE_SW      /*!< S/W OTA update */
} tbcmh_otaupdate_type_t;

// Don't call TBCMH API in these callback!
typedef const char *(*tbcmh_otaupdate_on_get_current_ota_title_t)(tbcmh_handle_t client, void *context);
typedef const char *(*tbcmh_otaupdate_on_get_current_ota_version_t)(tbcmh_handle_t client, void *context);
// return 1 on negotiate successful(next to F/W OTA), -1/ESP_FAIL on negotiate failure, 0/ESP_OK on already updated!
typedef tbc_err_t (*tbcmh_otaupdate_on_negotiate_t)(tbcmh_handle_t client, void *context,
                                                    const char *ota_title, const char *ota_version, int ota_size,
                                                    const char *ota_checksum, const char *ota_checksum_algorithm,
                                                    char *ota_error, int error_size);
// return 0/ESP_OK on successful, -1/ESP_FAIL on failure
typedef tbc_err_t (*tbcmh_otaupdate_on_write_t)(tbcmh_handle_t client, void *context,
                                                int request_id, int chunk_id /*current chunk_id*/, const void *ota_data, int data_size,
                                                char *ota_error, int error_size);
// return 0/ESP_OK on successful, -1/ESP_FAIL on failure
typedef tbc_err_t (*tbcmh_otaupdate_on_end_t)(tbcmh_handle_t client, void *context,
                                              int request_id, int chunk_id,
                                              char *ota_error, int error_size);
typedef void (*tbcmh_otaupdate_on_abort_t)(tbcmh_handle_t client, void *context,
                                           int request_id, int chunk_id /*current chunk_id*/);

//====0.tbcm client====================================================================================================
/**
 * ThingsBoard Client transport easy config.
 *
 */
// TODO: move to tbc_transport_config.c/h
typedef struct
{
  const char *uri;             /*!< Complete MQTT broker URI */
  const char *access_token;    /*!< Access Token */
  const bool log_rxtx_package; /*!< print Rx/Tx MQTT package */
} tbc_transport_config_esay_t;

#define TBCMH_FUNCTION_TIMESERIES_DATA      0x00000001
#define TBCMH_FUNCTION_ATTRIBUTES_REQUEST   0x00000002
#define TBCMH_FUNCTION_CLIENT_ATTRIBUTES    0x00000004
#define TBCMH_FUNCTION_SHARED_ATTRIBUTES    0x00000008
#define TBCMH_FUNCTION_SERVER_RPC           0x00000010
#define TBCMH_FUNCTION_CLIENT_RPC           0x00000020
#define TBCMH_FUNCTION_CLAIMING_DEVICE      0x00000040
#define TBCMH_FUNCTION_OTA_UPDATE           0x00000080

#define TBCMH_FUNCTION_DEVICE_PROVISION     0x00001000

#define TBCMH_FUNCTION_FULL_ATTRIBUTES      0x0000000E
#define TBCMH_FUNCTION_FULL_OTA_UPDATE      0x0000008A

#define TBCMH_FUNCTION_FULL_GENERAL         0x000000FF

tbcmh_handle_t tbcmh_init(bool is_running_in_mqtt_task);
void tbcmh_destroy(tbcmh_handle_t client);
bool tbcmh_connect_using_url(tbcmh_handle_t client, const tbc_transport_config_esay_t *config,
                             uint32_t function,
                             void *context,
                             tbcmh_on_connected_t on_connected,
                             tbcmh_on_disconnected_t on_disconnected);
bool tbcmh_connect(tbcmh_handle_t client, const tbc_transport_config_t *config,
                   uint32_t function,
                   void *context,
                   tbcmh_on_connected_t on_connected,
                   tbcmh_on_disconnected_t on_disconnected);

void tbcmh_disconnect(tbcmh_handle_t client);
bool tbcmh_is_connected(tbcmh_handle_t client);
bool tbcmh_has_events(tbcmh_handle_t client);
void tbcmh_run(tbcmh_handle_t client); // loop()/checkTimeout(), recv/parse/sendqueue/ack...

//====10.Publish Telemetry time-series data==============================================================================
tbc_err_t tbcmh_timeseriesdata_register(tbcmh_handle_t client, const char *key, void *context, tbcmh_tsdata_on_get_t on_get);
tbc_err_t tbcmh_timeseriesdata_unregister(tbcmh_handle_t client, const char *key);
tbc_err_t tbcmh_timeseriesdata_update(tbcmh_handle_t client, int count, /*const char *key,*/...);

//====20.Publish client-side device attributes to the server============================================================
tbc_err_t tbcmh_clientattribute_register(tbcmh_handle_t client, const char *key, void *context,
                                       tbcmh_clientattribute_on_get_t on_get);
tbc_err_t tbcmh_clientattribute_register_with_set(tbcmh_handle_t client, const char *key, void *context,
                                                tbcmh_clientattribute_on_get_t on_get,
                                                tbcmh_clientattribute_on_set_t on_set);
tbc_err_t tbcmh_clientattribute_unregister(tbcmh_handle_t client, const char *key);
tbc_err_t tbcmh_clientattribute_update(tbcmh_handle_t client, int count, /*const char *key,*/...);

//====21.Subscribe to shared device attribute updates from the server===================================================
tbc_err_t tbcmh_sharedattribute_append(tbcmh_handle_t client, const char *key, void *context,
                                       tbcmh_sharedattribute_on_set_t on_set);  // Call it before connect()
tbc_err_t tbcmh_sharedattribute_clear(tbcmh_handle_t client, const char *key); // remove shared_attribute from tbcmh_shared_attribute_list_t

//====22.Request client-side or shared device attributes from the server================================================
int tbcmh_attributesrequest_send(tbcmh_handle_t client,
                                 void *context,
                                 tbcmh_attributesrequest_on_response_t on_response,
                                 tbcmh_attributesrequest_on_timeout_t on_timeout,
                                 int count, /*const char *key,*/...); //return request_id on successful, otherwise return -1

//====30.Server-side RPC================================================================================================
tbc_err_t tbcmh_serverrpc_append(tbcmh_handle_t client, const char *method,
                                 void *context,
                                 tbcmh_serverrpc_on_request_t on_request); //Call it before connect()
tbc_err_t tbcmh_serverrpc_clear(tbcmh_handle_t client, const char *method); //remove from LIST_ENTRY(tbcmh_serverrpc_) & delete

//====31.Client-side RPC================================================================================================
// free params by caller/(user code)!
int tbcmh_clientrpc_of_oneway_request(tbcmh_handle_t client, const char *method, /*const*/ tbcmh_rpc_params_t *params);//add list
// free params by caller/(user code)!
int tbcmh_clientrpc_of_twoway_request(tbcmh_handle_t client, const char *method, /*const*/ tbcmh_rpc_params_t *params,
                                      void *context,
                                      tbcmh_clientrpc_on_response_t on_response,
                                      tbcmh_clientrpc_on_timeout_t on_timeout); //create to add to LIST_ENTRY(tbcmh_clientrpc_)

//====40.Claiming device using device-side key scenario============================================
tbc_err_t tbcmh_claiming_device_using_device_side_key(tbcmh_handle_t client,
                                                      const char *secret_key, uint32_t *duration_ms);

//====50.Device provisioning=======================================================================
// return request_id or ESP_FAIL
int tbcmh_provision_request(tbcmh_handle_t client,
                            const tbc_provison_config_t *config,
                            void *context,
                            tbcmh_provision_on_response_t on_response,
                            tbcmh_provision_on_timeout_t on_timeout);

//====60.Firmware update================================================================================================
/**
 * ThingsBoard MQTT Client Helper F/W update OTA config
 */
typedef struct tbcmh_otaupdate_config
{
  tbcmh_otaupdate_type_t ota_type; /*!< FW/TBCMH_OTAUPDATE_TYPE_FW or SW/TBCMH_OTAUPDATE_TYPE_SW  */
  int chunk_size;                  /*!< chunk_size, eg: 8192. 0 to get all F/W or S/W by request  */

  void *context;
  tbcmh_otaupdate_on_get_current_ota_title_t on_get_current_ota_title;     /*!< callback of getting current F/W or S/W OTA title */
  tbcmh_otaupdate_on_get_current_ota_version_t on_get_current_ota_version; /*!< callback of getting current F/W or S/W OTA version */

  tbcmh_otaupdate_on_negotiate_t on_ota_negotiate; /*!< callback of F/W or S/W OTA attributes */
  tbcmh_otaupdate_on_write_t on_ota_write;         /*!< callback of F/W or S/W OTA doing */
  tbcmh_otaupdate_on_end_t on_ota_end;             /*!< callback of F/W or S/W OTA success & end*/
  tbcmh_otaupdate_on_abort_t on_ota_abort;         /*!< callback of F/W or S/W OTA failure & abort */

  ////bool is_first_boot;            /*!< whether first boot after ota update  */
} tbcmh_otaupdate_config_t;

tbc_err_t tbcmh_otaupdate_append(tbcmh_handle_t client, const char *ota_description, const tbcmh_otaupdate_config_t *config);
tbc_err_t tbcmh_otaupdate_clear(tbcmh_handle_t client, const char *ota_description);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
