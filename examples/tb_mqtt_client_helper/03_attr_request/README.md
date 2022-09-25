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

**Note: Please execute example [01_telemetry_and_client_attr](../01_telemetry_and_client_attr) first, then extcute this one. Otherwise, `setpoint` cannot be obtained from the server when the attributes is requested.**

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
0x40081188: call_start_cpu1 at C:/Espressif/frameworks/esp-idf-v4.4.1/components/esp_system/port/cpu_start.c:160

I (0) cpu_start: App cpu up.
I (457) cpu_start: Pro cpu start user code
I (457) cpu_start: cpu freq: 160000000
I (458) cpu_start: Application information:
I (462) cpu_start: Project name:     attr_request
I (467) cpu_start: App version:      f0a9ef0-dirty
I (473) cpu_start: Compile time:     Sep 24 2022 12:21:07
I (479) cpu_start: ELF file SHA256:  a199db8282babc00...
I (485) cpu_start: ESP-IDF:          v4.4.1-dirty
I (491) heap_init: Initializing. RAM available for dynamic allocation:
I (498) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (504) heap_init: At 3FFB7618 len 000289E8 (162 KiB): DRAM
I (510) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (516) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (523) heap_init: At 4009449C len 0000BB64 (46 KiB): IRAM
I (530) spi_flash: detected chip: generic
I (534) spi_flash: flash io: dio
W (537) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (552) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (561) ATTR_REQUEST_EXAMPLE: [APP] Startup..
I (571) ATTR_REQUEST_EXAMPLE: [APP] Free memory: 276376 bytes
I (571) ATTR_REQUEST_EXAMPLE: [APP] IDF version: v4.4.1-dirty
I (641) wifi:wifi driver task: 3ffc0154, prio:23, stack:6656, core=0
I (641) system_api: Base MAC address is not set
I (641) system_api: read default base MAC address from EFUSE
I (671) wifi:wifi firmware version: 63017e0
I (671) wifi:wifi certification version: v7.0
I (671) wifi:config NVS flash: enabled
I (671) wifi:config nano formating: disabled
I (671) wifi:Init data frame dynamic rx buffer num: 32
I (671) wifi:Init management frame dynamic rx buffer num: 32
I (681) wifi:Init management short buffer num: 32
I (681) wifi:Init dynamic tx buffer num: 32
I (691) wifi:Init static rx buffer size: 1600
I (691) wifi:Init static rx buffer num: 10
I (701) wifi:Init dynamic rx buffer num: 32
I (701) wifi_init: rx ba win: 6
I (701) wifi_init: tcpip mbox: 32
I (711) wifi_init: udp mbox: 6
I (711) wifi_init: tcp mbox: 6
I (721) wifi_init: tcp tx win: 5744
I (721) wifi_init: tcp rx win: 5744
I (721) wifi_init: tcp mss: 1440
I (731) wifi_init: WiFi IRAM OP enabled
I (731) wifi_init: WiFi RX IRAM OP enabled
I (741) example_connect: Connecting to MySSID...
I (741) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (851) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (851) wifi:enable tsf
I (861) example_connect: Waiting for IP(s)
I (2911) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3651) wifi:state: init -> auth (b0)
I (3651) wifi:state: auth -> assoc (0)
I (3661) wifi:state: assoc -> run (10)
W (3671) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (3681) wifi:connected with MySSID, aid = 4, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (3681) wifi:security: WPA2-PSK, phy: bgn, rssi: -38
I (3681) wifi:pm start, type: 1

