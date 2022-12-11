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
#include "protocol_examples_common.h"

static const char *TAG = "ATTR_SUBSCRIBE_MAIN";

#define SHAREDATTRIBUTE_SNTP_SERVER     "sntp_server"

//on received: unpack & deal
// return 2 if calling tbcmh_disconnect()/tbcmh_destroy() inside on_update()
// return 1 if calling tbcmh_sharedattribute_unregister()/tbcmh_attributes_unsubscribe inside on_update()
// return 0 otherwise
int tb_on_sntpserver_update(tbcmh_handle_t client, void *context, const cJSON *object)
{
    ESP_LOGI(TAG, "Update sntp_server (a shared attribute)");

    cJSON *value = cJSON_GetObjectItem(object, SHAREDATTRIBUTE_SNTP_SERVER);
    if (value && cJSON_IsString(value)) {
        char *sntp_server = cJSON_GetStringValue(value);
        if (sntp_server) {
            ESP_LOGI(TAG, "Receive sntp_server = %s", sntp_server);
        }
    } else {
        ESP_LOGW(TAG, "Not recieve sntp_server, or sntp_server is NOT a string!");
    }

    return 0;
}

//on received: unpack & deal
// return 2 if calling tbcmh_disconnect()/tbcmh_destroy() inside on_update()
// return 1 if calling tbcmh_sharedattribute_unregister()/tbcmh_attributes_unsubscribe inside on_update()
// return 0 otherwise
int tb_on_anyone_update(tbcmh_handle_t client, void *context, const cJSON *object)
{
    ESP_LOGI(TAG, "Update anyone attributes");

    if (object) {
        char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
        ESP_LOGI(TAG, "Receiving attributes update: %s", pack);        
        cJSON_free(pack); // free memory
    }

    return 0;
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

    ESP_LOGI(TAG, "Append shared attribue: sntp_server...");
    int subscribe_id = tbcmh_attributes_subscribe(client, NULL/*context*/,
                                    tb_on_sntpserver_update,
                                    0/*count*/, SHAREDATTRIBUTE_SNTP_SERVER);
    if (subscribe_id < 0) {
        ESP_LOGE(TAG, "Failure to append sntp_server: %s, subscribe_id=%d!",
                SHAREDATTRIBUTE_SNTP_SERVER, subscribe_id);
        goto exit_destroy;
    }

    subscribe_id = tbcmh_attributes_subscribe(client, NULL/*context*/,
                                    tb_on_anyone_update,
                                    0/*count*/);
    if (subscribe_id < 0) {
        ESP_LOGE(TAG, "Failure to subscribe all attirbutes update! subscribe_id=%d!",
                subscribe_id);
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

