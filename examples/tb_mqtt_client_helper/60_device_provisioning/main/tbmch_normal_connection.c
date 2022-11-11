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

#include "tb_mqtt_client.h"
#include "tb_mqtt_client_helper.h"

//#include "tb_mqtt_client_helper_log.h"
//#include "timeseries_data_helper.h"
//#include "client_attribute_helper.h"
//#include "shared_attribute_observer.h"
//#include "attributes_request_observer.h"
//#include "server_rpc_observer.h"
//#include "client_rpc_observer.h"
//#include "provision_observer.h"
//#include "ota_update_observer.h"

//#include "tbmch_provision.h"

static const char *TAG = "NORMAL-CONN";

/*!< Callback of connected ThingsBoard MQTT */
void tb_normalconn_on_connected(tbmch_handle_t client, void *context)
{
   ESP_LOGI(TAG, "NORMAL CONN: Connected to thingsboard server!");
}

/*!< Callback of disconnected ThingsBoard MQTT */
void tb_normalconn_on_disconnected(tbmch_handle_t client, void *context)
{
   ESP_LOGI(TAG, "NORMAL CONN: Disconnected from thingsboard server!");
}

tbmch_handle_t tbmch_normalconn_create(const tbc_transport_config_t *transport)
{
    if (!transport) {
        ESP_LOGE(TAG, "NORMAL CONN: transport is NULL!");
        return NULL;
    }
        
    ESP_LOGI(TAG, "NORMAL CONN: Init tbmch ...");
    tbmch_handle_t client = tbmch_init();
    if (!client) {
        ESP_LOGE(TAG, "NORMAL CONN: Failure to init tbmch!");
        return NULL;
    }

    ESP_LOGI(TAG, "NORMAL CONN: Connect tbmch ...");
    bool result = tbmch_connect_ex(client, transport, NULL,
                                   tb_normalconn_on_connected,
                                   tb_normalconn_on_disconnected);
    if (!result) {
        ESP_LOGE(TAG, "NORMAL CONN: failure to connect to tbmch!");
        ESP_LOGI(TAG, "NORMAL CONN: Destroy tbmch ...");
        tbmch_destroy(client);
    }

    return client;
}

