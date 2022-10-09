# Core Components - ESP32 ThingsBoard MQTT Client library

[English](./README.md)

<https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client>

ESP32 ThingsBoard MQTT Client 适用于 C/C++ 开发人员。

它被封装成一个ESP-IDF组件，既可以在ESP-IDF和ESP-ADF下使用，也可以很简单移植到Arduino-ESP32。

此库提供了一些简单的 API 来使用 MQTT API 与 ThingsBoard 平台进行通信。

当前客户端版本基于 ESP-IDF-v4.4.1，兼容 ThingsBoard 3.4.0 及更新版本。

本组件模板基于 `esp-idf-v4.4.1\components\esp_ipc` and `esp-idf-v4.4.1\components\esp_lcd`。

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
