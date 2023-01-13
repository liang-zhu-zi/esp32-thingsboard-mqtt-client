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
#include "tbc_extension_clientattributes.h"

#include "protocol_examples_common.h"

#define CLIENTATTRIBUTE_MODEL       	"model"
#define CLIENTATTRIBUTE_SETPOINT    	"setpoint"

static const char *TAG = "CLIENT_ATTRIBUTES_MAIN";

tbce_clientattributes_handle_t _clientattributes = NULL;

static double setpoint = 24.0;


//Don't call TBCMH API in these callback!
//Free return value by caller/(tbcmh library)!
tbcmh_value_t* tb_clientattribute_on_get_model(void *context)
{
    ESP_LOGI(TAG, "Get model (a client attribute)");
    
    return cJSON_CreateString("TH_001");
}

//Don't call TBCMH API in these callback!
//Free return value by caller/(tbcmh library)!
tbcmh_value_t* tb_clientattribute_on_get_setpoint(void *context)
{
    ESP_LOGI(TAG, "Get setpoint (a client attribute) setpoint=%f", setpoint);
    
    return cJSON_CreateNumber(setpoint);
}

/**
 * @brief  callback of setting value of the device's client-side attribute.
 * Only for initilizing client-side attribute
 *
 * Notes:
 * - If you call tbce_clientattributes_register_with_set() & tbce_clientattributes_initialize(),
 *    this callback will be called when you receied a initialization vlaue of client-side attribute
 * - Don't call TBCMH API in this callback!
 *
 * @param context   context param 
 * @param value     initialization value of the device's client-side attribute
 *
 * @return 2 if tbcmh_disconnect() or tbcmh_destroy() is called inside in this callback
 *         1 if tbcmh_sharedattributes_unregister() or tbcmh_attributes_unsubscribe() 
 *              is called inside in this callback
 *         0/ESP_OK     on success
 *         -1/ESP_FAIL  on failure
 */
tbc_err_t tb_clientattribute_on_set_setpoint(void *context, const tbcmh_value_t *value)
{
    ESP_LOGI(TAG, "Initilize/Set setpoint (a client attribute)");

    if (cJSON_IsNumber(value)) {
        setpoint = cJSON_GetNumberValue(value);
        ESP_LOGI(TAG, "Receive setpoint = %f", setpoint);
    } else {
        ESP_LOGW(TAG, "Receive setpoint is NOT a Number!");
    }

    return ESP_OK;
}

void tb_clientattribute_send(tbcmh_handle_t client)
{
    ESP_LOGI(TAG, "Send client attributes: %s, %s",CLIENTATTRIBUTE_MODEL, CLIENTATTRIBUTE_SETPOINT);
    tbce_clientattributes_update(_clientattributes, client, 2, CLIENTATTRIBUTE_MODEL, CLIENTATTRIBUTE_SETPOINT);
}

/*!< Callback of connected ThingsBoard MQTT */
void tb_on_connected(tbcmh_handle_t client, void *context)
{
    ESP_LOGI(TAG, "Connected to thingsboard server!");

    ESP_LOGI(TAG, "Initilize client attributes those store at the server!");
    tbce_clientattributes_initialize(_clientattributes,
                            client, 10 /*max_attributes_per_request*/);
}

/*!< Callback of disconnected ThingsBoard MQTT */
void tb_on_disconnected(tbcmh_handle_t client, void *context)
{
    ESP_LOGI(TAG, "Disconnected from thingsboard server!");
}

static void mqtt_app_start(void)
{
	tbc_err_t err;
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

    ESP_LOGI(TAG, "Create client attribute set...");
    _clientattributes = tbce_clientattributes_create();
    if (!_clientattributes) {
        ESP_LOGE(TAG, "failure to create client attribute set!");
        goto exit_destroy;
    }

    ESP_LOGI(TAG, "Append client attribute: model...");
    err = tbce_clientattributes_register(_clientattributes, CLIENTATTRIBUTE_MODEL, NULL,
                            tb_clientattribute_on_get_model);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "failure to append client attribute: %s!", CLIENTATTRIBUTE_MODEL);
        goto exit_destroy;
    }
    ESP_LOGI(TAG, "Append client attribute: setpoint...");
    err = tbce_clientattributes_register_with_set(_clientattributes, CLIENTATTRIBUTE_SETPOINT, NULL,
                            tb_clientattribute_on_get_setpoint,
                            tb_clientattribute_on_set_setpoint);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "failure to append client attribute: %s!", CLIENTATTRIBUTE_SETPOINT);
        goto exit_destroy;
    }

    ESP_LOGI(TAG, "Connect tbcmh ...");
    tbc_transport_config_esay_t config = {
        .uri = uri,                     /*!< Complete ThingsBoard MQTT broker URI */
        .access_token = access_token,   /*!< ThingsBoard Access Token */
        .log_rxtx_package = true                /*!< print Rx/Tx MQTT package */
     };
    bool result = tbcmh_connect_using_url(client, &config, 
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
                tb_clientattribute_send(client);
            }
        } else {
            ESP_LOGI(TAG, "Still NOT connected to server!");
        }
        sleep(1);
    }


    ESP_LOGI(TAG, "Disconnect tbcmh ...");
    tbcmh_disconnect(client);

exit_destroy:
    if (!_clientattributes) {
        ESP_LOGE(TAG, "Unregister client attribute: %s!", CLIENTATTRIBUTE_MODEL);
        tbce_clientattributes_unregister(_clientattributes, CLIENTATTRIBUTE_MODEL);

        ESP_LOGE(TAG, "Unregister client attribute: %s!", CLIENTATTRIBUTE_SETPOINT);
        tbce_clientattributes_unregister(_clientattributes, CLIENTATTRIBUTE_SETPOINT);

        ESP_LOGE(TAG, "Destroy client attribute set!");
        tbce_clientattributes_destroy(_clientattributes);
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

