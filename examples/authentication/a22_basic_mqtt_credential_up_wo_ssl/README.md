| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Basic MQTT authentication - Authentication based on Username and Password (without SSL)

* [中文版](./README_CN.md)

This example implements some functions:

* Basic MQTT authentication - Authentication based on Username and Password (without SSL)
* Publish telemetry: temprature, humidity

Refer [here](https://thingsboard.io/docs/user-guide/basic-mqtt/#authentication-based-on-username-and-password).

## Hardware Required

* A development board with ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

## How to Use Example

1. Modify device credentials - MQTT Basic

   `Login in ThingsBoard CE/PE as tenant` --> `Devices` -->  Click on *my device* --> `Details` --> `Manage credentials` -> Select credentials type: *MQTT Basic* -> Input User name: *MY_USER_NAME* -> Input Password: *MY_PASSWORD* -> `Save`.

   ![image](./basic_mqtt_credential_up_wo_ssl_1.png)

2. set-targe (optional)

   Before project configuration and build, be sure to set the correct chip target using:

   ```bash
   idf.py set-target <chip_name>
   ```

3. menuconfig

   Then project configuration:

   ```bash
   idf.py menuconfig
   ```

   Configuration: ThingsBoard MQTT URI, access token, Wi-Fi SSID, password:

   ```menuconfig
   Example Configuration  --->
      (mqtt://MyThingsboardServerIP) Broker URL
      (1883) Port
      (MY_USER_NAME) User name
      (MY_PASSWORD) Password 
   Example Connection Configuration  --->
       [*] connect using WiFi interface
       (MySSID) WiFi SSID 
       (MyPassword) WiFi Password                  
   ```

4. build, flash and monitor

   Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

   (To exit the serial monitor, type ``Ctrl-]``.)

   See the [Getting Started Guide](https://idf.espressif.com/) for full steps to configure and use ESP-IDF to build projects.

## Example Output

```none
...
0x400811a8: call_start_cpu1 at C:/Espressif/frameworks/esp-idf-v4.4.3/components/esp_system/port/cpu_start.c:148

I (0) cpu_start: App cpu up.
I (466) cpu_start: Pro cpu start user code
I (466) cpu_start: cpu freq: 160000000
I (466) cpu_start: Application information:
I (470) cpu_start: Project name:     basic_mqtt_credential_cup_wo_ss
I (477) cpu_start: App version:      30a57f0-dirty
I (483) cpu_start: Compile time:     Dec 31 2022 20:05:07
I (489) cpu_start: ELF file SHA256:  df41a339ecdae0c9...
I (495) cpu_start: ESP-IDF:          v4.4.3-dirty
I (500) heap_init: Initializing. RAM available for dynamic allocation:
I (508) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (514) heap_init: At 3FFB7650 len 000289B0 (162 KiB): DRAM
I (520) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (526) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (533) heap_init: At 40094A04 len 0000B5FC (45 KiB): IRAM
I (540) spi_flash: detected chip: generic
I (543) spi_flash: flash io: dio
W (547) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (561) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (571) EXAM_ACCESS_TOKEN_WO_SSL: [APP] Startup..
I (581) EXAM_ACCESS_TOKEN_WO_SSL: [APP] Free memory: 275980 bytes
I (581) EXAM_ACCESS_TOKEN_WO_SSL: [APP] IDF version: v4.4.3-dirty
I (621) wifi:wifi driver task: 3ffbffcc, prio:23, stack:6656, core=0
I (621) system_api: Base MAC address is not set
I (621) system_api: read default base MAC address from EFUSE
I (631) wifi:wifi firmware version: 8cb87ff
I (631) wifi:wifi certification version: v7.0
I (631) wifi:config NVS flash: enabled
I (631) wifi:config nano formating: disabled
I (641) wifi:Init data frame dynamic rx buffer num: 32
I (641) wifi:Init management frame dynamic rx buffer num: 32
I (651) wifi:Init management short buffer num: 32
I (651) wifi:Init dynamic tx buffer num: 32
I (661) wifi:Init static rx buffer size: 1600
I (661) wifi:Init static rx buffer num: 10
I (661) wifi:Init dynamic rx buffer num: 32
I (671) wifi_init: rx ba win: 6
I (671) wifi_init: tcpip mbox: 32
I (681) wifi_init: udp mbox: 6
I (681) wifi_init: tcp mbox: 6
I (681) wifi_init: tcp tx win: 5744
I (691) wifi_init: tcp rx win: 5744
I (691) wifi_init: tcp mss: 1440
I (701) wifi_init: WiFi IRAM OP enabled
I (701) wifi_init: WiFi RX IRAM OP enabled
I (711) example_connect: Connecting to Duoman...
I (711) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (821) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (821) wifi:enable tsf
I (821) example_connect: Waiting for IP(s)
I (3231) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3981) wifi:state: init -> auth (b0)
I (3981) wifi:state: auth -> assoc (0)
I (3991) wifi:state: assoc -> run (10)
W (4001) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (4011) wifi:connected with Duoman, aid = 4, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (4011) wifi:security: WPA2-PSK, phy: bgn, rssi: -40
I (4011) wifi:pm start, type: 1

I (4061) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5611) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6111) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (6111) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (6121) example_connect: Connected to example_connect: sta
I (6121) example_connect: - IPv4 address: 192.168.0.124
I (6131) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6141) EXAM_ACCESS_TOKEN_WO_SSL: Init tbcmh ...
I (6151) EXAM_ACCESS_TOKEN_WO_SSL: Connect tbcmh ...
I (6151) tb_mqtt_client_helper: connecting to mqtt://192.168.0.187:1883 ...
I (6161) EXAM_ACCESS_TOKEN_WO_SSL: connect tbcmh ...
I (6161) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (6171) EXAM_ACCESS_TOKEN_WO_SSL: Still NOT connected to server!
I (7181) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (7181) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc5d04
I (7181) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (7181) tb_mqtt_client_helper: before call on_connected()...
I (7191) EXAM_ACCESS_TOKEN_WO_SSL: Connected to thingsboard server!
I (7201) tb_mqtt_client_helper: after call on_connected()
I (10501) EXAM_ACCESS_TOKEN_WO_SSL: Send telemetry: temprature, humidity
I (10501) EXAM_ACCESS_TOKEN_WO_SSL: Get temperature (a time-series data)
I (10501) EXAM_ACCESS_TOKEN_WO_SSL: Get humidity (a time-series data)
I (10511) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":25,"humidity":26}
I (11521) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=60122
I (15921) EXAM_ACCESS_TOKEN_WO_SSL: Send telemetry: temprature, humidity
I (15921) EXAM_ACCESS_TOKEN_WO_SSL: Get temperature (a time-series data)
I (15921) EXAM_ACCESS_TOKEN_WO_SSL: Get humidity (a time-series data)
I (15931) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":25.5,"humidity":27}
I (16941) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=47077
I (21341) EXAM_ACCESS_TOKEN_WO_SSL: Send telemetry: temprature, humidity
I (21341) EXAM_ACCESS_TOKEN_WO_SSL: Get temperature (a time-series data)
I (21341) EXAM_ACCESS_TOKEN_WO_SSL: Get humidity (a time-series data)
I (21351) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":26,"humidity":28}
I (22361) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=56026
I (26761) EXAM_ACCESS_TOKEN_WO_SSL: Send telemetry: temprature, humidity
I (26761) EXAM_ACCESS_TOKEN_WO_SSL: Get temperature (a time-series data)
I (26761) EXAM_ACCESS_TOKEN_WO_SSL: Get humidity (a time-series data)
I (26771) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":26.5,"humidity":29}
I (27781) EXAM_ACCESS_TOKEN_WO_SSL: Disconnect tbcmh ...
I (27781) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.187:1883 ...
I (27781) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=18547
I (27891) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (27891) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (27901) MQTT_CLIENT: Client asked to stop, but was not started
I (28001) EXAM_ACCESS_TOKEN_WO_SSL: Destroy tbcmh ...
I (28001) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!


```

## ThingsBoard CE/PE Data

* Check out the latest Telemetry data.

   `Login in ThingsBoard CE/PE as tenant` --> `Devices` --> Click on *my device* --> `Latest tememetry`. You can find `humidity` and `temprature`. Their values change over time.

   ![image](../../.docs/images/check-latest-telemetry/check-latest-telemetry-1.png)

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues) on GitHub. We will get back to you soon.
