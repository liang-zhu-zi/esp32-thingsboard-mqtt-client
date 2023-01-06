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

// ThingsBoard Client MQTT (low layer) API

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "sys/queue.h"
#include "esp_err.h"
#include "mqtt_client.h"

#include "tbc_utils.h"

#include "tbc_transport_config.h"
#include "tbc_transport_storage.h"

#include "tbc_mqtt_wapper.h"

#include "tbc_mqtt_payload_buffer.h"

/**
 * ThingsBoard MQTT Client
 */
typedef struct tbcm_client
{
    esp_mqtt_client_handle_t mqtt_handle;

    tbc_transport_storage_t config;     /*!< ThingsBoard MQTT config */
    void *context;

    tbcm_on_event_t on_event;           /*!< Callback of events */
    volatile tbcm_state_t state;

    SemaphoreHandle_t lock;

    tbcm_payload_buffer_t buffer;       /*!< If payload may be into multiple packets, then multiple packages need to be merged, eg: F/W OTA! */
    esp_timer_handle_t respone_timer;   /*!< timer for checking response timeout */
} tbcm_t;

static void _on_mqtt_event_handle(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

const static char *TAG = "tb_mqtt_wapper";

static bool __convert_timer_event(tbcm_event_t *dst_event)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(dst_event, false);
    
    memset(dst_event, 0x00, sizeof(tbcm_event_t));
    //dst_event->client        = src_event->client;          /*!< TB Client MQTT Adapter handle for this event */
    //dst_event->user_context  = src_event->user_context;    /*!< User context passed from MQTT client config */
    dst_event->event_id        = TBCM_EVENT_CHECK_TIMEOUT;   //src_event->event_id;        /*!< MQTT event type */
    //dst_event->msg_id          = src_event->msg_id;          /*!< MQTT messaged id of message */
    //dst_event->session_present = src_event->session_present; /*!< MQTT session_present flag for connection event */
    //dst_event->retain          = src_event->retain;          /*!< Retained flag of the message associated with this event */
    //memcpy(&dst_event->error_handle, src_event->error_handle, sizeof(esp_mqtt_error_codes_t));
                                                    /*!< esp-mqtt error handle including esp-tls errors as well as internal mqtt errors */
    //dst_event->data.topic       = data->topic;      /*!< Topic associated with this event */
    //dst_event->data.request_id  = data->request_id; /*!< The first pararm in topic */
    //dst_event->data.chunk_id    = data->chunk_id;   /*!< The second pararm in topic */
    //dst_event->data.payload     = data->payload;    /*!< Payload associated with this event */
    //dst_event->data.payload_len = data->payload_len;/*!< Length of the payload for this event */

    return true;
}

static bool __convert_data_event(tbcm_event_t *dst_event, 
                                const esp_mqtt_event_handle_t src_event,
                                const tbcm_publish_data_t *data)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(dst_event, false);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(src_event, false);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(data, false);
    
    memset(dst_event, 0x00, sizeof(tbcm_event_t));
    //dst_event->client        = src_event->client;          /*!< TB Client MQTT Adapter handle for this event */
    //dst_event->user_context  = src_event->user_context;    /*!< User context passed from MQTT client config */
    dst_event->event_id        = src_event->event_id;        /*!< MQTT event type */
    dst_event->msg_id          = src_event->msg_id;          /*!< MQTT messaged id of message */
    dst_event->session_present = src_event->session_present; /*!< MQTT session_present flag for connection event */
    dst_event->retain          = src_event->retain;          /*!< Retained flag of the message associated with this event */
    memcpy(&dst_event->error_handle, src_event->error_handle, sizeof(esp_mqtt_error_codes_t));
                                                    /*!< esp-mqtt error handle including esp-tls errors as well as internal mqtt errors */
    dst_event->data.topic       = data->topic;      /*!< Topic associated with this event */
    dst_event->data.request_id  = data->request_id; /*!< The first pararm in topic */
    dst_event->data.chunk_id    = data->chunk_id;   /*!< The second pararm in topic */
    dst_event->data.payload     = data->payload;    /*!< Payload associated with this event */
    dst_event->data.payload_len = data->payload_len;/*!< Length of the payload for this event */

    return true;
}

static bool __convert_nondata_event(tbcm_event_t *dst_event, const esp_mqtt_event_t *src_event)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(dst_event, false);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(src_event, false);

     memset(dst_event, 0x00, sizeof(tbcm_event_t));
     //dst_event->client        = src_event->client;          /*!< TB Client MQTT Adapter handle for this event */
     //dst_event->user_context  = src_event->user_context;    /*!< User context passed from MQTT client config */
     dst_event->event_id        = src_event->event_id;        /*!< MQTT event type */
     dst_event->msg_id          = src_event->msg_id;          /*!< MQTT messaged id of message */
     dst_event->session_present = src_event->session_present; /*!< MQTT session_present flag for connection event */
     memcpy(&dst_event->error_handle, src_event->error_handle, sizeof(esp_mqtt_error_codes_t));
                                                              /*!< esp-mqtt error handle including esp-tls errors as well as internal mqtt errors */
     dst_event->retain = src_event->retain; /*!< Retained flag of the message associated with this event */
     dst_event->data.topic = 0;             /*!< Topic associated with this event */
     dst_event->data.request_id = 0;        /*!< The first pararm in topic */
     dst_event->data.chunk_id = 0;          /*!< The second pararm in topic */
     dst_event->data.payload = NULL;        /*!< Payload associated with this event */
     dst_event->data.payload_len = 0;       /*!< Length of the payload for this event */
     return true;
}

static void __response_timer_timerout(void *client_/*timer_arg*/)
{
     tbcm_t *client = (tbcm_t *)client_;
     TBC_CHECK_PTR(client);

     tbcm_event_t dst_event = {0};
     __convert_timer_event(&dst_event);
     dst_event.client       = client;
     dst_event.user_context = client->context;
     client->on_event(&dst_event);
}

