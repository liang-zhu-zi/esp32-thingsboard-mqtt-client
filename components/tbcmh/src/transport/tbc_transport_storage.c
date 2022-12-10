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

// ThingsBoard Client transport storage

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
#include "tbc_transport_storage.h"

static const char *TAG = "TBC_TRANSPORT_STORAGE";

static void *_transport_address_storage_fill_from_config(tbc_transport_address_storage_t *storage,
                                             const tbc_transport_address_config_t *config)
{

    TBC_CHECK_PTR_WITH_RETURN_VALUE(storage, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, NULL);

    //storage->tlsEnabled = config->tlsEnabled;
    TBC_FIELD_STRDUP(storage->schema, config->schema);
    TBC_FIELD_STRDUP(storage->host, config->host);
    storage->port = config->port;
    TBC_FIELD_STRDUP(storage->path, config->path);
    return storage;
}

static void *_transport_credentials_storage_fill_from_config(tbc_transport_credentials_storage_t *storage,
                                      const tbc_transport_credentials_config_t *config)
{

    TBC_CHECK_PTR_WITH_RETURN_VALUE(storage, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, NULL);

    storage->type = config->type;
    TBC_FIELD_STRDUP(storage->client_id, config->client_id);
    TBC_FIELD_STRDUP(storage->username, config->username);
    TBC_FIELD_STRDUP(storage->password, config->password);
    TBC_FIELD_STRDUP(storage->token, config->token);
    return storage;
}

static void *_transport_verification_storage_fill_from_config(tbc_transport_verification_storage_t *storage,
                                const tbc_transport_verification_config_t *config)
{

    TBC_CHECK_PTR_WITH_RETURN_VALUE(storage, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, NULL);

    //storage->use_global_ca_store = config->use_global_ca_store;
    //storage->crt_bundle_attach = config->crt_bundle_attach;
    if (config->cert_len>0) {
        TBC_FIELD_MEMDUP(storage->cert_pem, config->cert_pem, config->cert_len);
    } else {
        TBC_FIELD_STRDUP(storage->cert_pem, config->cert_pem);
    }
    storage->cert_len = config->cert_len;              /*!< Length of the buffer pointed to by cert_pem. May be 0 for null-terminated pem */
    // // TODO:??? storage->psk_hint_key = malloc(sizeof(struct  psk_key_hint)); memcpy(storage->psk_hint_key, config->psk_hint_key, sizeof(struct  psk_key_hint));
    storage->skip_cert_common_name_check = config->skip_cert_common_name_check;
    // // TODO: char **alpn_protos;                    /*!< NULL-terminated list of supported application protocols to be used for ALPN */

    return storage;
}

static void *_transport_authentication_storage_fill_from_config(tbc_transport_authentication_storage_t *storage,
                        const tbc_transport_authentication_config_t *config)
{

    TBC_CHECK_PTR_WITH_RETURN_VALUE(storage, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, NULL);

    if (config->client_cert_len>0) {
        TBC_FIELD_MEMDUP(storage->client_cert_pem, config->client_cert_pem, config->client_cert_len); /*!< Pointer to certificate data in PEM or DER format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_key_pem` has to be provided. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in client_cert_len. */
    } else {
        TBC_FIELD_STRDUP(storage->client_cert_pem, config->client_cert_pem);
    }
    storage->client_cert_len = config->client_cert_len; /*!< Length of the buffer pointed to by client_cert_pem. May be 0 for null-terminated pem */

    if (config->client_key_len>0) {
        TBC_FIELD_MEMDUP(storage->client_key_pem, config->client_key_pem, config->client_key_len);  /*!< Pointer to private key data in PEM or DER format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_cert_pem` has to be provided. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in client_key_len */
    } else {
        TBC_FIELD_STRDUP(storage->client_key_pem, config->client_key_pem);
    }
    storage->client_key_len = config->client_key_len; /*!< Length of the buffer pointed to by client_key_pem. May be 0 for null-terminated pem */

    if (config->client_key_password_len>0) {
        TBC_FIELD_MEMDUP(storage->client_key_password, config->client_key_password, config->client_key_password_len); /*!< Client key decryption password string */
    } else {
        TBC_FIELD_STRDUP(storage->client_key_password, config->client_key_password); /*!< String length of the password pointed to by client_key_password */
    }
    storage->client_key_password_len = config->client_key_password_len;              

    //storage->use_secure_element = config->use_secure_element; /*!< Enable secure element, available in ESP32-ROOM-32SE, for SSL connection */
    // // TODO: void     *ds_data;                /*!< Carrier of handle for digital signature parameters, digital signature peripheral is available in some Espressif devices. */

    return storage;
}

