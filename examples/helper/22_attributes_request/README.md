| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Request attribute values from the server - ThingsBoard MQTT Client Example

* [中文版](./README_CN.md)

This example is based on [`$ESP-IDF\examples\protocols\mqtt\tcp`](https://github.com/espressif/esp-idf/tree/master/examples/protocols/mqtt/tcp).

This example implements shared attributes related functions:

* Request attribute values from the server: client-side attribute - setpoint, shared attribute - sntp_server
* Subscribe to and receive shared attribute updates from the server: sntp_server
* Publish client-side attributes: setpoint
  
## Hardware Required

* A development board with ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

## How to Use Example

**Note: Please execute example [20_client_attribute](../20_client_attribute) first, then extcute this one. Otherwise, `setpoint` cannot be obtained from the server when the attributes is requested.**

1. Get a device access token

   `Login in ThingsBoard CE/PE` --> `Devices` --> Click my device --> `Details` --> `Copy Access Token`.

1. add or update a shared attribute in ThingsBoard

   * Shared attributes `sntp_server`:
     * Add: `Login in ThingsBoard CE/PE` --> `Devices` --> Click my device --> `Attributes` --> `Shared attributes` --> `Add attribute` --> Key: "sntp_server", Value type: "String", String value: "uk.pool.ntp.org" --> `Add`.
     * Or update: `Login in ThingsBoard CE/PE` --> `Devices` --> Click my device --> `Attributes` --> `Shared attributes` --> `sntp_server` --> `Modify` --> Value type: "String", String value: "uk.pool.ntp.org" --> `Update`.

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

1. build, flash and monitor

   Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

   (To exit the serial monitor, type ``Ctrl-]``.)

   See the [Getting Started Guide](https://idf.espressif.com/) for full steps to configure and use ESP-IDF to build projects.

## Example Output

```none
...
I (449) cpu_start: Starting app cpu, entry point is 0x40081188
0x40081188: call_start_cpu1 at C:/Espressif/frameworks/esp-idf-v4.4.1/components/esp_system/port/cpu_start.c:160

I (0) cpu_start: App cpu up.
I (463) cpu_start: Pro cpu start user code
I (463) cpu_start: cpu freq: 160000000
I (463) cpu_start: Application information:
I (468) cpu_start: Project name:     attribute_request
I (474) cpu_start: App version:      3f8b5c2-dirty
I (479) cpu_start: Compile time:     Dec 11 2022 16:59:40
I (485) cpu_start: ELF file SHA256:  b8c30ef8c0804697...
I (491) cpu_start: ESP-IDF:          v4.4.1-dirty
I (497) heap_init: Initializing. RAM available for dynamic allocation:
I (504) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (510) heap_init: At 3FFB7618 len 000289E8 (162 KiB): DRAM
I (516) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (522) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (529) heap_init: At 4009449C len 0000BB64 (46 KiB): IRAM
I (536) spi_flash: detected chip: generic
I (540) spi_flash: flash io: dio
W (544) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (558) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (568) ATTR_REQUEST_EXAMPLE: [APP] Startup..
I (578) ATTR_REQUEST_EXAMPLE: [APP] Free memory: 276376 bytes
I (578) ATTR_REQUEST_EXAMPLE: [APP] IDF version: v4.4.1-dirty
I (618) wifi:wifi driver task: 3ffbff04, prio:23, stack:6656, core=0
I (618) system_api: Base MAC address is not set
I (618) system_api: read default base MAC address from EFUSE
I (628) wifi:wifi firmware version: 63017e0
I (628) wifi:wifi certification version: v7.0
I (628) wifi:config NVS flash: enabled
I (628) wifi:config nano formating: disabled
I (638) wifi:Init data frame dynamic rx buffer num: 32
I (638) wifi:Init management frame dynamic rx buffer num: 32
I (648) wifi:Init management short buffer num: 32
I (648) wifi:Init dynamic tx buffer num: 32
I (658) wifi:Init static rx buffer size: 1600
I (658) wifi:Init static rx buffer num: 10
I (658) wifi:Init dynamic rx buffer num: 32
I (668) wifi_init: rx ba win: 6
I (668) wifi_init: tcpip mbox: 32
I (678) wifi_init: udp mbox: 6
I (678) wifi_init: tcp mbox: 6
I (678) wifi_init: tcp tx win: 5744
I (688) wifi_init: tcp rx win: 5744
I (688) wifi_init: tcp mss: 1440
I (698) wifi_init: WiFi IRAM OP enabled
I (698) wifi_init: WiFi RX IRAM OP enabled
I (708) example_connect: Connecting to Duoman...
I (708) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (818) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (818) wifi:enable tsf
I (828) example_connect: Waiting for IP(s)
I (2868) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3618) wifi:state: init -> auth (b0)
I (3618) wifi:state: auth -> assoc (0)
I (3628) wifi:state: assoc -> run (10)
W (3638) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (3658) wifi:connected with Duoman, aid = 5, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (3658) wifi:security: WPA2-PSK, phy: bgn, rssi: -41
I (3658) wifi:pm start, type: 1

I (3708) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5608) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6108) esp_netif_handlers: example_connect: sta ip: 192.168.0.126, mask: 255.255.255.0, gw: 192.168.0.1
I (6108) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.126
I (6118) example_connect: Connected to example_connect: sta
I (6118) example_connect: - IPv4 address: 192.168.0.126
I (6128) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6138) ATTR_REQUEST_EXAMPLE: Init tbcmh ...
I (6148) ATTR_REQUEST_EXAMPLE: Connect tbcmh ...
I (6148) tb_mqtt_client_helper: connecting to mqtt://192.168.0.186:1883 ...
I (6158) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=0
I (7168) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (7168) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc5be4
I (7168) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (7168) tb_mqtt_client_helper: before call on_connected()...
I (7178) ATTR_REQUEST_EXAMPLE: Connected to thingsboard server!
I (7188) ATTR_REQUEST_EXAMPLE: Request attributes, client attributes: setpoint; shared attributes: sntp_server
I (7198) attributesrequest: sent subscribe successful, msg_id=52358, topic=v1/devices/me/attributes/response/+
I (7208) tb_mqtt_client: [Attributes Request][Tx] request_id=1, {"clientKeys":"setpoint", "sharedKeys":"sntp_server"}
I (7218) tb_mqtt_client_helper: after call on_connected()
I (7228) tb_mqtt_client_helper: TBCM_EVENT_SUBSCRIBED, msg_id=52358
I (7398) tb_mqtt_client: [Attributes Request][Rx] request_id=1 {"client":{"setpoint":25.5},"shared":{"sntp_server":"hk.pool.ntp.org"}}
I (8228) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=59702
I (8228) tb_mqtt_client_helper: TBCM_EVENT_DATA
I (8228) attributesrequest: sent unsubscribe successful, msg_id=8998, topic=v1/devices/me/attributes/response/+
I (8238) ATTR_REQUEST_EXAMPLE: Receiving response of the attributes request!
I (8248) ATTR_REQUEST_EXAMPLE: client_attributes: {"setpoint":25.5}
I (8248) ATTR_REQUEST_EXAMPLE: shared_attributes: {"sntp_server":"hk.pool.ntp.org"}
I (8258) tb_mqtt_client_helper: TBCM_EVENT_UNSUBSCRIBED, msg_id=8998
I (27968) ATTR_REQUEST_EXAMPLE: Disconnect tbcmh ...
I (27968) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.186:1883 ...
I (28068) tb_mqtt_client: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (28078) tb_mqtt_client: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (28078) MQTT_CLIENT: Client asked to stop, but was not started
I (28178) ATTR_REQUEST_EXAMPLE: Destroy tbcmh ...
I (28178) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!

```

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues) on GitHub. We will get back to you soon.
