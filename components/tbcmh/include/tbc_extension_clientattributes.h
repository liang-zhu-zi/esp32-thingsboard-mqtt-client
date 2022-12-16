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

#ifndef _TBCE_CLIENT_ATTRIBUTES_H_
#define _TBCE_CLIENT_ATTRIBUTES_H_

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * TBCE Client-side attribute set handle
 */
typedef struct tbce_clientattributes* tbce_clientattributes_handle_t;

/**
 * @brief  Callback of getting value of the device's client-side attribute
 *
 * Notes:
 * - If you call tbce_clientattributes_register() or tbce_clientattributes_register_with_set(),
 *    this callback will be called when you update client-side attribute to the server
 * - Don't call TBCMH API in this callback!
 * - Free return-value(tbcmh_value_t) by caller/(this library)!
 *
 * @param context           context param 
 *
 * @return current value of a client-side attribute on successful
 *         NULL on failure
 */
typedef tbcmh_value_t* (*tbce_clientattribute_on_get_t)(void *context);

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
typedef tbc_err_t (*tbce_clientattribute_on_set_t)(void *context, const tbcmh_value_t *value);

/**
 * @brief   Creates TBCE client-side attributes handle
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @return  tbce_clientattributes_handle_t if successfully created, NULL on error
 */
tbce_clientattributes_handle_t tbce_clientattributes_create(void);

/**
 * @brief   Destroys TBCE client-side attributes handle
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @param   clientattributes    TTBCE client-side attributes handle
 */
void tbce_clientattributes_destroy(
                                tbce_clientattributes_handle_t clientattributes);

/**
 * @brief Register a client-side attribute to TBCE client-side attributes
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @param clientattributes  TBCE client-side attributes handle
 * @param key               name of a client-side attribute
 * @param context       
 * @param on_get            Callback of getting value of the device's client-side attribute
 * 
 * @return  0/ESP_OK on success
 *         -1/ESP_FAIL on failure
 */
tbc_err_t tbce_clientattributes_register(
                                tbce_clientattributes_handle_t clientattributes,
                                const char *key, void *context,
                                tbce_clientattribute_on_get_t on_get);

/**
 * @brief Register a client-side attribute to TBCE client-side attributes
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @param clientattributes  TBCE client-side attributes handle
 * @param key               name of a client-side attribute
 * @param context       
 * @param on_get            Callback of getting value of the device's client-side attribute
 * @param on_set            Callback of setting value of the device's client-side attribute.
 * 
 * @return  0/ESP_OK on success
 *         -1/ESP_FAIL on failure
 */
tbc_err_t tbce_clientattributes_register_with_set(
                                tbce_clientattributes_handle_t clientattributes,
                                const char *key, void *context,
                                tbce_clientattribute_on_get_t on_get,
                                tbce_clientattribute_on_set_t on_set);

/**
 * @brief Unregister a client-side attribute from TBCE client-side attributes
 *
 * Notes:
 * - It may be called before the MQTT connection is established
 *
 * @param clientattributes  TBCE client-side attributes handle
 * @param key               name of a client-side attribute
 * 
 * @return  0/ESP_OK on success
 *         -1/ESP_FAIL on failure
 */
tbc_err_t tbce_clientattributes_unregister(
                                tbce_clientattributes_handle_t clientattributes,
                                const char *key);

/**
 * @brief Is it a client-side attribute set contained a client-side attribute?
 *
 * @param clientattributes  TBCE client-side attributes handle
 * @param key               name of a client-side attribute
 * 
 * @return true or false
 */
bool tbce_clientattributes_is_contained(
                                tbce_clientattributes_handle_t clientattributes,
                                const char *key);

/**
 * @brief Initilize some client-side attributes in TBCE client-side attribute set from the server
 *
 * Notes:
 * - It should be called after the MQTT connection is established
 * - In order to initialize client-side attributes need to 
 *   send mutiple attributes request to the servere
 * - Only client-side attributes with tbce_clientattribute_on_set_t() callback
 *   can get initial values from the server
 *
 * @param clientattributes      TBCE client-side attributes handle
 * @param client                ThingsBoard Client MQTT Helper handle
 * @param max_attributes_per_request    max client-side attributes in per attributes request
 *
 * @return  0/ESP_OK on success
 *         -1/ESP_FAIL on error
 */
tbc_err_t tbce_clientattributes_initialize(
                                tbce_clientattributes_handle_t clientattributes,
                                tbcmh_handle_t client,
                                uint32_t max_attributes_per_request);

/**
 * @brief Update some client-side attributes in TBCE client-side attribute set to the server
 *
 * Notes:
 * - It should be called after the MQTT connection is established
 *
 * @param clientattributes      TBCE client-side attributes handle
 * @param client                ThingsBoard Client MQTT Helper handle
 * @param count                 count of keys
 * @param keys                  names of some client-side attributes
 *
 * @return  0/ESP_OK on success
 *         -1/ESP_FAIL on error
 */
tbc_err_t tbce_clientattributes_update(
                                tbce_clientattributes_handle_t clientattributes,
                                tbcmh_handle_t client,
                                int count, /*const char *key,*/...);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

