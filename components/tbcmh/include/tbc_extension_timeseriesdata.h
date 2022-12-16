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

// This file is part of the ThingsBoard Client Extension (TBCE) API.

#ifndef _TBC_EXTENSION_TIMESERIESDATA_H_
#define _TBC_EXTENSION_TIMESERIESDATA_H_

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * TBCE Time-series data handle
 */
typedef struct tbce_timeseriesdata* tbce_timeseriesdata_handle_t;

/**
 * @brief  Callback of getting value of a time-series axis
 *
 * Notes:
 * - If you call tbce_timeseriesdata_register(), this callback will be called
 *      when you upload time-series data
 * - Don't call TBCMH API in this callback!
 * - Free return value(tbcmh_value_t) by caller/(this library)!
 *
 * @param context           context param 
 *
 * @return current value of a time-series axis on successful
 *         NULL on failure
 */
typedef tbcmh_value_t* (*tbce_timeseriesaxis_on_get_t)(void *context);

/**
 * @brief   Creates TBCE Time-series data handle
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @return  tbce_timeseriesdata_handle_t if successfully created, NULL on error
 */
tbce_timeseriesdata_handle_t tbce_timeseriesdata_create(void);

/**
 * @brief   Destroys TBCE Time-series data handle
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @param   tsdata  TBCE Time-series data handle
 */
void tbce_timeseriesdata_destroy(tbce_timeseriesdata_handle_t tsdata);

/**
 * @brief Register a time axis to TBCE Time-series data set
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @param tsdata        TBCE Time-series data handle
 * @param key           name of a Time-series axis
 * @param context       
 * @param on_get        Callback of getting value of a time-series axis
 * 
 * @return  0/ESP_OK on success
 *         -1/ESP_FAIL on failure
 */
tbc_err_t tbce_timeseriesdata_register(tbce_timeseriesdata_handle_t tsdata,
                                        const char *key,
                                        void *context,
                                        tbce_timeseriesaxis_on_get_t on_get);

/**
 * @brief Unregister a time-series axis from TBCE Time-series data set
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @param tsdata        TBCE Time-series data set
 * @param key           name of a Time-series axis
 * 
 * @return  0/ESP_OK on success
 *         -1/ESP_FAIL on failure
 */
tbc_err_t tbce_timeseriesdata_unregister(tbce_timeseriesdata_handle_t tsdata,
                                        const char *key);

/**
 * @brief Publish some Time-series axes in TBCE Time-series data to the server
 *
 * Notes:
 * - It should be called after the MQTT connection is established
 *
 * @param tsdata     TBCE Time-series data
 * @param client     ThingsBoard Client MQTT Helper handle
 * @param count      count of keys
 * @param keys       names of some Time-series axes
 
 *
 * @return  0/ESP_OK on success
 *         -1/ESP_FAIL on error
 */
tbc_err_t tbce_timeseriesdata_upload(tbce_timeseriesdata_handle_t tsdata,
                                        tbcmh_handle_t client,
                                        int count, /*const char *key,*/ ...);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
