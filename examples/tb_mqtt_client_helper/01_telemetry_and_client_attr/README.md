| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Send Telemetry and Client-side Attributes - ThingBoard MQTT Client Example

Based on `$ESP-IDF\esp-idf-v4.4.1\examples\protocols\mqtt\tcp`.

This example implement telemetry and client attributes related functions:

1. Send telemetry: temprature, humidity;
2. Send client-side attributes: model, fw_version, setpoint.

### Hardware Required

* A development board with ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

## How to Use Example

Before project configuration and build, be sure to set the correct chip target using:

```bash
idf.py set-target <chip_name>
```

Then project configuration:

```bash
idf.py menuconfig
```

* ThingsBoard MQTT URI, access token;

```
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

### Get a device token

`Login in ThingsBoard CE/PE` --> `Devices` --> Click a device --> `Details` --> `Copy Access Token`.


### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://idf.espressif.com/) for full steps to configure and use ESP-IDF to build projects.

## Example Output

```
...
0x40081188: call_start_cpu1 at C:/Espressif/frameworks/esp-idf-v4.4.1/components/esp_system/port/cpu_start.c:160

I (0) cpu_start: App cpu up.
I (457) cpu_start: Pro cpu start user code
I (457) cpu_start: cpu freq: 160000000
I (457) cpu_start: Application information:
I (462) cpu_start: Project name:     send_telemetry_and_client_attr
I (469) cpu_start: App version:      f00da47-dirty
I (474) cpu_start: Compile time:     Sep 17 2022 19:13:38
I (480) cpu_start: ELF file SHA256:  e4d55eca27f2d2d6...
I (486) cpu_start: ESP-IDF:          v4.4.1-dirty
I (492) heap_init: Initializing. RAM available for dynamic allocation:
I (499) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (505) heap_init: At 3FFB7620 len 000289E0 (162 KiB): DRAM
I (511) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (517) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (524) heap_init: At 4009449C len 0000BB64 (46 KiB): IRAM
I (531) spi_flash: detected chip: generic
I (535) spi_flash: flash io: dio
W (539) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (553) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (563) TELE_CLI_ATTR_EXAMPLE: [APP] Startup..
I (573) TELE_CLI_ATTR_EXAMPLE: [APP] Free memory: 276368 bytes
I (573) TELE_CLI_ATTR_EXAMPLE: [APP] IDF version: v4.4.1-dirty
I (643) wifi:wifi driver task: 3ffc015c, prio:23, stack:6656, core=0
I (643) system_api: Base MAC address is not set
I (643) system_api: read default base MAC address from EFUSE
I (673) wifi:wifi firmware version: 63017e0
I (673) wifi:wifi certification version: v7.0
I (673) wifi:config NVS flash: enabled
I (673) wifi:config nano formating: disabled
I (673) wifi:Init data frame dynamic rx buffer num: 32
I (673) wifi:Init management frame dynamic rx buffer num: 32
I (683) wifi:Init management short buffer num: 32
I (683) wifi:Init dynamic tx buffer num: 32
I (693) wifi:Init static rx buffer size: 1600
I (693) wifi:Init static rx buffer num: 10
I (703) wifi:Init dynamic rx buffer num: 32
I (703) wifi_init: rx ba win: 6
I (703) wifi_init: tcpip mbox: 32
I (713) wifi_init: udp mbox: 6
I (713) wifi_init: tcp mbox: 6
I (723) wifi_init: tcp tx win: 5744
I (723) wifi_init: tcp rx win: 5744
I (723) wifi_init: tcp mss: 1440
I (733) wifi_init: WiFi IRAM OP enabled
I (733) wifi_init: WiFi RX IRAM OP enabled
I (743) example_connect: Connecting to MySSID...
I (743) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (853) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (853) wifi:enable tsf
I (853) example_connect: Waiting for IP(s)
I (2903) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3643) wifi:state: init -> auth (b0)
I (3653) wifi:state: auth -> assoc (0)
I (3653) wifi:state: assoc -> run (10)
W (3663) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (3683) wifi:connected with MySSID, aid = 5, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (3683) wifi:security: WPA2-PSK, phy: bgn, rssi: -33
I (3693) wifi:pm start, type: 1

