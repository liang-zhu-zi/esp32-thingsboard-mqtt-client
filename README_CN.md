# ESP32 ThingsBoard MQTT Client library

[English](README.md)

一个使用 MQTT 协议连接到 ThingsBoard 物联网平台的 ESP32 库。

对 ESP-MQTT 组件的简单包裹，可以在ESP-IDF和ESP-ADF下使用。

当前客户端版本基于 ESP-IDF-v4.4.1，兼容 ThingsBoard 3.4.0 及更新版本。

* [代码说明](./components/tbcm)
* [示例说明](./examples)

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