static void _response_timer_create(tbcm_handle_t client)
{
    TBC_CHECK_PTR(client);

    esp_timer_create_args_t tmr_args = {
        .callback = &__response_timer_timerout,
        .arg = client,
        .name = "response_timer",
    };
    esp_timer_create(&tmr_args, &client->respone_timer);
}

static void _response_timer_start(tbcm_handle_t client)
{
    TBC_CHECK_PTR(client);

    esp_timer_start_periodic(client->respone_timer, (uint64_t)TB_MQTT_TIMEOUT * 1000 * 1000);
}

static void _response_timer_stop(tbcm_handle_t client)
{
    TBC_CHECK_PTR(client);

    esp_timer_stop(client->respone_timer);
}

static void _response_timer_destroy(tbcm_handle_t client)
{
    TBC_CHECK_PTR(client);

    esp_timer_stop(client->respone_timer);
    esp_timer_delete(client->respone_timer);
    client->respone_timer = NULL;
}

static void *_tbc_transport_config_fill_to_mqtt_client_config(
                                        const tbc_transport_config_t *transport,
                                        esp_mqtt_client_config_t *mqtt_config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(mqtt_config, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(transport, NULL);

    // address
    bool tlsEnabled = false;
    if (strcmp(transport->address.schema, "mqtt") == 0) {
        mqtt_config->transport = MQTT_TRANSPORT_OVER_TCP;
        mqtt_config->port = 1883;
    } else if (strcmp(transport->address.schema, "mqtts") == 0) {
        mqtt_config->transport = MQTT_TRANSPORT_OVER_SSL;
        mqtt_config->port = 8883;
        tlsEnabled = true;
    } else if (strcmp(transport->address.schema, "ws") == 0) {
        mqtt_config->transport = MQTT_TRANSPORT_OVER_WS;
        mqtt_config->port = 80;
    } else if (strcmp(transport->address.schema, "wss") == 0) {
        mqtt_config->transport = MQTT_TRANSPORT_OVER_WSS;
        mqtt_config->port = 443;
        tlsEnabled = true;
    } else {
        ESP_LOGE(TAG, "address->schema(%s) is error!", transport->address.schema);
        return NULL;
    }
   
    if (transport->address.host) {
        mqtt_config->host = transport->address.host;
    } else {
        ESP_LOGE(TAG, "mqtt_config->host is NULL!");
        return NULL;
    }

    if (transport->address.port) {
        mqtt_config->port = transport->address.port;
    }
    if (transport->address.path) {
        mqtt_config->path = transport->address.path;
    }

    //credentials
    switch (transport->credentials.type) {
    case TBC_TRANSPORT_CREDENTIALS_TYPE_NONE: // for provision
        mqtt_config->username = transport->credentials.username; //"provision"
        break;
    case TBC_TRANSPORT_CREDENTIALS_TYPE_ACCESS_TOKEN: // Access Token
        if (!transport->credentials.token) {
             TBC_LOGE("credentials->token is NULL! %s()", __FUNCTION__);
             return NULL;
        }
        mqtt_config->username = transport->credentials.token;
        break;
        
    case TBC_TRANSPORT_CREDENTIALS_TYPE_BASIC_MQTT: // Basic MQTT Credentials.for MQTT
        if (!transport->credentials.client_id && !transport->credentials.username) {
             TBC_LOGE("credentials->client_id && credentials->username are NULL in Basic MQTT authentication! %s()",
                __FUNCTION__);
             return NULL;
        }
        mqtt_config->client_id = transport->credentials.client_id;
        mqtt_config->username = transport->credentials.username;
        mqtt_config->password = transport->credentials.password;
        break;
        
    case TBC_TRANSPORT_CREDENTIALS_TYPE_X509:      // X.509 Certificate
        if (!tlsEnabled) {
            TBC_LOGE("credentials->type(%d) and address->schema(%s) is not match! ()%s",
                transport->credentials.type, transport->address.schema, __FUNCTION__);
            return NULL;
        }
        // NOTE: transport->credentials.token: At TBC_TRANSPORT_CREDENTIALS_TYPE_X509 it's a client public key. DON'T USE IT! */
        break;
        
    default:
        ESP_LOGE(TAG, "credentials->type(%d) is error!", transport->credentials.type);
        return NULL;
    }

    //authentication
    if (tlsEnabled) {
        if (!transport->verification.cert_pem) {
            TBC_LOGE("verification->cert_pem is request but it is NULL! %s()", __FUNCTION__);
            return NULL;
        }
        mqtt_config->cert_pem = transport->verification.cert_pem;
        mqtt_config->cert_len = transport->verification.cert_len;
        mqtt_config->skip_cert_common_name_check = transport->verification.skip_cert_common_name_check;

        // SSL mutual authentication (two-way SSL)
        if (transport->credentials.type == TBC_TRANSPORT_CREDENTIALS_TYPE_X509) {
            if (!transport->authentication.client_cert_pem) {
                TBC_LOGE("authentication->client_cert_pem is request but it is NULL! %s()", __FUNCTION__);
                return NULL;
            }
            if (!transport->authentication.client_key_pem) {
                TBC_LOGE("authentication->client_key_pem is request but it is NULL! %s()", __FUNCTION__);
                return NULL;
            }
            mqtt_config->client_cert_pem         = transport->authentication.client_cert_pem;
            mqtt_config->client_cert_len         = transport->authentication.client_cert_len;
            mqtt_config->client_key_pem          = transport->authentication.client_key_pem;
            mqtt_config->client_key_len          = transport->authentication.client_key_len;
            mqtt_config->clientkey_password      = transport->authentication.client_key_password;
            mqtt_config->clientkey_password_len  = transport->authentication.client_key_password_len;
        }
    }

    return mqtt_config;
}

// Initializes tbcm_handle_t with network client.
tbcm_handle_t tbcm_init(void)
{
     tbcm_t *client = TBC_MALLOC(sizeof(tbcm_t));
     if (!client) {
          TBC_LOGE("Unable to malloc memeory!");
          return NULL;
     }

     client->mqtt_handle = NULL;
     memset(&client->config, 0x00, sizeof(client->config));
     client->context = NULL;
     client->on_event = NULL;

     client->state = TBCM_STATE_DISCONNECTED;

     client->lock = xSemaphoreCreateMutex();

     tbcm_payload_buffer_init(&client->buffer);
     _response_timer_create(client);
     return client;
}

// Destroys tbcm_handle_t with network client.
void tbcm_destroy(tbcm_handle_t client)
{
     TBC_CHECK_PTR(client);

     if (client->mqtt_handle) {
          tbcm_disconnect(client);
          client->mqtt_handle = NULL;
     }
     if (client->lock) {
          vSemaphoreDelete(client->lock);
          client->lock = NULL;
     }

     tbcm_payload_buffer_clear(&client->buffer);
     _response_timer_stop(client);
     _response_timer_destroy(client);

     TBC_FREE(client);
}

// Connects to the specified ThingsBoard server and port.
// Access token is used to authenticate a client.
// Returns true on success, false otherwise.
bool tbcm_connect(tbcm_handle_t client,
                  const tbc_transport_config_t *config,
                  void *context,
                  tbcm_on_event_t on_event)
{
     /*const char *host, int port = 1883, */
     /*min_reconnect_delay=1, timeout=120, tls=False, ca_certs=None, cert_file=None, key_file=None*/
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, false);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(config, false);
     //TBC_CHECK_PTR_WITH_RETURN_VALUE(client->mqtt_handle, false);

     tbc_transport_storage_free_fields(&client->config);
     client->context = NULL;
     client->on_event = NULL;
     client->state = TBCM_STATE_DISCONNECTED;

     esp_mqtt_client_config_t mqtt_cfg = {0};
     _tbc_transport_config_fill_to_mqtt_client_config(config, &mqtt_cfg);
     client->mqtt_handle = esp_mqtt_client_init(&mqtt_cfg);
     if (!client->mqtt_handle)
     {
          TBC_LOGW("unable to init mqtt client");
          return false;
     }
     /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
     esp_mqtt_client_register_event(client->mqtt_handle, ESP_EVENT_ANY_ID, _on_mqtt_event_handle, client);
     int32_t result = esp_mqtt_client_start(client->mqtt_handle);
     if (result != ESP_OK)
     {
          TBC_LOGW("unable to start mqtt client");
          esp_mqtt_client_destroy(client->mqtt_handle);
          client->mqtt_handle = NULL;
          return false;
     }

     tbc_transport_storage_fill_from_config(&client->config, config);
     client->context = context;
     client->on_event = on_event;
     client->state = TBCM_STATE_CONNECTING;
     return true;
}

