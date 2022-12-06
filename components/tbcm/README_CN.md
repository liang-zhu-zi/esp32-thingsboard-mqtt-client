# Core Components - ESP32 ThingsBoard MQTT Client library

[English](./README.md)

<https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client>

一个使用 MQTT 协议连接到 ThingsBoard 物联网平台的 ESP32 库。

对 ESP-MQTT 组件的简单包裹，可以在ESP-IDF和ESP-ADF下使用。

当前客户端版本基于 ESP-IDF-v4.4.1，兼容 ThingsBoard 3.4.0 及更新版本。

本组件模板基于 `esp-idf-v4.4.1\components\esp_ipc` and `esp-idf-v4.4.1\components\esp_lcd`。

## Supported ThingsBoard Features

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

## 支持的 esp-mqtt API

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
