| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Device provisioning - Devices supplies Basic MQTT Credentials, Client ID, Username and Password - MQTT over SSL (with SSL)

* [中文版](./README_CN.md)


## Introduction

This example implements the following functions:

* Device provisioning - Devices supplies Basic MQTT Credentials, Client ID, Username and Password - MQTT over SSL (with SSL)
  * Use Case 1 - Allowing creating new devices with **device name**. [here](#use-case-1---allowing-creating-new-devices-with-device-name)
  * Use Case 2 - Checking pre-provisioned devices with **device name**. [here](#use-case-2---checking-pre-provisioned-devices-with-device-name)
* Publish telemetry: temprature, humidity
  * Publish: `{"temprature":25,"humidity":26}`

Refer [here](https://thingsboard.io/docs/user-guide/device-provisioning/).

## Hardware Required

* A development board with ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

## How to Use Example

1. ThingsBoard CE/PE SSL configuration using PEM certificates file

   See [MQTT over SSL - using Self-signed PEM certificates file](../../.docs/mqtt-over-ssl-ssl-configuration-using-pem-certificates-file.md)


1. Follow-up test steps [here](../pd23_dev_sup_basic_mqtt_credential_cup_wo_ssl/README.md).

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

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues) on GitHub. We will get back to you soon.
