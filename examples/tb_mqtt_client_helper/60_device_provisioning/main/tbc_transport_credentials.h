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

// ThingsBoard Client transprot authenticaiton API

#ifndef _MY_TRANSPORT_AUTH_CONFIG_H_
#define _MY_TRANSPORT_AUTH_CONFIG_H_

#include "tbc_transport_config.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define TBC_TRANSPORT_AUTH_CONFIG_PATH "xxxx"

void tbc_transport_credentials_init(void);
void tbc_transport_credentials_uninit(void);
const tbc_transport_credentials_config_t *tbc_transport_credentials_get(void);
bool tbc_transport_credentials_save(const tbc_transport_credentials_config_t *credentials);
void tbc_transport_credentials_clean(void);
bool tbc_transport_credentials_is_existed(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
