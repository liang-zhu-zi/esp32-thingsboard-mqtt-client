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

// ThingsBoard Client transport config API

#ifndef _TBC_TRANSPROT_CONFIG_H_
#define _TBC_TRANSPROT_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ThingsBoard Client transport address configuration
 * 
 * Refence esp_mqtt_client_config_t.broker.address & esp_http_client_config_t.
 * 
 * MQTT:
 *   schema:  mqtt, mqtts, --ws--, --wss--  // esp_mqtt_transport_t transport
 *   host:    ...
 *   port:    1883, 8883,  80, 443
 * 
 * HTTP:
 *   schema:  http, https                   // esp_http_client_transport_t transport
 *   host:    ...
 *   port:
 * 
 * CoAP:
 *   schema:  coap, coaps
 *   host:    ...
 *   port:
 * 
 */
typedef struct tbc_transport_address_config
{
    //bool tlsEnabled; /*!< Enabled TLS/SSL or DTLS */

    const char *schema;     /*!< MQTT: mqtt/mqtts/ws/wss, HTTP: http/https, CoAP: coap/coaps */
    const char *host;       /*!< MQTT/HTTP/CoAP server domain, hostname, to set ipv4 pass it as string */
    uint32_t    port;       /*!< MQTT/HTTP/CoAP server port */
    const char *path;       /*!< Path in the URI*/
} tbc_transport_address_config_t;

/**
 * ThingsBoard Client transport credentials type
 * 
 */
typedef enum tbc_transport_credentials_type
{
  TBC_TRANSPORT_CREDENTIALS_TYPE_NONE = 0,         /*!< None. for provision.   for MQTT, HTTP, CoAP */
  TBC_TRANSPORT_CREDENTIALS_TYPE_ACCESS_TOKEN,     /*!< Access Token.          for MQTT, HTTP, CoAP */
  TBC_TRANSPORT_CREDENTIALS_TYPE_BASIC_MQTT,       /*!< Basic MQTT Credentials.for MQTT             */
  TBC_TRANSPORT_CREDENTIALS_TYPE_X509              /*!< X.509 Certificate.     for MQTT,       CoAP */

  //HTTP_AUTH_TYPE_BASIC,                   /*!< HTTP Basic authentication */
  //HTTP_AUTH_TYPE_DIGEST,                  /*!< HTTP Disgest authentication */
} tbc_transport_credentials_type_t;

/**
 * ThingsBoard Client transport credentials configuration
 *
 * reference esp_mqtt_client_config_t.credentials
 * 
 * reference https://thingsboard.io/docs/user-guide/device-provisioning/#mqtt-device-apis
 *
 * MQTT/HTTP/CoAP - Access Token based authentication configuration, e.g.:
          {
            "credentialsType":"ACCESS_TOKEN",
            "credentialsValue":"sLzc0gDAZPkGMzFVTyUY"
            "status":"SUCCESS"
          }
 *
 * MQTT - Basic MQTT authentication configuration, e.g.:
          {
            "credentialsType":"MQTT_BASIC",
            "credentialsValue": {
              "clientId":"DEVICE_CLIENT_ID_HERE",
              "userName":"DEVICE_USERNAME_HERE",
              "password":"DEVICE_PASSWORD_HERE"
              },
            "status":"SUCCESS"
          }
 *
 * MQTT/CoAP? - X.509 Certificate Based Authentication configuration, e.g.:
          {
            "deviceId":"3b829220-232f-11eb-9d5c-e9ed3235dff8",
            "credentialsType":"X509_CERTIFICATE",
            "credentialsId":"f307a1f717a12b32c27203cf77728d305d29f64694a8311be921070dd1259b3a",
            "credentialsValue":"MIIB........AQAB",
            "provisionDeviceStatus":"SUCCESS"
          }
 */
typedef struct tbc_transport_credentials_config {
    tbc_transport_credentials_type_t type; /*!< ThingsBoard Client transport authentication/credentials type */

    const char *client_id;          /*!< MQTT.           default client id is ``ESP32_%CHIPID%`` where %CHIPID% are last 3 bytes of MAC address in hex format */
    const char *username;           /*!< MQTT/HTTP.      username */
    const char *password;           /*!< MQTT/HTTP.      password */

    const char *token;              /*!< MQTT/HTTP/CoAP: username/path param/path param */
                                    /*!< At TBC_TRANSPORT_CREDENTIALS_TYPE_X509 it's a client public key. DON'T USE IT! */
} tbc_transport_credentials_config_t;

/**
 * ThingsBoard Client transport security verification of the broker/server (TLS/DTLS) configuration.
 * 
 * Refence esp_mqtt_client_config_t.broker.verification & esp_http_client_config_t.
 * 
 */