// Disconnects from ThingsBoard. Returns true on success.
void tbcm_disconnect(tbcm_handle_t client) // disconnect()//...stop()
{
     TBC_CHECK_PTR(client);
     TBC_CHECK_PTR(client->mqtt_handle);

     //void *context = client->context;

     TBC_LOGI("tbcm_disconnect(): call esp_mqtt_client_stop()...");
     int32_t result = esp_mqtt_client_stop(client->mqtt_handle);
     if (result != ESP_OK) {
          TBC_LOGE("unable to stop mqtt client");
          return;
     }
     TBC_LOGI("tbcm_disconnect(): call esp_mqtt_client_destroy()...");
     result = esp_mqtt_client_destroy(client->mqtt_handle);
     if (result != ESP_OK) {
          TBC_LOGE("unable to stop mqtt client");
          return;
     }
     client->mqtt_handle = NULL;

     tbc_transport_storage_free_fields(&client->config);
     client->context = NULL;
     client->on_event = NULL;

     client->state = TBCM_STATE_DISCONNECTED;
}

// Returns true if connected, false otherwise.
bool tbcm_is_connected(tbcm_handle_t client) // isConnected
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, false);

     return client->state == TBCM_STATE_CONNECTED; 
}

bool tbcm_is_connecting(tbcm_handle_t client)
{
     return client->state == TBCM_STATE_CONNECTING; 
}

bool tbcm_is_disconnected(tbcm_handle_t client)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, false);

     return client->state == TBCM_STATE_DISCONNECTED; 
}

tbcm_state_t tbcm_get_state(tbcm_handle_t client)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, TBCM_STATE_DISCONNECTED);

     return client->state;
}

/**
 * @brief Subscribe the client to defined topic with defined qos
 *
 * Notes:
 * - Client must be connected to send subscribe message
 * - This API is could be executed from a user task or
 * from a mqtt event callback i.e. internal mqtt task
 * (API is protected by internal mutex, so it might block
 * if a longer data receive operation is in progress.
 *
 * @param client    mqtt client handle
 * @param topic
 * @param qos
 *
 * @return message_id of the subscribe message on success
 *         -1 on failure
 */
int tbcm_subscribe(tbcm_handle_t client, const char *topic, int qos /*=0*/)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client->mqtt_handle, -1);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(topic, -1);

     return esp_mqtt_client_subscribe(client->mqtt_handle, topic, qos);
}

/**
 * @brief Unsubscribe the client from defined topic
 *
 * Notes:
 * - Client must be connected to send unsubscribe message
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 *
 * @param client    mqtt client handle
 * @param topic
 *
 * @return message_id of the subscribe message on success
 *         -1 on failure
 */
int tbcm_unsubscribe(tbcm_handle_t client, const char *topic)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client->mqtt_handle, -1);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(topic, -1);

     return esp_mqtt_client_unsubscribe(client->mqtt_handle, topic);
}


/**
 * @brief Client to send a publish message to the broker
 *
 * Notes:
 * - Client doesn't have to be connected to send publish message
 *   (although it would drop all qos=0 messages, qos>1 messages would be enqueued)
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 *
 * @param topic     topic string
 * @param payload   payload string (set to NULL, sending empty payload message)
 * @param qos       qos of publish message
 * @param retain    ratain flag
 *
 * @return message_id of the subscribe message on success
 *         0 if cannot publish
 *        -1 if error
 */
