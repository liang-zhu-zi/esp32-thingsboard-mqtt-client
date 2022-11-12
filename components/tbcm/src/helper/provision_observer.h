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

#ifndef _TBCMH_PROVISION_OBSERVER_H_
#define _TBCMH_PROVISION_OBSERVER_H_

#include <stdint.h>
#include <stdbool.h>

#include "tbc_utils.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//====6.Client-side RPC================================================================================================
/**
 * ThingsBoard MQTT Client Helper client-RPC
 */
typedef struct tbcmh_provision
{
     tbcmh_handle_t client;        /*!< ThingsBoard MQTT Client Helper */

     ////const char *params_key;   /*!< params key, default "params" */
     ////const char *results_key;  /*!< results key, default "results" */

     //char *method; /*!< method value */
     tbcmh_provision_params_t *params;
     int request_id;
     void *context;                             /*!< Context of callback */
     tbcmh_provision_on_response_t on_response; /*!< Callback of provision response success */
     tbcmh_provision_on_timeout_t on_timeout;   /*!< Callback of provision response timeout */

     LIST_ENTRY(tbcmh_provision) entry;
} tbcmh_provision_t;

// static tbcmh_provision_t *_tbcmh_provision_init(tbcmh_handle_t client, int request_id,
//                                          const tbcmh_provision_params_t *params,
//                                          void *context,
//                                          tbcmh_provision_on_response_t on_response,
//                                          tbcmh_provision_on_timeout_t on_timeout); /*!< Initialize tbcmh_provision_t */
// static tbcmh_provision_t *_tbcmh_provision_clone_wo_listentry(tbcmh_provision_t *src);
// static tbc_err_t _tbcmh_provision_destroy(tbcmh_provision_t *provision);                /*!< Destroys the tbcmh_provision_t */

// static int _tbcmh_provision_get_request_id(tbcmh_provision_t *provision);
// static void _tbcmh_provision_do_response(tbcmh_provision_t *provision, const tbcmh_provision_results_t *results);
// static void _tbcmh_provision_do_timeout(tbcmh_provision_t *provision);

/*static*/ tbc_err_t _tbcmh_provision_empty(tbcmh_handle_t client_);
/*static*/ void _tbcmh_provision_on_response(tbcmh_handle_t client_, int request_id, const cJSON *object);
/*static*/ void _tbcmh_provision_on_timeout(tbcmh_handle_t client_, int request_id);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