void *tbc_transport_storage_fill_from_config(tbc_transport_storage_t *storage,
                                             const tbc_transport_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(storage, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, NULL);

    _transport_address_storage_fill_from_config(&storage->address, &config->address);
    _transport_credentials_storage_fill_from_config(&storage->credentials, &config->credentials);
    _transport_verification_storage_fill_from_config(&storage->verification, &config->verification);
    _transport_authentication_storage_fill_from_config(&storage->authentication, &config->authentication);

    storage->log_rxtx_package = config->log_rxtx_package;
    return storage;
}

static void _transport_address_storage_free_fields(tbc_transport_address_storage_t *address)
{
    TBC_CHECK_PTR(address);

    //address->tlsEnabled = false;    /*!< Enabled TLS/SSL or DTLS */
    TBC_FIELD_FREE(address->schema);
    TBC_FIELD_FREE(address->host);
    address->port = 0;
    TBC_FIELD_FREE(address->path);
}

static void _transport_credentials_storage_free_fields(tbc_transport_credentials_storage_t *credentials)
{
    TBC_CHECK_PTR(credentials);

    credentials->type = 0;
    TBC_FIELD_FREE(credentials->client_id);
    TBC_FIELD_FREE(credentials->username);
    TBC_FIELD_FREE(credentials->password);

    TBC_FIELD_FREE(credentials->token);
}

static void _transport_verification_storage_free_fields(tbc_transport_verification_storage_t *verification)
{
    TBC_CHECK_PTR(verification);

    //bool      use_global_ca_store;    /*!< Use a global ca_store, look esp-tls documentation for details. */
    //esp_err_t (*crt_bundle_attach)(void *conf); 
                                        /*!< Pointer to ESP x509 Certificate Bundle attach function for the usage of certificate bundles. */
    TBC_FIELD_FREE(verification->cert_pem); /*!< Pointer to certificate data in PEM or DER format for server verify (with SSL), default is NULL, not required to verify the server. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in cert_len. */
    verification->cert_len = 0;         /*!< Length of the buffer pointed to by cert_pem. May be 0 for null-terminated pem */
    //struct  psk_key_hint *psk_hint_key;
                                        /*!< Pointer to PSK struct defined in esp_tls.h to enable PSK
                                             authentication (as alternative to certificate verification).
                                             PSK is enabled only if there are no other ways to
                                             verify broker.*/
    verification->skip_cert_common_name_check = false;
    //char **alpn_protos;               /*!< NULL-terminated list of supported application protocols to be used for ALPN */
}

static void _transport_authentication_storage_free_fields(tbc_transport_authentication_storage_t *authentication)
{
    TBC_CHECK_PTR(authentication);

    TBC_FIELD_FREE(authentication->client_cert_pem);        /*!< Pointer to certificate data in PEM or DER format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_key_pem` has to be provided. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in client_cert_len. */
    authentication->client_cert_len = 0;                /*!< Length of the buffer pointed to by client_cert_pem. May be 0 for null-terminated pem */
    TBC_FIELD_FREE(authentication->client_key_pem);         /*!< Pointer to private key data in PEM or DER format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_cert_pem` has to be provided. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in client_key_len */
    authentication->client_key_len = 0;                 /*!< Length of the buffer pointed to by client_key_pem. May be 0 for null-terminated pem */
    TBC_FIELD_FREE(authentication->client_key_password);    /*!< Client key decryption password string */
    authentication->client_key_password_len = 0;        /*!< String length of the password pointed to by client_key_password */
    //bool      use_secure_element;     /*!< Enable secure element, available in ESP32-ROOM-32SE, for SSL connection */
    //void     *ds_data;                /*!< Carrier of handle for digital signature parameters, digital signature peripheral is available in some Espressif devices. */
}

void tbc_transport_storage_free_fields(tbc_transport_storage_t *storage)
{
    TBC_CHECK_PTR(storage);

    _transport_address_storage_free_fields(&storage->address);
    _transport_credentials_storage_free_fields(&storage->credentials);
    _transport_verification_storage_free_fields(&storage->verification);
    _transport_authentication_storage_free_fields(&storage->authentication);
    storage->log_rxtx_package = false;
}

