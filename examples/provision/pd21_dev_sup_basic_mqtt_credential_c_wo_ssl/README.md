| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Device provisioning - Devices supplies Basic MQTT Credentials, Client ID - Plain MQTT (without SSL)

* [中文版](./README_CN.md)

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

This example implements the following functions:

* Device provisioning - Devices supplies Basic MQTT Credentials, Client ID - Plain MQTT (without SSL)
  * Use Case 1 - Allowing creating new devices with **device name**. [here](#use-case-1---allowing-creating-new-devices-with-device-name)
  * Use Case 2 - Checking pre-provisioned devices with **device name**. [here](#use-case-2---checking-pre-provisioned-devices-with-device-name)
* Publish telemetry: temprature, humidity
  * Publish: `{"temprature":25,"humidity":26}`

Refer [here](https://thingsboard.io/docs/user-guide/device-provisioning/).

## Hardware Required

* A development board with ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

## Use Case 1 - Allowing creating new devices with **device name**

### How to Use Example

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
         (1883) Port
      Provisioning config  --->
         (MY_DEVICE_NAME) Device name (Optional)
         (MY_PROVISION_KEY) Device key
         (MY_PROVISION_SECRET) Device secret
         (MY_DEVICE_CLIENT_ID) Client ID
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
I (454) cpu_start: Starting app cpu, entry point is 0x400811a8
0x400811a8: call_start_cpu1 at C:/Espressif/frameworks/esp-idf-v4.4.3/components/esp_system/port/cpu_start.c:148

I (0) cpu_start: App cpu up.
I (470) cpu_start: Pro cpu start user code
I (470) cpu_start: cpu freq: 160000000
I (470) cpu_start: Application information:
I (475) cpu_start: Project name:     dev_sup_basic_mqtt_credential_c
I (482) cpu_start: App version:      c556820-dirty
I (487) cpu_start: Compile time:     Jan  8 2023 15:32:07
I (493) cpu_start: ELF file SHA256:  3aa4d43070f3b5fa...
I (499) cpu_start: ESP-IDF:          v4.4.3-dirty
I (505) heap_init: Initializing. RAM available for dynamic allocation:
I (512) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (518) heap_init: At 3FFB76A0 len 00028960 (162 KiB): DRAM
I (524) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (530) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (537) heap_init: At 40094A04 len 0000B5FC (45 KiB): IRAM
I (544) spi_flash: detected chip: generic
I (548) spi_flash: flash io: dio
W (552) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (566) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (576) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: [APP] Startup..
I (586) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: [APP] Free memory: 275900 bytes
I (586) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: [APP] IDF version: v4.4.3-dirty
I (626) wifi:wifi driver task: 3ffc0028, prio:23, stack:6656, core=0
I (626) system_api: Base MAC address is not set
I (626) system_api: read default base MAC address from EFUSE
I (636) wifi:wifi firmware version: 8cb87ff
I (636) wifi:wifi certification version: v7.0
I (636) wifi:config NVS flash: enabled
I (636) wifi:config nano formating: disabled
I (646) wifi:Init data frame dynamic rx buffer num: 32
I (646) wifi:Init management frame dynamic rx buffer num: 32
I (656) wifi:Init management short buffer num: 32
I (656) wifi:Init dynamic tx buffer num: 32
I (666) wifi:Init static rx buffer size: 1600
I (666) wifi:Init static rx buffer num: 10
I (666) wifi:Init dynamic rx buffer num: 32
I (676) wifi_init: rx ba win: 6
I (676) wifi_init: tcpip mbox: 32
I (686) wifi_init: udp mbox: 6
I (686) wifi_init: tcp mbox: 6
I (686) wifi_init: tcp tx win: 5744
I (696) wifi_init: tcp rx win: 5744
I (696) wifi_init: tcp mss: 1440
I (706) wifi_init: WiFi IRAM OP enabled
I (706) wifi_init: WiFi RX IRAM OP enabled
I (716) example_connect: Connecting to Duoman...
I (716) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (826) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (826) wifi:enable tsf
I (826) example_connect: Waiting for IP(s)
I (3236) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3986) wifi:state: init -> auth (b0)
I (3986) wifi:state: auth -> assoc (0)
I (3986) wifi:state: assoc -> run (10)
W (3996) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (4016) wifi:connected with Duoman, aid = 4, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (4016) wifi:security: WPA2-PSK, phy: bgn, rssi: -30
I (4016) wifi:pm start, type: 1

I (4066) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5616) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6116) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (6116) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (6126) example_connect: Connected to example_connect: sta
I (6126) example_connect: - IPv4 address: 192.168.0.124
I (6136) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6146) FRONT-CONN: Init tbcmh ...
I (6146) FRONT-CONN: Connect tbcmh ...
I (6156) tb_mqtt_client_helper: connecting to mqtt://192.168.0.210:1883 ...
I (6166) tb_mqtt_wapper: src_event->event_id=7
I (6166) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (6176) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: Still NOT connected to server!
I (7186) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (7186) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc5d60
I (7186) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (7186) tb_mqtt_client_helper: before call on_connected()...
I (7196) FRONT-CONN: Connected to thingsboard server!
I (8206) provision: sent subscribe successful, msg_id=11578, topic=/provision/response
I (8206) tb_mqtt_wapper: [Provision][Tx] request_id=1 {"deviceName":"MY_DEVICE_NAME","provisionDeviceKey":"MY_PROVISION_KEY","provisionDeviceSecret":"MY_PROVISION_SECRET","credentialsType":"MQTT_BASIC","clientId":"MY_DEVICE_CLIENT_ID"}
I (8226) tb_mqtt_client_helper: after call on_connected()
I (8376) tb_mqtt_wapper: src_event->event_id=5
I (8386) tb_mqtt_wapper: [Provision][Rx] topic_type=6, payload_len=133 {"credentialsValue":{"clientId":"MY_DEVICE_CLIENT_ID","userName":"","password":""},"credentialsType":"MQTT_BASIC","status":"SUCCESS"}
I (9236) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=63547
I (9236) tb_mqtt_client_helper: TBCM_EVENT_DATA
I (9236) provision: sent unsubscribe successful, msg_id=58292, topic=/provision/response
I (9246) FRONT-CONN: Provision successful and the device will work!
I (9246) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.210:1883 ...
I (9356) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (9366) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (9366) MQTT_CLIENT: Client asked to stop, but was not started
I (9466) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
I (9466) NORMAL-CONN: Init tbcmh ...
I (9466) NORMAL-CONN: Connect tbcmh ...
I (9466) tb_mqtt_client_helper: connecting to mqtt://192.168.0.210:1883 ...
I (9476) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: Still NOT connected to server!
I (9476) tb_mqtt_wapper: src_event->event_id=7
I (10496) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (10496) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (10496) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc9f20
I (10496) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (10506) tb_mqtt_client_helper: before call on_connected()...
I (10516) NORMAL-CONN: Connected to thingsboard server!
I (10526) NORMAL-CONN: Send telemetry: temprature, humidity
I (10526) NORMAL-CONN: Get temperature (a time-series data)
I (10536) NORMAL-CONN: Get humidity (a time-series data)
I (10536) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":25,"humidity":26}
I (10556) tb_mqtt_client_helper: after call on_connected()
I (10566) tb_mqtt_wapper: src_event->event_id=5
I (11556) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=23927
I (50956) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: Disconnect tbcmh ...
I (50956) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.210:1883 ...
I (51056) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (51066) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (51066) MQTT_CLIENT: Client asked to stop, but was not started
I (51166) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: Destroy tbcmh ...
I (51166) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
```

### ThingsBoard CE/PE Data

1. Device provisiong results.

   Refer [here](../../.docs/device-provisioning-results-that-allow-to-create-new-devices.md).

1. **Delete new provisioned device**. 

   In order to ensure that the example runs successfully next time, the newly added device needs to be deleted.

   See [here](../../.docs/delete-provisioned-device.md).


## Use Case 2 - Checking pre-provisioned devices with **device name**

### How to Use Example

1. **ThingsBoard CE/PE**: add or modify a device profile for device provisioning - checking pre-provisioned devices. 

   See [here](../../.docs/add-or-modify-device-profile-for-device-provisioning-using-pre-provisioned-devices.md)

1. **ThingsBoard CE/PE**: pre-provisioning device with basic MQTT credentials -  Client ID

   See [here](../../.docs/pre-provisioning-device-with-basic-mqtt-c)

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
         (1883) Port
      Provisioning config  --->
         (MY_DEVICE_NAME) Device name (Optional)
         (MY_PROVISION_KEY) Device key
         (MY_PROVISION_SECRET) Device secret
         (MY_DEVICE_CLIENT_ID) Client ID
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
I (454) cpu_start: Starting app cpu, entry point is 0x400811a8
0x400811a8: call_start_cpu1 at C:/Espressif/frameworks/esp-idf-v4.4.3/components/esp_system/port/cpu_start.c:148

I (0) cpu_start: App cpu up.
I (470) cpu_start: Pro cpu start user code
I (470) cpu_start: cpu freq: 160000000
I (470) cpu_start: Application information:
I (475) cpu_start: Project name:     dev_sup_basic_mqtt_credential_c
I (482) cpu_start: App version:      c556820-dirty
I (487) cpu_start: Compile time:     Jan  8 2023 15:32:07
I (493) cpu_start: ELF file SHA256:  3aa4d43070f3b5fa...
I (499) cpu_start: ESP-IDF:          v4.4.3-dirty
I (505) heap_init: Initializing. RAM available for dynamic allocation:
I (512) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (518) heap_init: At 3FFB76A0 len 00028960 (162 KiB): DRAM
I (524) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (530) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (537) heap_init: At 40094A04 len 0000B5FC (45 KiB): IRAM
I (544) spi_flash: detected chip: generic
I (548) spi_flash: flash io: dio
W (552) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (566) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (576) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: [APP] Startup..
I (586) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: [APP] Free memory: 275900 bytes
I (586) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: [APP] IDF version: v4.4.3-dirty
I (626) wifi:wifi driver task: 3ffc0028, prio:23, stack:6656, core=0
I (626) system_api: Base MAC address is not set
I (626) system_api: read default base MAC address from EFUSE
I (636) wifi:wifi firmware version: 8cb87ff
I (636) wifi:wifi certification version: v7.0
I (636) wifi:config NVS flash: enabled
I (636) wifi:config nano formating: disabled
I (646) wifi:Init data frame dynamic rx buffer num: 32
I (646) wifi:Init management frame dynamic rx buffer num: 32
I (656) wifi:Init management short buffer num: 32
I (656) wifi:Init dynamic tx buffer num: 32
I (666) wifi:Init static rx buffer size: 1600
I (666) wifi:Init static rx buffer num: 10
I (666) wifi:Init dynamic rx buffer num: 32
I (676) wifi_init: rx ba win: 6
I (676) wifi_init: tcpip mbox: 32
I (686) wifi_init: udp mbox: 6
I (686) wifi_init: tcp mbox: 6
I (686) wifi_init: tcp tx win: 5744
I (696) wifi_init: tcp rx win: 5744
I (696) wifi_init: tcp mss: 1440
I (706) wifi_init: WiFi IRAM OP enabled
I (706) wifi_init: WiFi RX IRAM OP enabled
I (716) example_connect: Connecting to Duoman...
I (716) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (826) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (826) wifi:enable tsf
I (826) example_connect: Waiting for IP(s)
I (3236) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3976) wifi:state: init -> auth (b0)
I (4006) wifi:state: auth -> assoc (0)
I (4016) wifi:state: assoc -> run (10)
W (4036) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (4126) wifi:connected with Duoman, aid = 4, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (4126) wifi:security: WPA2-PSK, phy: bgn, rssi: -31
I (4126) wifi:pm start, type: 1

I (4136) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5616) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6616) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (6616) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (6626) example_connect: Connected to example_connect: sta
I (6626) example_connect: - IPv4 address: 192.168.0.124
I (6636) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6646) FRONT-CONN: Init tbcmh ...
I (6656) FRONT-CONN: Connect tbcmh ...
I (6656) tb_mqtt_client_helper: connecting to mqtt://192.168.0.210:1883 ...
I (6666) tb_mqtt_wapper: src_event->event_id=7
I (6666) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (6676) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: Still NOT connected to server!
I (7686) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (7686) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc5d4c
I (7686) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (7686) tb_mqtt_client_helper: before call on_connected()...
I (7696) FRONT-CONN: Connected to thingsboard server!
I (8706) provision: sent subscribe successful, msg_id=64475, topic=/provision/response
I (8706) tb_mqtt_wapper: [Provision][Tx] request_id=1 {"deviceName":"MY_DEVICE_NAME","provisionDeviceKey":"MY_PROVISION_KEY","provisionDeviceSecret":"MY_PROVISION_SECRET","credentialsType":"MQTT_BASIC","clientId":"MY_DEVICE_CLIENT_ID"}
I (8726) tb_mqtt_client_helper: after call on_connected()
I (8806) tb_mqtt_wapper: src_event->event_id=5
I (8806) tb_mqtt_wapper: [Provision][Rx] topic_type=6, payload_len=130 {"credentialsValue":{"clientId":"MY_CLIENT_ID","userName":null,"password":null},"credentialsType":"MQTT_BASIC","status":"SUCCESS"}
I (9736) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=5265
I (9736) tb_mqtt_client_helper: TBCM_EVENT_DATA
I (9736) provision: sent unsubscribe successful, msg_id=15212, topic=/provision/response
W (9746) provision: username is NULL!
W (9746) provision: userName is NULL!
W (9746) provision: password is NULL!
I (9756) FRONT-CONN: Provision successful and the device will work!
I (9766) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.210:1883 ...
I (9866) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (9876) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (9876) MQTT_CLIENT: Client asked to stop, but was not started
I (9976) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
I (9976) NORMAL-CONN: Init tbcmh ...
I (9976) NORMAL-CONN: Connect tbcmh ...
I (9976) tb_mqtt_client_helper: connecting to mqtt://192.168.0.210:1883 ...
I (9986) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: Still NOT connected to server!
I (9986) tb_mqtt_wapper: src_event->event_id=7
I (11006) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (11006) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (11006) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffca15c
I (11006) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (11016) tb_mqtt_client_helper: before call on_connected()...
I (11026) NORMAL-CONN: Connected to thingsboard server!
I (11026) NORMAL-CONN: Send telemetry: temprature, humidity
I (11036) NORMAL-CONN: Get temperature (a time-series data)
I (11046) NORMAL-CONN: Get humidity (a time-series data)
I (11046) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":25,"humidity":26}
I (11066) tb_mqtt_client_helper: after call on_connected()
I (11076) tb_mqtt_wapper: src_event->event_id=5
I (12066) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=6695
I (51466) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: Disconnect tbcmh ...
I (51466) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.210:1883 ...
I (51566) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (51576) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (51576) MQTT_CLIENT: Client asked to stop, but was not started
I (51676) DEV_SUP_BASIC_MQTT_CRED_C_WO_SSL_MAIN: Destroy tbcmh ...
I (51676) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!

```

### ThingsBoard CE/PE Data

1. Device provisiong results.

   Refer [here](../../.docs/device-provisioning-results-using-pre-provisioned-devices.md).

1. **Delete new provisioned device**. 

   In order to ensure that the example runs successfully next time, the newly added device needs to be deleted.

   See [here](../../.docs/delete-provisioned-device.md).

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues) on GitHub. We will get back to you soon.
