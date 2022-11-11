| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Client-side RPC - ThingsBoard MQTT Client Example

* [中文版](./README_CN.md)

This example is based on [`$ESP-IDF\examples\protocols\mqtt\tcp`](https://github.com/espressif/esp-idf/tree/master/examples/protocols/mqtt/tcp).

This example implements server-side RPC related functions:

* Publish client-side RPC to the server and receive the response:
  * rpcPublishLocalTime (One-way RPC):
    * Publish: `{"method":"rpcPublishLocalTime","params":{"localTime":1664603252}}`
  * rpcGetCurrentTime (Two-way RPC):
    * Publish: `{"method":"rpcGetCurrentTime","params":{}}`
    * Receive: `{"method":"rpcGetCurrentTime","results":{"currentTime":1664603253888}}
  * rpcLoopback (Two-way RPC):
    * Publish: `{"method":"rpcLoopback","params":{"id":9002}}`
    * Receive: `{"method":"rpcLoopback","results":{"id":9002}}`
  * rpcNotImplementedTwoway (Two-way RPC, but NO response from the server):
    * Pubish: `{"method":"rpcNotImplementedTwoway","params":{"id":4002}}`

***Note: Please use `params` in a request and `results` in a response, otherwise you will get an exception !***

## Hardware Required

* A development board with ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

## How to Use Example

1. Get a device access token

   `Login in ThingsBoard CE/PE` --> `Devices` --> Click my device --> `Details` --> `Copy Access Token`.

1. `RPC Reply with the Rule Engine` in ThingsBoard

   Referece [RPC Reply With data from Related Device](https://thingsboard.io/docs/user-guide/rule-engine-2-0/tutorials/rpc-reply-tutorial/)

   * Create a new Rule Chain: `ESP-IDF-Thingsboard-MQTT Client-side RPC Test Rule Chain`

       ![image](./ESP-IDF-Thingsboard-MQTT_Client-side_RPC_Test_Rule_Chain.png)

      * filter rpcPublishLocalTime
        * Name: filter rpcPublishLocalTime
        * Type: Filter - script
        * Code:

           ```json
           return msg.method === 'rpcPublishLocalTime';
           ```

      * filter rpcGetCurrentTime
        * Name: filter rpcGetCurrentTime
        * Type: Filter - script
        * Code:

           ```json
           return msg.method === 'rpcGetCurrentTime'; 
           ```

      * filter rpcLoopback
        * Name: filter rpcLoopback
        * Type: Filter - script
        * Code:

           ```json
           return msg.method === 'rpcLoopback'; 
           ```

      * filter rpcNotImplementedTwoway
        * Name: filter rpcNotImplementedTwoway
        * Type: Filter - script
        * Code:

           ```json
           return msg.method === 'rpcNotImplementedTwoway'; 
           ```

      * build resp rpcGetCurrentTime
        * Name: build resp rpcGetCurrentTime
        * Type: Transformation - script
        * Code:

           ```json
           var rpcResponse = {
              method: msg.method, //rpcGetCurrentTime
              results: {
                 currentTime: new Date().getTime()
              }
           };

           return {
              msg: rpcResponse,
              metadata: metadata,
              msgType: msgType
           };
           ```

      * build resp rpcLoopback
        * Name: build resp rpcLoopback
        * Type: Transformation - script
        * Code:

           ```json
           var rpcResponse = {
               method: msg.method, //rpcLoopback
               results: msg.params
           };

           return {
               msg: rpcResponse,
               metadata: metadata,
               msgType: msgType
           };
           ```

   * Modify Root Rule Chain:

      ![image](./Root_Rule_Chain.png)

      * filter client-side RPC
        * Name: filter client-side RPC
        * Type: Filter - script
        * Code:

           ```json
           return (msg.method === "rpcPublishLocalTime")
            || (msg.method === "rpcGetCurrentTime")
            || (msg.method === "rpcLoopback")
            || (msg.method === "rpcNotImplementedTwoway");
           ```

      * Log Other RPC from Device
        * Name: Log Other RPC from Device
        * Type: Action - log
        * Code:

           ```json
           return 'Unexpected RPC call request message:\n' + JSON.stringify(msg) + '\metadata:\n' + JSON.stringify(metadata);
           ```

      * Rule Chain ESP-IDF-TB-MQTT Client RPC
        * Name: Rule Chain ESP-IDF-TB-MQTT Client RPC
        * Type: Flow - rule chain
        * Rule Chain: ESP-IDF-Thingsboard-MQTT Client-side RPC Test Rule Chain

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
I (458) cpu_start: cpu freq: 160000000
I (458) cpu_start: Application information:
I (462) cpu_start: Project name:     client_side_rpc
I (468) cpu_start: App version:      4fdceab-dirty
I (473) cpu_start: Compile time:     Sep 30 2022 10:36:38
I (479) cpu_start: ELF file SHA256:  5e7b2ad0a4e36a29...
I (485) cpu_start: ESP-IDF:          v4.4.1-dirty
I (491) heap_init: Initializing. RAM available for dynamic allocation:
I (498) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (504) heap_init: At 3FFB7628 len 000289D8 (162 KiB): DRAM
I (510) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (516) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (523) heap_init: At 4009449C len 0000BB64 (46 KiB): IRAM
I (530) spi_flash: detected chip: generic
I (534) spi_flash: flash io: dio
W (538) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (552) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (562) CLIENT_RPC_EXAMPLE: [APP] Startup..
I (572) CLIENT_RPC_EXAMPLE: [APP] Free memory: 276360 bytes
I (572) CLIENT_RPC_EXAMPLE: [APP] IDF version: v4.4.1-dirty
I (642) wifi:wifi driver task: 3ffc0160, prio:23, stack:6656, core=0
I (642) system_api: Base MAC address is not set
I (642) system_api: read default base MAC address from EFUSE
I (672) wifi:wifi firmware version: 63017e0
I (672) wifi:wifi certification version: v7.0
I (672) wifi:config NVS flash: enabled
I (672) wifi:config nano formating: disabled
I (672) wifi:Init data frame dynamic rx buffer num: 32
I (682) wifi:Init management frame dynamic rx buffer num: 32
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
I (862) example_connect: Waiting for IP(s)
I (2902) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3652) wifi:state: init -> auth (b0)
I (3652) wifi:state: auth -> assoc (0)
I (3662) wifi:state: assoc -> run (10)
W (3672) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (3692) wifi:connected with MySSID, aid = 4, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (3692) wifi:security: WPA2-PSK, phy: bgn, rssi: -27
I (3692) wifi:pm start, type: 1

I (3792) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5632) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6132) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (6132) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (6142) example_connect: Connected to example_connect: sta
I (6142) example_connect: - IPv4 address: 192.168.0.124
I (6152) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6162) CLIENT_RPC_EXAMPLE: Init tbmch ...
I (6172) CLIENT_RPC_EXAMPLE: Connect tbmch ...
I (6172) tb_mqtt_client_helper: connecting to mqtt://192.168.0.186...
I (6182) tb_mqtt_client: MQTT_EVENT_BEFORE_CONNECT, msg_id=0, topic_len=0, data_len=0
I (6282) CLIENT_RPC_EXAMPLE: Still NOT connected to server!
I (6462) tb_mqtt_client: MQTT_EVENT_CONNECTED
I (6462) tb_mqtt_client: client->mqtt_handle = 0x3ffc5fe0
I (6472) tb_mqtt_client: sent subscribe successful, msg_id=39866, topic=v1/devices/me/attributes
I (6472) tb_mqtt_client: sent subscribe successful, msg_id=30290, topic=v1/devices/me/attributes/response/+
I (6482) tb_mqtt_client: sent subscribe successful, msg_id=57341, topic=v1/devices/me/rpc/request/+
I (6492) tb_mqtt_client: sent subscribe successful, msg_id=13473, topic=v1/devices/me/rpc/response/+
I (6502) tb_mqtt_client: sent subscribe successful, msg_id=26925, topic=v2/fw/response/+/chunk/+
I (6512) tb_mqtt_client: before call on_connected()...
I (6522) tb_mqtt_client: after call on_connected()
I (6532) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=39866
I (6532) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=30290
I (6542) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=57341
I (6542) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=13473
I (6552) tb_mqtt_client: MQTT_EVENT_SUBSCRIBED, msg_id=26925
I (7282) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (7282) CLIENT_RPC_EXAMPLE: Connected to thingsboard server!
I (21582) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcPublishLocalTime
I (21582) CLIENT_RPC_EXAMPLE: The current date/time in HongKong is: Thu Jan  1 08:00:21 1970
I (21582) tb_mqtt_client: Don't append to request list if no response & no timeout!
I (21592) tb_mqtt_client: [Client-Side RPC][Tx] RequestID=1 {"method":"rpcPublishLocalTime","params":{"localTime":21}}
I (21612) CLIENT_RPC_EXAMPLE: Send Client-side RPC: request_id=1
I (21612) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcGetCurrentTime
I (21622) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=20374
I (21622) tb_mqtt_client: [Client-Side RPC][Tx] RequestID=2 {"method":"rpcGetCurrentTime","params":{}}
I (21642) CLIENT_RPC_EXAMPLE: Send Client-side RPC: request_id=2
I (21642) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcLoopback
I (21642) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=9477
I (21652) tb_mqtt_client: [Client-Side RPC][Tx] RequestID=3 {"method":"rpcLoopback","params":{"id":9001}}
I (21672) CLIENT_RPC_EXAMPLE: Send Client-side RPC: request_id=3
I (21672) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcNotImplementedTwoway
I (21672) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=23000
I (21692) tb_mqtt_client: [Client-Side RPC][Tx] RequestID=4 {"method":"rpcNotImplementedTwoway","params":{"id":4001}}
I (21702) CLIENT_RPC_EXAMPLE: Send Client-side RPC: request_id=4
I (21702) tb_mqtt_client: MQTT_EVENT_DATA
I (21712) tb_mqtt_client: [Client-Side RPC][Rx] RequestID=2 {"method":"rpcGetCurrentTime","results":{"currentTime":1664603237404}}
I (21722) tb_mqtt_client: MQTT_EVENT_DATA
I (21722) tb_mqtt_client: [Client-Side RPC][Rx] RequestID=3 {"method":"rpcLoopback","results":{"id":9001}}
I (21742) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=2286
I (22742) CLIENT_RPC_EXAMPLE: Client-side RPC response: request_id=2, method=rpcGetCurrentTime
I (22742) CLIENT_RPC_EXAMPLE: The current date/time in HongKong is: Thu Jan  1 08:00:22 1970
I (22752) CLIENT_RPC_EXAMPLE: The current date/time in HongKong is: Sat Oct  1 13:47:17 2022
I (22762) CLIENT_RPC_EXAMPLE: Client-side RPC response: request_id=3, method=rpcLoopback
I (38062) CLIENT_RPC_EXAMPLE: Client-side RPC timeout: request_id=4, method=rpcNotImplementedTwoway
I (38062) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcPublishLocalTime
I (38062) CLIENT_RPC_EXAMPLE: The current date/time in HongKong is: Sat Oct  1 13:47:32 2022
I (38072) tb_mqtt_client: Don't append to request list if no response & no timeout!
I (38082) tb_mqtt_client: [Client-Side RPC][Tx] RequestID=5 {"method":"rpcPublishLocalTime","params":{"localTime":1664603252}}
I (38102) CLIENT_RPC_EXAMPLE: Send Client-side RPC: request_id=5
I (38102) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcGetCurrentTime
I (38112) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=35834
I (38112) tb_mqtt_client: [Client-Side RPC][Tx] RequestID=6 {"method":"rpcGetCurrentTime","params":{}}
I (38132) CLIENT_RPC_EXAMPLE: Send Client-side RPC: request_id=6
I (38132) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcLoopback
I (38142) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=29720
I (38152) tb_mqtt_client: [Client-Side RPC][Tx] RequestID=7 {"method":"rpcLoopback","params":{"id":9002}}
I (38162) CLIENT_RPC_EXAMPLE: Send Client-side RPC: request_id=7
I (38162) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcNotImplementedTwoway
I (38172) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=43033
I (38182) tb_mqtt_client: [Client-Side RPC][Tx] RequestID=8 {"method":"rpcNotImplementedTwoway","params":{"id":4002}}
I (38182) tb_mqtt_client: MQTT_EVENT_DATA
I (38192) tb_mqtt_client: [Client-Side RPC][Rx] RequestID=6 {"method":"rpcGetCurrentTime","results":{"currentTime":1664603253888}}
I (38212) tb_mqtt_client: MQTT_EVENT_DATA
I (38212) tb_mqtt_client: [Client-Side RPC][Rx] RequestID=7 {"method":"rpcLoopback","results":{"id":9002}}
I (38222) tb_mqtt_client: MQTT_EVENT_PUBLISHED, msg_id=2493
I (38232) CLIENT_RPC_EXAMPLE: Send Client-side RPC: request_id=8
I (39232) CLIENT_RPC_EXAMPLE: Client-side RPC response: request_id=6, method=rpcGetCurrentTime
I (39232) CLIENT_RPC_EXAMPLE: The current date/time in HongKong is: Sat Oct  1 13:47:33 2022
I (39242) CLIENT_RPC_EXAMPLE: The current date/time in HongKong is: Sat Oct  1 13:47:33 2022
I (39242) CLIENT_RPC_EXAMPLE: Client-side RPC response: request_id=7, method=rpcLoopback
I (50152) CLIENT_RPC_EXAMPLE: Disconnect tbmch ...
I (50152) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.186...
I (50252) tb_mqtt_client: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (50262) tb_mqtt_client: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (50262) MQTT_CLIENT: Client asked to stop, but was not started
I (50262) CLIENT_RPC_EXAMPLE: Client-side RPC timeout: request_id=8, method=rpcNotImplementedTwoway
I (50372) CLIENT_RPC_EXAMPLE: Destroy tbmch ...
I (50372) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
```

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues) on GitHub. We will get back to you soon.
