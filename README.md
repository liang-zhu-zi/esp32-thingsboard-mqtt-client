
[![License](https://img.shields.io/badge/License-Apache%202.0-yellow.svg)](https://opensource.org/licenses/Apache-2.0)
[![ESP32](https://img.shields.io/badge/ESP-32-green.svg?style=flat-square)](https://www.espressif.com/en/products/socs/esp32)
[![GitHub stars](https://img.shields.io/github/stars/liang-zhu-zi/esp32-thingsboard-mqtt-client?style=flat&logo=github)](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/stargazers)
[![GitHub release](https://img.shields.io/github/release/liang-zhu-zi/esp32-thingsboard-mqtt-client/all.svg?style=flat-square)](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/releases/)

# ESP32 ThingsBoard MQTT Client library

* [中文](README_CN.md)

This library for ESP32 to connect to ThingsBoard IoT platform over MQTT protocol, thin wrapper on ESP-MQTT component, which can be used under ESP-IDF and ESP-ADF.

Current version is based on ESP-IDF-v4.4.1, and is compatible with ThingsBoard IoT platform starting from version 3.4.0.

## Supported ThingsBoard MQTT API Features

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

Example implementations for all features can be found in the examples folder.

## How to use

[ESP32 ThingsBoard MQTT Client library](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client) is a [ESP-IDF](https://github.com/espressif/esp-idf) component. Please refer to [ESP-IDF](https://github.com/espressif/esp-idf) for more usage instructions.

* Git or download code of this library;
* Modify your project's `CMakeLists.txt`, insert this line `set(EXTRA_COMPONENT_DIRS ..../components/tbcmh)`， replace `....` with your library path, eg:

    ```CMake
    cmake_minimum_required(VERSION 3.5)
    
    set(EXTRA_COMPONENT_DIRS C:/esp32-thingsboard-mqtt-client/components/tbcmh)
    
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(hello_world)
    ```

* And/or modify your project's `Makefile`, insert this line `EXTRA_COMPONENT_DIRS := ..../components/tbcmh`, replace `....` with your library path, eg:

    ```Makefile
    PROJECT_NAME := hello_world
    
    EXTRA_COMPONENT_DIRS := C:/esp32-thingsboard-mqtt-client/components/tbcmh
    
    include $(IDF_PATH)/make/project.mk
    ```

* Now, you can call API in
  * [tbc_mqtt_helper.h](./components/tbcmh/include/tbc_mqtt_helper.h)
  * [tbc_extension.h](./components/tbcmh/include/tbc_extension.h)
    * [tbc_extension_timeseriesdata.h](./components/tbcmh/include/tbc_extension_timeseriesdata.h)
    * [tbc_extension_clientattributes.h](./components/tbcmh/include/tbc_extension_clientattributes.h)
    * [tbc_extension_sharedattributes.h](./components/tbcmh/include/tbc_extension_sharedattributes.h)

* A simplified version example

  * Modify your entry function:

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

  * Modify the main function of the MQTT application:

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

  * Add two callback functions:

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

  * Modify `EXTRA_COMPONENT_DIRS` in `Makefile` and `CMakeLists.txt` of your project to using this library。

  * Run the program, you will receive a log similar to:

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

## Comeponent

For software design documents related to this library. See [here](./components/tbcmh).

## Examples

This library comes with a number of example. See [here](./examples).

## Documentation

* [Here](README.docs.md)

## Have a question or proposal?

You are welcomed in our [issues](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues).

## Liense

This code is released under the Apache-2.0 License.
