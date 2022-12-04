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

// ThingsBoard Client MQTT wapper (low layer) API

#ifndef _TBC_MQTT_WAPPER_H_
#define _TBC_MQTT_WAPPER_H_

#include <stdint.h>
#include <stdbool.h>

#include "mqtt_client.h"

#include "tbc_mqtt_protocol.h"
#include "tbc_transport_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ThingsBoard Client MQTT state
 */
typedef enum
{
  TBCM_STATE_DISCONNECTED = 0,
  TBCM_STATE_CONNECTING,
  TBCM_STATE_CONNECTED
} tbcm_state_t;

/**
 * ThingsBoard Client MQTT handle
 */
typedef struct tbcm_client *tbcm_handle_t;

/**
 * @brief ThingsBoard Client MQTT topic ID
 *
 */
typedef enum {
    TBCM_RX_TOPIC_ERROR = 0,
    TBCM_RX_TOPIC_ATTRIBUTES_RESPONSE,  /*!< request_id,           payload, payload_len */
    TBCM_RX_TOPIC_SHARED_ATTRIBUTES,    /*!<                       payload, payload_len */
    TBCM_RX_TOPIC_SERVERRPC_REQUEST,    /*!< request_id,           payload, payload_len */
    TBCM_RX_TOPIC_CLIENTRPC_RESPONSE,   /*!< request_id,           payload, payload_len */
    TBCM_RX_TOPIC_FW_RESPONSE,          /*!< request_id, chunk_id, payload, payload_len */
    TBCM_RX_TOPIC_PROVISION_RESPONSE,   /*!< (no request_id)       payload, payload_len */

    // TBCM_TX_TOPIC_TELEMETRY,            /*!<                       */
    // TBCM_TX_TOPIC_CLIENT_ATTRIBUTES,    /*!<                       */
    // TBCM_TX_TOPIC_ATTRIBUTES_REQUEST,   /*!< request_id            */
    // TBCM_TX_TOPIC_SERVERRPC_RESPONSE,   /*!< request_id            */
    // TBCM_TX_TOPIC_CLIENTRPC_REQUEST,    /*!< request_id            */
    // TBCM_TX_TOPIC_CLAIMING_DEVICE,      /*!<                       */
    // TBCM_TX_TOPIC_FW_REQUEST,           /*!< request_id, chunk_id  */
    // TBCM_TX_TOPIC_PROVISION_REQUEST     /*!< (fake_request_id)     */
} tbcm_topic_id_t;

/**
 * @brief ThingsBoard Client Extension MQTT event.
 *
 * User event handler receives context data in `esp_mqtt_event_t` structure with
 *  - `user_context` - user data from `esp_mqtt_client_config_t`
 *  - `client` - mqtt client handle
 *  - various other data depending on event type
 *
 */
typedef enum {
    TBCM_EVENT_ERROR            = MQTT_EVENT_ERROR, // = 0
    TBCM_EVENT_CONNECTED        = MQTT_EVENT_CONNECTED,
    TBCM_EVENT_DISCONNECTED     = MQTT_EVENT_DISCONNECTED,
    TBCM_EVENT_SUBSCRIBED       = MQTT_EVENT_SUBSCRIBED,
    TBCM_EVENT_UNSUBSCRIBED     = MQTT_EVENT_UNSUBSCRIBED,
    TBCM_EVENT_PUBLISHED        = MQTT_EVENT_PUBLISHED,
    TBCM_EVENT_DATA             = MQTT_EVENT_DATA,
    TBCM_EVENT_BEFORE_CONNECT   = MQTT_EVENT_BEFORE_CONNECT,
    TBCM_EVENT_DELETED          = MQTT_EVENT_DELETED,

    TBCM_EVENT_CHECK_TIMEOUT    = MQTT_EVENT_DELETED + 100
} tbcm_event_id_t;

/**
 * @brief TB Client MQTT Publish data of sending or receiving
 *
 */
typedef struct tbcm_publish_data{
    tbcm_topic_id_t topic;          /*!< Topic associated with this event */
    uint32_t   request_id;          /*!< The first pararm in topic */
    uint32_t   chunk_id;            /*!< The second pararm in topic */
    char *payload;                  /*!< Payload associated with this event */
    int   payload_len;              /*!< Length of the payload for this event */
} tbcm_publish_data_t;

/**
 * @brief TB Client MQTT event structure
 */
typedef struct tbcm_event {
    tbcm_handle_t client;               /*!< TB Client MQTT Adapter handle for this event */
    void         *user_context;         /*!< User context passed from MQTT client config */

    tbcm_event_id_t event_id;       /*!< MQTT event type */
    int  msg_id;                        /*!< MQTT messaged id of message */
    int  session_present;               /*!< MQTT session_present flag for connection event */
    bool retain;                        /*!< Retained flag of the message associated with this event */

    esp_mqtt_error_codes_t error_handle;/*!< esp-mqtt error handle including esp-tls errors as well as internal mqtt errors */

    tbcm_publish_data_t data;          /*!< Publish data of sending or receiving */
} tbcm_event_t;

