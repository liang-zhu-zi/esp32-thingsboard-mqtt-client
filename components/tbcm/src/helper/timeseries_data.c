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

// This file is called by tbc_mqtt_helper.c/.h.

#include <string.h>

#include "esp_err.h"

#include "tbc_utils.h"
#include "tbc_mqtt_helper_internal.h"

static const char *TAG = "TELEMETRY_UPLOAD";

/**
 * @brief Client to send a 'Telemetry' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/telemetry'
 *      Data:  '{"key1":"value1", "key2":true, "key3": 3.0, "key4": 4}', '[{"key1":"value1"}, {"key2":true}]'
 *
 * @param telemetry  telemetry. example: {"key1":"value1", "key2":true, "key3": 3.0, "key4": 4}, (字符串要符合 json 数据格式)
 * @param qos        qos of publish message
 * @param retain     ratain flag
 *
 * @return msg_id of the subscribe message on success
 *         0 if cannot publish
 *        -1/ESP_FAIL if error
 */
int tbcmh_telemetry_publish(tbcmh_handle_t client, const char *telemetry,
                            int qos/*= 1*/, int retain/*= 0*/)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(telemetry, ESP_FAIL);

    return tbcm_telemetry_publish(client->tbmqttclient, telemetry, qos, retain);
}

/**
 * @brief Client to send a 'Telemetry' publish message to the broker
 *
 * Notes:
 * - It is thread safe, please refer to `esp_mqtt_client_subscribe` for details
 * - A ThingsBoard MQTT Protocol message example:
 *      Topic: 'v1/devices/me/telemetry'
 *      Data:  '{"key1":"value1", "key2":true, "key3": 3.0, "key4": 4}', '[{"key1":"value1"}, {"key2":true}]'
 *
 * @param object     cJSON object or array of object?
 * @param qos        qos of publish message
 * @param retain     ratain flag
 *
 * @return msg_id of the subscribe message on success
 *         0 if cannot publish
 *        -1/ESP_FAIL if error
 */
int tbcmh_telemetry_publish_ex(tbcmh_handle_t client, tbcmh_value_t *object,
                              int qos/*= 1*/, int retain/*= 0*/)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(object, ESP_FAIL);

    char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
    int msg_id = tbcmh_telemetry_publish(client, pack, qos, retain);
    cJSON_free(pack); // free memory
    return msg_id;
}
