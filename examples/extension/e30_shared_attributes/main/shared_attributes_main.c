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

#include "tbc_mqtt_helper.h"
#include "tbc_extension_sharedattributes.h"

#include "protocol_examples_common.h"

#define SHAREDATTRIBUTE_SNTP_SERVER     "sntp_server"

static const char *TAG = "SHARED_ATTR_MAIN";

tbce_sharedattributes_handle_t _sharedattributes = NULL;

// Set value of the device's shared-attribute
// Caller (TBCMH library) of this callback will release memory of the `value` param
// return 2 if tbcmh_disconnect()/tbcmh_destroy() is called inside it.
//      Caller (TBCMH library) will not update other shared attributes received this time.
//      If this callback is called while processing the response of an attribute request - _tbcmh_attributesrequest_on_data(),
//      the response callback of the attribute request - tbcmh_attributes_on_response_t/on_response, will not be called.
// return 1 if tbce_sharedattributes_unregister() is called.
//      Caller (TBCMH library) will not update other shared attributes received this time.
// return 0/ESP_OK on success
// return -1/ESP_FAIL on failure
tbc_err_t tb_sharedattribute_on_set_sntp_server(void *context, const tbcmh_value_t *value)
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

/*!< Callback of connected ThingsBoard MQTT */
void tb_on_connected(tbcmh_handle_t client, void *context)
{
    ESP_LOGI(TAG, "Connected to thingsboard server!");

    ESP_LOGI(TAG, "Subscribe shared attributes!");
    tbce_sharedattributes_subscribe(_sharedattributes,
                                    client, 10/*max_attributes_per_subscribe*/);

    ESP_LOGI(TAG, "Initilize shared attributes!");
    tbce_sharedattributes_initialized(_sharedattributes,
                                      client, 10/*max_attributes_per_request*/);
}

/*!< Callback of disconnected ThingsBoard MQTT */
void tb_on_disconnected(tbcmh_handle_t client, void *context)
{
    ESP_LOGI(TAG, "Disconnected from thingsboard server!");
}

static void mqtt_app_start(void)
{
	//tbc_err_t err;
    const char *access_token = CONFIG_ACCESS_TOKEN;
    const char *uri = CONFIG_BROKER_URL;

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


    ESP_LOGI(TAG, "Init tbcmh ...");
    tbcmh_handle_t client = tbcmh_init();
    if (!client) {
        ESP_LOGE(TAG, "Failure to init tbcmh!");
        return;
    }

    ESP_LOGI(TAG, "Create shared attribue set...");
    _sharedattributes = tbce_sharedattributes_create();
    if (!_sharedattributes) {
        ESP_LOGE(TAG, "Failure to create shared attribue set!");
        goto exit_destroy;
    }

    ESP_LOGI(TAG, "Append shared attribue: sntp_server...");
    tbc_err_t err = tbce_sharedattributes_register(_sharedattributes, SHAREDATTRIBUTE_SNTP_SERVER,
                      client, tb_sharedattribute_on_set_sntp_server);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failure to append sntp_server: %s!", SHAREDATTRIBUTE_SNTP_SERVER);
        goto exit_destroy;
    }

    ESP_LOGI(TAG, "Connect tbcmh ...");
    tbc_transport_config_esay_t config = {
        .uri = uri,                     /*!< Complete ThingsBoard MQTT broker URI */
        .access_token = access_token,   /*!< ThingsBoard Access Token */
        .log_rxtx_package = true        /*!< print Rx/Tx MQTT package */
    };
    bool result = tbcmh_connect_using_url(client, &config,
                    NULL, tb_on_connected, tb_on_disconnected);
    if (!result) {
        ESP_LOGE(TAG, "failure to connect to tbcmh!");
        goto exit_destroy;
    }

    // Do...
    int i = 0;
    while (i<20) {
        if (tbcmh_has_events(client)) {
            tbcmh_run(client);
        }

        i++;
        if (!tbcmh_is_connected(client)) {
            ESP_LOGI(TAG, "Still NOT connected to server!");
        }
        sleep(1);
    }

    ESP_LOGI(TAG, "Disconnect tbcmh ...");
    tbcmh_disconnect(client);

exit_destroy:
    if (!_sharedattributes) {
        ESP_LOGI(TAG, "Unregister shared attribue...");
        tbce_sharedattributes_unregister(_sharedattributes, SHAREDATTRIBUTE_SNTP_SERVER);
                                           
        ESP_LOGI(TAG, "Destory shared attribue set...");
        tbce_sharedattributes_destroy(_sharedattributes);
        _sharedattributes = NULL;
    }

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

