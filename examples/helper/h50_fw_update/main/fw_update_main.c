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

#include "esp_ota_ops.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"

#include "tbc_mqtt_helper.h"
#include "protocol_examples_common.h"

#define FW_DESCRIPTION              "FW_DESCRIPTION"
//#define CURRENT_FW_TITLE            "MY_ESP32_FW"
//#define CURRENT_FW_VERSION          "7.8.9"

//#define SW_DESCRIPTION              "SW_DESCRIPTION"
//#define CURRENT_SW_TITLE            "MY_ESP32_SW"
//#define CURRENT_SW_VERSION          "2.3.4"

static const char *TAG = "OTA_UPDATE_EXAMPLE";

static volatile bool _my_fwupdate_request_reboot = false;

//Don't call TBCMH API in the callback!
static const char* _my_fwupdate_on_get_current_title(void *context)
{
    // TODO: division F/W or S/W !!!!

    static char title[64] = {0};

    //get titile from running_partition
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    if (running_partition) {
        esp_app_desc_t running_app_info;
        if (esp_ota_get_partition_description(running_partition, &running_app_info) == ESP_OK) { //esp_ota_get_app_description()
            ESP_LOGI(TAG, "Current running F/W title: %s", running_app_info.project_name);
            strncpy(title, running_app_info.project_name, sizeof(title)-1); //version
            return title;
        }
    }

    return NULL; //CURRENT_FW_TITLE;
}
//Don't call TBCMH API in the callback!
static const char* _my_fwupdate_on_get_current_version(void *context)
{
    // TODO: division F/W or S/W !!!!

    static char version[64] = {0};

    //get version from running_partition
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    if (running_partition) {
        esp_app_desc_t running_app_info;
        if (esp_ota_get_partition_description(running_partition, &running_app_info) == ESP_OK) {
            ESP_LOGI(TAG, "Current running F/W version: %s", running_app_info.version);
            strncpy(version, running_app_info.version, sizeof(version)-1);
            return version;
        }
    }

    return NULL; //CURRENT_FW_VERSION;
}

void _my_fwupdate_on_updated(void *context, bool result)
{
    if (result) {
        _my_fwupdate_request_reboot = true;
    }
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

    ESP_LOGI(TAG, "Append F/W OTA Update...");
    tbc_err_t err = tbcmh_otaupdate_subscribe(client,
                                            FW_DESCRIPTION,
                                            TBCMH_OTAUPDATE_TYPE_FW,
                                            NULL,
                                            _my_fwupdate_on_get_current_title,
                                            _my_fwupdate_on_get_current_version,
                                            _my_fwupdate_on_updated);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failure to append F/W OTA Update!");
        goto exit_destroy;
    }

    /*ESP_LOGI(TAG, "Append S/W OTA Update...");
    err = my_swupdate_init(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failure to append S/W OTA Update!");
        goto exit_destroy;
    }*/

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
    while (i<300 && !_my_fwupdate_request_reboot) { //!_my_swupdate_request_reboot
        if (tbcmh_has_events(client)) {
            tbcmh_run(client);
        }

        i++;
        if (!tbcmh_is_connected(client)) {
            ESP_LOGI(TAG, "Still NOT connected to server!");
        }
        sleep(1);
        //printf(".");
    }

    ESP_LOGI(TAG, "Disconnect tbcmh ...");
    tbcmh_disconnect(client);

exit_destroy:
    ESP_LOGI(TAG, "Destroy tbcmh ...");
    tbcmh_destroy(client);

    //restart esp32
    if (_my_fwupdate_request_reboot) { // || _my_swupdate_request_reboot
        ESP_LOGI(TAG, "Prepare to restart system!");
        sleep(2);
        esp_restart();
    }
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

    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "nvs_flash_erase...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();
}

