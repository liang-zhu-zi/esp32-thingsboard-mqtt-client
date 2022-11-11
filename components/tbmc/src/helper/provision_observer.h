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

#ifndef _TBMCH_PROVISION_OBSERVER_H_
#define _TBMCH_PROVISION_OBSERVER_H_

#include <stdint.h>
#include <stdbool.h>

#include "sys/queue.h"
#include "tbc_mqtt_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

//====6.Client-side RPC================================================================================================
/**
 * ThingsBoard MQTT Client Helper client-RPC
 */
typedef struct tbmch_provision
{
     tbmch_handle_t client;        /*!< ThingsBoard MQTT Client Helper */

     ////const char *params_key;   /*!< params key, default "params" */
     ////const char *results_key;  /*!< results key, default "results" */

     //char *method; /*!< method value */
     tbmch_provision_params_t *params;
     int request_id;
     void *context;                             /*!< Context of callback */
     tbmch_provision_on_response_t on_response; /*!< Callback of provision response success */
     tbmch_provision_on_timeout_t on_timeout;   /*!< Callback of provision response timeout */

     LIST_ENTRY(tbmch_provision) entry;
} tbmch_provision_t;

tbmch_provision_t *_tbmch_provision_init(tbmch_handle_t client, int request_id,
                                         const tbmch_provision_params_t *params,
                                         void *context,
                                         tbmch_provision_on_response_t on_response,
                                         tbmch_provision_on_timeout_t on_timeout); /*!< Initialize tbmch_provision_t */
tbmch_provision_t *_tbmch_provision_clone_wo_listentry(tbmch_provision_t *src);
tbmch_err_t _tbmch_provision_destroy(tbmch_provision_t *provision);                /*!< Destroys the tbmch_provision_t */

int _tbmch_provision_get_request_id(tbmch_provision_t *provision);
void _tbmch_provision_do_response(tbmch_provision_t *provision, const tbmch_provision_results_t *results);
void _tbmch_provision_do_timeout(tbmch_provision_t *provision);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
