| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Subscribe and receive Shared Attributes - ThingsBoard MQTT Client Example

* [中文版](./README_CN.md)

This example is based on [`$ESP-IDF\examples\protocols\mqtt\tcp`](https://github.com/espressif/esp-idf/tree/master/examples/protocols/mqtt/tcp).

This example implements shared attributes related functions:

* Subscribe to shared attribute updates from the server
* Receive the shared attributes update: sntp_server

## Hardware Required

* A development board with ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

## How to Use Example

1. Get a device access token

   `Login in ThingsBoard CE/PE` --> `Devices` --> Click my device --> `Details` --> `Copy Access Token`.

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
       (MyDeviceToken) Access Token 
   Example Connection Configuration  --->
       [*] connect using WiFi interface
       (MySSID) WiFi SSID 
       (MyPassword) WiFi Password                  
   Component config  --->
       ThingsBoard MQTT Client library (TBMQTTClient)  ---> 
           [*] Enable TBMQTTClient Helper
   ```

4. build, flash and monitor

   Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

   (To exit the serial monitor, type ``Ctrl-]``.)

   See the [Getting Started Guide](https://idf.espressif.com/) for full steps to configure and use ESP-IDF to build projects.

5. add & modify a shared attribute in ThingsBoard

   * `Login in ThingsBoard CE/PE` --> `Devices` --> Click my device --> `Attributes` --> `Shared attributes` --> `Add attribute` --> Key: "sntp_server", Value type: "String", String value: "uk.pool.ntp.org" --> `Add`.

   * `Login in ThingsBoard CE/PE` --> `Devices` --> Click my device --> `Attributes` --> `Shared attributes` --> `sntp_server` --> `Modify` --> Value type: "String", String value: "hk.pool.ntp.org" --> `Update`.

## Example Output

```none
...
0x40081188: call_start_cpu1 at C:/Espressif/frameworks/esp-idf-v4.4.1/components/esp_system/port/cpu_start.c:160

I (0) cpu_start: App cpu up.
I (458) cpu_start: Pro cpu start user code
I (458) cpu_start: cpu freq: 160000000
I (458) cpu_start: Application information:
I (463) cpu_start: Project name:     shared_attribute
I (468) cpu_start: App version:      b5865a7-dirty
I (473) cpu_start: Compile time:     Sep 23 2022 12:42:51
I (479) cpu_start: ELF file SHA256:  4fc2513e9a07a99c...
I (485) cpu_start: ESP-IDF:          v4.4.1-dirty
I (491) heap_init: Initializing. RAM available for dynamic allocation:
I (498) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (504) heap_init: At 3FFB7618 len 000289E8 (162 KiB): DRAM
I (510) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (517) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (523) heap_init: At 4009449C len 0000BB64 (46 KiB): IRAM
I (531) spi_flash: detected chip: generic
I (534) spi_flash: flash io: dio
W (538) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (552) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (562) SHARED_ATTR_EXAMPLE: [APP] Startup..
I (572) SHARED_ATTR_EXAMPLE: [APP] Free memory: 276376 bytes
I (572) SHARED_ATTR_EXAMPLE: [APP] IDF version: v4.4.1-dirty
I (642) wifi:wifi driver task: 3ffc0154, prio:23, stack:6656, core=0
I (642) system_api: Base MAC address is not set
I (642) system_api: read default base MAC address from EFUSE
I (672) wifi:wifi firmware version: 63017e0
I (672) wifi:wifi certification version: v7.0
I (672) wifi:config NVS flash: enabled
I (672) wifi:config nano formating: disabled
I (672) wifi:Init data frame dynamic rx buffer num: 32
I (672) wifi:Init management frame dynamic rx buffer num: 32
I (682) wifi:Init management short buffer num: 32
I (682) wifi:Init dynamic tx buffer num: 32
I (692) wifi:Init static rx buffer size: 1600
I (692) wifi:Init static rx buffer num: 10
I (702) wifi:Init dynamic rx buffer num: 32
I (702) wifi_init: rx ba win: 6
I (702) wifi_init: tcpip mbox: 32
I (712) wifi_init: udp mbox: 6
I (712) wifi_init: tcp mbox: 6
I (722) wifi_init: tcp tx win: 5744
I (722) wifi_init: tcp rx win: 5744
I (722) wifi_init: tcp mss: 1440
I (732) wifi_init: WiFi IRAM OP enabled
I (732) wifi_init: WiFi RX IRAM OP enabled
I (742) example_connect: Connecting to MySSID...
I (742) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (852) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (852) wifi:enable tsf
I (852) example_connect: Waiting for IP(s)
I (2902) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3652) wifi:state: init -> auth (b0)
I (3652) wifi:state: auth -> assoc (0)
I (3652) wifi:state: assoc -> run (10)
W (3672) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (3682) wifi:connected with MySSID, aid = 5, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (3682) wifi:security: WPA2-PSK, phy: bgn, rssi: -39
I (3692) wifi:pm start, type: 1

