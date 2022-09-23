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

#include "tb_mqtt_client_helper.h"
#include "protocol_examples_common.h"

static const char *TAG = "SHARED_ATTR_EXAMPLE";

#define SHAREDATTRIBUTE_SNTP_SERVER     "sntp_server"

/*!< Callback of connected ThingsBoard MQTT */
void tb_on_connected(tbmch_handle_t client, void *context)
{
    ESP_LOGI(TAG, "Connected to thingsboard server!");
}

/*!< Callback of disconnected ThingsBoard MQTT */
void tb_on_disconnected(tbmch_handle_t client, void *context)
{
    ESP_LOGI(TAG, "Disconnected from thingsboard server!");
}

//Don't call TBMCH API in this callback!
//Free value by caller/(tbmch library)!
tbmch_err_t tb_sharedattribute_on_set_sntp_server(tbmch_handle_t client, void *context, const tbmch_value_t *value)
{
    ESP_LOGI(TAG, "Set sntp_server (a shared attribute)");

    if (cJSON_IsString(value)) {
        char *sntp_server = cJSON_GetStringValue(value);
        if (sntp_server) {
            ESP_LOGI(TAG, "Receive sntp_server = %s", sntp_server);
        }
    } else {
        ESP_LOGW(TAG, "Receive sntp_server is NOT a string!");
    }

    return ESP_OK;
}

static void mqtt_app_start(void)
{
#if 0
    const esp_mqtt_client_config_t config = {
        .uri = CONFIG_BROKER_URL
    };
#else
    const char *access_token = CONFIG_ACCESS_TOKEN;
    const char *uri = CONFIG_BROKER_URL;
#endif

#if CONFIG_BROKER_URL_FROM_STDIN
    char line_uri[128];

    if (strcmp(uri, "FROM_STDIN") == 0) { //mqtt_cfg.
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line_uri[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line_uri[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        uri = line_uri; //mqtt_cfg.
        printf("Broker url: %s\n", line_uri);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

#if CONFIG_ACCESS_TOKEN_FROM_STDIN
    char line_token[128];

    if (strcmp(access_token, "FROM_STDIN") == 0) { //mqtt_cfg.
        int count = 0;
        printf("Please enter access token \n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line_token[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line_token[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        access_token = line_token; //mqtt_cfg.
        printf("Access token: %s\n", line_token);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong access token");
        abort();
    }
#endif /* CONFIG_ACCESS_TOKEN_FROM_STDIN */

#if 0
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
#else
    ESP_LOGI(TAG, "Init tbmch ...");
    tbmch_handle_t client = tbmch_init();
    if (!client) {
        ESP_LOGE(TAG, "Failure to init tbmch!");
        return;
    }

    ESP_LOGI(TAG, "Append shared attribue: sntp_server...");
    tbmch_err_t err = tbmch_sharedattribute_append(client, SHAREDATTRIBUTE_SNTP_SERVER, NULL, tb_sharedattribute_on_set_sntp_server);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failure to append sntp_server: %s!", SHAREDATTRIBUTE_SNTP_SERVER);
        goto exit_destroy;
    }

    ESP_LOGI(TAG, "Connect tbmch ...");
    tbmch_config_t config = {
        .uri = uri,                     /*!< Complete ThingsBoard MQTT broker URI */
        .access_token = access_token,   /*!< ThingsBoard Access Token */
        .cert_pem = NULL,               /*!< Reserved. Pointer to certificate data in PEM format for server verify (with SSL), default is NULL, not required to verify the server */
        .client_cert_pem = NULL,        /*!< Reserved. Pointer to certificate data in PEM format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_key_pem` has to be provided. */
        .client_key_pem = NULL,         /*!< Reserved. Pointer to private key data in PEM format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_cert_pem` has to be provided. */

        .context = NULL,                        /*!< Context parameter of the below two callbacks */
        .on_connected = tb_on_connected,        /*!< Callback of connected ThingsBoard MQTT */
        .on_disconnected = tb_on_disconnected,  /*!< Callback of disconnected ThingsBoard MQTT */

        .log_rxtx_package = true                /*!< print Rx/Tx MQTT package */
     };
    bool result = tbmch_connect(client, &config);
    if (!result) {
        ESP_LOGE(TAG, "failure to connect to tbmch!");
        goto exit_destroy;
    }


    ESP_LOGI(TAG, "connect tbmch ...");
    int i = 0;
    while (i<20) {
        if (tbmch_has_events(client)) {
            tbmch_run(client);
        }

        i++;
        if (!tbmch_is_connected(client)) {
            ESP_LOGI(TAG, "Still NOT connected to server!");
        }
        sleep(1);
    }


    ESP_LOGI(TAG, "Disconnect tbmch ...");
    tbmch_disconnect(client);

exit_destroy:
    ESP_LOGI(TAG, "Destroy tbmch ...");
    tbmch_destroy(client);
#endif
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
    esp_log_level_set("client_attribute", ESP_LOG_VERBOSE);
    esp_log_level_set("client_rpc", ESP_LOG_VERBOSE);
    esp_log_level_set("fw_update", ESP_LOG_VERBOSE);
    esp_log_level_set("server_rpc", ESP_LOG_VERBOSE);
    esp_log_level_set("shared_attribute", ESP_LOG_VERBOSE);
    esp_log_level_set("timeseries_data", ESP_LOG_VERBOSE);

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