static int _tbcm_publish(tbcm_handle_t client, const char *topic, const char *payload,
                        int qos /*= 1*/, int retain /*= 0*/)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client->mqtt_handle, -1);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(topic, -1);

     int len = (payload == NULL) ? 0 : strlen(payload); //// +1
     return esp_mqtt_client_publish(client->mqtt_handle, topic, payload, len, qos, retain); ////return msg_id or -1(failure)
}

/**
 * @brief Client to send a 'Telemetry' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/telemetry'
 *      Data:  '{"key1":"value1", "key2":true, "key3": 3.0, "key4": 4}', '[{"key1":"value1"}, {"key2":true}]'
 *
 * @param telemetry  telemetry. example: {"key1":"value1", "key2":true, "key3": 3.0, "key4": 4}, (字符串要符合 json 数据格式)
 * @param qos        qos of publish message
 * @param retain     ratain flag
 *
 * @return msg_id of the subscribe message on success
 *         0 if cannot publish
 *        -1 if error
 */
int tbcm_telemetry_publish(tbcm_handle_t client, const char *telemetry,
                           int qos /*= 1*/, int retain /*= 0*/)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);

     if (client->config.log_rxtx_package) {
        TBC_LOGI("[Telemetry][Tx] %.*s", strlen(telemetry), telemetry);
     }

     int msg_id = _tbcm_publish(client, TB_MQTT_TOPIC_TELEMETRY_PUBLISH, telemetry, qos, retain);
     return msg_id;
}

/**
 * @brief Client to send a 'Attributes' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/attributes'
 *      Data:  '{"attribute1":"value1", "attribute2":true, "attribute3":42.0, "attribute4":73}'
 *
 * @param attributes    attributes. example: {"attribute1":"value1", "attribute2":true, "attribute3":42.0, "attribute4":73} (字符串要符合 json 数据格式)
 * @param qos           qos of publish message
 * @param retain        ratain flag
 *
 * @return message_id of the subscribe message on success
 *         0 if cannot publish
 *        -1 if error
 */
int tbcm_clientattributes_publish(tbcm_handle_t client, const char *attributes,
                                         int qos /*= 1*/, int retain /*= 0*/)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);

     if (client->config.log_rxtx_package) {
        TBC_LOGI("[Client-Side Attributes][Tx] %.*s", strlen(attributes), attributes);
     }

     int message_id = _tbcm_publish(client, TB_MQTT_TOPIC_CLIENT_ATTRIBUTES_PUBLISH, attributes, qos, retain);
     return message_id;
}

/**
 * @brief Client to send a 'Attributes Request' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/attributes/request/$request_id'
 *      Data:  '{"clientKeys":"attribute1,attribute2", "sharedKeys":"shared1,shared2"}'
 *
 * @param payload        payload
 * @param on_attrrequest_response     Attributes response callback
 * @param on_attrrequest_timeout  Attributes response timeout callback
 * @param qos            qos of publish message
 * @param retain         ratain flag
 *
 * @return msg_id of the subscribe message on success
 *        -1 if error
 */
int tbcm_attributes_request(tbcm_handle_t client, const char *payload,
                            uint32_t request_id,
                            int qos /*= 1*/, int retain /*= 0*/)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client->mqtt_handle, -1);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(payload, -1);

     int size = strlen(TB_MQTT_TOPIC_ATTRIBUTES_REQUEST_PREFIX) + 20;
     char *topic = TBC_MALLOC(size);
     if (!topic) {
          TBC_LOGE("Unable to malloc memory");
          return -1;
     }
     memset(topic, 0x00, size);
     snprintf(topic, size - 1, TB_MQTT_TOPIC_ATTRIBUTES_REQUEST_PATTERN, request_id);

     if (client->config.log_rxtx_package) {
        TBC_LOGI("[Attributes Request][Tx] request_id=%u, %.*s",
            request_id, strlen(payload), payload);
     }

     int message_id = _tbcm_publish(client, topic, payload, qos, retain);
     TBC_FREE(topic);
     return message_id;
}

/**
 * @brief Client to send a 'Attributes Request' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/attributes/request/$request_id'
 *      Data:  '{"clientKeys":"attribute1,attribute2", "sharedKeys":"shared1,shared2"}'
 *
 * @param client_keys   client attribute names. A ending char is '\0'. example: "attribute1,attribute2" (字符串要自带双引号,逗号分隔!!)
 * @param shared_keys   shared attribute names. A ending char is '\0'. example: "shared1,shared2"       (字符串要自带双引号,逗号分隔!!)
 * @param on_attrrequest_response     Attributes response callback
 * @param on_attrrequest_timeout  Attributes response timeout callback
 * @param qos           qos of publish message
 * @param retain        ratain flag
 *
 * @return msg_id of the subscribe message on success
 *        -1 if error
 */
int tbcm_attributes_request_ex(tbcm_handle_t client,
                               const char *client_keys, const char *shared_keys,
                               uint32_t request_id,
                               int qos /*= 1*/, int retain /*= 0*/)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);

     int client_len = 0;
     int shared_len = 0;
     if (client_keys && strlen(client_keys) > 0) {
          client_len = strlen(client_keys);
     }
     if (shared_keys && strlen(shared_keys) > 0) {
          shared_len = strlen(shared_keys);
     }
     if ((client_len<=0) && (shared_len<=0)) {
          TBC_LOGW("There are no keys to request");
          return -1;
     }

     int size = strlen(TB_MQTT_KEY_ATTRIBUTES_REQUEST_CLIENTKEYS) + client_len 
               + strlen(TB_MQTT_KEY_ATTRIBUTES_REQUEST_SHAREDKEYS) + shared_len + 20;
     char *payload = TBC_MALLOC(size);
     if (!payload)
     {
          TBC_LOGE("Unable to malloc memory");
          return -1;
     }
     memset(payload, 0x00, size);

     if ((client_len>0) && (shared_len>0)) {
         snprintf(payload, size - 1, "{\"clientKeys\":\"%s\", \"sharedKeys\":\"%s\"}",
                  client_keys, shared_keys);
     } else if (client_len>0) {
         snprintf(payload, size - 1, "{\"clientKeys\":\"%s\"}", client_keys);
     } else if (shared_len>0) {
         snprintf(payload, size - 1, "{\"sharedKeys\":\"%s\"}", shared_keys);
     }
     int msg_id = tbcm_attributes_request(client, payload,
                                          request_id,
                                          //context,
                                          //on_attrrequest_response,
                                          //on_attrrequest_timeout,
                                          qos, retain);
     TBC_FREE(payload);
     return msg_id;
}

