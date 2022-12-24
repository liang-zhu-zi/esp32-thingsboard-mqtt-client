/* Publish telemetry data to ThingsBoard platform Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"

#include "tbc_mqtt_helper.h"
#include "protocol_examples_common.h"

extern const uint8_t server_cert_pem_start[] asm("_binary_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_cert_pem_end");

static const char *TAG = "EXAM_ACCESS_TOKEN_WO_SSL";

#define TELEMETYR_TEMPRATUE         	"temprature"
#define TELEMETYR_HUMIDITY          	"humidity"

//Don't call TBCMH API in this callback!
//Free return value by caller/(tbcmh library)!
tbcmh_value_t* tb_telemetry_on_get_temperature(void)
{
    ESP_LOGI(TAG, "Get temperature (a time-series data)");
    static float temp_array[] = {25.0, 25.5, 26.0, 26.5, 27.0, 27.5, 28.0, 27.5, 27.0, 26.5};
    static int i = 0;

    cJSON* temp = cJSON_CreateNumber(temp_array[i]);
    i++;
    i = i % (sizeof(temp_array)/sizeof(temp_array[0]));

    return temp;
}

//Don't call TBCMH API in this callback!
//Free return value by caller/(tbcmh library)!
tbcmh_value_t* tb_telemetry_on_get_humidity(void)
{
    ESP_LOGI(TAG, "Get humidity (a time-series data)");

    static int humi_array[] = {26, 27, 28, 29, 30, 31, 32, 31, 30, 29};
    static int i = 0;

    cJSON* humi = cJSON_CreateNumber(humi_array[i]);
    i++;
    i = i % (sizeof(humi_array)/sizeof(humi_array[0]));

    return humi;
}

void tb_telemetry_send(tbcmh_handle_t client)
{
    ESP_LOGI(TAG, "Send telemetry: %s, %s", TELEMETYR_TEMPRATUE, TELEMETYR_HUMIDITY);

    cJSON *object = cJSON_CreateObject(); // create json object
    cJSON_AddItemToObject(object, TELEMETYR_TEMPRATUE, tb_telemetry_on_get_temperature());
    cJSON_AddItemToObject(object, TELEMETYR_HUMIDITY, tb_telemetry_on_get_humidity());
    tbcmh_telemetry_upload_ex(client, object, 1/*qos*/, 0/*retain*/);
    cJSON_Delete(object); // delete json object
}

/*!< Callback of connected ThingsBoard MQTT */
void tb_on_connected(tbcmh_handle_t client, void *context)
{
    ESP_LOGI(TAG, "Connected to thingsboard server!");
}

/*!< Callback of disconnected ThingsBoard MQTT */
void tb_on_disconnected(tbcmh_handle_t client, void *context)
{
    ESP_LOGI(TAG, "Disconnected from thingsboard server!");
}

