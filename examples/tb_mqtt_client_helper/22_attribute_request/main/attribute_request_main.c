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

static const char *TAG = "ATTR_REQUEST_EXAMPLE";

#define CLIENTATTRIBUTE_SETPOINT    	"setpoint"
#define SHAREDATTRIBUTE_SNTP_SERVER     "sntp_server"

static bool g_attributes_are_initialized_from_server = false;

//Don't call TBCMH API in these callback!
//Free return value by caller/(tbcmh library)!
tbcmh_value_t* tb_clientattribute_on_get_setpoint(tbcmh_handle_t client, void *context)
{
    ESP_LOGI(TAG, "Get setpoint (a client attribute)");
    
    return cJSON_CreateNumber(25.5);
}

//Don't call TBCMH API in these callback!
//Free value by caller/(tbcmh library)!
void tb_clientattribute_on_set_setpoint(tbcmh_handle_t client, void *context, const tbcmh_value_t *value)
{
    ESP_LOGI(TAG, "Set setpoint (a client-side attribute)");
    if (!value) {
        ESP_LOGW(TAG, "value is NULL!");
        return;
    }

    if (cJSON_IsNumber(value)) {
        double setpoint = cJSON_GetNumberValue(value);
        //if (setpoint) {
            ESP_LOGI(TAG, "Receive setpoint = %lf", setpoint);
        //}
    } else {
        ESP_LOGW(TAG, "Receive setpoint is NOT a number!");
    }
}

void tb_clientattribute_send(tbcmh_handle_t client)
{
    ESP_LOGI(TAG, "Send client attributes: %s", CLIENTATTRIBUTE_SETPOINT);
    tbcmh_clientattribute_update(client, 1, CLIENTATTRIBUTE_SETPOINT);
}

//Don't call TBCMH API in this callback!
//Free value by caller/(tbcmh library)!
tbc_err_t tb_sharedattribute_on_set_sntp_server(tbcmh_handle_t client, void *context, const tbcmh_value_t *value)
{
    ESP_LOGI(TAG, "Set sntp_server (a shared attribute)");
    if (!value) {
        ESP_LOGW(TAG, "value is NULL!");
        return ESP_FAIL;
    }

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

void tb_attributesrequest_on_response(tbcmh_handle_t client, void *context, int request_id)
{
    ESP_LOGI(TAG, "Receiving response of the attribute request! request_id=%d", request_id);

    g_attributes_are_initialized_from_server = true;
}
void tb_attributesrequest_on_timeout(tbcmh_handle_t client, void *context, int request_id)
{
    ESP_LOGI(TAG, "Timeout of the attribute request! request_id=%d", request_id);
}

void tb_attributesrequest_send(tbcmh_handle_t client)
{
    ESP_LOGI(TAG, "Request attributes, client attributes: %s; shared attributes: %s",
        CLIENTATTRIBUTE_SETPOINT, SHAREDATTRIBUTE_SNTP_SERVER);

    tbcmh_attributesrequest_send(client, NULL,
                                     tb_attributesrequest_on_response,
                                     tb_attributesrequest_on_timeout,
                                     2, CLIENTATTRIBUTE_SETPOINT, SHAREDATTRIBUTE_SNTP_SERVER);
}

/*!< Callback of connected ThingsBoard MQTT */
void tb_on_connected(tbcmh_handle_t client, void *context)
{
    ESP_LOGI(TAG, "Connected to thingsboard server!");

    tb_attributesrequest_send(client);
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
    bool is_running_in_mqtt_task = false;
    tbcmh_handle_t client = tbcmh_init(is_running_in_mqtt_task);
    if (!client) {
        ESP_LOGE(TAG, "Failure to init tbcmh!");
        return;
    }

    ESP_LOGI(TAG, "Append client attribute: setpoint...");
    err = tbcmh_clientattribute_register_with_set(client, CLIENTATTRIBUTE_SETPOINT, NULL, 
                            tb_clientattribute_on_get_setpoint, tb_clientattribute_on_set_setpoint);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "failure to append client attribute: %s!", CLIENTATTRIBUTE_SETPOINT);
        goto exit_destroy;
    }

    ESP_LOGI(TAG, "Append shared attribue: sntp_server...");
    err = tbcmh_sharedattribute_register(client, SHAREDATTRIBUTE_SNTP_SERVER, NULL, 
                            tb_sharedattribute_on_set_sntp_server);
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
    bool result = tbcmh_connect_using_url(client, &config, TBCMH_FUNCTION_FULL_ATTRIBUTES,
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
        if (tbcmh_is_connected(client)) {
			if (g_attributes_are_initialized_from_server) {
	            if (i%5 == 0){
	                tb_clientattribute_send(client);
	            }
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
    esp_log_level_set("clientattribute", ESP_LOG_VERBOSE);
    esp_log_level_set("clientrpc", ESP_LOG_VERBOSE);
    esp_log_level_set("otaupdate", ESP_LOG_VERBOSE);
    esp_log_level_set("serverrpc", ESP_LOG_VERBOSE);
    esp_log_level_set("sharedattribute", ESP_LOG_VERBOSE);
    esp_log_level_set("timeseriesdata", ESP_LOG_VERBOSE);

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

