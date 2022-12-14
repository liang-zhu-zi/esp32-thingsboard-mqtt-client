
[![License](https://img.shields.io/badge/License-Apache%202.0-yellow.svg)](https://opensource.org/licenses/Apache-2.0)
[![ESP32](https://img.shields.io/badge/ESP-32-green.svg?style=flat-square)](https://www.espressif.com/en/products/socs/esp32)
[![GitHub stars](https://img.shields.io/github/stars/liang-zhu-zi/esp32-thingsboard-mqtt-client?style=flat&logo=github)](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/stargazers)
[![GitHub release](https://img.shields.io/github/release/liang-zhu-zi/esp32-thingsboard-mqtt-client/all.svg?style=flat-square)](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/releases/)

# ESP32 ThingsBoard MQTT Client library

* [English Version](README.md)

一个使用 MQTT 协议连接到 ThingsBoard 物联网平台的 ESP32 库。它是对 ESP-MQTT 组件的简单包裹，可以在 ESP-IDF 和 ESP-ADF 下使用。

当前客户端版本基于 ESP-IDF-v4.4.x，兼容 ThingsBoard 3.4.x 及更新版本。

## 支持的 ThingsBoard MQTT API 特性

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

示例路径下提供所有特性的例子。

## 如何使用

[ESP32 ThingsBoard MQTT Client library](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client) 是一个 [ESP-IDF](https://github.com/espressif/esp-idf) 组件。请参考[ESP-IDF](https://github.com/espressif/esp-idf)获取更多使用说明。

* 用 Git 克隆本库的代码，或其接下载 Zip 压缩包划解压。
* 修改你的项目的 `CMakeLists.txt` 文件, 插入一行 `set(EXTRA_COMPONENT_DIRS ..../components/tbcmh)`， 请用你本机上的本库路径替换 `....`。最后的结果类似：

    ```CMake
    cmake_minimum_required(VERSION 3.5)
    
    set(EXTRA_COMPONENT_DIRS C:/esp32-thingsboard-mqtt-client/components/tbcmh)
    
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(hello_world)
    ```

* 和/或修改你的项目的 `Makefile` 文件，插入一行 `EXTRA_COMPONENT_DIRS := ..../components/tbcmh`，请用你本机上的本库路径替换 `....`。最后的结果类似：

    ```Makefile
    PROJECT_NAME := hello_world
    
    EXTRA_COMPONENT_DIRS := C:/esp32-thingsboard-mqtt-client/components/tbcmh
    
    include $(IDF_PATH)/make/project.mk
    ```

* 现在，你可以使用以下文件内的API了：
  * [tbc_mqtt_helper.h](./components/tbcmh/include/tbc_mqtt_helper.h)
  * [tbc_extension.h](./components/tbcmh/include/tbc_extension.h)
    * [tbc_extension_timeseriesdata.h](./components/tbcmh/include/tbc_extension_timeseriesdata.h)
    * [tbc_extension_clientattributes.h](./components/tbcmh/include/tbc_extension_clientattributes.h)
    * [tbc_extension_sharedattributes.h](./components/tbcmh/include/tbc_extension_sharedattributes.h)

* 一个简单的例子：

  * 修改你的入口函数：

    ```c
    void app_main(void)
    {
        ESP_LOGI(TAG, "[APP] Startup..");
        ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
        ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    
        esp_log_level_set("*", ESP_LOG_INFO); //ESP_LOG_DEBUG
    
        ESP_ERROR_CHECK(nvs_flash_init());
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
    
        /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
         * Read "Establishing Wi-Fi or Ethernet Connection" section in
         * examples/protocols/README.md for more information about this function.
         */
        ESP_ERROR_CHECK(example_connect());
    
        mqtt_app_start();
    }
    ```

  * 修改 MQTT 应用主函数：

    ```c
    void mqtt_app_start(void)
    {
        ESP_LOGI(TAG, "Init tbcmh ...");
        tbcmh_handle_t client = tbcmh_init();
        if (!client) {
            ESP_LOGE(TAG, "Failure to init tbcmh!");
            return;
        }
    
        ESP_LOGI(TAG, "Connect tbcmh ...");
        tbc_transport_config_esay_t config = {
            .uri = "mqtt://192.168.0.187",          // TODO: replace it with your ThingsBoard URI
            .access_token = "mKqOP8kQwxdDsVnCRU20", // TODO: replace it with your device's Access Token in ThingsBoard
            .log_rxtx_package = true                // Print Rx/Tx MQTT package
         };
        bool result = tbcmh_connect_using_url(client, &config,
                            NULL, tb_on_connected, tb_on_disconnected);
        if (!result) {
            ESP_LOGE(TAG, "failure to connect to tbcmh!");
            goto exit_destroy;
        }
    
        ESP_LOGI(TAG, "connect tbcmh ...");
        int i = 0;
        while (i<4) {
            if (tbcmh_has_events(client)) {
                tbcmh_run(client);
            }
    
            i++;
            if (tbcmh_is_connected(client)) {
                tbcmh_telemetry_upload(client, "{\"temprature\": 25.5}",
                        1/*qos*/, 0/*retain*/);
            } else {
                ESP_LOGI(TAG, "Still NOT connected to server!");
            }
            sleep(5);
        }
    
        ESP_LOGI(TAG, "Disconnect tbcmh ...");
        tbcmh_disconnect(client);
    
    exit_destroy:
        ESP_LOGI(TAG, "Destroy tbcmh ...");
        tbcmh_destroy(client);
    }
    ```

  * 添加两个回调函数：

    ```c
    /*!< Callback of connected ThingsBoard MQTT */
    void tb_on_connected(tbcmh_handle_t client, void *context)
    {
        ESP_LOGI(TAG, "Connected to thingsboard server!");
    }
    
    /*!< Callback of disconnected ThingsBoard MQTT */
    void tb_on_disconnected(tbcmh_handle_t client, void *context)
    {
        ESP_LOGI(TAG, "Disconnected from thingsboard server!");
    }
    ```

  * 修改你的项目 `Makefile` 和 `CMakeLists.txt` 文件中的`EXTRA_COMPONENT_DIRS`，使用本组件。

  * 运行程序，将会接收到类似的日志：

    ```log
    ...
    I (5607) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
    I (6107) esp_netif_handlers: example_connect: sta ip: 192.168.0.126, mask: 255.255.255.0, gw: 192.168.0.1
    I (6107) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.126
    I (6117) example_connect: Connected to example_connect: sta
    I (6117) example_connect: - IPv4 address: 192.168.0.126
    I (6127) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
    I (6137) TELEMETRY_UPLOAD_MAIN: Init tbcmh ...
    I (6147) TELEMETRY_UPLOAD_MAIN: Connect tbcmh ...
    I (6147) tb_mqtt_client_helper: connecting to mqtt://192.168.0.187:1883 ...
    I (6157) TELEMETRY_UPLOAD_MAIN: connect tbcmh ...
    I (6157) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=0
    I (6167) TELEMETRY_UPLOAD_MAIN: Still NOT connected to server!
    I (11177) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
    I (11177) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc5c24
    I (11177) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
    I (11177) tb_mqtt_client_helper: before call on_connected()...
    I (11187) TELEMETRY_UPLOAD_MAIN: Connected to thingsboard server!
    I (11197) tb_mqtt_client_helper: after call on_connected()
    I (11197) tb_mqtt_wapper: [Telemetry][Tx] {"temprature": 25.5}
    I (16217) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=4395
    I (16217) tb_mqtt_wapper: [Telemetry][Tx] {"temprature": 25.5}
    I (21217) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=3118
    I (21217) tb_mqtt_wapper: [Telemetry][Tx] {"temprature": 25.5}
    I (26217) TELEMETRY_UPLOAD_MAIN: Disconnect tbcmh ...
    I (26217) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.187:1883 ...
    I (26217) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=63413
    I (26327) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
    I (26337) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
    W (26337) MQTT_CLIENT: Client asked to stop, but was not started
    I (26437) TELEMETRY_UPLOAD_MAIN: Destroy tbcmh ...
    I (26437) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
    ```

## 组件

本库的软件设计文档，详见 [这里](./components/tbcmh)。

## 示例

本为库提供了很多示例，参见 [这里](./examples)。

## 文档

* [这里](README.docs.md)

## 问题或建议

欢迎提问题或建议 [issues](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues)。

## 许可

此代码在 [Apache-2.0](./LICENSE) 许可证下发布。
