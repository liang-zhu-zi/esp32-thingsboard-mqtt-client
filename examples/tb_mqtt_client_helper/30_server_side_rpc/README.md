| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Server-side RPC - ThingsBoard MQTT Client Example

* [中文版](./README_CN.md)

This example is based on [`$ESP-IDF\examples\protocols\mqtt\tcp`](https://github.com/espressif/esp-idf/tree/master/examples/protocols/mqtt/tcp).

This example implements server-side RPC related functions:

* Subscribe to and receive server-side RPC from the server:
  * rpcChangeSetpoint
  * rpcQuerySetpoint

## Hardware Required

* A development board with ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

## How to Use Example

1. Get a device access token

   `Login in ThingsBoard CE/PE` --> `Devices` --> Click my device --> `Details` --> `Copy Access Token`.

1. `Sending server-side RPC` `Using the Rule Engine` in ThingsBoard

   Referece [here](https://thingsboard.io/docs/user-guide/rpc/#using-the-rule-engine).

   * Modify Root Rule Chain:

      ![image](./Root-Rule-Chain_4_Server-RPC.png)

     * Generator rpcChangeSetpoint:
       * Name: rpcChangeSetpoint
       * Type: Action - generator
       * Period in seconds: 20
       * Originator Type: Device
       * Device: *My Device*
       * Generate:

         ```json
         var msg = { method: "rpcChangeSetpoint", params: { setpoint: 26.0 } };

         var metadata = {
            expirationTime: new Date().getTime() + 60000,
            oneway: true,
            persistent: false
         };
         var msgType = "RPC_CALL_FROM_SERVER_TO_DEVICE";

         return { msg: msg, metadata: metadata, msgType: msgType };
         ```

     * Generator rpcQuerySetpoint:
       * Name: rpcQuerySetpoint
       * Type: Action - generator
       * Period in seconds: 20
       * Originator Type: Device
       * Device: *My Device*
       * Generate:

         ```json
         var msg = { method: "rpcQuerySetpoint", params: { } };
         var metadata = { 
             expirationTime: new Date().getTime() + 60000,
             oneway: false,
             persistent: false
         };
         var msgType = "RPC_CALL_FROM_SERVER_TO_DEVICE";

         return { msg: msg, metadata: metadata, msgType: msgType };
         ```

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
I (459) cpu_start: Pro cpu start user code
I (459) cpu_start: cpu freq: 160000000
I (459) cpu_start: Application information:
I (463) cpu_start: Project name:     server_side_rpc
I (469) cpu_start: App version:      a94c359-dirty
I (474) cpu_start: Compile time:     Sep 24 2022 16:23:27
I (480) cpu_start: ELF file SHA256:  a10e71b77ab48724...
I (486) cpu_start: ESP-IDF:          v4.4.1-dirty
I (492) heap_init: Initializing. RAM available for dynamic allocation:
I (499) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (505) heap_init: At 3FFB7620 len 000289E0 (162 KiB): DRAM
I (511) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (518) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (524) heap_init: At 4009449C len 0000BB64 (46 KiB): IRAM
I (531) spi_flash: detected chip: generic
I (535) spi_flash: flash io: dio
W (539) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (553) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (563) SERVER_RPC_EXAMPLE: [APP] Startup..
I (563) SERVER_RPC_EXAMPLE: [APP] Free memory: 276368 bytes
I (573) SERVER_RPC_EXAMPLE: [APP] IDF version: v4.4.1-dirty
I (643) wifi:wifi driver task: 3ffc0158, prio:23, stack:6656, core=0
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
I (3663) wifi:state: assoc -> run (10)
W (3673) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (3683) wifi:connected with MySSID, aid = 1, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (3683) wifi:security: WPA2-PSK, phy: bgn, rssi: -51
I (3693) wifi:pm start, type: 1

I (3763) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5633) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6133) esp_netif_handlers: example_connect: sta ip: 192.168.0.126, mask: 255.255.255.0, gw: 192.168.0.1
I (6133) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.126
I (6143) example_connect: Connected to example_connect: sta
I (6143) example_connect: - IPv4 address: 192.168.0.126
I (6153) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6163) SERVER_RPC_EXAMPLE: Init tbmch ...
I (6163) SERVER_RPC_EXAMPLE: Append server RPC: rpcChangeSetpoint...
I (6173) SERVER_RPC_EXAMPLE: Append server RPC: rpcQuerySetpoint...
I (6183) SERVER_RPC_EXAMPLE: Connect tbmch ...
I (6183) tb_mqtt_client_helper: connecting to mqtt://192.168.0.186...
I (6193) SERVER_RPC_EXAMPLE: connect tbmch ...
I (6193) tb_mqtt_client: MQTT_EVENT_BEFORE_CONNECT, msg_id=0, topic_len=0, data_len=0
I (6313) SERVER_RPC_EXAMPLE: Still NOT connected to server!
I (6813) tb_mqtt_client: MQTT_EVENT_CONNECTED
I (6813) tb_mqtt_client: client->mqtt_handle = 0x3ffc60a0
I (6813) tb_mqtt_client: sent subscribe successful, msg_id=37523, topic=v1/devices/me/attributes
I (6823) tb_mqtt_client: sent subscribe successful, msg_id=28219, topic=v1/devices/me/attributes/response/+
I (6833) tb_mqtt_client: sent subscribe successful, msg_id=45575, topic=v1/devices/me/rpc/request/+
I (6843) tb_mqtt_client: sent subscribe successful, msg_id=6168, topic=v1/devices/me/rpc/response/+
I (6853) tb_mqtt_client: sent subscribe successful, msg_id=8569, topic=v2/fw/response/+/chunk/+
I (6853) tb_mqtt_client: before call on_connected()...
I (6863) tb_mqtt_client: after call on_connected()
I (6873) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=37523
I (6883) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=28219
I (6883) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=45575
I (6893) tb_mqtt_client: MQTT_EVENT_DATA
I (6893) tb_mqtt_client: [Server-Side RPC][Rx] RequestID=347 Payload={"method":"rpcChangeSetpoint","params":{"setpoint":26}}
I (6913) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=6168
I (6913) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=8569
I (7313) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (7313) SERVER_RPC_EXAMPLE: Connected to thingsboard server!
I (7313) SERVER_RPC_EXAMPLE: Receive server RPC request: rpcChangeSetpoint
I (7323) SERVER_RPC_EXAMPLE: Receive setpoint = 26.000000
I (13293) tb_mqtt_client: MQTT_EVENT_DATA
I (13293) tb_mqtt_client: [Server-Side RPC][Rx] RequestID=348 Payload={"method":"rpcQuerySetpoint","params":{}}
I (13823) SERVER_RPC_EXAMPLE: Receive server RPC request: rpcQuerySetpoint
I (13823) tb_mqtt_client: [Server-Side RPC][Tx] RequestID=348 Payload={"setpoint":26}
I (13843) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=14010
I (18213) tb_mqtt_client: MQTT_EVENT_DATA
I (18213) tb_mqtt_client: [Server-Side RPC][Rx] RequestID=349 Payload={"method":"rpcChangeSetpoint","params":{"setpoint":26}}
I (18223) SERVER_RPC_EXAMPLE: Receive server RPC request: rpcChangeSetpoint
I (18233) SERVER_RPC_EXAMPLE: Receive setpoint = 26.000000
I (28033) SERVER_RPC_EXAMPLE: Disconnect tbmch ...
I (28033) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.186...
W (28143) MQTT_CLIENT: Client asked to stop, but was not started
I (28243) SERVER_RPC_EXAMPLE: Destroy tbmch ...
I (28243) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
```

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues) on GitHub. We will get back to you soon.
