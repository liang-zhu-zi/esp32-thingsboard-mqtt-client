| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Device provisioning - Devices supplies Access Token - Plain MQTT (without SSL)

* [中文版](./README_CN.md)

- [Device provisioning - Devices supplies Access Token - Plain MQTT (without SSL)](#device-provisioning---devices-supplies-access-token---plain-mqtt-without-ssl)
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

This example implements the fllowing functions:

* Device provisioning - Devices supplies Access Token - Plain MQTT (without SSL)
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
         (MY_ACCESS_TOKEN) Access Token 
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
I (475) cpu_start: Project name:     dev_sup_access_token_wo_ssl
I (481) cpu_start: App version:      ac24217-dirty
I (487) cpu_start: Compile time:     Jan  7 2023 12:57:43
I (493) cpu_start: ELF file SHA256:  c20838421ca34f5d...
I (499) cpu_start: ESP-IDF:          v4.4.3-dirty
I (505) heap_init: Initializing. RAM available for dynamic allocation:
I (512) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (518) heap_init: At 3FFB76A0 len 00028960 (162 KiB): DRAM
I (524) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (530) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (537) heap_init: At 40094A04 len 0000B5FC (45 KiB): IRAM
I (544) spi_flash: detected chip: generic
I (548) spi_flash: flash io: dio
W (551) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (566) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (575) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: [APP] Startup..
I (585) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: [APP] Free memory: 275900 bytes
I (585) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: [APP] IDF version: v4.4.3-dirty
I (625) wifi:wifi driver task: 3ffc0024, prio:23, stack:6656, core=0
I (625) system_api: Base MAC address is not set
I (625) system_api: read default base MAC address from EFUSE
I (635) wifi:wifi firmware version: 8cb87ff
I (635) wifi:wifi certification version: v7.0
I (635) wifi:config NVS flash: enabled
I (635) wifi:config nano formating: disabled
I (645) wifi:Init data frame dynamic rx buffer num: 32
I (645) wifi:Init management frame dynamic rx buffer num: 32
I (655) wifi:Init management short buffer num: 32
I (655) wifi:Init dynamic tx buffer num: 32
I (665) wifi:Init static rx buffer size: 1600
I (665) wifi:Init static rx buffer num: 10
I (665) wifi:Init dynamic rx buffer num: 32
I (675) wifi_init: rx ba win: 6
I (675) wifi_init: tcpip mbox: 32
I (685) wifi_init: udp mbox: 6
I (685) wifi_init: tcp mbox: 6
I (685) wifi_init: tcp tx win: 5744
I (695) wifi_init: tcp rx win: 5744
I (695) wifi_init: tcp mss: 1440
I (705) wifi_init: WiFi IRAM OP enabled
I (705) wifi_init: WiFi RX IRAM OP enabled
I (715) example_connect: Connecting to Duoman...
I (715) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (825) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (825) wifi:enable tsf
I (835) example_connect: Waiting for IP(s)
I (3245) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3985) wifi:state: init -> auth (b0)
I (3985) wifi:state: auth -> assoc (0)
I (3995) wifi:state: assoc -> run (10)
W (4005) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (4015) wifi:connected with Duoman, aid = 4, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (4015) wifi:security: WPA2-PSK, phy: bgn, rssi: -30
I (4025) wifi:pm start, type: 1