typedef void (*tbcm_on_event_t)(tbcm_event_t *event);

tbcm_handle_t tbcm_init(void);
void tbcm_destroy(tbcm_handle_t client);
bool tbcm_connect(tbcm_handle_t client, const tbc_transport_config_t *config,
                  void *context, tbcm_on_event_t on_event);  
void tbcm_disconnect(tbcm_handle_t client);
bool tbcm_is_connected(tbcm_handle_t client);
bool tbcm_is_connecting(tbcm_handle_t client);
bool tbcm_is_disconnected(tbcm_handle_t client);
tbcm_state_t tbcm_get_state(tbcm_handle_t client);

int tbcm_subscribe(tbcm_handle_t client, const char *topic, int qos /*=0*/);
int tbcm_unsubscribe(tbcm_handle_t client, const char *topic);

int tbcm_telemetry_publish(tbcm_handle_t client, const char *telemetry,
                           int qos /*= 1*/, int retain /*= 0*/);
int tbcm_clientattributes_publish(tbcm_handle_t client, const char *attributes,
                                  int qos /*= 1*/, int retain /*= 0*/);
int tbcm_attributes_request(tbcm_handle_t client, const char *payload,
                            uint32_t request_id,
                            int qos /*= 1*/, int retain /*= 0*/);
int tbcm_attributes_request_ex(tbcm_handle_t client, const char *client_keys, const char *shared_keys,
                               uint32_t request_id,
                               int qos /*= 1*/, int retain /*= 0*/);
int tbcm_serverrpc_response(tbcm_handle_t client, uint32_t request_id, const char *response,
                            int qos /*= 1*/, int retain /*= 0*/);
int tbcm_clientrpc_request(tbcm_handle_t client, const char *payload,
                           uint32_t request_id,
                           int qos /*= 1*/, int retain /*= 0*/);
int tbcm_clientrpc_request_ex(tbcm_handle_t client, const char *method, const char *params,
                              uint32_t request_id,
                              int qos /*= 1*/, int retain /*= 0*/);
int tbcm_claiming_device_publish(tbcm_handle_t client, const char *claiming,
                                 int qos /*= 1*/, int retain /*= 0*/);
int tbcm_provision_request(tbcm_handle_t client, const char *payload,
                           uint32_t request_id,
                           int qos /*= 1*/, int retain /*= 0*/);
int tbcm_otaupdate_chunk_request(tbcm_handle_t client,
                           uint32_t request_id, uint32_t chunk_id, const char *payload, //?payload
                           int qos /*= 1*/, int retain /*= 0*/);

#define TBCM_TELEMETRY_PUBLISH(client, payload) \
          tbcm_telemetry_publish(client, payload, /*int qos =*/1, /*int retain =*/0)
#define TBCM_CLIENTATTRIBUTES_PUBLISH(client, payloady) \
          tbcm_clientattributes_publish(client, payload, /*int qos =*/1, /*int retain =*/0)
#define TBCM_ATTRIUTES_REQUEST(client, payload, context, on_attrrequest_response, on_attrrequest_timeout) \
          tbcm_attributes_request(client, payload, context, on_attrrequest_response, on_attrrequest_timeout, /*int qos =*/1, /*int retain =*/0)
#define TBCM_ATTRIUTES_REQUEST_EX(client, client_keys, shared_keys, context, on_attrrequest_response, on_attrrequest_timeout) \
          tbcm_attributes_request_ex(client, client_keys, shared_keys, context, on_attrrequest_response, on_attrrequest_timeout, /*int qos =*/1, /*int retain =*/0)
#define TBCM_SERVERRPC_RESPONSE(client, request_id, response) \
          tbcm_serverrpc_response(client, request_id, response, /*int qos =*/1, /*int retain =*/0)
#define TBCM_CLIENTRPC_REQUEST(client, payload, context, on_clientrpc_response, on_clientrpc_timeout) \
          tbcm_clientrpc_request(client, payload, context, on_clientrpc_response, on_clientrpc_timeout, /*int qos =*/1, /*int retain =*/0)
#define TBCM_CLIENTRPC_REQUEST_EX(client, method, params, context, on_clientrpc_response, on_clientrpc_timeout) \
          tbcm_clientrpc_request_ex(client, method, params, context, on_clientrpc_response, on_clientrpc_timeout, /*int qos =*/1, /*int retain =*/0)
#define TBCM_OTA_REQUEST_SEND(client, request_id, chunk_id, payload) \
          tbcm_ota_request_send(client, request_id, chunk_id, payload, /*int qos =*/1, /*int retain =*/0)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