/**
 * @brief Client to send a 'Server-Side RPC Response' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/rpc/response/$request_id'
 *      Data:  '{"example_response":23.1}' ???
 *
 * @param request_id  Server-Side RPC Request ID
 * @param response    payload string (set to NULL, sending empty payload message). A ending char is '\0'.
 * @param qos         qos of publish message
 * @param retain      ratain flag
 *
 * @return message_id of the subscribe message on success
 *         0 if cannot publish
 *        -1 if error
 */
int tbcm_serverrpc_response(tbcm_handle_t client, 
                            uint32_t request_id, const char *response,
                            int qos /*= 1*/, int retain /*= 0*/)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client->mqtt_handle, -1);

     int size = strlen(TB_MQTT_TOPIC_SERVERRPC_RESPONSE_PREFIX) + 20;
     char *topic = TBC_MALLOC(size);
     if (!topic) {
          TBC_LOGE("Unable to malloc memory!");
          return -1;
     }
     memset(topic, 0x00, size);
     snprintf(topic, size - 1, TB_MQTT_TOPIC_SERVERRPC_RESPONSE_PATTERN, request_id);

     if (client->config.log_rxtx_package) {
        TBC_LOGI("[Server-Side RPC][Tx] request_id=%u Payload=%.*s",
              request_id, strlen(response), response);
     }

     int message_id = _tbcm_publish(client, topic, response, qos, retain);
     TBC_FREE(topic);
     return message_id;
}

/**
 * @brief Client to send a 'Client-Side RPC Request' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/rpc/request/$request_id'
 *      Data:  '{"method":"getTime","params":{}}'
 *
 * @param method        RPC method name.   A ending char is '\0'. example:　"getTime"   (字符串要自带双引号!!)
 * @param params        RPC method params. A ending char is '\0'. example:　{}          (字符串是 json 格式!!)
 * @param on_clientrpc_response      Client-RPC response callback
 * @param on_clientrpc_timeout  Client-RPC response timeout callback
 * @param qos           qos of publish message
 * @param retain        ratain flag
 *
 * @return rpc_msg_id of the subscribe message on success
 *        -1 if error
 */
int tbcm_clientrpc_request(tbcm_handle_t client, const char *payload,
                           uint32_t request_id,
                           int qos /*= 1*/, int retain /*= 0*/)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client->mqtt_handle, -1);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(payload, -1);

     int size = strlen(TB_MQTT_TOPIC_CLIENTRPC_REQUEST_PREFIX) + 20;
     char *topic = TBC_MALLOC(size);
     if (!topic) {
          TBC_LOGE("Unable to malloc memory");
          return -1;
     }
     memset(topic, 0x00, size);
     snprintf(topic, size - 1, TB_MQTT_TOPIC_CLIENTRPC_REQUEST_PATTERN, request_id);

     if (client->config.log_rxtx_package) {
        TBC_LOGI("[Client-Side RPC][Tx] request_id=%u %.*s",
              request_id, strlen(payload), payload);
     }

     int msg_id = _tbcm_publish(client, topic, payload, qos, retain);
     TBC_FREE(topic);
     return msg_id;
}

/**
 * @brief Client to send a 'Client-Side RPC Request' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/rpc/request/$request_id'
 *      Data:  '{"method":"getTime","params":{}}'
 *
 * @param method        RPC method name.   A ending char is '\0'. example:　"getTime"   (字符串要自带双引号!!)
 * @param params        RPC method params. A ending char is '\0'. example:　{}          (字符串是 json 格式!!)
 * @param on_clientrpc_response      Client-RPC response callback
 * @param on_clientrpc_timeout  Client-RPC response timeout callback
 * @param qos           qos of publish message
 * @param retain        ratain flag
 *
 * @return rpc_msg_id of the subscribe message on success
 *        -1 if error
 */
int tbcm_clientrpc_request_ex(tbcm_handle_t client,
                              const char *method, const char *params,
                              uint32_t request_id,
                              int qos /*= 1*/, int retain /*= 0*/)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);

     int size = strlen(TB_MQTT_KEY_RPC_METHOD) + strlen(method) + strlen(TB_MQTT_KEY_RPC_PARAMS) + strlen(params) + 20;
     char *payload = TBC_MALLOC(size);
     if (!payload) {
          TBC_LOGE("Unable to malloc memory");
          return -1;
     }
     memset(payload, 0x00, size);
     snprintf(payload, size - 1, "{\"method\":\"%s\",\"params\":%s}", method, params); //{%s}
     int msg_id = tbcm_clientrpc_request(client, payload,
                                         request_id,
                                         //context,
                                         //on_clientrpc_response,
                                         //on_clientrpc_timeout,
                                         qos, retain);
     TBC_FREE(payload);
     return msg_id;
}
 
 /**
  * @brief Client to send a 'claiming_device_using_device_side_key' publish message to the broker
  *
  * Notes:
  * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
  * - A ThingsBoard MQTT Protocol message example:
  *      Topic: 'v1/devices/me/claim'
  *      Data:  '{"secretKey":"value", "durationMs":60000}'
  *
  * @param claiming   example: {"secretKey":"value", "durationMs":60000}, (字符串要符合 json 数据格式)
  * @param qos        qos of publish message
  * @param retain     ratain flag
  *
  * @return message_id of the subscribe message on success
  *         0 if cannot publish
  *        -1 if error
  */
