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

// This file is called by user.

#ifndef _TBC_EXTENSION_TELEMETRY_H_
#define _TBC_EXTENSION_TELEMETRY_H_

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

// Get value of telemetry time-series data
// Don't call TBCMH API in this callback!
// Caller (TBCE library) of this callback will release memory of the return value
typedef tbcmh_value_t* (*tbce_telemetry_on_get_t)(void *context);

typedef struct tbce_telemetry* tbce_telemetry_handle_t;

tbce_telemetry_handle_t tbce_telemetry_create(void);
void                    tbce_telemetry_destroy(tbce_telemetry_handle_t telemetry);

tbc_err_t tbce_telemetry_register(tbce_telemetry_handle_t telemetry,
                                        const char *key,
                                        void *context,
                                        tbce_telemetry_on_get_t on_get);
tbc_err_t tbce_telemetry_unregister(tbce_telemetry_handle_t telemetry,
                                         const char *key);
tbc_err_t tbce_telemetry_update(tbce_telemetry_handle_t telemetry,
                                      tbcmh_handle_t client,
                                      int count, /*const char *key,*/...);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
