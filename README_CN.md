
[![License](https://img.shields.io/badge/License-Apache%202.0-yellow.svg)](https://opensource.org/licenses/Apache-2.0)
[![ESP32](https://img.shields.io/badge/ESP-32-green.svg?style=flat-square)](https://www.espressif.com/en/products/socs/esp32)
[![GitHub stars](https://img.shields.io/github/stars/liang-zhu-zi/esp32-thingsboard-mqtt-client?style=flat&logo=github)](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/stargazers)
[![GitHub release](https://img.shields.io/github/release/liang-zhu-zi/esp32-thingsboard-mqtt-client/all.svg?style=flat-square)](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/releases/)

# ESP32 ThingsBoard MQTT Client library

[English](README.md)

一个使用 MQTT 协议连接到 ThingsBoard 物联网平台的 ESP32 库。它是对 ESP-MQTT 组件的简单包裹，可以在 ESP-IDF 和 ESP-ADF 下使用。

当前客户端版本基于 ESP-IDF-v4.4.1，兼容 ThingsBoard 3.4.0 及更新版本。

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

示例目路下提供所有特性的例子。

## 如何使用

[ESP32 ThingsBoard MQTT Client library](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client) 是一个 [ESP-IDF](https://github.com/espressif/esp-idf) 组件.

* 用 Git 克隆本库的代码，或其接下载 Zip 压缩包划解压;
* 修改你的项目的 `CMakeLists.txt` 文件, 插入一行 `set(EXTRA_COMPONENT_DIRS ..../components/tbcm)`， 请用你本机上的本库路径替换 `....`。最后的结果类似:

    ```CMake
    cmake_minimum_required(VERSION 3.5)
    
    set(EXTRA_COMPONENT_DIRS ../../../components/tbcm)
    
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(hello_world)
    ```

* 和/或修改你的项目的 `Makefile` 文件, 插入一行 `EXTRA_COMPONENT_DIRS := ..../components/tbcm`， 请用你本机上的本库路径替换 `....`。最后的结果类似:

    ```Makefile
    PROJECT_NAME := hello_world
    
    EXTRA_COMPONENT_DIRS := ../../../components/tbcm
    
    include $(IDF_PATH)/make/project.mk
    ```

* 现在，你可以使用以下文件内的API了：
  * [tbc_mqtt_helper.h](./components/tbcm/include/tbc_mqtt_helper.h)
  * [tbc_extension.h](./components/tbcm/include/tbc_extension.h)
    * [tbc_extension_timeseriesdata.h](./components/tbcm/include/tbc_extension_timeseriesdata.h)
    * [tbc_extension_clientattributes.h](./components/tbcm/include/tbc_extension_clientattributes.h)
    * [tbc_extension_sharedattributes.h](./components/tbcm/include/tbc_extension_sharedattributes.h)

## 组件

本库的软件设计文档，详见 [这里](./components/tbcm)。

## 示例

本为库提供了很多示例，参见 [这里](./examples)。

## 文档

*即将提供...*

## 问题或建议

欢迎提问题或建议 [issues](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues)。

## 许可

此代码在 Apache-2.0 许可证下发布。
