| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Client-side RPC - ThingsBoard MQTT Client 示例

* [English Version](./README.md)

本示例实现了 Client-side RPC 相关功能：

* 发布 client-side RPC 并接收响应:
  * rpcPublishLocalTime (One-way RPC):
    * Publish: `{"method":"rpcPublishLocalTime","params":{"localTime":1664603252}}`
  * rpcGetCurrentTime (Two-way RPC):
    * Publish: `{"method":"rpcGetCurrentTime","params":{}}`
    * Receive: `{"method":"rpcGetCurrentTime","results":{"currentTime":1664603253888}}`
  * rpcLoopback (Two-way RPC):
    * Publish: `{"method":"rpcLoopback","params":{"id":9002}}`
    * Receive: `{"method":"rpcLoopback","results":{"id":9002}}`
  * rpcNotImplementedTwoway (Two-way RPC, but NO response from the server):
    * Pubish: `{"method":"rpcNotImplementedTwoway","params":{"id":4002}}`

***注意: 请在请求(request)中使用 `params`，在响应(response)中使用 `results`, 否则会出现异常!***

## 硬件需求

* 一个载有 ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC 的开发板(例如, ESP32-DevKitC, ESP-WROVER-KIT 等等)
* 一条用于供电与编程的 USB 线

参考 [Development Boards](https://www.espressif.com/en/products/devkits) 获得更多信息.

## 如何使用例子

1. 在 ThingsBoard 上使用 Rule Engine 接收 Client-side RPC 并发送响应。参考 [RPC Reply With data from Related Device](https://thingsboard.io/docs/user-guide/rule-engine-2-0/tutorials/rpc-reply-tutorial/) 和 and [Processing the client-side RPC by the platform](https://thingsboard.io/docs/user-guide/rpc/#processing-the-client-side-rpc-by-the-platform)。

   * 导入一个规则链 Rule Chain: 
         `Login in ThingsBoard CE/PE` --> `Rule chanins` --> `+` --> `Import Rule Chain` --> Drag and drop the JSON file [规则链 ESP-IDF-Thingsboard-MQTT Client-side RPC Test Rule Chains](./esp_idf_thingsboard_mqtt_client_side_rpc_test_rule_chain.json) --> `Import`。

   * 创建一个新的 Rule Chain: 
         `Login in ThingsBoard CE/PE` --> `Rule chanins` --> `+` --> `Create a new Rule Chain` --> 输入名称: `ESP-IDF-Thingsboard-MQTT Client-side RPC Test Rule Chain` --> `Add` --> Click on this new Rule chain --> the modified content is as follows --> `Applys changes` (red icon).

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

   * 修改 `Root Rule Chain`:

      `Login in ThingsBoard CE/PE` --> `Rule chanins` --> 点击 `Root Rule Chain` --> 修改以下内容 --> `Applys changes` (红色图标)。参考 [这里](https://thingsboard.io/docs/user-guide/rpc/#using-the-rule-engine)。

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

2. 获取 Access token

   `Login in ThingsBoard CE/PE` --> `Devices` --> 单击选择我的设备 --> `Details` --> Copy *my Access Token*.

3. 设定 Target (optional)

   在项目 configuration 与 build 之前, 请务必使用设置正确的芯片目标:

   ```bash
   idf.py set-target <chip_name>
   ```

4. 编译配置 menuconfig

   项目 configuration:

   ```bash
   idf.py menuconfig
   ```

   配置以下选项 ThingsBoard MQTT URI, access token, Wi-Fi SSID, password:

   ```menuconfig
   Example Configuration  --->
       (mqtt://MyThingsboardServerIP) Broker URL
       (MyDeviceToken) Access Token 
   Example Connection Configuration  --->
       [*] connect using WiFi interface
       (MySSID) WiFi SSID 
       (MyPassword) WiFi Password                  
   ```

5. 编译与运行 build, flash and monitor

   运行 `idf.py -p PORT flash monitor` 来编译、烧录、监控项目.

   (如果要退出串口监控，请输入 ``Ctrl-]``.)

   有关配置和使用 ESP-IDF 构建项目的完整步骤，请参阅 [入门指南](https://idf.espressif.com/)。

## 日志输出

```none
...
I (451) cpu_start: Starting app cpu, entry point is 0x40081188
0x40081188: call_start_cpu1 at C:/Espressif/frameworks/esp-idf-v4.4.1/components/esp_system/port/cpu_start.c:160

I (0) cpu_start: App cpu up.
I (464) cpu_start: Pro cpu start user code
I (465) cpu_start: cpu freq: 160000000
I (465) cpu_start: Application information:
I (469) cpu_start: Project name:     client_side_rpc
I (475) cpu_start: App version:      e54a4ec-dirty
I (480) cpu_start: Compile time:     Dec 17 2022 09:59:44
I (486) cpu_start: ELF file SHA256:  ff6d168d8bae87f5...
I (492) cpu_start: ESP-IDF:          v4.4.1-dirty
I (498) heap_init: Initializing. RAM available for dynamic allocation:
I (505) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (511) heap_init: At 3FFB7628 len 000289D8 (162 KiB): DRAM
I (517) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (523) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (530) heap_init: At 4009449C len 0000BB64 (46 KiB): IRAM
I (537) spi_flash: detected chip: generic
I (541) spi_flash: flash io: dio
W (545) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (559) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (569) CLIENT_RPC_EXAMPLE: [APP] Startup..
I (569) CLIENT_RPC_EXAMPLE: [APP] Free memory: 276360 bytes
I (579) CLIENT_RPC_EXAMPLE: [APP] IDF version: v4.4.1-dirty
I (619) wifi:wifi driver task: 3ffbff10, prio:23, stack:6656, core=0
I (619) system_api: Base MAC address is not set
I (619) system_api: read default base MAC address from EFUSE
I (629) wifi:wifi firmware version: 63017e0
I (629) wifi:wifi certification version: v7.0
I (629) wifi:config NVS flash: enabled
I (629) wifi:config nano formating: disabled
I (639) wifi:Init data frame dynamic rx buffer num: 32
I (639) wifi:Init management frame dynamic rx buffer num: 32
I (649) wifi:Init management short buffer num: 32
I (649) wifi:Init dynamic tx buffer num: 32
I (659) wifi:Init static rx buffer size: 1600
I (659) wifi:Init static rx buffer num: 10
I (659) wifi:Init dynamic rx buffer num: 32
I (669) wifi_init: rx ba win: 6
I (669) wifi_init: tcpip mbox: 32
I (679) wifi_init: udp mbox: 6
I (679) wifi_init: tcp mbox: 6
I (679) wifi_init: tcp tx win: 5744
I (689) wifi_init: tcp rx win: 5744
I (689) wifi_init: tcp mss: 1440
I (699) wifi_init: WiFi IRAM OP enabled
I (699) wifi_init: WiFi RX IRAM OP enabled
I (709) example_connect: Connecting to MySSID...
I (709) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (819) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (819) wifi:enable tsf
I (819) example_connect: Waiting for IP(s)
I (2869) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3609) wifi:state: init -> auth (b0)
I (3619) wifi:state: auth -> assoc (0)
I (3619) wifi:state: assoc -> run (10)
W (3639) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (3649) wifi:connected with MySSID, aid = 6, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (3649) wifi:security: WPA2-PSK, phy: bgn, rssi: -41
I (3649) wifi:pm start, type: 1

I (3699) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5609) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (8609) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (8609) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (8619) example_connect: Connected to example_connect: sta
I (8619) example_connect: - IPv4 address: 192.168.0.124
I (8629) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (8639) CLIENT_RPC_EXAMPLE: Init tbcmh ...
I (8649) CLIENT_RPC_EXAMPLE: Connect tbcmh ...
I (8649) tb_mqtt_client_helper: connecting to mqtt://192.168.0.187:1883 ...
I (8659) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=0
I (8669) CLIENT_RPC_EXAMPLE: Still NOT connected to server!
I (9669) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (9669) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc5bdc
I (9669) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (9669) tb_mqtt_client_helper: before call on_connected()...
I (9679) CLIENT_RPC_EXAMPLE: Connected to thingsboard server!
I (9689) tb_mqtt_client_helper: after call on_connected()
I (23989) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcPublishLocalTime
I (23989) CLIENT_RPC_EXAMPLE: The current date/time in HongKong is: Thu Jan  1 08:00:23 1970
I (23989) tb_mqtt_wapper: [Client-Side RPC][Tx] request_id=1 {"method":"rpcPublishLocalTime","params":{"localTime":23}}
I (24009) CLIENT_RPC_EXAMPLE: Send Client-side RPC: result=0
I (24009) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcGetCurrentTime
I (24019) clientrpc: sent subscribe successful, msg_id=59215, topic=v1/devices/me/rpc/response/+
I (24029) tb_mqtt_wapper: [Client-Side RPC][Tx] request_id=2 {"method":"rpcGetCurrentTime","params":{}}
I (24039) CLIENT_RPC_EXAMPLE: Send Client-side RPC: result=0
I (24039) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcLoopback
I (24049) tb_mqtt_wapper: [Client-Side RPC][Tx] request_id=3 {"method":"rpcLoopback","params":{"id":9001}}
I (24059) CLIENT_RPC_EXAMPLE: Send Client-side RPC: result=0
I (24069) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcNotImplementedTwoway
I (24079) tb_mqtt_wapper: [Client-Side RPC][Tx] request_id=4 {"method":"rpcNotImplementedTwoway","params":{"id":4001}}
I (24089) CLIENT_RPC_EXAMPLE: Send Client-side RPC: result=0
I (24389) tb_mqtt_wapper: [Client-Side RPC][Rx] request_id=3 {"method":"rpcLoopback","results":{"id":9001}}
I (24399) tb_mqtt_wapper: [Client-Side RPC][Rx] request_id=2 {"method":"rpcGetCurrentTime","results":{"currentTime":1671243917164}}
I (25089) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=21078
I (25089) tb_mqtt_client_helper: TBCM_EVENT_SUBSCRIBED, msg_id=59215
I (25089) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=50243
I (25099) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=15018
I (25099) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=10433
I (25109) tb_mqtt_client_helper: TBCM_EVENT_DATA
I (25119) CLIENT_RPC_EXAMPLE: Client-side RPC response: method=rpcLoopback
I (25119) tb_mqtt_client_helper: TBCM_EVENT_DATA
I (25129) CLIENT_RPC_EXAMPLE: Client-side RPC response: method=rpcGetCurrentTime
I (25139) CLIENT_RPC_EXAMPLE: The current date/time in HongKong is: Thu Jan  1 08:00:24 1970
I (25139) CLIENT_RPC_EXAMPLE: The current date/time in HongKong is: Sat Dec 17 10:25:17 2022
I (39349) clientrpc: sent unsubscribe successful, msg_id=35908, topic=v1/devices/me/rpc/response/+
I (39349) CLIENT_RPC_EXAMPLE: Client-side RPC timeout: method=rpcNotImplementedTwoway
I (39359) tb_mqtt_client_helper: TBCM_EVENT_UNSUBSCRIBED, msg_id=35908
I (40469) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcPublishLocalTime
I (40469) CLIENT_RPC_EXAMPLE: The current date/time in HongKong is: Sat Dec 17 10:25:32 2022
I (40469) tb_mqtt_wapper: [Client-Side RPC][Tx] request_id=5 {"method":"rpcPublishLocalTime","params":{"localTime":1671243932}}
I (40489) CLIENT_RPC_EXAMPLE: Send Client-side RPC: result=0
I (40489) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcGetCurrentTime
I (40499) clientrpc: sent subscribe successful, msg_id=27545, topic=v1/devices/me/rpc/response/+
I (40509) tb_mqtt_wapper: [Client-Side RPC][Tx] request_id=6 {"method":"rpcGetCurrentTime","params":{}}
I (40519) CLIENT_RPC_EXAMPLE: Send Client-side RPC: result=0
I (40519) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcLoopback
I (40529) tb_mqtt_wapper: [Client-Side RPC][Tx] request_id=7 {"method":"rpcLoopback","params":{"id":9002}}
I (40549) CLIENT_RPC_EXAMPLE: Send Client-side RPC: result=0
I (40549) CLIENT_RPC_EXAMPLE: Send Client-side RPC: method=rpcNotImplementedTwoway
I (40559) tb_mqtt_wapper: [Client-Side RPC][Rx] request_id=6 {"method":"rpcGetCurrentTime","results":{"currentTime":1671243933380}}
I (40559) tb_mqtt_wapper: [Client-Side RPC][Tx] request_id=8 {"method":"rpcNotImplementedTwoway","params":{"id":4002}}
I (40579) CLIENT_RPC_EXAMPLE: Send Client-side RPC: result=0
I (40599) tb_mqtt_wapper: [Client-Side RPC][Rx] request_id=7 {"method":"rpcLoopback","results":{"id":9002}}
I (41589) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=26611
I (41589) tb_mqtt_client_helper: TBCM_EVENT_SUBSCRIBED, msg_id=27545
I (41589) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=14523
I (41599) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=54207
I (41599) tb_mqtt_client_helper: TBCM_EVENT_DATA
I (41609) CLIENT_RPC_EXAMPLE: Client-side RPC response: method=rpcGetCurrentTime
I (41619) CLIENT_RPC_EXAMPLE: The current date/time in HongKong is: Sat Dec 17 10:25:33 2022
I (41629) CLIENT_RPC_EXAMPLE: The current date/time in HongKong is: Sat Dec 17 10:25:33 2022
I (41629) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=62696
I (41639) tb_mqtt_client_helper: TBCM_EVENT_DATA
I (41649) CLIENT_RPC_EXAMPLE: Client-side RPC response: method=rpcLoopback
I (52549) CLIENT_RPC_EXAMPLE: Disconnect tbcmh ...
I (52549) tb_mqtt_client_helper: disconnecting from mqtt://192.168.0.187:1883 ...
I (52649) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (52659) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (52659) MQTT_CLIENT: Client asked to stop, but was not started
I (52759) CLIENT_RPC_EXAMPLE: Client-side RPC timeout: method=rpcNotImplementedTwoway
I (52759) CLIENT_RPC_EXAMPLE: Destroy tbcmh ...
I (52759) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
```

## ThingsBoard CE/PE

**注意:** 测试完成之后，请把 `Root Rule Chain` 恢复到原来的状态!


## 故障排除

如有任何技术问题，请打开 [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues)。 我们会尽快回复您。
