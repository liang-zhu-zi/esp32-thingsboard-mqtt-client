
[![License](https://img.shields.io/badge/License-Apache%202.0-yellow.svg)](https://opensource.org/licenses/Apache-2.0)
[![ESP32](https://img.shields.io/badge/ESP-32-green.svg?style=flat-square)](https://www.espressif.com/en/products/socs/esp32)
[![GitHub stars](https://img.shields.io/github/stars/liang-zhu-zi/esp32-thingsboard-mqtt-client?style=flat&logo=github)](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/stargazers)
[![GitHub release](https://img.shields.io/github/release/liang-zhu-zi/esp32-thingsboard-mqtt-client/all.svg?style=flat-square)](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/releases/)

# ESP32 ThingsBoard MQTT Client library

[中文](README_CN.md)

This library for ESP32 to connect to ThingsBoard IoT platform over MQTT protocol，thin wrapper on ESP-MQTT component, which can be used under ESP-IDF and ESP-ADF.

Current version is based on ESP-IDF-v4.4.1, and is compatible with ThingsBoard IoT platform starting from version 3.4.0.

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

Example implementations for all features can be found in the examples folder.

## How to use

[ESP32 ThingsBoard MQTT Client library](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client) is a [ESP-IDF](https://github.com/espressif/esp-idf) component.

* Git or download code of this library;
* Modify your project's `CMakeLists.txt`, insert this line `set(EXTRA_COMPONENT_DIRS ..../components/tbcm)`， replace `....` with your library path,, eg:

    ```CMake
    cmake_minimum_required(VERSION 3.5)
    
    set(EXTRA_COMPONENT_DIRS ../../../components/tbcm)
    
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(hello_world)
    ```

* And/or modify your project's `Makefile`, insert this line `EXTRA_COMPONENT_DIRS := ..../components/tbcm`, replace `....` with your library path, eg:

    ```Makefile
    PROJECT_NAME := hello_world
    
    EXTRA_COMPONENT_DIRS := ../../../components/tbcm
    
    include $(IDF_PATH)/make/project.mk
    ```

* Now, you can call API in
  * [tbc_mqtt_helper.h](./components/tbcm/include/tbc_mqtt_helper.h)
  * [tbc_extension.h](./components/tbcm/include/tbc_extension.h)
    * [tbc_extension_timeseriesdata.h](./components/tbcm/include/tbc_extension_timeseriesdata.h)
    * [tbc_extension_clientattributes.h](./components/tbcm/include/tbc_extension_clientattributes.h)
    * [tbc_extension_sharedattributes.h](./components/tbcm/include/tbc_extension_sharedattributes.h)

## Comeponent

For software design documents related to this library, see [here](./components/tbcm).

## Examples

This library comes with a number of example. See [here](./examples).

## Documentation

*Coming soon...*

## Have a question or proposal?

You are welcomed in our [issues](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues).

## Liense

This code is released under the Apache-2.0 License.
