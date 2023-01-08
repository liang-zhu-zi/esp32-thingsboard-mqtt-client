| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Device provisioning - Devices supplies X.509 Certificate - MQTT over SSL (with SSL)

* [English Version](./README.md)

- [Device provisioning - Devices supplies Basic MQTT Credentials- Plain MQTT (without SSL)](#device-provisioning---devices-supplies-basic-mqtt-credentials--plain-mqtt-without-ssl)
  - [Introduction](#introduction)
  - [Hardware Required](#hardware-required)
  - [Use Case 1 - Allowing creating new devices with **device name**](#use-case-1---allowing-creating-new-devices-with-device-name)
    - [How to Use Example](#how-to-use-example)
    - [Example Output](#example-output)
    - [ThingsBoard CE/PE Data](#thingsboard-cepe-data)
  - [Use Case 2 - Checking pre-provisioned devices with **device name**](#use-case-2---checking-pre-provisioned-devices-with-device-name)
    - [How to Use Example](#how-to-use-example-1)
    - [Example Output](#example-output-1)
    - [ThingsBoard CE/PE Data](#thingsboard-cepe-data-1)
  - [Troubleshooting](#troubleshooting)

## Introduction

本示例实现了以下功能：

* Device provisioning - Devices supplies X.509 Certificate - MQTT over SSL (with SSL)
  * Use Case 1 - Allowing creating new devices with **device name**. [here](#use-case-1---allowing-creating-new-devices-with-device-name)
  * Use Case 2 - Checking pre-provisioned devices with **device name**. [here](#use-case-2---checking-pre-provisioned-devices-with-device-name)
* Publish telemetry: temprature, humidity
  * Publish: `{"temprature":25,"humidity":26}`

参考 [这里](https://thingsboard.io/docs/user-guide/device-provisioning/), [这里](https://thingsboard.io/docs/user-guide/certificates/).

## 硬件需求

* 一个载有 ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC 的开发板(例如, ESP32-DevKitC, ESP-WROVER-KIT 等等)
* 一条用于供电与编程的 USB 线

参考 [Development Boards](https://www.espressif.com/en/products/devkits) 获得更多信息.

## Use Case 1 - Allowing creating new devices with **device name**

### How to Use Example

1. ThingsBoard CE/PE SSL configuration using PEM certificates file

   See [MQTT over SSL - using Self-signed PEM certificates file](../../.docs/mqtt-over-ssl-ssl-configuration-using-pem-certificates-file.md)

1. Generate Client certificate

   See [here](../../.docs/generate-client-certificate.md).

1. **ThingsBoard CE/PE**: add or modify a device profile for device provisioning - Allow to create new devices.

   See [here](../../.docs/add-or-modify-device-profile-for-device-provisioning-that-allow-to-create-new-devices.md)

1. set-targe (optional)

   Before project configuration and build, be sure to set the correct chip target using:

   ```bash
   idf.py set-target <chip_name>
   ```

1. menuconfig

   Then project configuration:

   ```bash
   idf.py menuconfig
   ```

   Configuration: ThingsBoard MQTT URI, access token, Wi-Fi SSID, password:

   ```menuconfig
   Example ThingsBoard MQTT Configuration  ---> 
      Transport server address  --->
         (MyThingsboardServerIP) Hostname, to set ipv4 pass it as string
         (8883) Port
      Provisioning config  --->
         (MY_DEVICE_NAME) Device name (Optional)
         (MY_PROVISION_KEY) Device key
         (MY_PROVISION_SECRET) Device secret
      Transport verification / Server certificate  --->
         [*] Skip any validation of server certificate CN field
   Example Connection Configuration  --->
       [*] connect using WiFi interface
       (MySSID) WiFi SSID 
       (MyPassword) WiFi Password                  
   ```

1. build, flash and monitor

   Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

   (To exit the serial monitor, type ``Ctrl-]``.)

   See the [Getting Started Guide](https://idf.espressif.com/) for full steps to configure and use ESP-IDF to build projects.

### Example Output

```none
...
```

### ThingsBoard CE/PE Data

1. Device provisiong results.

   Refer [here](../../.docs/device-provisioning-results-that-allow-to-create-new-devices.md).

1. **Delete new provisioned device**. 

   In order to ensure that the example runs successfully next time, the newly added device needs to be deleted.

   See [here](../../.docs/delete-provisioned-device.md).


## Use Case 2 - Checking pre-provisioned devices with **device name**

### How to Use Example

1. ThingsBoard CE/PE SSL configuration using PEM certificates file

   See [MQTT over SSL - using Self-signed PEM certificates file](../../.docs/mqtt-over-ssl-ssl-configuration-using-pem-certificates-file.md)

1. Generate Client certificate

   See [here](../../.docs/generate-client-certificate.md).

1. **ThingsBoard CE/PE**: add or modify a device profile for device provisioning - checking pre-provisioned devices. 

   See [here](../../.docs/add-or-modify-device-profile-for-device-provisioning-using-pre-provisioned-devices.md)

1. **ThingsBoard CE/PE**: pre-provisioning device with basic MQTT credentials -  Client ID, Username and Password

   See [here](../../.docs/pre-provisioning-device-with-x509-ceritificate.md)

1. **ThingsBoard CE/PE**: device provisioning results using pre-provisioned devices. 

   See [here](../../.docs/pre-provisioning-device-status.md)

1. set-targe (optional)

   Before project configuration and build, be sure to set the correct chip target using:

   ```bash
   idf.py set-target <chip_name>
   ```

1. menuconfig

   Then project configuration:

   ```bash
   idf.py menuconfig
   ```

   Configuration: ThingsBoard MQTT URI, access token, Wi-Fi SSID, password:

   ```menuconfig
   Example ThingsBoard MQTT Configuration  ---> 
      Transport server address  --->
         (MyThingsboardServerIP) Hostname, to set ipv4 pass it as string
         (8883) Port
      Provisioning config  --->
         (MY_DEVICE_NAME) Device name (Optional)
         (MY_PROVISION_KEY) Device key
         (MY_PROVISION_SECRET) Device secret
      Transport verification / Server certificate  --->
         [*] Skip any validation of server certificate CN field
   Example Connection Configuration  --->
       [*] connect using WiFi interface
       (MySSID) WiFi SSID 
       (MyPassword) WiFi Password                  
   ```

1. build, flash and monitor

   Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

   (To exit the serial monitor, type ``Ctrl-]``.)

   See the [Getting Started Guide](https://idf.espressif.com/) for full steps to configure and use ESP-IDF to build projects.

### Example Output

```none
...
```

### ThingsBoard CE/PE Data

1. Device provisiong results.

   Refer [here](../../.docs/device-provisioning-results-using-pre-provisioned-devices.md).

1. **Delete new provisioned device**. 

   In order to ensure that the example runs successfully next time, the newly added device needs to be deleted.

   See [here](../../.docs/delete-provisioned-device.md).

## 故障排除

如有任何技术问题，请打开 [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues)。 我们会尽快回复您。
