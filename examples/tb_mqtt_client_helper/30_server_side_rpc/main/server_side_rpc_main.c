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

static const char *TAG = "SERVER_RPC_EXAMPLE";

#define SERVER_RPC_CHANGE_SETPOINT      "rpcChangeSetpoint"
#define SERVER_RPC_QUERY_SETPOINT       "rpcQuerySetpoint"

static double setpoint = 25.5;

// return NULL or cJSON* of object
// free return-value by caller/(tbcmh library)!
// free params by caller/(tbcmh library)!
tbcmh_rpc_results_t *tb_serverrpc_on_request_change_setpoint(tbcmh_handle_t client,
                            void *context, int request_id, 
                            const char *method, const tbcmh_rpc_params_t *params)
{
    if (!client || !method || !params) {
        ESP_LOGW(TAG, "client, method or params is NULL in %s()!", __FUNCTION__);
        return NULL;
    }

    ESP_LOGI(TAG, "Receive server RPC request: %s", method);
    cJSON *value = cJSON_GetObjectItem(params, "setpoint");
    if (!value) {
        return NULL;
    }

    if (cJSON_IsNumber(value)) {
        setpoint = cJSON_GetNumberValue(value);
        //if (setpoint) {
            ESP_LOGI(TAG, "Receive setpoint = %lf", setpoint);
        //}
    } else {
        ESP_LOGW(TAG, "Receive setpoint is NOT a number!");
    }

    return NULL;
}

// return NULL or cJSON* of object
// free return-value by caller/(tbcmh library)!
// free params by caller/(tbcmh library)!
tbcmh_rpc_results_t *tb_serverrpc_on_request_query_setpoint(tbcmh_handle_t client,
                            void *context, int request_id, 
                            const char *method, const tbcmh_rpc_params_t *params)
{
    if (!client || !method ) {
        ESP_LOGW(TAG, "client or method is NULL in %s()!", __FUNCTION__);
        return NULL;
    }

    ESP_LOGI(TAG, "Receive server RPC request: %s", method);

    cJSON *result = cJSON_CreateObject(); // It MUST create a cJSON object for the result of two-way server RPC!
    cJSON_AddNumberToObject(result, "setpoint", setpoint);
    return result;
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
	tbc_err_t err;
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
    ESP_LOGI(TAG, "Init tbcmh ...");
    tbcmh_handle_t client = tbcmh_init();
    if (!client) {
        ESP_LOGE(TAG, "Failure to init tbcmh!");
        return;
    }

    ESP_LOGI(TAG, "Append server RPC: rpcChangeSetpoint...");
    err = tbcmh_serverrpc_append(client, SERVER_RPC_CHANGE_SETPOINT, NULL,
                                 tb_serverrpc_on_request_change_setpoint);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "failure to append server RPC: %s!", SERVER_RPC_CHANGE_SETPOINT);
        goto exit_destroy;
    }
    ESP_LOGI(TAG, "Append server RPC: rpcQuerySetpoint...");
    err = tbcmh_serverrpc_append(client, SERVER_RPC_QUERY_SETPOINT, NULL,
                                 tb_serverrpc_on_request_query_setpoint);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "failure to append server RPC: %s!", SERVER_RPC_QUERY_SETPOINT);
        goto exit_destroy;
    }

    ESP_LOGI(TAG, "Connect tbcmh ...");
    tbc_transport_config_esay_t config = {
        .uri = uri,                     /*!< Complete ThingsBoard MQTT broker URI */
        .access_token = access_token,   /*!< ThingsBoard Access Token */
        .log_rxtx_package = true        /*!< print Rx/Tx MQTT package */
    };
    bool result = tbcmh_connect_using_url(client, &config, NULL, tb_on_connected, tb_on_disconnected);
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
    esp_log_level_set("ota_update", ESP_LOG_VERBOSE);
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