typedef struct tbc_transport_verification_config
{
    //bool      use_global_ca_store;    /*!< Use a global ca_store, look esp-tls documentation for details. */
    //esp_err_t (*crt_bundle_attach)(void *conf); 
                                        /*!< Pointer to ESP x509 Certificate Bundle attach function for the usage of certificate bundles. */
    const char *cert_pem;               /*!< Pointer to certificate data in PEM or DER format for server verify (with SSL), default is NULL, not required to verify the server. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in cert_len. */
    size_t      cert_len;               /*!< Length of the buffer pointed to by cert_pem. May be 0 for null-terminated pem */
    //const struct psk_key_hint *psk_hint_key;
                                        /*!< Pointer to PSK struct defined in esp_tls.h to enable PSK
                                             authentication (as alternative to certificate verification).
                                             PSK is enabled only if there are no other ways to
                                             verify broker.*/
    bool skip_cert_common_name_check;   /*!< Skip any validation of server certificate CN field, this reduces the security of TLS and makes the mqtt client susceptible to MITM attacks  */
    //const char **alpn_protos;         /*!< NULL-terminated list of supported application protocols to be used for ALPN */
} tbc_transport_verification_config_t;

/**
 * ThingsBoard Client transport client-authentication (TLS/DTLS) configuration.
 * 
 * Reference esp_mqtt_client_config_t.credentials.authentication & esp_http_client_config_t.
 * 
 */
typedef struct tbc_transport_authentication_config
{
    const char *client_cert_pem;        /*!< Pointer to certificate data in PEM or DER format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_key_pem` has to be provided. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in client_cert_len. */
    size_t      client_cert_len;        /*!< Length of the buffer pointed to by client_cert_pem. May be 0 for null-terminated pem */
    const char *client_key_pem;         /*!< Pointer to private key data in PEM or DER format for SSL mutual authentication, default is NULL, not required if mutual authentication is not needed. If it is not NULL, also `client_cert_pem` has to be provided. PEM-format must have a terminating NULL-character. DER-format requires the length to be passed in client_key_len */
    size_t      client_key_len;         /*!< Length of the buffer pointed to by client_key_pem. May be 0 for null-terminated pem */
    const char *client_key_password;    /*!< Client key decryption password string */
    size_t      client_key_password_len;/*!< String length of the password pointed to by client_key_password */
    //bool      use_secure_element;     /*!< Enable secure element, available in ESP32-ROOM-32SE, for SSL connection */
    //void     *ds_data;                /*!< Carrier of handle for digital signature parameters, digital signature peripheral is available in some Espressif devices. */
} tbc_transport_authentication_config_t;

/**
 * ThingsBoard Client transport client-authentication (TLS/DTLS) configuration.
 * 
 * Reference esp_mqtt_client_config_t & esp_http_client_config_t.
 * 
 */
typedef struct tbc_transport_config
{
  tbc_transport_address_config_t address;               /*!< MQTT: broker, HTTP: server, CoAP: server */
  tbc_transport_credentials_config_t credentials;       /*!< Client related credentials for authentication */
  tbc_transport_verification_config_t verification;     /*!< Security verification of the broker/server */
  tbc_transport_authentication_config_t authentication; /*!< Client authentication for mutual authentication using TLS */

  bool log_rxtx_package; /*!< print Rx/Tx MQTT package */
} tbc_transport_config_t;

/**
 * ThingsBoard Client transport easy config.
 *
 */
typedef struct
{
  const char *uri;             /*!< Complete MQTT broker URI */
  const char *access_token;    /*!< Access Token */
  const bool log_rxtx_package; /*!< print Rx/Tx MQTT package */
} tbc_transport_config_esay_t;

/**
 * @brief  Clone a transport credentials configuration
 *
 * @param credentials    ThingsBoard MQTT Client Helper handle. client param of tbcmh_connect()
 *
 * @return a transport credentials configuration copy on successful
 *         NULL on failure
 * 
 */
tbc_transport_credentials_config_t *tbc_transport_credentials_clone(
                                const tbc_transport_credentials_config_t *credentials);

/**
 * @brief  Copy a transport credentials configuration
 *
 * @param dest    
 * @param src    
 *
 * @return dest on successful
 *         NULL on failure
 */
tbc_transport_credentials_config_t *tbc_transport_credentials_config_copy(
                                tbc_transport_credentials_config_t *dest,
                                const tbc_transport_credentials_config_t *src);

/**
 * @brief  Copy a transport configuration
 *
 * @param dest    
 * @param src    
 *
 * @return dest on successful
 *         NULL on failure
 */
tbc_transport_config_t *tbc_transport_config_copy(
                                tbc_transport_config_t *dest,
                                const tbc_transport_config_t *src);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
