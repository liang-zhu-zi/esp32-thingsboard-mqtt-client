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

#ifndef _TBCE_SHARED_ATTRIBUTES_H_
#define _TBCE_SHARED_ATTRIBUTES_H_

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

// Set value of the device's shared-attribute
// Caller (TBCMH library) of this callback will release memory of the `value` param
// return 2 if tbcmh_disconnect()/tbcmh_destroy() is called inside it.
//      Caller (TBCMH library) will not update other shared attributes received this time.
//      If this callback is called while processing the response of an attribute request - _tbcmh_attributesrequest_on_data(),
//      the response callback of the attribute request - tbcmh_attributesrequest_on_response_t/on_response, will not be called.
// return 1 if tbce_sharedattributes_unregister() is called.
//      Caller (TBCMH library) will not update other shared attributes received this time.
// return 0/ESP_OK on success
// return -1/ESP_FAIL on failure
typedef tbc_err_t (*tbce_sharedattribute_on_set_t)(void *context, const tbcmh_value_t *value);

typedef struct tbce_sharedattributes* tbce_sharedattributes_handle_t;

tbce_sharedattributes_handle_t tbce_sharedattributes_create(void);
void                           tbce_sharedattributes_destroy(tbce_sharedattributes_handle_t sharedattriburtes);

tbc_err_t tbce_sharedattributes_register(tbce_sharedattributes_handle_t sharedattriburtes,
                                const char *key, void *context,
                                tbce_sharedattribute_on_set_t on_set);
tbc_err_t tbce_sharedattributes_unregister(tbce_sharedattributes_handle_t sharedattriburtes,
                                const char *key);
void tbce_sharedattributes_subscribe(tbce_sharedattributes_handle_t sharedattriburtes,
                                tbcmh_handle_t client, uint32_t max_attributes_per_subscribe);
void tbce_sharedattributes_unsubscribe(tbce_sharedattributes_handle_t sharedattriburtes);

void tbce_sharedattributes_initialized(tbce_sharedattributes_handle_t sharedattriburtes,
                                                  uint32_t max_attributes_per_request);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
