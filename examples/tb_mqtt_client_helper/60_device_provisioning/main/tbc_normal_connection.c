// Copyright 2022 liangzhuzhi2020@gmail.com, https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// ThingsBoard MQTT Client high layer API

#include <stdio.h>
#include <string.h>
//#include <stdarg.h>

//#include "freertos/FreeRTOS.h"
//#include "freertos/queue.h"
//#include "freertos/semphr.h"
#include "sys/queue.h"
#include "esp_err.h"
#include "esp_log.h"

#include "tbc_mqtt.h"
#include "tbc_mqtt_helper.h"

//#include "tbc_utils.h"
//#include "timeseriesdata.h"
//#include "client_attribute.h"
//#include "shared_attribute.h"
//#include "attributes_request.h"
//#include "server_rpc.h"
//#include "client_rpc.h"
//#include "device_provision.h"
//#include "ota_update.h"

//#include "device_provision.h"

static const char *TAG = "NORMAL-CONN";

/*!< Callback of connected ThingsBoard MQTT */
void tb_normalconn_on_connected(tbcmh_handle_t client, void *context)
{
   ESP_LOGI(TAG, "NORMAL CONN: Connected to thingsboard server!");
}

/*!< Callback of disconnected ThingsBoard MQTT */
void tb_normalconn_on_disconnected(tbcmh_handle_t client, void *context)
{
   ESP_LOGI(TAG, "NORMAL CONN: Disconnected from thingsboard server!");
}

tbcmh_handle_t tbcmh_normalconn_create(const tbc_transport_config_t *transport)
{
    if (!transport) {
        ESP_LOGE(TAG, "NORMAL CONN: transport is NULL!");
        return NULL;
    }
        
    ESP_LOGI(TAG, "NORMAL CONN: Init tbcmh ...");
    bool is_running_in_mqtt_task = false;
    tbcmh_handle_t client = tbcmh_init(is_running_in_mqtt_task);
    if (!client) {
        ESP_LOGE(TAG, "NORMAL CONN: Failure to init tbcmh!");
        return NULL;
    }

    ESP_LOGI(TAG, "NORMAL CONN: Connect tbcmh ...");
    bool result = tbcmh_connect(client, transport, TBCMH_FUNCTION_FULL_GENERAL, NULL,
                                   tb_normalconn_on_connected,
                                   tb_normalconn_on_disconnected);
    if (!result) {
        ESP_LOGE(TAG, "NORMAL CONN: failure to connect to tbcmh!");
        ESP_LOGI(TAG, "NORMAL CONN: Destroy tbcmh ...");
        tbcmh_destroy(client);
    }

    return client;
}