int tbcm_claiming_device_publish(tbcm_handle_t client, const char *claiming,
                                 int qos /*= 1*/, int retain /*= 0*/)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);

     if (client->config.log_rxtx_package)
     {
          TBC_LOGI("[Claiming][Tx] %.*s", strlen(claiming), claiming);
     }

     int message_id = _tbcm_publish(client, TB_MQTT_TOPIC_CLAIMING_DEVICE, claiming, qos, retain);
     return message_id;
 }

/**
 * @brief Client to send a 'Provisoin Request' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic:   '/provision/request'
 *      payload: '{"deviceName": "DEVICE_NAME", "provisionDeviceKey": "u7piawkboq8v32dmcmpp", "provisionDeviceSecret": "jpmwdn8ptlswmf4m29bw"}'
 *
 * @param payload           
 * @param on_provision_response    f/w update response callback
 * @param on_provision_timeout     f/w update response timeout callback
 * @param qos            qos of publish message
 * @param retain         ratain flag
 *
 * @return rpc_msg_id of the subscribe message on success
 *        -1 if error
 */
 int tbcm_provision_request(tbcm_handle_t client, const char *payload,
                            uint32_t request_id,
                            int qos /*= 1*/, int retain /*= 0*/)
 {
      TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);
      TBC_CHECK_PTR_WITH_RETURN_VALUE(client->mqtt_handle, -1);
      TBC_CHECK_PTR_WITH_RETURN_VALUE(payload, -1);

      if (client->config.log_rxtx_package)
      {
           TBC_LOGI("[Provision][Tx] request_id=%u %.*s",
                    request_id, strlen(payload), payload);
      }

      int message_id = _tbcm_publish(client, TB_MQTT_TOPIC_PROVISION_REQUESTC, payload, qos, retain);
      return message_id;
}

/**
 * @brief Client to send a 'Client-Side RPC Request' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/rpc/request/$request_id'
 *      Data:  '{"method":"getTime","params":{}}'
 *
 * @param request_id_   0 on first f/w request(chunk_id is 0), otherwise if it is result of last tbcm_otaupdate_chunk_request()
 * @param chunk_id      0,1,2,3,...           
 * @param on_otaupdate_response    f/w update response callback
 * @param on_otaupdate_timeout     f/w update response timeout callback
 * @param qos            qos of publish message
 * @param retain         ratain flag
 *
 * @return rpc_msg_id of the subscribe message on success
 *        -1 if error
 */
int tbcm_otaupdate_chunk_request(tbcm_handle_t client,
                          uint32_t request_id, uint32_t chunk_id, const char *payload,
                          int qos /*= 1*/, int retain /*= 0*/)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, -1);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client->mqtt_handle, -1);
     //TBC_CHECK_PTR_WITH_RETURN_VALUE(payload, -1);

     int size = strlen(TB_MQTT_TOPIC_FW_REQUEST_PATTERN) + 20;
     char *topic = TBC_MALLOC(size);
     if (!topic) {
          TBC_LOGE("Unable to malloc memory");
          return -1;
     }
     memset(topic, 0x00, size);
     snprintf(topic, size - 1, TB_MQTT_TOPIC_FW_REQUEST_PATTERN, request_id, chunk_id);

     if (client->config.log_rxtx_package) {
        TBC_LOGI("[FW update][Tx] request_id=%u chunk_id=%u payload=%.*s",
              request_id, chunk_id, strlen(payload), payload);
     }

     int msg_id = _tbcm_publish(client, topic, payload, qos, retain);
     TBC_FREE(topic);
     return msg_id;
}

