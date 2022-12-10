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

#ifndef _CLAIMING_DEVICE_HELPER_H_
#define _CLAIMING_DEVICE_HELPER_H_

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

void _tbcmh_claimingdevice_on_create(tbcmh_handle_t client);
void _tbcmh_claimingdevice_on_destroy(tbcmh_handle_t client);
void _tbcmh_claimingdevice_on_connected(tbcmh_handle_t client);
void _tbcmh_claimingdevice_on_disconnected(tbcmh_handle_t client);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
