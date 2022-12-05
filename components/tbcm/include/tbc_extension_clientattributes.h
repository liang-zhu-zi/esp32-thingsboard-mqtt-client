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

#ifndef _TBCE_CLIENT_ATTRIBUTES_H_
#define _TBCE_CLIENT_ATTRIBUTES_H_

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

// Get value of the device's client-side attribute
// Don't call TBCMH API in these callback!
// Caller (TBCMH library) of this callback will release memory of the return value
typedef tbcmh_value_t* (*tbce_clientattributes_on_get_t)(void *context);

// Set value of the device's client-side attribute. Only for initilizing client-side attribute
// Don't call TBCMH API in these callback!
// Caller (TBCMH library) of this callback will release memory of the `value` param
typedef void (*tbce_clientattributes_on_set_t)(void *context, const tbcmh_value_t *value);

typedef struct tbce_clientattributes* tbce_clientattributes_handle_t;

tbce_clientattributes_handle_t tbce_clientattributes_create(void);
void                           tbce_clientattributes_destroy(tbce_clientattributes_handle_t clientattributes);

// Call it before tbcmh_connect()
tbc_err_t tbce_clientattributes_register(tbce_clientattributes_handle_t clientattributes,
                                const char *key, void *context,
                                tbce_clientattributes_on_get_t on_get);
// Call it before tbcmh_connect()
tbc_err_t tbce_clientattributes_register_with_set(tbce_clientattributes_handle_t clientattributes,
                                const char *key, void *context,
                                tbce_clientattributes_on_get_t on_get,
                                tbce_clientattributes_on_set_t on_set);
tbc_err_t tbce_clientattributes_unregister(tbce_clientattributes_handle_t clientattributes, const char *key);

bool      tbce_clientattributes_is_contained(tbce_clientattributes_handle_t clientattributes,
                                        const char *key);
tbc_err_t tbce_clientattributes_initialized(
                                        tbce_clientattributes_handle_t clientattributes,
                                        tbcmh_handle_t client,
                                        uint32_t max_attributes_per_request);
tbc_err_t tbce_clientattributes_update(tbce_clientattributes_handle_t clientattributes,
                                        tbcmh_handle_t client, int count, /*const char *key,*/...);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