I (3703) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5633) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6133) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (6133) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (6143) example_connect: Connected to example_connect: sta
I (6143) example_connect: - IPv4 address: 192.168.0.124
I (6153) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6163) TELE_CLI_ATTR_EXAMPLE: Init tbmch ...
I (6173) TELE_CLI_ATTR_EXAMPLE: Append telemetry: temprature...
I (6173) TELE_CLI_ATTR_EXAMPLE: Append telemetry: humidity...
I (6183) TELE_CLI_ATTR_EXAMPLE: Append client attribute: model...
I (6193) TELE_CLI_ATTR_EXAMPLE: Append client attribute: fw_version...
I (6193) TELE_CLI_ATTR_EXAMPLE: Append client attribute: setpoint...
I (6203) TELE_CLI_ATTR_EXAMPLE: Connect tbmch ...
I (6203) tb_mqtt_client_helper: connecting to mqtt://192.168.0.187...
I (6213) TELE_CLI_ATTR_EXAMPLE: connect tbmch ...
I (6213) tb_mqtt_client: MQTT_EVENT_BEFORE_CONNECT, msg_id=0, topic_len=0, data_len=0
I (6633) tb_mqtt_client: MQTT_EVENT_CONNECTED
I (6633) tb_mqtt_client: client->mqtt_handle = 0x3ffc6084
I (6633) tb_mqtt_client: sent subscribe successful, msg_id=34111, topic=v1/devices/me/attributes
I (6643) tb_mqtt_client: sent subscribe successful, msg_id=59879, topic=v1/devices/me/attributes/response/+
I (6653) tb_mqtt_client: sent subscribe successful, msg_id=29765, topic=v1/devices/me/rpc/request/+
I (6663) tb_mqtt_client: sent subscribe successful, msg_id=55729, topic=v1/devices/me/rpc/response/+
I (6673) tb_mqtt_client: sent subscribe successful, msg_id=27382, topic=v2/fw/response/+/chunk/+
I (6683) tb_mqtt_client: before call on_connected()...
I (6683) tb_mqtt_client: after call on_connected()
I (6693) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=34111
I (6703) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=59879
I (6713) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=29765
I (6713) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=55729
I (6723) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=27382
I (7333) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (7333) TELE_CLI_ATTR_EXAMPLE: Connected to thingsboard server!
I (10633) TELE_CLI_ATTR_EXAMPLE: Send telemetry: temprature, humidity
I (10633) TELE_CLI_ATTR_EXAMPLE: Get temperature (a time-series data)
I (10633) TELE_CLI_ATTR_EXAMPLE: Get humidity (a time-series data)
I (10643) tb_mqtt_client: [Telemetry][Tx] {"temprature":25,"humidity":26}
I (10653) TELE_CLI_ATTR_EXAMPLE: Send client attributes: model, fw_version, setpoint
I (10653) TELE_CLI_ATTR_EXAMPLE: Get model (a client attribute)
I (10663) TELE_CLI_ATTR_EXAMPLE: Get local F/W version (a client attribute)
I (10673) TELE_CLI_ATTR_EXAMPLE: Get setpoint (a client attribute)
I (10663) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=55579
I (10683) tb_mqtt_client: [Client-Side Attributes][Tx] {"model":"TH_001","fw_version":"1.0.1","setpoint":25.5}
I (10703) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=65152
I (16193) TELE_CLI_ATTR_EXAMPLE: Send telemetry: temprature, humidity
I (16193) TELE_CLI_ATTR_EXAMPLE: Get temperature (a time-series data)
I (16193) TELE_CLI_ATTR_EXAMPLE: Get humidity (a time-series data)
I (16203) tb_mqtt_client: [Telemetry][Tx] {"temprature":25.5,"humidity":27}
I (16213) TELE_CLI_ATTR_EXAMPLE: Send client attributes: model, fw_version, setpoint
I (16223) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=61674
I (16223) TELE_CLI_ATTR_EXAMPLE: Get model (a client attribute)
I (16223) TELE_CLI_ATTR_EXAMPLE: Get local F/W version (a client attribute)
I (16233) TELE_CLI_ATTR_EXAMPLE: Get setpoint (a client attribute)
I (16243) tb_mqtt_client: [Client-Side Attributes][Tx] {"model":"TH_001","fw_version":"1.0.1","setpoint":25.5}
I (16263) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=50986
I (21753) TELE_CLI_ATTR_EXAMPLE: Send telemetry: temprature, humidity
I (21753) TELE_CLI_ATTR_EXAMPLE: Get temperature (a time-series data)
I (21753) TELE_CLI_ATTR_EXAMPLE: Get humidity (a time-series data)
I (21763) tb_mqtt_client: [Telemetry][Tx] {"temprature":26,"humidity":28}
I (21773) TELE_CLI_ATTR_EXAMPLE: Send client attributes: model, fw_version, setpoint
I (21783) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=64837
I (21783) TELE_CLI_ATTR_EXAMPLE: Get model (a client attribute)
I (21783) TELE_CLI_ATTR_EXAMPLE: Get local F/W version (a client attribute)
I (21793) TELE_CLI_ATTR_EXAMPLE: Get setpoint (a client attribute)
I (21803) tb_mqtt_client: [Client-Side Attributes][Tx] {"model":"TH_001","fw_version":"1.0.1","setpoint":25.5}
I (21823) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=61716
I (27313) TELE_CLI_ATTR_EXAMPLE: Send telemetry: temprature, humidity
I (27313) TELE_CLI_ATTR_EXAMPLE: Get temperature (a time-series data)
I (27313) TELE_CLI_ATTR_EXAMPLE: Get humidity (a time-series data)
I (27323) tb_mqtt_client: [Telemetry][Tx] {"temprature":26.5,"humidity":29}
I (27333) TELE_CLI_ATTR_EXAMPLE: Send client attributes: model, fw_version, setpoint
I (27333) TELE_CLI_ATTR_EXAMPLE: Get model (a client attribute)
I (27343) TELE_CLI_ATTR_EXAMPLE: Get local F/W version (a client attribute)
I (27343) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=24466
I (27353) TELE_CLI_ATTR_EXAMPLE: Get setpoint (a client attribute)
I (27363) tb_mqtt_client: [Client-Side Attributes][Tx] {"model":"TH_001","fw_version":"1.0.1","setpoint":25.5}
I (27383) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=52614
I (32873) TELE_CLI_ATTR_EXAMPLE: Send telemetry: temprature, humidity
I (32873) TELE_CLI_ATTR_EXAMPLE: Get temperature (a time-series data)
I (32873) TELE_CLI_ATTR_EXAMPLE: Get humidity (a time-series data)
I (32883) tb_mqtt_client: [Telemetry][Tx] {"temprature":27,"humidity":30}
I (32893) TELE_CLI_ATTR_EXAMPLE: Send client attributes: model, fw_version, setpoint
I (32893) TELE_CLI_ATTR_EXAMPLE: Get model (a client attribute)
I (32903) TELE_CLI_ATTR_EXAMPLE: Get local F/W version (a client attribute)
I (32903) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=51643
I (32913) TELE_CLI_ATTR_EXAMPLE: Get setpoint (a client attribute)
I (32923) tb_mqtt_client: [Client-Side Attributes][Tx] {"model":"TH_001","fw_version":"1.0.1","setpoint":25.5}
I (32943) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=61266
I (38333) TELE_CLI_ATTR_EXAMPLE: Send telemetry: temprature, humidity
I (38333) TELE_CLI_ATTR_EXAMPLE: Get temperature (a time-series data)
I (38333) TELE_CLI_ATTR_EXAMPLE: Get humidity (a time-series data)
I (38343) tb_mqtt_client: [Telemetry][Tx] {"temprature":27.5,"humidity":31}
I (38353) TELE_CLI_ATTR_EXAMPLE: Send client attributes: model, fw_version, setpoint
I (38363) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=58923
I (38363) TELE_CLI_ATTR_EXAMPLE: Get model (a client attribute)
I (38363) TELE_CLI_ATTR_EXAMPLE: Get local F/W version (a client attribute)
I (38373) TELE_CLI_ATTR_EXAMPLE: Get setpoint (a client attribute)
I (38383) tb_mqtt_client: [Client-Side Attributes][Tx] {"model":"TH_001","fw_version":"1.0.1","setpoint":25.5}
I (38403) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=6811
I (43893) TELE_CLI_ATTR_EXAMPLE: Send telemetry: temprature, humidity
I (43893) TELE_CLI_ATTR_EXAMPLE: Get temperature (a time-series data)
I (43893) TELE_CLI_ATTR_EXAMPLE: Get humidity (a time-series data)
I (43903) tb_mqtt_client: [Telemetry][Tx] {"temprature":28,"humidity":32}
I (43913) TELE_CLI_ATTR_EXAMPLE: Send client attributes: model, fw_version, setpoint
I (43913) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=56451
I (43923) TELE_CLI_ATTR_EXAMPLE: Get model (a client attribute)
I (43923) TELE_CLI_ATTR_EXAMPLE: Get local F/W version (a client attribute)
I (43933) TELE_CLI_ATTR_EXAMPLE: Get setpoint (a client attribute)
I (43943) tb_mqtt_client: [Client-Side Attributes][Tx] {"model":"TH_001","fw_version":"1.0.1","setpoint":25.5}
I (43963) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=11067
I (49453) TELE_CLI_ATTR_EXAMPLE: Send telemetry: temprature, humidity
I (49453) TELE_CLI_ATTR_EXAMPLE: Get temperature (a time-series data)
I (49453) TELE_CLI_ATTR_EXAMPLE: Get humidity (a time-series data)
I (49463) tb_mqtt_client: [Telemetry][Tx] {"temprature":27.5,"humidity":31}
I (49473) TELE_CLI_ATTR_EXAMPLE: Send client attributes: model, fw_version, setpoint
I (49483) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=11894
I (49483) TELE_CLI_ATTR_EXAMPLE: Get model (a client attribute)
I (49483) TELE_CLI_ATTR_EXAMPLE: Get local F/W version (a client attribute)
I (49493) TELE_CLI_ATTR_EXAMPLE: Get setpoint (a client attribute)
I (49503) tb_mqtt_client: [Client-Side Attributes][Tx] {"model":"TH_001","fw_version":"1.0.1","setpoint":25.5}
I (49523) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=31117
I (55013) TELE_CLI_ATTR_EXAMPLE: Send telemetry: temprature, humidity
I (55013) TELE_CLI_ATTR_EXAMPLE: Get temperature (a time-series data)
I (55013) TELE_CLI_ATTR_EXAMPLE: Get humidity (a time-series data)
I (55023) tb_mqtt_client: [Telemetry][Tx] {"temprature":27,"humidity":30}
I (55033) TELE_CLI_ATTR_EXAMPLE: Send client attributes: model, fw_version, setpoint
I (55033) TELE_CLI_ATTR_EXAMPLE: Get model (a client attribute)
I (55043) TELE_CLI_ATTR_EXAMPLE: Get local F/W version (a client attribute)
I (55043) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=11883
I (55053) TELE_CLI_ATTR_EXAMPLE: Get setpoint (a client attribute)
I (55063) tb_mqtt_client: [Client-Side Attributes][Tx] {"model":"TH_001","fw_version":"1.0.1","setpoint":25.5}
I (55083) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=8388
I (60573) TELE_CLI_ATTR_EXAMPLE: Send telemetry: temprature, humidity
I (60573) TELE_CLI_ATTR_EXAMPLE: Get temperature (a time-series data)
I (60573) TELE_CLI_ATTR_EXAMPLE: Get humidity (a time-series data)
I (60583) tb_mqtt_client: [Telemetry][Tx] {"temprature":26.5,"humidity":29}
I (60593) TELE_CLI_ATTR_EXAMPLE: Send client attributes: model, fw_version, setpoint
I (60603) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=16018
I (60603) TELE_CLI_ATTR_EXAMPLE: Get model (a client attribute)
I (60603) TELE_CLI_ATTR_EXAMPLE: Get local F/W version (a client attribute)
I (60613) TELE_CLI_ATTR_EXAMPLE: Get setpoint (a client attribute)
I (60623) tb_mqtt_client: [Client-Side Attributes][Tx] {"model":"TH_001","fw_version":"1.0.1","setpoint":25.5}
I (60643) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=7408
I (61633) TELE_CLI_ATTR_EXAMPLE: Disconnect tbmch ...
I (61633) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.187...
W (61733) MQTT_CLIENT: Client asked to stop, but was not started
I (61843) TELE_CLI_ATTR_EXAMPLE: Destroy tbmch ...
I (61843) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
```

## ThingsBoard Data

`Login in ThingsBoard CE/PE` --> `Devices` --> Click your device --> `Attributes` --> `Client attributesn`, your can find `fw_version`, `model` and `setpoint`.

`Login in ThingsBoard CE/PE` --> `Devices` --> Click your device --> `Attributes` --> `Latest tememetry`, your can find `humidity` and `temprature`. Their values change over time.

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/liang-zhu-zi/thingsboard-mqttclient-basedon-espmqtt/issues) on GitHub. We will get back to you soon.
