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

#include "tbc_mqtt_protocol.h"
#include "tbc_mqtt_helper.h"

static const char *TAG = "NORMAL-CONN";

#define TELEMETYR_TEMPRATUE         	"temprature"
#define TELEMETYR_HUMIDITY          	"humidity"

//Don't call TBCMH API in this callback!
//Free return value by caller/(tbcmh library)!
tbcmh_value_t* tb_telemetry_on_get_temperature(void)
{
    TBC_LOGI("Get temperature (a time-series data)");
    static float temp_array[] = {25.0, 25.5, 26.0, 26.5, 27.0, 27.5, 28.0, 27.5, 27.0, 26.5};
    static int i = 0;

    cJSON* temp = cJSON_CreateNumber(temp_array[i]);
    i++;
    i = i % (sizeof(temp_array)/sizeof(temp_array[0]));

    return temp;
}

//Don't call TBCMH API in this callback!
//Free return value by caller/(tbcmh library)!
tbcmh_value_t* tb_telemetry_on_get_humidity(void)
{
    TBC_LOGI("Get humidity (a time-series data)");

    static int humi_array[] = {26, 27, 28, 29, 30, 31, 32, 31, 30, 29};
    static int i = 0;

    cJSON* humi = cJSON_CreateNumber(humi_array[i]);
    i++;
    i = i % (sizeof(humi_array)/sizeof(humi_array[0]));

    return humi;
}

void tb_telemetry_send(tbcmh_handle_t client)
{
    TBC_LOGI("Send telemetry: %s, %s", TELEMETYR_TEMPRATUE, TELEMETYR_HUMIDITY);

    cJSON *object = cJSON_CreateObject(); // create json object
    cJSON_AddItemToObject(object, TELEMETYR_TEMPRATUE, tb_telemetry_on_get_temperature());
    cJSON_AddItemToObject(object, TELEMETYR_HUMIDITY, tb_telemetry_on_get_humidity());
    tbcmh_telemetry_upload_ex(client, object, 1/*qos*/, 0/*retain*/);
    cJSON_Delete(object); // delete json object
}


/*!< Callback of connected ThingsBoard MQTT */
void tb_normalconn_on_connected(tbcmh_handle_t client, void *context)
{
   TBC_LOGI("Connected to thingsboard server!");

   tb_telemetry_send(client);
}

/*!< Callback of disconnected ThingsBoard MQTT */
void tb_normalconn_on_disconnected(tbcmh_handle_t client, void *context)
{
   TBC_LOGI("Disconnected from thingsboard server!");
}

tbcmh_handle_t tbcmh_normalconn_create(const tbc_transport_config_t *transport)
{
    if (!transport) {
        TBC_LOGE("transport is NULL!");
        return NULL;
    }
        
    TBC_LOGI("Init tbcmh ...");
    tbcmh_handle_t client = tbcmh_init();
    if (!client) {
        TBC_LOGE("Failure to init tbcmh!");
        return NULL;
    }

    TBC_LOGI("Connect tbcmh ...");
    bool result = tbcmh_connect(client, transport, NULL,
                                   tb_normalconn_on_connected,
                                   tb_normalconn_on_disconnected);
    if (!result) {
        TBC_LOGE("failure to connect to tbcmh!");
        TBC_LOGI("Destroy tbcmh ...");
        tbcmh_destroy(client);
    }

    return client;
}