static void _on_mqtt_data_handle(void *client_, esp_mqtt_event_handle_t src_event,
                                      char *topic, int topic_len,
                                      char *payload, int payload_len)
{
    tbcm_t *client = (tbcm_t *)client_;
    tbcm_event_t dst_event = {0};
    tbcm_publish_data_t publish_data;

    TBC_CHECK_PTR(client);
    TBC_CHECK_PTR(src_event);
    TBC_CHECK_PTR(topic);
    TBC_CHECK_PTR(payload);

    memset(&dst_event, 0x00, sizeof(dst_event));
    memset(&publish_data, 0x00, sizeof(publish_data));

    if (strncmp(topic, TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PREFIX,
                strlen(TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PREFIX)) == 0) {
         // 3.TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PREFIX

         char temp[32] = {0};
         strncpy(temp, topic+strlen(TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PREFIX),
                 topic_len-strlen(TB_MQTT_TOPIC_ATTRIBUTES_RESPONSE_PREFIX));
         uint32_t request_id = 0;
         sscanf(temp, "%u", &request_id);
         if (client->config.log_rxtx_package) {
             TBC_LOGI("[Attributes Request][Rx] request_id=%u %.*s",
                  request_id, payload_len, payload);
         }

        publish_data.topic = TBCM_RX_TOPIC_ATTRIBUTES_RESPONSE;  /*!< Topic associated with this event */
        publish_data.request_id = request_id;                     /*!< The first pararm in topic */
        publish_data.chunk_id   = 0;                              /*!< The second pararm in topic */
        publish_data.payload    = payload;                        /*!< Payload associated with this event */
        publish_data.payload_len= payload_len;                    /*!< Length of the payload for this event */
        __convert_data_event(&dst_event, src_event, &publish_data);
        dst_event.client       = client;
        dst_event.user_context = client->context;
        client->on_event(&dst_event);

         /*
         tbcmh_request_t *tbcmh_request = _request_list_search_and_remove(client, request_id);
         if (tbcmh_request) {
              if (tbcmh_request->on_response) {
                   tbcm_on_response_t on_response = tbcmh_request->on_response;
                   on_response(client->context, request_id, payload, payload_len);
              }
              _request_destroy(tbcmh_request);
              tbcmh_request = NULL;
         } else {
              TBC_LOGE("Unable to find attributes requset(%d), (%.*s, %.*s)", request_id,
                      topic_len, topic, payload_len, payload);
              return; // -1;
         }*/
    
    } else if (strncmp(topic, TB_MQTT_TOPIC_SHARED_ATTRIBUTES,
                    strlen(TB_MQTT_TOPIC_SHARED_ATTRIBUTES)) == 0) {
                                            //if (strcmp(topic, TB_MQTT_TOPIC_SHARED_ATTRIBUTES) == 0) {
         // 1.TB_MQTT_TOPIC_SHARED_ATTRIBUTES
    
         if (client->config.log_rxtx_package) {
             TBC_LOGI("[Subscribe Shared Attributes][Rx] %.*s", payload_len, payload);
         }

         publish_data.topic = TBCM_RX_TOPIC_SHARED_ATTRIBUTES;  /*!< Topic associated with this event */
         publish_data.request_id = 0;                              /*!< The first pararm in topic */
         publish_data.chunk_id   = 0;                              /*!< The second pararm in topic */
         publish_data.payload    = payload;                        /*!< Payload associated with this event */
         publish_data.payload_len= payload_len;                    /*!< Length of the payload for this event */
         __convert_data_event(&dst_event, src_event, &publish_data);
         dst_event.client       = client;
         dst_event.user_context = client->context;
         client->on_event(&dst_event);

         /*
         if (client->on_sharedattr_received) {
              client->on_sharedattr_received(client->context, payload, payload_len);
         } else {
              TBC_LOGW("Unable to find shared-attributes, (%.*s, %.*s)",
                      topic_len, topic, payload_len, payload);
         }
         */
    
    } else if (strncmp(topic, TB_MQTT_TOPIC_SERVERRPC_REQUEST_PREFIX,
                    strlen(TB_MQTT_TOPIC_SERVERRPC_REQUEST_PREFIX)) == 0) {
         // 2.TB_MQTT_TOPIC_SERVERRPC_REQUEST_PREFIX

         char temp[32] = {0};
         strncpy(temp, topic+strlen(TB_MQTT_TOPIC_SERVERRPC_REQUEST_PREFIX),
                 topic_len-strlen(TB_MQTT_TOPIC_SERVERRPC_REQUEST_PREFIX));
         uint32_t request_id = 0;
         sscanf(temp, "%u", &request_id);
         if (client->config.log_rxtx_package) {
             TBC_LOGI("[Server-Side RPC][Rx] request_id=%d Payload=%.*s",
                  request_id, payload_len, payload);
         }

         publish_data.topic = TBCM_RX_TOPIC_SERVERRPC_REQUEST;    /*!< Topic associated with this event */
         publish_data.request_id = request_id;                     /*!< The first pararm in topic */
         publish_data.chunk_id   = 0;                              /*!< The second pararm in topic */
         publish_data.payload    = payload;                        /*!< Payload associated with this event */
         publish_data.payload_len= payload_len;                    /*!< Length of the payload for this event */
         __convert_data_event(&dst_event, src_event, &publish_data);
         dst_event.client       = client;
         dst_event.user_context = client->context;
         client->on_event(&dst_event);

         /*
         if (client->on_serverrpc_request) {
              client->on_serverrpc_request(client->context, request_id, payload, payload_len);
         } else {
              TBC_LOGW("Unable to find server-rpc request(%d), (%.*s, %.*s)", request_id,
                      topic_len, topic, payload_len, payload);
         }
         */
    } else if (strncmp(topic, TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PREFIX,
                    strlen(TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PREFIX)) == 0) {
         // 4.TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PREFIX
    
         char temp[32] = {0};
         strncpy(temp, topic+strlen(TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PREFIX),
                topic_len-strlen(TB_MQTT_TOPIC_CLIENTRPC_RESPONSE_PREFIX));
         uint32_t request_id = 0;
         sscanf(temp, "%u", &request_id);
         if (client->config.log_rxtx_package) {
             TBC_LOGI("[Client-Side RPC][Rx] request_id=%d %.*s",
                  request_id, payload_len, payload);
         }

         publish_data.topic = TBCM_RX_TOPIC_CLIENTRPC_RESPONSE;   /*!< Topic associated with this event */
         publish_data.request_id = request_id;                     /*!< The first pararm in topic */
         publish_data.chunk_id   = 0;                              /*!< The second pararm in topic */
         publish_data.payload    = payload;                        /*!< Payload associated with this event */
         publish_data.payload_len= payload_len;                    /*!< Length of the payload for this event */
         __convert_data_event(&dst_event, src_event, &publish_data);
         dst_event.client       = client;
         dst_event.user_context = client->context;
         client->on_event(&dst_event);

         /*
         tbcmh_request_t *tbcmh_request = _request_list_search_and_remove(client, request_id);
         if (tbcmh_request) {
              if (tbcmh_request->on_response) {
                   tbcm_on_response_t on_response = tbcmh_request->on_response;
                   on_response(client->context, request_id, payload, payload_len);
              }
              _request_destroy(tbcmh_request);
              tbcmh_request = NULL;
         } else {
              TBC_LOGE("Unable to find client-RPC requset(%d), (%.*s, %.*s)", request_id,
                      topic_len, topic, payload_len, payload);
              return; // -1;
         }*/
          
    } else if (strncmp(topic, TB_MQTT_TOPIC_FW_RESPONSE_PREFIX,
                strlen(TB_MQTT_TOPIC_FW_RESPONSE_PREFIX)) == 0) {
         // 5.TB_MQTT_TOPIC_FW_RESPONSE_PREFIX
         uint32_t request_id = 0;
         sscanf(topic, TB_MQTT_TOPIC_FW_RESPONSE_PATTERN, &request_id);

         uint32_t chunk_id = 0;
         const char *chunk_str = strstr(topic, "/chunk/");
         if (chunk_str) {
             char temp[32] = {0};
             int offset = (uint32_t)chunk_str - (uint32_t)topic;
             strncpy(temp, topic+offset+strlen("/chunk/"), topic_len-offset-strlen("/chunk/"));
             sscanf(temp, "%u", &chunk_id);
         }

         if (client->config.log_rxtx_package) {
             TBC_LOGI("[FW update][Rx] request_id=%u, chunk_id=%u, payload_len=%d",
                   request_id, chunk_id, payload_len);
         }

         publish_data.topic = TBCM_RX_TOPIC_FW_RESPONSE;          /*!< Topic associated with this event */
         publish_data.request_id = request_id;                     /*!< The first pararm in topic */
         publish_data.chunk_id   = chunk_id;                       /*!< The second pararm in topic */
         publish_data.payload    = payload;                        /*!< Payload associated with this event */
         publish_data.payload_len= payload_len;                    /*!< Length of the payload for this event */
         __convert_data_event(&dst_event, src_event, &publish_data);
         dst_event.client       = client;
         dst_event.user_context = client->context;
         client->on_event(&dst_event);

         /*
         tbcmh_request_t *tbcmh_request = _request_list_search_and_remove(client, request_id);
         if (tbcmh_request) {
              tbcm_on_otaupdate_response_t on_otaupdate_response = tbcmh_request->on_response;
              if (on_otaupdate_response) {
                   on_otaupdate_response(client->context, request_id, chunk_id, payload, payload_len);
              }
              _request_destroy(tbcmh_request);
              tbcmh_request = NULL;
         } else {
              TBC_LOGE("Unable to find FW update requset(%d), (%.*s, %.*s)",
                        request_id,
                        topic_len, topic, payload_len, payload);
              return; // -1;
         }*/
    
    } else if (strncmp(topic, TB_MQTT_TOPIC_PROVISION_RESPONSE,
                strlen(TB_MQTT_TOPIC_PROVISION_RESPONSE)) == 0) {
         // 6.TB_MQTT_TOPIC_PROVISION_RESPONSE
         if (client->config.log_rxtx_package) {
             TBC_LOGI("[Provision][Rx] topic_type=%d, payload_len=%d %.*s",
                   TBCM_RX_TOPIC_PROVISION_RESPONSE, payload_len, payload_len, payload);
         }

         publish_data.topic = TBCM_RX_TOPIC_PROVISION_RESPONSE;   /*!< Topic associated with this event */
         publish_data.request_id = 0;                              /*!< The first pararm in topic */
         publish_data.chunk_id   = 0;                              /*!< The second pararm in topic */
         publish_data.payload    = payload;                        /*!< Payload associated with this event */
         publish_data.payload_len= payload_len;                    /*!< Length of the payload for this event */
         __convert_data_event(&dst_event, src_event, &publish_data);
         dst_event.client       = client;
         dst_event.user_context = client->context;
         client->on_event(&dst_event);

         /*
         tbcmh_request_t *tbcmh_request = _request_list_search_and_remove_by_type(client, TBCMH_REQUEST_PROVISION);
         if (tbcmh_request) {
              tbcm_on_provision_response_t on_provision_response = tbcmh_request->on_response;
              if (on_provision_response) {
                   on_provision_response(client->context, tbcmh_request->request_id, payload, payload_len);
              }
              _request_destroy(tbcmh_request);
              tbcmh_request = NULL;
         } else {
              TBC_LOGE("Unable to find Provision request_type(%d), (%.*s, %.*s)",
                        TBCMH_REQUEST_PROVISION,
                        topic_len, topic, payload_len, payload);
              return; // -1;
         }*/
    
    }  else {
         // Payload is too long, then Serial
         TBC_LOGW("[Unkown-Msg][Rx] topic=%.*s, payload=%.*s, payload_len=%d",
                   topic_len, topic, payload_len, payload, payload_len);
    }
}