I (3772) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5632) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6132) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (6132) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (6142) example_connect: Connected to example_connect: sta
I (6142) example_connect: - IPv4 address: 192.168.0.124
I (6152) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6162) SHARED_ATTR_EXAMPLE: Init tbmch ...
I (6172) SHARED_ATTR_EXAMPLE: Append shared attribue: sntp_server...
I (6172) SHARED_ATTR_EXAMPLE: Connect tbmch ...
I (6182) tb_mqtt_client_helper: connecting to mqtt://192.168.0.186...
I (6192) SHARED_ATTR_EXAMPLE: connect tbmch ...
I (6192) tb_mqtt_client: MQTT_EVENT_BEFORE_CONNECT, msg_id=0, topic_len=0, data_len=0
I (6302) SHARED_ATTR_EXAMPLE: Still NOT connected to server!
I (6502) tb_mqtt_client: MQTT_EVENT_CONNECTED
I (6502) tb_mqtt_client: client->mqtt_handle = 0x3ffc5fc0
I (6502) tb_mqtt_client: sent subscribe successful, msg_id=19697, topic=v1/devices/me/attributes
I (6512) tb_mqtt_client: sent subscribe successful, msg_id=40302, topic=v1/devices/me/attributes/response/+
I (6522) tb_mqtt_client: sent subscribe successful, msg_id=61335, topic=v1/devices/me/rpc/request/+
I (6532) tb_mqtt_client: sent subscribe successful, msg_id=14969, topic=v1/devices/me/rpc/response/+
I (6542) tb_mqtt_client: sent subscribe successful, msg_id=23712, topic=v2/fw/response/+/chunk/+
I (6552) tb_mqtt_client: before call on_connected()...
I (6552) tb_mqtt_client: after call on_connected()
I (6562) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=19697
I (6572) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=40302
I (6582) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=61335
I (6582) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=14969
I (6592) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=23712
I (7302) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (7302) SHARED_ATTR_EXAMPLE: Connected to thingsboard server!
I (11562) tb_mqtt_client: MQTT_EVENT_DATA
I (11562) tb_mqtt_client: [Subscribe Shared Attributes][Rx] {"sntp_server":"uk.pool.ntp.org"}
I (11602) SHARED_ATTR_EXAMPLE: Set sntp_server (a shared attribute)
I (11602) SHARED_ATTR_EXAMPLE: Receive sntp_server = uk.pool.ntp.org
I (27632) tb_mqtt_client: MQTT_EVENT_DATA
I (27632) tb_mqtt_client: [Subscribe Shared Attributes][Rx] {"sntp_server":"hk.pool.ntp.org"}
I (28002) SHARED_ATTR_EXAMPLE: Disconnect tbmch ...
I (28002) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.186...
I (28002) SHARED_ATTR_EXAMPLE: Set sntp_server (a shared attribute)
I (28002) SHARED_ATTR_EXAMPLE: Receive sntp_server = hk.pool.ntp.org
W (28112) MQTT_CLIENT: Client asked to stop, but was not started
I (28222) SHARED_ATTR_EXAMPLE: Destroy tbmch ...
I (28222) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
```

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues) on GitHub. We will get back to you soon.
