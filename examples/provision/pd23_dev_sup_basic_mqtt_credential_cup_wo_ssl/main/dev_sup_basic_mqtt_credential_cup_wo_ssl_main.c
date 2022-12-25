/* MQTT (over TCP) Example

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

#include "tbc_utils.h"

#include "tbc_transport_config.h"

#include "tbc_mqtt_helper.h"
#include "protocol_examples_common.h"

#include "tbc_transport_credentials_memory.h"

static const char *TAG = "DEVICE_PROVISION_MAIN";

extern tbcmh_handle_t tbcmh_frontconn_create(const tbc_transport_config_t *transport,
                                            const tbc_provison_config_t *provision);
extern tbcmh_handle_t tbcmh_normalconn_create(const tbc_transport_config_t *transport);

/*
 * Define psk key and hint as defined in mqtt broker
 * example for mosquitto server, content of psk_file:
 * hint:BAD123
 *
 */
/*
static const uint8_t s_key[] = { 0xBA, 0xD1, 0x23 };

static const psk_hint_key_t psk_hint_key = {
            .key = s_key,
            .key_size = sizeof(s_key),
            .hint = "hint"
        };
*/

static tbc_transport_address_config_t _address = /*!< MQTT: broker, HTTP: server, CoAP: server */
            {
                //bool tlsEnabled,                              /*!< Enabled TLS/SSL or DTLS */
                .schema = "mqtt",                              /*!< MQTT: mqtt/mqtts/ws/wss, HTTP: http/https, CoAP: coap/coaps */
                .host   = CONFIG_TBC_TRANSPORT_ADDRESS_HOST,    /*!< MQTT/HTTP/CoAP server domain, hostname, to set ipv4 pass it as string */
                .port   = CONFIG_TBC_TRANSPORT_ADDRESS_PORT,    /*!< MQTT/HTTP/CoAP server port */
                .path   = NULL,                                 /*!< Path in the URI*/
            };

static tbc_provison_config_t  _provision = 
            {
              .provisionType = TBC_PROVISION_TYPE_DEVICE_SUPPLIES_BASIC_MQTT_CREDENTIALS,               // Generates/Supplies credentials type. // Hardcode or device profile.
             
              .deviceName = CONFIG_TBC_PROVISION_DEVICE_NAME,               // Device name in TB        // Chip name + Chip id, e.g., "esp32-C8:D6:93:12:BC:01". Each device is different.
              .provisionDeviceKey = CONFIG_TBC_PROVISION_DEVICE_KEY,        // Provision device key     // Hardcode or device profile. Each model is different. 
              .provisionDeviceSecret = CONFIG_TBC_PROVISION_DEVICE_SECRET,  // Provision device secret  // Hardcode or device profile. Each model is different.

              .token    = NULL,                           // Access token for device             // Randomly generated. Each device is different.
              .clientId = CONFIG_TBC_PROVISION_CLIENT_ID, // Client id for device                // Randomly generated. Each device is different.
              .username = CONFIG_TBC_PROVISION_USER_NAME, // Username for device                 // Randomly generated. Each device is different.
              .password = CONFIG_TBC_PROVISION_PASSWORD,  // Password for device                 // Randomly generated. Each device is different.
              .hash     = NULL,                           // Public key X509 hash for device     // Public key X509.    Each device is different.
            };

static tbc_transport_verification_config_t _verification = /*!< Security verification of the broker/server */
            {       
                 //bool      use_global_ca_store;               /*!< Use a global ca_store, look esp-tls documentation for details. */
                 //esp_err_t (*crt_bundle_attach)(void *conf); 
                                                                /*!< Pointer to ESP x509 Certificate Bundle attach function for the usage of certificate bundles. */
                 .cert_pem = NULL,
                                                                /*!< Pointer to certificate data in PEM or DER format for server verify (with SSL), default is NULL, not required to verify the server. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in cert_len. */
                 .cert_len = 0,                                 /*!< Length of the buffer pointed to by cert_pem. May be 0 for null-terminated pem */
                 //.psk_hint_key = &psk_hint_key,
                                                                /*!< Pointer to PSK struct defined in esp_tls.h to enable PSK
                                                                  authentication (as alternative to certificate verification).
                                                                  PSK is enabled only if there are no other ways to
                                                                  verify broker.*/
                 .skip_cert_common_name_check = false,
                                                                /*!< Skip any validation of server certificate CN field, this reduces the security of TLS and makes the mqtt client susceptible to MITM attacks  */
                 //const char **alpn_protos;                    /*!< NULL-terminated list of supported application protocols to be used for ALPN */
            };

