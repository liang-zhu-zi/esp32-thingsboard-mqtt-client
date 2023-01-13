# ESP32 ThingsBoard Client MQTT Helper library design document

* [中文](./README_CN.md)

<https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client>

***This design document is not updated synchronously with the code.***

- [ESP32 ThingsBoard Client MQTT Helper library design document](#esp32-thingsboard-client-mqtt-helper-library-design-document)
  - [ThingsBoard Device Connectivity APIs](#thingsboard-device-connectivity-apis)
    - [Device Connectivity APIs](#device-connectivity-apis)
    - [Authentication options](#authentication-options)
    - [Provision options](#provision-options)
    - [Provision params](#provision-params)
    - [Python command params of authentication](#python-command-params-of-authentication)
    - [Firmware/Software OTA updates](#firmwaresoftware-ota-updates)
  - [ESP-MQTT API \& Task model](#esp-mqtt-api--task-model)
  - [TBCMH component](#tbcmh-component)
    - [Supported ThingsBoard MQTT API Features](#supported-thingsboard-mqtt-api-features)
    - [TBCMH overview](#tbcmh-overview)
    - [TBCMH Subscribe \& Unsubscribe](#tbcmh-subscribe--unsubscribe)

## ThingsBoard Device Connectivity APIs

### Device Connectivity APIs

| Features API         |                                                        | MQTT                                                          | HTTP                                                            | CoAP                                                    |
|----------------------|--------------------------------------------------------|---------------------------------------------------------------|-----------------------------------------------------------------|---------------------------------------------------------|
| Telemetry upload API |                                                        | PUBLISH, c->s                                                 | POST (REQ)                                                      | POST (REQ)                                              |
| Attributes API       | Publish attribute update to the server                 | PUBLISH, c->s                                                 | POST (REQ)                                                      | POST (REQ)                                              |
|                      | Request attribute values from the server               | SUBSCRIBE, c->s<br/>PUBLISH(REQ), c->s<br/>PUBLISH(RES), c<-s | GET (REQ/request<br/>   + RES/reply)                            | GET (REQ/request<br/>   + RES/reply)                    |
|                      | Subscribe to attribute updates from the server         | SUBSCRIBE, c->s<br/>PUBLISH, c<-s                             | GET+timeout<br/>     (RES/attr/timeout?)                        | GET+observe<br/>     (RES/attr/?)                       |
| RPC API              | Server-side RPC                                        | SUBSCRIBE, c->s<br/>PUBLISH(REQ), c<-s<br/>PUBLISH(RES), c->s | GET+timeout<br/>     (RES/request/timeout)<br/>POST (REQ/reply) | GET+observe <br/>     (RES/attr/?)<br/>POST (REQ/reply) |
|                      | Client-side RPC                                        | SUBSCRIBE, c->s<br/>PUBLISH(REQ), c->s<br/>PUBLISH(RES), c<-s | POST (REQ/request<br/>    + RES/reply)                          | POST (REQ/request<br/>    + RES/reply)                  |
| Claiming devices     |                                                        | PUBLISH                                                       | POST (REQ)                                                      | POST (REQ)                                              |
| Device provisioning  |                                                        | SUBSCRIBE, c->s<br/>PUBLISH(REQ), c->s<br/>PUBLISH(RES), c<-s | POST (REQ/request<br/>    + RES/reply)                          | POST (REQ/request<br/>    + RES/reply)                  |
| Firmware API         | Request attribute values from the server(fw/sw)        | SUBSCRIBE, c->s<br/>PUBLISH(REQ), c->s<br/>PUBLISH(RES), c<-s | GET (REQ/request<br/>   + RES/reply)                            | GET (REQ/request<br/>   + RES/reply)                    |
|                      | Subscribe to attribute updates from the server (fw/sw) | SUBSCRIBE, c->s<br/>PUBLISH, c<-s                             | ???                                                             | ???                                                     |
|                      | New firmware update (fw/sw)                            | SUBSCRIBE, c->s<br/>PUBLISH(REQ), c->s<br/>PUBLISH(RES), c<-s | GET (REQ/request<br/>   + RES/reply)                            | ???                                                     |

### Authentication options

| Device authentication options          |             | MQTT<br/>(SSL/TLS) | HTTP<br/>(SSL/TLS) | CoAP<br/>(DTLS)   |
|----------------------------------------|-------------|--------------------|--------------------|-------------------|
| Access Token                           | without SSL | Plain MQTT         | HTTP               | CoAP              |
|                                        | one-way SSL | MQTT over SSL      | HTTPS              | CoAP over DTLS    |
| Basic MQTT authentication              | without SSL | Plain MQTT         | /                  | /                 |
|                                        | one-way SSL | MQTT over SSL      | /                  | /                 |
| X.509 Certificate Based Authentication | two-way SSL | MQTT with X.509    | /?                 | CoAP with X.509 ? |

### Provision options

| Provision options                                          | MQTT<br/>(SSL/TLS) | HTTP<br/>(SSL/TLS) | CoAP<br/>(DTLS)" |
|------------------------------------------------------------|--------------------|--------------------|------------------|
| <br/>"Credentials generated by <br/>the ThingsBoard server | v                  | v                  | v                |
| Devices supplies<br/>Access Token                          | v                  | v                  | v                |
| Devices supplies<br/>Basic MQTT Credentials                | v                  |                    |                  |
| Devices supplies<br/>X.509 Certificate                     | v                  |                    | ?                |

### Provision params

| Provisioning request        | Parameter                 | Description                                                                    | Credentials generated by <br/>the ThingsBoard server | Devices supplies<br/>Access Token | Devices supplies<br/>Basic MQTT Credentials | Devices supplies<br/>X.509 Certificate                           |
|-----------------------------|---------------------------|--------------------------------------------------------------------------------|------------------------------------------------------|-----------------------------------|---------------------------------------------|------------------------------------------------------------------|
|                             | deviceName                | Device name in ThingsBoard.                                                    | (O) DEVICE_NAME                                      | (O) DEVICE_NAME                   | (O) DEVICE_NAME                             | (O) DEVICE_NAME                                                  |
|                             | provisionDeviceKey        | Provisioning device key, you should take it from configured device profile.    | (M) PUT_PROVISION_KEY_HERE                           | (M) PUT_PROVISION_KEY_HERE        | (M) PUT_PROVISION_KEY_HERE                  | (M) PUT_PROVISION_KEY_HERE                                       |
|                             | provisionDeviceSecret     | Provisioning device secret, you should take it from configured device profile. | (M) PUT_PROVISION_SECRET_HERE                        | (M) PUT_PROVISION_SECRET_HERE     | (M) PUT_PROVISION_SECRET_HERE               | (M) PUT_PROVISION_SECRET_HERE                                    |
|                             | credentialsType           | Credentials type parameter.                                                    |                                                      | (M) ACCESS_TOKEN                  | (M) MQTT_BASIC                              | (M) X509_CERTIFICATE                                             |
|                             | token                     | Access token for device in ThingsBoard.                                        |                                                      | (M) DEVICE_ACCESS_TOKEN           |                                             |                                                                  |
|                             | clientId                  | Client id for device in ThingsBoard.                                           |                                                      |                                   | (M) DEVICE_CLIENT_ID_HERE                   |                                                                  |
|                             | username                  | Username for device in ThingsBoard.                                            |                                                      |                                   | (M) DEVICE_USERNAME_HERE                    |                                                                  |
|                             | password                  | Password for device in ThingsBoard.                                            |                                                      |                                   | (M) DEVICE_PASSWORD_HERE                    |                                                                  |
|                             | hash                      | Public key X509 hash for device in ThingsBoard.                                |                                                      |                                   |                                             | (M) MIIB……..AQAB                                                 |
|                             | (O) Optional, (M) Must    |                                                                                |                                                      |                                   |                                             |                                                                  |
|                             |                           |                                                                                |                                                      |                                   |                                             |                                                                  |
| Provisioning <br/>response  | deviceId                  |                                                                                |                                                      |                                   |                                             | 3b829220-232f-11eb-9d5c-e9ed3235dff8                             |
|                             | credentialsType           |                                                                                | ACCESS_TOKEN                                         | ACCESS_TOKEN                      | MQTT_BASIC                                  | X509_CERTIFICATE                                                 |
|                             | credentialsId             |                                                                                |                                                      |                                   |                                             | f307a1f717a12b32c27203cf77728d305d29f64694a8311be921070dd1259b3a |
|                             | credentialsValue          |                                                                                | sLzc0gDAZPkGMzFVTyUY                                 | DEVICE_ACCESS_TOKEN               |                                             | MIIB........AQAB                                                 |
|                             | credentialsValue.clientId |                                                                                |                                                      |                                   | DEVICE_CLIENT_ID_HERE                       |                                                                  |
|                             | credentialsValue.userName |                                                                                |                                                      |                                   | DEVICE_USERNAME_HERE                        |                                                                  |
|                             | credentialsValue.password |                                                                                |                                                      |                                   | DEVICE_PASSWORD_HERE                        |                                                                  |
|                             | provisionDeviceStatus     |                                                                                |                                                      |                                   |                                             | SUCCESS                                                          |
|                             | status                    |                                                                                | SUCCESS                                              | SUCCESS                           | SUCCESS                                     |                                                                  |
|                             |                           |                                                                                |                                                      |                                   |                                             |                                                                  |
| one-way SSL                 |                           |                                                                                |                                                      |                                   |                                             | ca_certs="mqttserver.pub.pem"                                    |
| two-way SSL                 |                           |                                                                                |                                                      |                                   |                                             | certfile="mqtt_thingsboard_server_cert.pem"                                              |
| two-way SSL                 |                           |                                                                                |                                                      |                                   |                                             | keyfile="key.pem"                                                |

### Python command params of authentication

| Device authentication options          |                                                          | token                | clientId            | username                  | password                  |  | ca_certs="mqttserver.pub.pem" | certfile="mqtt_thingsboard_server_cert.pem" | keyfile="key.pem" |  | Default Port |
|----------------------------------------|----------------------------------------------------------|----------------------|---------------------|---------------------------|---------------------------|--|-------------------------------|---------------------|-------------------|--|--------------|
| Access Token                           | Plain MQTT (without SSL)                                 | -u YOUR_ACCESS_TOKEN |                     |                           |                           |  |                               |                     |                   |  | -p "1883"    |
|                                        | MQTTS (MQTT over SSL)                                    | -u YOUR_ACCESS_TOKEN |                     |                           |                           |  | --cafile tb-server-chain.pem  |                     |                   |  | -p "8883"    |
| Basic MQTT authentication              | Authentication based on Client ID only                   |                      | -i "YOUR_CLIENT_ID" |                           |                           |  |                               |                     |                   |  | -p "1883"    |
|                                        | Authentication based on Username and Password            |                      |                     | -u "YOUR_CLIENT_USERNAME" | -P "YOUR_CLIENT_PASSWORD" |  |                               |                     |                   |  | -p "1883"    |
|                                        | Authentication based on Client ID, Username and Password |                      | -i "YOUR_CLIENT_ID" | -u "YOUR_CLIENT_USERNAME" | -P "YOUR_CLIENT_PASSWORD" |  |                               |                     |                   |  | -p "1883"    |
|                                        | MQTTS (MQTT over TLS)                                    |                      | -i "YOUR_CLIENT_ID" | -u "YOUR_CLIENT_USERNAME" | -P "YOUR_CLIENT_PASSWORD" |  | --cafile tb-server-chain.pem  |                     |                   |  | -p "8883"    |
| X.509 Certificate Based Authentication | (two-way SSL)                                            |                      |                     |                           |                           |  | --cafile tb-server-chain.pem  | --cert mqtt_thingsboard_server_cert.pem     | --key key.pem     |  | -p "8883"    |
|                                        |                                                          |                      |                     |                           |                           |  |                               |                     |                   |  |              |
|                                        | -i: client ID                                            |                      |                     |                           |                           |  |                               |                     |                   |  |              |
|                                        | -u: user name                                            |                      |                     |                           |                           |  |                               |                     |                   |  |              |
|                                        | -P: password                                             |

### Firmware/Software OTA updates

* On connected:
   1. Subscribe to `v1/devices/me/attributes/response/+`
   1. Subscribe to `v1/devices/me/attributes`
   1. Subscribe to `v2/fw/response/+`
   1. Send telemetry: *current firmware info*
      * Topic: `v1/devices/me/telemetry`
      * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0"}`

      Replace `Initial` and `v0` with your F/W title and version.

   1. Send attributes request: *request firmware info*
      * Topic: `v1/devices/me/attributes/request/{request_id}`
      * Payload: `{"sharedKeys": "fw_checksum,fw_checksum_algorithm,fw_size,fw_title,fw_version"}`

* If receiving *fw_title* & *fw_version* in `v1/devices/me/attributes` or `v1/devices/me/attributes/response/{request_id}`, and they are not the same as *current_fw_title* & *current_fw_version*, then
   1. chunk_id = 0;
   2. Send telemetry: *current firmware info*
      * Topic: `v1/devices/me/telemetry`
      * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0","fw_state":"DOWNLOADING"}`

      Replace `Initial` and `v0` with your F/W title and version.

   3. Send f/w request: *getting firmware*
      * Topic: `v2/fw/request/{request_id}/chunk/{chunk_id}`
      * Payload: `chunk_size`

      Replace `{chunk_id}` with your chunk_id -- `0`

      Replace `chunk_size` with your chunk size, eg: `16384`

* If receiving `v2/fw/response/{request_id}/chunk/{chunk_id}`, then
   1. Saves *msg.payload*.
   2. chunk_id++;
   3. If *accumulated received firmware data length* is less than `fw_size`, then
      1. Send f/w request: *getting firmware*
         * Topic: `v2/fw/request/{request_id}/chunk/{chunk_id}`
         * Payload: `chunk_size`

         Replace `{chunk_id}` with your chunk id;

         Replace `chunk_size` with your chunk size, eg: `16384`

   4. Else *processing firmware*:
      1. Send telemetry: *current firmware info*
         * Topic: `v1/devices/me/telemetry`
         * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0","fw_state":"DOWNLOADED"}`
      2. *Verify checksum*
      3. If verification failure, then
         1. Send telemetry: *current firmware info -- failed*
            * Topic: `v1/devices/me/telemetry`
            * Payload: `{"fw_state":"FAILED","fw_error":"Checksum verification failed!"}`

            You may replace `Checksum verification failed!` with your text.

         2. End.
      4. Else
         1. Send telemetry: *current firmware info*
            * Topic: `v1/devices/me/telemetry`
            * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0","fw_state":"VERIFIED"}`
         2. Do upgrade...
         3. Send telemetry: *current firmware info*
            * Topic: `v1/devices/me/telemetry`
            * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0","fw_state":"UPDATING"}`
         4. If upgrade failue, then
            1. Send telemetry: *current firmware info -- failed*
               * Topic: `v1/devices/me/telemetry`
               * Payload: `{"fw_state":"FAILED","fw_error":"Update failed!"}`

               You may replace `Update failed!` with your text.

            2. End.
         5. Else
            1. send telemetry: *current firmware info*
               * Topic: `v1/devices/me/telemetry`
               * Payload:`{"current_fw_title":"new_fw_title","current_fw_version":"new_fw_version","fw_state":"UPDATED"}`

               Replace `new_fw_title` and `new_fw_version` with your new F/W title and version.

            2. End.

## ESP-MQTT API & Task model

| esp-mqtt API                      | User context<br/>(User task) | MQTT event handler<br/>(Internal MQTT task) | Memo                                                                                                                                                                                                                                   |
|-----------------------------------|------------------------------|---------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| esp_mqtt_client_init()            | √                            |                                             |                                                                                                                                                                                                                                        |
| esp_mqtt_client_set_uri()         | √                            | *                                           |                                                                                                                                                                                                                                        |
| esp_mqtt_client_start()           | √                            |                                             |                                                                                                                                                                                                                                        |
| esp_mqtt_client_reconnect()       | √                            |                                             |                                                                                                                                                                                                                                        |
| esp_mqtt_client_disconnect()      | √                            |                                             |                                                                                                                                                                                                                                        |
| esp_mqtt_client_stop()            | √                            | X                                           |                                                                                                                                                                                                                                        |
| esp_mqtt_client_subscribe()       | √                            | √                                           | Client must be connected to send subscribe message                                                                                                                                                                                     |
| esp_mqtt_client_unsubscribe()     | √                            | √                                           | Client must be connected to send unsubscribe message                                                                                                                                                                                   |
| esp_mqtt_client_publish()         | √                            | √                                           | This API might block for several seconds, either due to network timeout (10s) or if publishing payloads longer than internal buffer (due to message fragmentation)                                                                     |
| esp_mqtt_client_enqueue()         | √                            | √                                           | This API generates and stores the publish message into the internal outbox and the actual sending to the network is performed in the mqtt-task context. Thus, it could be used as a non blocking version of esp_mqtt_client_publish(). |
| esp_mqtt_client_destroy()         | √                            | X                                           |                                                                                                                                                                                                                                        |
| esp_mqtt_set_config()             | √                            | *                                           |                                                                                                                                                                                                                                        |
| esp_mqtt_client_register_event()  | √                            |                                             |                                                                                                                                                                                                                                        |
| esp_mqtt_client_get_outbox_size() | √                            | *                                           |                                                                                                                                                                                                                                        |
|                                   |                              |                                             | √:MUST, *:SHOULD, X:MUST NOT                                                                                                                                                                                                           |

## TBCMH component

This library for ESP32 to connect to ThingsBoard IoT platform over MQTT protocol, thin wrapper on ESP-MQTT component, which can be used under ESP-IDF and ESP-ADF.

Current version is based on ESP-IDF-v4.4.x, and is compatible with ThingsBoard IoT platform starting from version 3.4.x.

### Supported ThingsBoard MQTT API Features

* [Telemetry data upload](https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api)
* [Device attribute](https://thingsboard.io/docs/reference/mqtt-api/#attributes-api)
  * [Publish client-side device attribute update to the server](https://thingsboard.io/docs/reference/mqtt-api/#publish-attribute-update-to-the-server)
  * [Request client-side and shared device attribute values from the server](https://thingsboard.io/docs/reference/mqtt-api/#request-attribute-values-from-the-server)
  * [Subscribe to shared device ttribute updates from the server](https://thingsboard.io/docs/reference/mqtt-api/#subscribe-to-attribute-updates-from-the-server)
* [RPC commands](https://thingsboard.io/docs/reference/mqtt-api/#rpc-api)
  * [Server-side RPC](https://thingsboard.io/docs/reference/mqtt-api/#server-side-rpc)
  * [Client-side RPC](https://thingsboard.io/docs/reference/mqtt-api/#client-side-rpc)
* [Claiming devices](https://thingsboard.io/docs/reference/mqtt-api/#claiming-devices)
* [Device provisioning](https://thingsboard.io/docs/reference/mqtt-api/#device-provisioning)
* [Firmware OTA update](https://thingsboard.io/docs/reference/mqtt-api/#firmware-api)

* [Device authentication options](https://thingsboard.io/docs/user-guide/device-credentials/)
  * [Access Token based authentication](https://thingsboard.io/docs/user-guide/access-token/)
    * [Plain MQTT (without SSL)](https://thingsboard.io/docs/user-guide/access-token/#plain-mqtt-without-ssl)
    * [MQTTS (MQTT over SSL)](https://thingsboard.io/docs/user-guide/access-token/#mqtts-mqtt-over-ssl)
  * [Basic MQTT authentication](https://thingsboard.io/docs/user-guide/basic-mqtt/)
    * [Authentication based on Client ID only.](https://thingsboard.io/docs/user-guide/basic-mqtt/#authentication-based-on-client-id-only)
    * [Authentication based on Username and Password.](https://thingsboard.io/docs/user-guide/basic-mqtt/#authentication-based-on-username-and-password)
    * [Authentication based on Client ID, Username and Password.](https://thingsboard.io/docs/user-guide/basic-mqtt/#authentication-based-on-client-id-username-and-password)
    * [MQTTS (MQTT over TLS)](https://thingsboard.io/docs/user-guide/basic-mqtt/#mqtts-mqtt-over-tls)
  * [X.509 Certificate Based Authentication](https://thingsboard.io/docs/user-guide/certificates/)

### TBCMH overview

| SN | Function                                       | on_create()        | after_create()                              | on_connected()  | working…                                                                                    | on_disconnected()              | on_destroy()                          |  | Mode                       | Mode              |
|----|------------------------------------------------|--------------------|---------------------------------------------|-----------------|---------------------------------------------------------------------------------------------|--------------------------------|---------------------------------------|--|----------------------------|-------------------|
| 1  | Telemetry                                      | list_create()      | item_register()<br/>item_unregister()       |                 | item_update()                                                                               |                                | item_empty()<br/>list_destroy()       |  | only send                  |                   |
| 2  | Attributes request                             | list_create()      |                                             | sub_resp()      | send_req()?<br/>recv_resp(client_attr & shared attr)<br/>timeout_resp()                     | empty_req(timeout_resp())      | list_destroy()                        |  | req-resp                   | depend connection |
| 3  | Client attribute                               | list_create()      | item_register()<br/>item_unregister()       |                 | send_update()                                                                               |                                | item_empty()<br/>list_destroy()       |  | only send<br/>or init+send |                   |
| 4  | Shared attribute                               | list_create()      | item_register()<br/>item_unregister()       | sub_update()    | recv_update()                                                                               |                                | item_empty()<br/>list_destroy()       |  | only recv                  |                   |
| 5  | Server RPC                                     | list_create()      | item_register()<br/>item_unregister()       | sub_req()       | recv_req()<br/>send_resp()                                                                  |                                | items_empty()<br/>list_destroy()      |  | recv+send                  |                   |
| 6  | Client RPC                                     | list_create()      |                                             | sub_resp()      | send_req()<br/>recv_resp()<br/>timeout_resp()                                               | empty_req(timeout_resp())      | list_destroy()                        |  | req-resp                   | depend connection |
| 7  | OTA update<br/>(Merge ota & chunk list to ONE) | list_create(ota)   | item_register(ota)<br/>item_unregister(ota) |                 | send_shared_attr_req(ota)?<br/>recv_shared_attr_resp(ota)<br/>timeout_shared_attr_resp(ota) |                                | item_empty(ota)<br/>list_destroy(ota) |  |                            |                   |
|    |                                                | list_create(chunk) |                                             | sub_resp(chunk) | send_req(chunk)<br/>recv_resp(chunk)<br/>timeout_resp(chunk)                                | empty_req(timeout_resp(chunk)) | list_destroy(chunk)                   |  | req-resp                   | depend connection |
| 8  | Device provision                               | list_create()      |                                             | sub_resp()      | send_req()<br/>recv_resp()<br/>timeout_resp()                                               | empty_req(timeout_resp())      | list_destroy()                        |  | req-resp                   | depend connection |
| 9  | Claiming device                                |                    |                                             |                 | send_claiming()                                                                             |                                |                                       |  | only send                  |

### TBCMH Subscribe & Unsubscribe

| SN | Features             | Subscribe              | Unsubscribe                        |
|----|----------------------|------------------------|------------------------------------|
| 1  | timeseries_data      | —————                  | —————                              |
| 2  | attributes_request   | before sending request | after receiving respong or timeout |
| 3  | attributes_update    | —————                  | —————                              |
| 4  | attributes_subscribe | on connected           | on disconnected                    |
| 5  | client_rpc           | before sending request | after receiving respong or timeout |
| 6  | server_rpc           | on connected           | on disconnected                    |
| 7  | claiming_device      | —————                  | —————                              |
| 8  | ota_update           | before sending request | after receiving respong or timeout |
|    |                      |                        |                                    |
| 9  | device_provision     | before sending request | after receiving respong or timeout |