// The callback for when a MQTT event is received.
static void _on_mqtt_event_handle(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
     tbcm_t *client = (tbcm_t*)handler_args;
     esp_mqtt_event_handle_t src_event = event_data;

     TBC_CHECK_PTR(client);
     TBC_CHECK_PTR(src_event);

     switch (src_event->event_id) {
     case MQTT_EVENT_DATA:
          ////TBC_LOGI("MQTT_EVENT_DATA");
          ////TBC_LOGI("TOPIC=%.*s", event->topic_len, event->topic);
          ////TBC_LOGI("DATA=%.*s", event->data_len, event->data);
          {
              // If payload may be into multiple packets, then multiple packages need to be merged, eg: F/W OTA!
              tbcm_payload_buffer_pocess(&client->buffer, src_event, client, _on_mqtt_data_handle);
          }
          break;

    case MQTT_EVENT_CONNECTED:
        {
            client->state = TBCM_STATE_CONNECTED;
            _response_timer_start(client);
            
            tbcm_event_t dst_event;
            __convert_nondata_event(&dst_event, src_event);
            dst_event.client       = client;
            dst_event.user_context = client->context;
            client->on_event(&dst_event);
        }
        break;

    case MQTT_EVENT_DISCONNECTED:
        {
            client->state = TBCM_STATE_DISCONNECTED;

            tbcm_event_t dst_event;
            __convert_nondata_event(&dst_event, src_event);
            dst_event.client       = client;
            dst_event.user_context = client->context;
            client->on_event(&dst_event);
        }
        break;

    case MQTT_EVENT_SUBSCRIBED:
    case MQTT_EVENT_UNSUBSCRIBED:
    case MQTT_EVENT_PUBLISHED:
    case MQTT_EVENT_DELETED:
    case MQTT_EVENT_ERROR:
    case MQTT_EVENT_BEFORE_CONNECT:
    default:
        {
            TBC_LOGI("src_event->event_id=%d", src_event->event_id);
            tbcm_event_t dst_event;
            __convert_nondata_event(&dst_event, src_event);
            dst_event.client       = client;
            dst_event.user_context = client->context;
            client->on_event(&dst_event);
        }
        break;
     }

     return;
}

