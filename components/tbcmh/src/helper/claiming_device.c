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

#include "tbc_mqtt_helper_internal.h"

const static char *TAG = "claiming_device";

//==== Claiming device using device-side key scenario =================================
void _tbcmh_claimingdevice_on_create(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);
    //...
}

void _tbcmh_claimingdevice_on_destroy(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);
    //...
}

void _tbcmh_claimingdevice_on_connected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);
    //......
}

void _tbcmh_claimingdevice_on_disconnected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);
    // ...
}

tbc_err_t tbcmh_claiming_device_initiate_using_device_side_key(tbcmh_handle_t client,
                    const char *secret_key, uint32_t *duration_ms)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);

     // Take semaphore
     if (xSemaphoreTakeRecursive(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // send package...
     cJSON *object = cJSON_CreateObject(); // create json object
     if (secret_key) {
        cJSON_AddStringToObject(object, TB_MQTT_KEY_CLAIMING_DEVICE_SECRETKEY, secret_key);
     }
     if (duration_ms) {
        cJSON_AddNumberToObject(object, TB_MQTT_KEY_CLAIMING_DEVICE_DURATIONMS, *duration_ms);
     }
     char *pack = cJSON_PrintUnformatted(object); //cJSON_Print()
     int result = tbcm_claiming_device_publish(client->tbmqttclient, pack, 1/*qos*/, 0/*retain*/);
     cJSON_free(pack); // free memory
     cJSON_Delete(object); // delete json object

     // Give semaphore
     xSemaphoreGiveRecursive(client->_lock);
     return (result > -1) ? ESP_OK : ESP_FAIL;
}

