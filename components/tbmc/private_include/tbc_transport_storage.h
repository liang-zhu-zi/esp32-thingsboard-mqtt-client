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

#ifndef _TBC_TRANSPORT_STORAGE_H_
#define _TBC_TRANSPORT_STORAGE_H_

#include <stdint.h>
//#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ThingsBoard Client transport address storage
 * 
 * Refence tbc_transport_address_config
 * 
 */
typedef struct tbc_transport_address_storage
{
    //bool tlsEnabled;    /*!< Enabled TLS/SSL or DTLS */

    char     *schema;     /*!< MQTT: mqtt/mqtts/ws/wss, HTTP: http/https, CoAP: coap/coaps */
    char     *host;       /*!< MQTT/HTTP/CoAP server domain, hostname, to set ipv4 pass it as string */
    uint32_t  port;       /*!< MQTT/HTTP/CoAP server port */
    char     *path;       /*!< Path in the URI*/
} tbc_transport_address_storage_t;


/**
 * ThingsBoard Client transport credentials storage
 *
 * reference tbc_transport_credentials_config_t
 * 
 */
typedef struct tbc_transport_credentials_storage {
    tbc_transport_credentials_type_t type; /*!< ThingsBoard Client transport authentication/credentials type */

    char *client_id;          /*!< MQTT.           default client id is ``ESP32_%CHIPID%`` where %CHIPID% are last 3 bytes of MAC address in hex format */
    char *username;           /*!< MQTT/HTTP.      username */
    char *password;           /*!< MQTT/HTTP.      password */

    char *token;              /*!< MQTT/HTTP/CoAP: username/path param/path param */
} tbc_transport_credentials_storage_t;

/**
 * ThingsBoard Client transport security verification storage.
 * 
 * Refence tbc_transport_verification_config_t.
 * 
 */
typedef struct tbc_transport_verification_storage
{
    //bool      use_global_ca_store;    /*!< Use a global ca_store, look esp-tls documentation for details. */
    //esp_err_t (*crt_bundle_attach)(void *conf); 
                                        /*!< Pointer to ESP x509 Certificate Bundle attach function for the usage of certificate bundles. */
    char        *cert_pem;              /*!< Pointer to certificate data in PEM or DER format for server verify (with SSL), default is NULL, not required to verify the server. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in cert_len. */
    size_t       cert_len;              /*!< Length of the buffer pointed to by cert_pem. May be 0 for null-terminated pem */
    //struct  psk_key_hint *psk_hint_key;
                                        /*!< Pointer to PSK struct defined in esp_tls.h to enable PSK
                                             authentication (as alternative to certificate verification).
                                             PSK is enabled only if there are no other ways to
                                             verify broker.*/
    bool skip_cert_common_name_check;   /*!< Skip any validation of server certificate CN field, this reduces the security of TLS and makes the mqtt client susceptible to MITM attacks  */
    //char **alpn_protos;               /*!< NULL-terminated list of supported application protocols to be used for ALPN */
} tbc_transport_verification_storage_t;

/**
 * ThingsBoard Client transport client-authentication (TLS/DTLS) storage.
 * 
 * Reference tbc_transport_authentication_config_t.
 * 
 */
typedef struct tbc_transport_authentication_storage
{
    char       *client_cert_pem;        /*!< Pointer to certificate data in PEM or DER format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_key_pem` has to be provided. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in client_cert_len. */
    size_t      client_cert_len;        /*!< Length of the buffer pointed to by client_cert_pem. May be 0 for null-terminated pem */
    char       *client_key_pem;         /*!< Pointer to private key data in PEM or DER format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_cert_pem` has to be provided. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in client_key_len */
    size_t      client_key_len;         /*!< Length of the buffer pointed to by client_key_pem. May be 0 for null-terminated pem */
    char       *client_key_password;    /*!< Client key decryption password string */
    size_t      client_key_password_len;/*!< String length of the password pointed to by client_key_password */
    //bool      use_secure_element;     /*!< Enable secure element, available in ESP32-ROOM-32SE, for SSL connection */
    //void     *ds_data;                /*!< Carrier of handle for digital signature parameters, digital signature peripheral is available in some Espressif devices. */
} tbc_transport_authentication_storage_t;

/**
 * ThingsBoard Client transport storage.
 * 
 * Reference tbc_transport_config_t.
 * 
 */
typedef struct tbc_transport_storage
{
  tbc_transport_address_storage_t address;               /*!< MQTT: broker, HTTP: server, CoAP: server */
  tbc_transport_credentials_storage_t credentials;       /*!< Client related credentials for authentication */
  tbc_transport_verification_storage_t verification;     /*!< Security verification of the broker/server */
  tbc_transport_authentication_storage_t authentication; /*!< Client authentication for mutual authentication using TLS */

  bool log_rxtx_package; /*!< print Rx/Tx MQTT package */
} tbc_transport_storage_t;

void *tbc_transport_storage_fill_from_config(tbc_transport_storage_t *storage,
                                             const tbc_transport_config_t *config);
void tbc_transport_storage_free_fields(tbc_transport_storage_t *storage);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
