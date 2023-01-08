| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Device provisioning - Devices supplies Access Token - MQTT over SSL (with SSL)

* [English Version](./README.md)

本示例实现了以下功能：

* Device provisioning - Devices supplies Access Token - MQTT over SSL (with SSL)
  * Use Case 1 - Allowing creating new devices with **device name**. [here](#use-case-1---allowing-creating-new-devices-with-device-name)
  * Use Case 2 - Checking pre-provisioned devices with **device name**. [here](#use-case-2---checking-pre-provisioned-devices-with-device-name)
* Publish telemetry: temprature, humidity
  * Publish: `{"temprature":25,"humidity":26}`

Refer [here](https://thingsboard.io/docs/user-guide/device-provisioning/).

## 硬件需求

* 一个载有 ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC 的开发板(例如, ESP32-DevKitC, ESP-WROVER-KIT 等等)
* 一条用于供电与编程的 USB 线

参考 [Development Boards](https://www.espressif.com/en/products/devkits) 获得更多信息.

## 如何使用例子

1. ThingsBoard CE/PE SSL configuration using PEM certificates file

   See [MQTT over SSL - using Self-signed PEM certificates file](../../.docs/mqtt-over-ssl-ssl-configuration-using-pem-certificates-file.md)


1. Follow-up test steps [here](../pd23_dev_sup_basic_mqtt_credential_cup_wo_ssl/README_CN.md).

   **Note:** Then project configuration:

   ```bash
   idf.py menuconfig
   ```

   ```menuconfig
   ...
   Example ThingsBoard MQTT Configuration  ---> 
      Transport server address  --->
         (8883) Port
   ...
   ```

## 故障排除

如有任何技术问题，请打开 [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues)。 我们会尽快回复您。
