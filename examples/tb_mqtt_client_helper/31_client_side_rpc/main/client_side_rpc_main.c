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

static const char *TAG = "CLIENT_RPC_EXAMPLE";

#define CLIENT_RPC_PUBLISH_LOCAL_TIME       "rpcPublishLocalTime"
#define CLIENT_RPC_GET_CURRENT_TIME         "rpcGetCurrentTime"
#define CLIENT_RPC_LOOPBACK                 "rpcLoopback"
#define CLIENT_RPC_NOT_IMPLEMENTED_TWOWAY   "rpcNotImplementedTwoway"

static void _set_timezone(void)
{
    // set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();
}
/*static*/ void _set_timestamp(long timestamp) 
{
    struct timeval tv;
    tv.tv_sec = timestamp; //seconds
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
}
/*static*/ long _get_timestamp(void) 
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec; //seconds
    //tv.tv_usec = 0;
}
static void _print_localtime(void)
{
    char strftime_buf[64];

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in HongKong is: %s", strftime_buf);
}


void tb_clientrpc_publish_local_time_send(tbcmh_handle_t client)
{
    ESP_LOGI(TAG, "Send Client-side RPC: method=%s", CLIENT_RPC_PUBLISH_LOCAL_TIME);
    _print_localtime();

    cJSON *params = cJSON_CreateObject();
    cJSON_AddNumberToObject(params, "localTime", _get_timestamp());
    // free params by caller/(user code)!
    int request_id = tbcmh_clientrpc_of_oneway_request(client, CLIENT_RPC_PUBLISH_LOCAL_TIME, params);
    ESP_LOGI(TAG, "Send Client-side RPC: request_id=%d", request_id);
    cJSON_Delete(params);
}


// free results by caller/(tbcmh library)!
void tb_clientrpc_get_current_time_on_response(tbcmh_handle_t client, void *context,
                              int request_id, const char *method, const tbcmh_rpc_results_t *results)
{
    ESP_LOGI(TAG, "Client-side RPC response: request_id=%d, method=%s", request_id, method);

    // set local time
    _print_localtime();
    uint64_t timestamp;
    if (results && cJSON_HasObjectItem(results, "currentTime")) {
        cJSON *currentTime = cJSON_GetObjectItem(results, "currentTime");
        if (cJSON_IsNumber(currentTime)) {
            timestamp = (uint64_t)cJSON_GetNumberValue(currentTime);
            // convert millisecond to seconds
            if (timestamp>1000000000000) {
                timestamp = timestamp / 1000;
            }
            _set_timestamp(timestamp);
            _print_localtime();
        }
    }
}
void tb_clientrpc_get_current_time_on_timeout(tbcmh_handle_t client, void *context,
                             int request_id, const char *method)
{
    ESP_LOGI(TAG, "Client-side RPC timeout: request_id=%d, method=%s", request_id, method);
}
void tb_clientrpc_get_current_time_send(tbcmh_handle_t client)
{
    ESP_LOGI(TAG, "Send Client-side RPC: method=%s", CLIENT_RPC_GET_CURRENT_TIME);

    //cJSON *params = cJSON_CreateObject();
    // free params by caller/(user code)!
    int request_id = tbcmh_clientrpc_of_twoway_request(client, CLIENT_RPC_GET_CURRENT_TIME, NULL, //params,
                                                 NULL,
                                                 tb_clientrpc_get_current_time_on_response,
                                                 tb_clientrpc_get_current_time_on_timeout);
    ESP_LOGI(TAG, "Send Client-side RPC: request_id=%d", request_id);
    //cJSON_Delete(params);
}