I (4045) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5615) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (8615) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (8615) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (8625) example_connect: Connected to example_connect: sta
I (8625) example_connect: - IPv4 address: 192.168.0.124
I (8635) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (8645) FRONT-CONN: Init tbcmh ...
I (8655) FRONT-CONN: Connect tbcmh ...
I (8655) tb_mqtt_client_helper: connecting to mqtt://192.168.0.210:1883 ...
I (8665) tb_mqtt_wapper: src_event->event_id=7
I (8665) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (8675) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: Still NOT connected to server!
I (9685) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (9685) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc5d48
I (9685) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (9685) tb_mqtt_client_helper: before call on_connected()...
I (9695) FRONT-CONN: Connected to thingsboard server!
I (10705) provision: sent subscribe successful, msg_id=29714, topic=/provision/response
I (10705) tb_mqtt_wapper: [Provision][Tx] request_id=1 {"deviceName":"MY_DEVICE_NAME","provisionDeviceKey":"MY_PROVISION_KEY","provisionDeviceSecret":"MY_PROVISION_SECRET","credentialsType":"ACCESS_TOKEN","token":"MY_ACCESS_TOKEN"}
I (10725) tb_mqtt_client_helper: after call on_connected()
I (11015) tb_mqtt_wapper: src_event->event_id=5
I (11025) tb_mqtt_wapper: [Provision][Rx] topic_type=6, payload_len=90 {"credentialsValue":"MY_ACCESS_TOKEN","credentialsType":"ACCESS_TOKEN","status":"SUCCESS"}
I (11735) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=64559
I (11735) tb_mqtt_client_helper: TBCM_EVENT_DATA
I (11735) provision: sent unsubscribe successful, msg_id=27611, topic=/provision/response
I (11745) FRONT-CONN: Provision successful and the device will work!
I (11745) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.210:1883 ...
I (11855) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (11865) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (11865) MQTT_CLIENT: Client asked to stop, but was not started
I (11975) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
I (11975) NORMAL-CONN: Init tbcmh ...
I (11975) NORMAL-CONN: Connect tbcmh ...
I (11975) tb_mqtt_client_helper: connecting to mqtt://192.168.0.210:1883 ...
I (11985) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: Still NOT connected to server!
I (11985) tb_mqtt_wapper: src_event->event_id=7
I (13005) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (13005) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (13005) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc9d54
I (13005) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (13015) tb_mqtt_client_helper: before call on_connected()...
I (13025) NORMAL-CONN: Connected to thingsboard server!
I (13025) NORMAL-CONN: Send telemetry: temprature, humidity
I (13035) NORMAL-CONN: Get temperature (a time-series data)
I (13045) NORMAL-CONN: Get humidity (a time-series data)
I (13045) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":25,"humidity":26}
I (13065) tb_mqtt_client_helper: after call on_connected()
I (13085) tb_mqtt_wapper: src_event->event_id=5
I (14065) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=16882
I (53465) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: Disconnect tbcmh ...
I (53465) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.210:1883 ...
I (53565) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (53575) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (53575) MQTT_CLIENT: Client asked to stop, but was not started
I (53675) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: Destroy tbcmh ...
I (53675) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
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

   See [here](../../.docs/add-or-modify-device-profile-for-device-provisioning-that-allow-to-create-new-devices.d)

1. **ThingsBoard CE/PE**: pre-provisioning device with Access token. 

   See [here](../../.docs/pre-provisioning-device-with-access-token.md)

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
         (MY_ACCESS_TOKEN) Access Token 
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
W (551) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (565) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (575) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: [APP] Startup..
I (585) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: [APP] Free memory: 275900 bytes
I (585) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: [APP] IDF version: v4.4.3-dirty
I (625) wifi:wifi driver task: 3ffc0024, prio:23, stack:6656, core=0
I (625) system_api: Base MAC address is not set
I (625) system_api: read default base MAC address from EFUSE
I (635) wifi:wifi firmware version: 8cb87ff
I (635) wifi:wifi certification version: v7.0
I (635) wifi:config NVS flash: enabled
I (635) wifi:config nano formating: disabled
I (645) wifi:Init data frame dynamic rx buffer num: 32
I (645) wifi:Init management frame dynamic rx buffer num: 32
I (655) wifi:Init management short buffer num: 32
I (655) wifi:Init dynamic tx buffer num: 32
I (665) wifi:Init static rx buffer size: 1600
I (665) wifi:Init static rx buffer num: 10
I (665) wifi:Init dynamic rx buffer num: 32
I (675) wifi_init: rx ba win: 6
I (675) wifi_init: tcpip mbox: 32
I (685) wifi_init: udp mbox: 6
I (685) wifi_init: tcp mbox: 6
I (685) wifi_init: tcp tx win: 5744
I (695) wifi_init: tcp rx win: 5744
I (695) wifi_init: tcp mss: 1440
I (705) wifi_init: WiFi IRAM OP enabled
I (705) wifi_init: WiFi RX IRAM OP enabled
I (715) example_connect: Connecting to Duoman...
I (715) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (825) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (825) wifi:enable tsf
I (825) example_connect: Waiting for IP(s)
I (3235) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3975) wifi:state: init -> auth (b0)
I (3985) wifi:state: auth -> assoc (0)
I (3985) wifi:state: assoc -> run (10)
W (3995) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (4015) wifi:connected with Duoman, aid = 4, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (4015) wifi:security: WPA2-PSK, phy: bgn, rssi: -31
I (4015) wifi:pm start, type: 1