static tbc_transport_authentication_config_t _authentication = /*!< Client authentication for mutual authentication using TLS */
            {
                .client_cert_pem = NULL,                            /*!< Pointer to certificate data in PEM or DER format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_key_pem` has to be provided. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in client_cert_len. */
                .client_cert_len = 0,                               /*!< Length of the buffer pointed to by client_cert_pem. May be 0 for null-terminated pem */
                .client_key_pem = NULL,                             /*!< Pointer to private key data in PEM or DER format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_cert_pem` has to be provided. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in client_key_len */
                .client_key_len = 0,                                /*!< Length of the buffer pointed to by client_key_pem. May be 0 for null-terminated pem */
                .client_key_password = NULL,                        /*!< Client key decryption password string */
                .client_key_password_len = 0,                       /*!< String length of the password pointed to by client_key_password */
                //bool      use_secure_element;                     /*!< Enable secure element, available in ESP32-ROOM-32SE, for SSL connection */
                //void     *ds_data;                                /*!< Carrier of handle for digital signature parameters, digital signature peripheral is available in some Espressif devices. */
            };

static void mqtt_app_start()
{
    tbcmh_handle_t client = NULL;
    bool is_front_connection = false;

    tbc_transport_credentials_memory_init();
    tbc_transport_credentials_memory_clean();

    //init transport
    tbc_transport_config_t transport = {0};
    memcpy(&transport.address, &_address, sizeof(_address));
    //memcpy(&transport.credentials, &_credentials, sizeof(_credentials));
    memcpy(&transport.verification, &_verification, sizeof(_verification));
    //memcpy(&transport.authentication, &_authentication, sizeof(_authentication));
    transport.log_rxtx_package = true;  /*!< print Rx/Tx MQTT package */

    // create_front_connect(client)
    client = tbcmh_frontconn_create(&transport, &_provision);
    is_front_connection = true;

    int i = 0;
    while (client && i<40) // TB_MQTT_TIMEOUT is 30 seconds!
    {
        if (tbcmh_has_events(client)) {
            tbcmh_run(client);
        }

        if (is_front_connection==true && tbc_transport_credentials_memory_is_existed()) {
            // destory_front_conn(client)
            if (client) tbcmh_disconnect(client);
            if (client) tbcmh_destroy(client);
            client = NULL;

            // create_normal_connect(client) // If the credentials type is X.509, the front connection (provisioing) is one-way SSL, but the normal connection is two-way (mutual) SSL.
            memcpy(&transport.address, &_address, sizeof(_address));
            tbc_transport_credentials_config_copy(&transport.credentials, tbc_transport_credentials_memory_get()); //memcpy(&transport.credentials, &_credentials, sizeof(_credentials));
            memcpy(&transport.verification, &_verification, sizeof(_verification));
            memcpy(&transport.authentication, &_authentication, sizeof(_authentication));
            client = tbcmh_normalconn_create(&transport);
            is_front_connection = false;
        }

        i++;
        if (tbcmh_is_connected(client)) {
            //no code
        } else {
            ESP_LOGI(TAG, "Still NOT connected to server!");
        }
        sleep(1);
    };

    ESP_LOGI(TAG, "Disconnect tbcmh ...");
    if (client) {
        tbcmh_disconnect(client);
    }
    ESP_LOGI(TAG, "Destroy tbcmh ...");
    if (client) {
        tbcmh_destroy(client);
        client = NULL;
    }

    tbc_transport_credentials_memory_uninit();
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