static void mqtt_app_start(void)
{
	//tbc_err_t err;

    ESP_LOGI(TAG, "Init tbcmh ...");
    tbcmh_handle_t client = tbcmh_init();
    if (!client) {
        ESP_LOGE(TAG, "Failure to init tbcmh!");
        return;
    }

    ESP_LOGI(TAG, "Connect tbcmh ...");
    tbc_transport_config_t transport = {
        .address = {    /*!< MQTT: broker, HTTP: server, CoAP: server */
            //bool tlsEnabled;                          /*!< Enabled TLS/SSL or DTLS */
            .schema = "mqtts",                          /*!< MQTT: mqtt/mqtts/ws/wss, HTTP: http/https, CoAP: coap/coaps */
            .host = CONFIG_TBC_TRANSPORT_ADDRESS_HOST,  /*!< MQTT/HTTP/CoAP server domain, hostname, to set ipv4 pass it as string */
            .port = CONFIG_TBC_TRANSPORT_ADDRESS_PORT,  /*!< MQTT/HTTP/CoAP server port */
            .path = NULL,                               /*!< Path in the URI*/
        },
        .credentials = {/*!< Client related credentials for authentication */
             .type = TBC_TRANSPORT_CREDENTIALS_TYPE_BASIC_MQTT,
                                                                     /*!< ThingsBoard Client transport authentication/credentials type */
             .client_id = CONFIG_TBC_TRANSPORT_CREDENTIALS_CLIENT_ID,/*!< MQTT.           default client id is ``ESP32_%CHIPID%`` where %CHIPID% are last 3 bytes of MAC address in hex format */
             .username  = CONFIG_TBC_TRANSPORT_CREDENTIALS_USER_NAME,/*!< MQTT/HTTP.      username */
             .password  = CONFIG_TBC_TRANSPORT_CREDENTIALS_PASSWORD, /*!< MQTT/HTTP.      password */

             .token = NULL,                                          /*!< MQTT/HTTP/CoAP: username/path param/path param */
                                                                     /*!< At TBC_TRANSPORT_CREDENTIALS_TYPE_X509 it's a client public key. DON'T USE IT! */
        },
        .verification = { /*!< Security verification of the broker/server */
             //bool      use_global_ca_store;               /*!< Use a global ca_store, look esp-tls documentation for details. */
             //esp_err_t (*crt_bundle_attach)(void *conf); 
                                                            /*!< Pointer to ESP x509 Certificate Bundle attach function for the usage of certificate bundles. */
             .cert_pem = (const char*)server_cert_pem_start,/*!< Pointer to certificate data in PEM or DER format for server verify (with SSL), default is NULL, not required to verify the server. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in cert_len. */
             .cert_len = 0,                                 /*!< Length of the buffer pointed to by cert_pem. May be 0 for null-terminated pem */
             //const struct psk_key_hint *psk_hint_key;
                                                            /*!< Pointer to PSK struct defined in esp_tls.h to enable PSK
                                                              authentication (as alternative to certificate verification).
                                                              PSK is enabled only if there are no other ways to
                                                              verify broker.*/
             .skip_cert_common_name_check = CONFIG_TBC_TRANSPORT_SKIP_CERT_COMMON_NAME_CHECK,
                                                            /*!< Skip any validation of server certificate CN field, this reduces the security of TLS and makes the mqtt client susceptible to MITM attacks */
             //const char **alpn_protos;                    /*!< NULL-terminated list of supported application protocols to be used for ALPN */
        },
        .authentication = { /*!< Client authentication for mutual authentication using TLS */
             .client_cert_pem = NULL,           /*!< Pointer to certificate data in PEM or DER format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_key_pem` has to be provided. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in client_cert_len. */
             .client_cert_len = 0,              /*!< Length of the buffer pointed to by client_cert_pem. May be 0 for null-terminated pem */
             .client_key_pem = NULL,            /*!< Pointer to private key data in PEM or DER format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_cert_pem` has to be provided. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in client_key_len */
             .client_key_len = 0,               /*!< Length of the buffer pointed to by client_key_pem. May be 0 for null-terminated pem */
             .client_key_password = NULL,       /*!< Client key decryption password string */
             .client_key_password_len = 0,      /*!< String length of the password pointed to by client_key_password */
             //bool      use_secure_element;    /*!< Enable secure element, available in ESP32-ROOM-32SE, for SSL connection */
             //void     *ds_data;               /*!< Carrier of handle for digital signature parameters, digital signature peripheral is available in some Espressif devices. */

        },

        .log_rxtx_package = true                /*!< print Rx/Tx MQTT package */
    };
    
    bool result = tbcmh_connect(client, &transport,
                        NULL, tb_on_connected, tb_on_disconnected);
    if (!result) {
        ESP_LOGE(TAG, "failure to connect to tbcmh!");
        goto exit_destroy;
    }


    ESP_LOGI(TAG, "connect tbcmh ...");
    int i = 0;
    while (i<20) {
        if (tbcmh_has_events(client)) {
            tbcmh_run(client);
        }

        i++;
        if (tbcmh_is_connected(client)) {
            if (i%5 == 0){
                tb_telemetry_send(client);
            }
        } else {
            ESP_LOGI(TAG, "Still NOT connected to server!");
        }
        sleep(1);
    }


    ESP_LOGI(TAG, "Disconnect tbcmh ...");
    tbcmh_disconnect(client);

exit_destroy:
    ESP_LOGI(TAG, "Destroy tbcmh ...");
    tbcmh_destroy(client);
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO); //ESP_LOG_DEBUG
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    //esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);
    esp_log_level_set("tb_mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("tb_mqtt_client_helper", ESP_LOG_VERBOSE);
    esp_log_level_set("attributes_reques", ESP_LOG_VERBOSE);
    esp_log_level_set("clientattribute", ESP_LOG_VERBOSE);
    esp_log_level_set("clientrpc", ESP_LOG_VERBOSE);
    esp_log_level_set("otaupdate", ESP_LOG_VERBOSE);
    esp_log_level_set("serverrpc", ESP_LOG_VERBOSE);
    esp_log_level_set("sharedattribute", ESP_LOG_VERBOSE);
    esp_log_level_set("telemetry_upload", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();
}