// free results by caller/(tbcmh library)!
void tb_clientrpc_loopback_on_response(tbcmh_handle_t client, void *context,
                              int request_id, const char *method, const tbcmh_rpc_results_t *results)
{
    ESP_LOGI(TAG, "Client-side RPC response: request_id=%d, method=%s", request_id, method);
    // print response!
}
void tb_clientrpc_loopback_on_timeout(tbcmh_handle_t client, void *context,
                             int request_id, const char *method)
{
    ESP_LOGI(TAG, "Client-side RPC timeout: request_id=%d, method=%s", request_id, method);
}
void tb_clientrpc_loopback_send(tbcmh_handle_t client)
{
    static int i = 9001;

    ESP_LOGI(TAG, "Send Client-side RPC: method=%s", CLIENT_RPC_LOOPBACK);

    cJSON *params = cJSON_CreateObject();
    cJSON_AddNumberToObject(params, "id", i++);
    // free params by caller/(user code)!
    int request_id = tbcmh_clientrpc_of_twoway_request(client, CLIENT_RPC_LOOPBACK, params,
                                                 NULL,
                                                 tb_clientrpc_loopback_on_response,
                                                 tb_clientrpc_loopback_on_timeout);
    ESP_LOGI(TAG, "Send Client-side RPC: request_id=%d", request_id);
    cJSON_Delete(params);
}


// free results by caller/(tbcmh library)!
void tb_clientrpc_not_implemented_twoway_on_response(tbcmh_handle_t client, void *context,
                              int request_id, const char *method, const tbcmh_rpc_results_t *results)
{
    ESP_LOGI(TAG, "Client-side RPC response: request_id=%d, method=%s", request_id, method);
}
void tb_clientrpc_not_implemented_twoway_on_timeout(tbcmh_handle_t client, void *context,
                             int request_id, const char *method)
{
    ESP_LOGI(TAG, "Client-side RPC timeout: request_id=%d, method=%s", request_id, method);
}
void tb_clientrpc_not_implemented_twoway_send(tbcmh_handle_t client)
{
    static int i = 4001;

    ESP_LOGI(TAG, "Send Client-side RPC: method=%s", CLIENT_RPC_NOT_IMPLEMENTED_TWOWAY);

    cJSON *params = cJSON_CreateObject();
    cJSON_AddNumberToObject(params, "id", i++);
    // free params by caller/(user code)!
    int request_id = tbcmh_clientrpc_of_twoway_request(client, CLIENT_RPC_NOT_IMPLEMENTED_TWOWAY, params,
                                                 NULL,
                                                 tb_clientrpc_not_implemented_twoway_on_response,
                                                 tb_clientrpc_not_implemented_twoway_on_timeout);
    ESP_LOGI(TAG, "Send Client-side RPC: request_id=%d", request_id);
    cJSON_Delete(params);
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

    ESP_LOGI(TAG, "Connect tbcmh ...");
    tbc_transport_config_esay_t config = {
        .uri = uri,                     /*!< Complete ThingsBoard MQTT broker URI */
        .access_token = access_token,   /*!< ThingsBoard Access Token */
        .log_rxtx_package = true        /*!< print Rx/Tx MQTT package */
    };
    bool result = tbcmh_connect_using_url(client, &config, TBCMH_FUNCTION_CLIENT_RPC,
                        NULL, tb_on_connected, tb_on_disconnected);
    if (!result) {
        ESP_LOGE(TAG, "failure to connect to tbcmh!");
        goto exit_destroy;
    }

    // Do...
    _set_timezone();
    int i = 0;
    while (i<40) { // TB_MQTT_TIMEOUT is 30 seconds!
        if (tbcmh_has_events(client)) {
            tbcmh_run(client);
        }

        i++;
        if (tbcmh_is_connected(client)) {
            if (i%15 == 0){
                tb_clientrpc_publish_local_time_send(client);
                tb_clientrpc_get_current_time_send(client);
                tb_clientrpc_loopback_send(client);
                tb_clientrpc_not_implemented_twoway_send(client);
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
    esp_log_level_set("client_rpc", ESP_LOG_VERBOSE);
    esp_log_level_set("ota_update", ESP_LOG_VERBOSE);
    esp_log_level_set("server_rpc", ESP_LOG_VERBOSE);
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