I (4055) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5615) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6115) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (6115) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (6125) example_connect: Connected to example_connect: sta
I (6125) example_connect: - IPv4 address: 192.168.0.124
I (6135) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6145) FRONT-CONN: Init tbcmh ...
I (6145) FRONT-CONN: Connect tbcmh ...
I (6155) tb_mqtt_client_helper: connecting to mqtt://192.168.0.210:1883 ...
I (6165) tb_mqtt_wapper: src_event->event_id=7
I (6165) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (6175) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: Still NOT connected to server!
I (7185) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (7185) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc5d5c
I (7185) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (7185) tb_mqtt_client_helper: before call on_connected()...
I (7195) FRONT-CONN: Connected to thingsboard server!
I (8205) provision: sent subscribe successful, msg_id=13620, topic=/provision/response
I (8205) tb_mqtt_wapper: [Provision][Tx] request_id=1 {"deviceName":"MY_DEVICE_NAME","provisionDeviceKey":"MY_PROVISION_KEY","provisionDeviceSecret":"MY_PROVISION_SECRET","credentialsType":"ACCESS_TOKEN","token":"MY_ACCESS_TOKEN"}
I (8225) tb_mqtt_client_helper: after call on_connected()
I (8305) tb_mqtt_wapper: src_event->event_id=5
I (8315) tb_mqtt_wapper: [Provision][Rx] topic_type=6, payload_len=90 {"credentialsValue":"MY_ACCESS_TOKEN","credentialsType":"ACCESS_TOKEN","status":"SUCCESS"}
I (9235) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=18509
I (9235) tb_mqtt_client_helper: TBCM_EVENT_DATA
I (9235) provision: sent unsubscribe successful, msg_id=13276, topic=/provision/response
I (9245) FRONT-CONN: Provision successful and the device will work!
I (9245) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.210:1883 ...
I (9355) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (9355) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (9365) MQTT_CLIENT: Client asked to stop, but was not started
I (9465) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
I (9465) NORMAL-CONN: Init tbcmh ...
I (9465) NORMAL-CONN: Connect tbcmh ...
I (9465) tb_mqtt_client_helper: connecting to mqtt://192.168.0.210:1883 ...
I (9475) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: Still NOT connected to server!
I (9475) tb_mqtt_wapper: src_event->event_id=7
I (10495) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (10495) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (10495) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc9fc8
I (10495) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (10505) tb_mqtt_client_helper: before call on_connected()...
I (10515) NORMAL-CONN: Connected to thingsboard server!
I (10515) NORMAL-CONN: Send telemetry: temprature, humidity
I (10525) NORMAL-CONN: Get temperature (a time-series data)
I (10535) NORMAL-CONN: Get humidity (a time-series data)
I (10535) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":25,"humidity":26}
I (10555) tb_mqtt_client_helper: after call on_connected()
I (10555) tb_mqtt_wapper: src_event->event_id=5
I (11555) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=62390
I (50955) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: Disconnect tbcmh ...
I (50955) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.210:1883 ...
I (51055) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (51065) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (51065) MQTT_CLIENT: Client asked to stop, but was not started
I (51165) DEV_SUP_ACCESS_TOKEN_WO_SSL_MAIN: Destroy tbcmh ...
I (51165) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
```

### ThingsBoard CE/PE Data

1. Device provisiong results.

   Refer [here](../../.docs/device-provisioning-results-using-pre-provisioned-devices.md).

1. **Delete new provisioned device**. 

   In order to ensure that the example runs successfully next time, the newly added device needs to be deleted.

   See [here](../../.docs/delete-provisioned-device.md).

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues) on GitHub. We will get back to you soon.