I (3691) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5631) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6131) esp_netif_handlers: example_connect: sta ip: 192.168.0.126, mask: 255.255.255.0, gw: 192.168.0.1
I (6131) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.126
I (6141) example_connect: Connected to example_connect: sta
I (6141) example_connect: - IPv4 address: 192.168.0.126
I (6151) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6161) ATTR_REQUEST_EXAMPLE: Init tbmch ...
I (6171) ATTR_REQUEST_EXAMPLE: Append client attribute: setpoint...
I (6171) ATTR_REQUEST_EXAMPLE: Append shared attribue: sntp_server...
I (6181) ATTR_REQUEST_EXAMPLE: Connect tbmch ...
I (6181) tb_mqtt_client_helper: connecting to mqtt://192.168.0.186...
I (6191) ATTR_REQUEST_EXAMPLE: connect tbmch ...
I (6191) tb_mqtt_client: MQTT_EVENT_BEFORE_CONNECT, msg_id=0, topic_len=0, data_len=0
I (6311) ATTR_REQUEST_EXAMPLE: Still NOT connected to server!
I (6831) tb_mqtt_client: MQTT_EVENT_CONNECTED
I (6831) tb_mqtt_client: client->mqtt_handle = 0x3ffc5ff0
I (6831) tb_mqtt_client: sent subscribe successful, msg_id=55794, topic=v1/devices/me/attributes
I (6841) tb_mqtt_client: sent subscribe successful, msg_id=56076, topic=v1/devices/me/attributes/response/+
I (6851) tb_mqtt_client: sent subscribe successful, msg_id=7836, topic=v1/devices/me/rpc/request/+
I (6861) tb_mqtt_client: sent subscribe successful, msg_id=27332, topic=v1/devices/me/rpc/response/+
I (6871) tb_mqtt_client: sent subscribe successful, msg_id=12936, topic=v2/fw/response/+/chunk/+
I (6881) tb_mqtt_client: before call on_connected()...
I (6891) tb_mqtt_client: after call on_connected()
I (6891) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=55794
I (6901) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=56076
I (6911) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=7836
I (6911) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=27332
I (6921) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=12936
I (7311) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (7311) ATTR_REQUEST_EXAMPLE: Connected to thingsboard server!
I (7311) ATTR_REQUEST_EXAMPLE: Request attributes, client attributes: setpoint; shared attributes: sntp_server
I (7321) tb_mqtt_client: [Attributes Request][Tx] RequestID=1, {"clientKeys":"setpoint", "sharedKeys":"sntp_server"}
I (7361) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=41524
I (7461) tb_mqtt_client: MQTT_EVENT_DATA
I (7461) tb_mqtt_client: [Attributes Request][Rx] RequestID=1 {"client":{"setpoint":25.5},"shared":{"sntp_server":"uk.pool.ntp.org"}}
I (8341) ATTR_REQUEST_EXAMPLE: Set setpoint (a client-side attribute)
I (8341) ATTR_REQUEST_EXAMPLE: Receive setpoint = 25.500000
I (8341) ATTR_REQUEST_EXAMPLE: Set sntp_server (a shared attribute)
I (8351) ATTR_REQUEST_EXAMPLE: Receive sntp_server = uk.pool.ntp.org
I (8351) ATTR_REQUEST_EXAMPLE: Receiving response of the attribute request! request_id=1
I (10561) ATTR_REQUEST_EXAMPLE: Send client attributes: setpoint
I (10561) ATTR_REQUEST_EXAMPLE: Get setpoint (a client attribute)
I (10561) tb_mqtt_client: [Client-Side Attributes][Tx] {"setpoint":25.5}
I (10581) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=8637
I (16071) ATTR_REQUEST_EXAMPLE: Send client attributes: setpoint
I (16071) ATTR_REQUEST_EXAMPLE: Get setpoint (a client attribute)
I (16071) tb_mqtt_client: [Client-Side Attributes][Tx] {"setpoint":25.5}
I (16091) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=40224
I (21581) ATTR_REQUEST_EXAMPLE: Send client attributes: setpoint
I (21581) ATTR_REQUEST_EXAMPLE: Get setpoint (a client attribute)
I (21581) tb_mqtt_client: [Client-Side Attributes][Tx] {"setpoint":25.5}
I (21601) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=55256
I (27091) ATTR_REQUEST_EXAMPLE: Send client attributes: setpoint
I (27091) ATTR_REQUEST_EXAMPLE: Get setpoint (a client attribute)
I (27091) tb_mqtt_client: [Client-Side Attributes][Tx] {"setpoint":25.5}
I (27111) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=39696
I (28101) ATTR_REQUEST_EXAMPLE: Disconnect tbmch ...
I (28101) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.186...
W (28201) MQTT_CLIENT: Client asked to stop, but was not started
I (28301) ATTR_REQUEST_EXAMPLE: Destroy tbmch ...
I (28301) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
```

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/liang-zhu-zi/esp-idf-thingsboard-mqtt/issues) on GitHub. We will get back to you soon.
