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

#ifndef _TBCE_SHARED_ATTRIBUTES_H_
#define _TBCE_SHARED_ATTRIBUTES_H_

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * TBCE Shared attribute set handle
 */
typedef struct tbce_sharedattributes* tbce_sharedattributes_handle_t;

/**
 * @brief  callback of setting value of the device's shared attribute.
 *
 * Notes:
 * - If you call tbce_sharedattributes_register(),
 *    this callback will be called when you receied update of shared attribute
 * - Don't call TBCMH API in this callback!
 *
 * @param context   context param 
 * @param value     initialization value of the device's shared attribute
 *
 * @return 2 if tbcmh_disconnect() or tbcmh_destroy() is called inside in this callback
 *         1 if tbce_sharedattributes_unregister() is called inside in this callback
 *         0/ESP_OK     on success
 *         -1/ESP_FAIL  on failure
 */
typedef tbc_err_t (*tbce_sharedattribute_on_set_t)(void *context, const tbcmh_value_t *value);

/**
 * @brief   Creates TBCE shared attributes handle
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @return  tbce_sharedattributes_handle_t if successfully created, NULL on error
 */
tbce_sharedattributes_handle_t tbce_sharedattributes_create(void);

/**
 * @brief   Destroys TBCE shared attributes handle
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @param   sharedattriburtes    TTBCE shared attributes handle
 */
void tbce_sharedattributes_destroy(tbce_sharedattributes_handle_t sharedattriburtes);

/**
 * @brief Register a shared attribute to TBCE shared attribute set
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @param sharedattriburtes TBCE shared attributes handle
 * @param key               name of a shared attribute
 * @param context       
 * @param on_set            callback of setting value of the device's shared attribute.
 * 
 * @return  0/ESP_OK on success
 *         -1/ESP_FAIL on failure
 */
tbc_err_t tbce_sharedattributes_register(
                                tbce_sharedattributes_handle_t sharedattriburtes,
                                const char *key, void *context,
                                tbce_sharedattribute_on_set_t on_set);

/**
 * @brief Unregister a shared attribute from TBCE shared attributes
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @param sharedattriburtes   TBCE shared attributes handle
 * @param key                 name of a shared attribute
 * 
 * @return  0/ESP_OK on success
 *         -1/ESP_FAIL on failure
 */
tbc_err_t tbce_sharedattributes_unregister(
                                tbce_sharedattributes_handle_t sharedattriburtes,
                                const char *key);

/**
 * @brief Subscirbe all of shared attributes in TBCE shared attribute set to the server
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @param sharedattriburtes             TBCE shared attributes handle
 * @param client                        ThingsBoard Client MQTT Helper handle
 * @param max_attributes_per_subscribe  max shared attributes in per attributes subscribe
 *
 */
void tbce_sharedattributes_subscribe(
                                tbce_sharedattributes_handle_t sharedattriburtes,
                                tbcmh_handle_t client,
                                uint32_t max_attributes_per_subscribe);

/**
 * @brief Unsubscirbe all of shared attributes in TBCE shared attribute set from the server
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @param sharedattriburtes             TBCE shared attributes handle
 *
 */
void tbce_sharedattributes_unsubscribe(
                                tbce_sharedattributes_handle_t sharedattriburtes);

                                
/**
 * @brief Initilize all of shared attributes in TBCE shared attribute set from the server
 *
 * Notes:
 * - It should be called after the MQTT connection is established
 * - In order to initialize shared attributes need to 
 *   send mutiple attributes request to the servere
 * - All of shared attributes in TBCE shared attribute set
 *   can get initial values from the server
 *
 * @param sharedattriburtes     TBCE shared attributes handle
 * @param client                ThingsBoard Client MQTT Helper handle
 * @param max_attributes_per_request    max shared attributes in per attributes request
 *
 * @return  0/ESP_OK on success
 *         -1/ESP_FAIL on error
 */
tbc_err_t tbce_sharedattributes_initialized(
                                tbce_sharedattributes_handle_t sharedattriburtes,
                                tbcmh_handle_t client,
                                uint32_t max_attributes_per_request);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

