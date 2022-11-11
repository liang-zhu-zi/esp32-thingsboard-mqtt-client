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

// ThingsBoard Client credentials API

// TODO: SPIFFS credentials...
// TODO: NVS flash credentials...

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "tbc_transport_credentials.h"

static tbc_transport_credentials_config_t *_memory_credentials = NULL;

void tbc_transport_credentials_init(void)
{
     // TODO: SPIFFS/NVS: init SPIFFS or NVS flash/partition.

     _memory_credentials = NULL;
}

void tbc_transport_credentials_uninit(void)
{
    // TODO: SPIFFS/NVS: free _credentials

    if (_memory_credentials) {
        free(_memory_credentials);
        _memory_credentials = NULL;
    }
}

const tbc_transport_credentials_config_t *tbc_transport_credentials_get(void)
{
     // TODO: SPIFFS/NVS: read credentials from file/flash, cache credentials
     /*
     new_credentials = None
     try:
          with open("credentials", "r") as credentials_file:
               new_credentials = credentials_file.read()
     except Exception as e:
          print(e)
     return new_credentials

     if (new_credentials) {
         free(_memory_credentials);
         _memory_credentials = new_credentials;
     }
     */

     return _memory_credentials;
}

bool tbc_transport_credentials_save(const tbc_transport_credentials_config_t *credentials)
{
    // TODO: SPIFFS/NVS: save to file/flash , free cache, duplicate credentials
    /*
    with open("credentials", "w") as credentials_file:
        credentials_file.write(credentials)
    */

    if (!credentials) {
        return false;
    }

    if (_memory_credentials) {
        free(_memory_credentials);
        _memory_credentials = NULL;
    }

    _memory_credentials = tbc_transport_credentials_clone(credentials);
    return _memory_credentials ? true : false;
}

void tbc_transport_credentials_clean(void)
{
     // TODO: SPIFFS/NVS: remove file &  free cache
     /*
     open("credentials", "w").close()
     */

     if (_memory_credentials) {
         free(_memory_credentials);
         _memory_credentials = NULL;
     }
}

bool tbc_transport_credentials_is_existed(void)
{
    //SPIFFS/NVS: Note: Don't read file/falsh every time!
    return _memory_credentials ? true : false;
}
