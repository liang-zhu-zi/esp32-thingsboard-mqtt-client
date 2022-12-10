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

// ThingsBoard MQTT Client low layer API

#include <stdio.h>
#include <string.h>
//#include <time.h>

//#include "freertos/FreeRTOS.h"
//#include "sys/queue.h"
#include "esp_err.h"
//#include "mqtt_client.h"

#include "tbc_utils.h"
#include "tbc_mqtt_wapper.h"

#include "tbc_utils.h"

#include "tbc_transport_config.h"

static const char *TAG = "TBC_TRANSPORT_CONFIG";

static void *_transport_address_config_copy(tbc_transport_address_config_t *dest,
                                        const tbc_transport_address_config_t *src)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(dest, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(src, NULL);

    //dest->tlsEnabled = src->tlsEnabled;
    dest->schema = src->schema;
    dest->host = src->host;
    dest->port = src->port;
    dest->path = src->path;
    return dest;
}

tbc_transport_credentials_config_t *tbc_transport_credentials_config_copy(tbc_transport_credentials_config_t *dest,
                                              const tbc_transport_credentials_config_t *src)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(dest, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(src, NULL);

    dest->type = src->type;

    dest->client_id = src->client_id;
    dest->username = src->username;
    dest->password = src->password;

    dest->token = src->token;
    return dest;
}

static void *_transport_verification_config_copy(tbc_transport_verification_config_t *dest,
                                              const tbc_transport_verification_config_t *src)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(dest, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(src, NULL);

    //dest->use_global_ca_store = src->use_global_ca_store;
    //dest->crt_bundle_attach = src->crt_bundle_attach;

    dest->cert_pem = src->cert_pem;
    dest->cert_len = src->cert_len;

    //dest->psk_hint_key = src->psk_hint_key;
    dest->skip_cert_common_name_check = src->skip_cert_common_name_check;
    //dest->alpn_protos = src->alpn_protos;
    return dest;
}

static void *_transport_authentication_config_copy(tbc_transport_authentication_config_t *dest,
                                              const tbc_transport_authentication_config_t *src)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(dest, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(src, NULL);

    dest->client_cert_pem = src->client_cert_pem;
    dest->client_cert_len = src->client_cert_len;
    dest->client_key_pem = src->client_key_pem;
    dest->client_key_len = src->client_key_len;
    dest->client_key_password = src->client_key_password;
    dest->client_key_password_len = src->client_key_password_len;
    //dest->use_secure_element = src->use_secure_element;
    //dest->ds_data = src->ds_data;
    return dest;
}

tbc_transport_config_t *tbc_transport_config_copy(tbc_transport_config_t *dest, const tbc_transport_config_t *src)
{
    void *result;

    TBC_CHECK_PTR_WITH_RETURN_VALUE(dest, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(src, NULL);

    result = _transport_address_config_copy(&dest->address, &src->address);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(result, NULL);
    result = tbc_transport_credentials_config_copy(&dest->credentials, &src->credentials);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(result, NULL);
    result = _transport_verification_config_copy(&dest->verification, &src->verification);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(result, NULL);
    result = _transport_authentication_config_copy(&dest->authentication, &src->authentication);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(result, NULL);

    dest->log_rxtx_package = src->log_rxtx_package;
    return dest;
}

tbc_transport_credentials_config_t *tbc_transport_credentials_clone(
                                        const tbc_transport_credentials_config_t *credentials)
{

    TBC_CHECK_PTR_WITH_RETURN_VALUE(credentials, NULL);

    tbc_transport_credentials_config_t *new_credentials = calloc(1, sizeof(tbc_transport_credentials_config_t));
    TBC_MEM_CHECK(TAG, new_credentials, return NULL);

    new_credentials->type = credentials->type;
    TBC_FIELD_STRDUP(new_credentials->client_id, credentials->client_id);
    TBC_FIELD_STRDUP(new_credentials->username, credentials->username);
    TBC_FIELD_STRDUP(new_credentials->password, credentials->password);
    TBC_FIELD_STRDUP(new_credentials->token, credentials->token);

    return new_credentials;
}
