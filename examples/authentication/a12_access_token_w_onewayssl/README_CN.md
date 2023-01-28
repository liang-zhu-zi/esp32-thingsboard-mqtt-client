| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# # Access Token based authentication - MQTTS (MQTT over SSL)

* [English Version](./README.md)

本示例实现了以下功能：

* 基于 Access Token 的认证 - MQTTS (有 SSL)
* 发送 telemetry: temprature, humidity

流程参考 [这里](https://thingsboard.io/docs/user-guide/access-token/#mqtts-mqtt-over-ssl).

## 硬件需求

* 一个载有 ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC 的开发板(例如, ESP32-DevKitC, ESP-WROVER-KIT 等等)
* 一条用于供电与编程的 USB 线

参考 [Development Boards](https://www.espressif.com/en/products/devkits) 获得更多信息.

## 如何使用例子

1. ThingsBoard CE/PE SSL configuration using PEM certificates file
   
   Refer [here](https://thingsboard.io/docs/user-guide/mqtt-over-ssl/#ssl-configuration-using-pem-certificates-file).

   Configure the following environment variables via [configuration](https://thingsboard.io/docs/user-guide/install/config/) file, docker-compose or kubernetes scripts. We will use thingsboard.conf for example:

   If ThingsBoard is installed on Linux as a monolithic application, you may specify the environment variables in the thingsboard.conf file:

   ```bash
   sudo nano /usr/share/thingsboard/conf/thingsboard.conf
   ```

   ```bash
   ...
   export MQTT_SSL_ENABLED=true
   export MQTT_SSL_CREDENTIALS_TYPE=PEM
   export MQTT_SSL_PEM_CERT=server.pem
   export MQTT_SSL_PEM_KEY=server_key.pem
   export MQTT_SSL_PEM_KEY_PASSWORD=secret
   ...
   ```

   where:

   * MQTT_SSL_ENABLED - Enable/disable SSL support;
   * MQTT_SSL_CREDENTIALS_TYPE - Server credentials type. PEM - pem certificate file; KEYSTORE - java keystore;
   * MQTT_SSL_PEM_CERT - Path to the server certificate file. Holds server certificate or certificate chain, may also include server private key;
   * MQTT_SSL_PEM_KEY - Path to the server certificate private key file. Optional by default. Required if the private key is not present in server certificate file;
   * MQTT_SSL_PEM_KEY_PASSWORD - Optional server certificate private key password.
   * After completing the setup, start or restart the ThingsBoard server.

1. Self-signed certificates generation - PEM certificate file

   See [PEM certificate file](https://thingsboard.io/docs/user-guide/mqtt-over-ssl/#pem-certificate-file).

   Use instructions below to generate your own certificate files. Useful for tests, but time consuming and not recommended for production.

   Note This step requires Linux based OS with openssl installed.

   To generate a server self-signed PEM certificate and private key, use the following command:

   ```bash
   openssl ecparam -out server_key.pem -name secp256r1 -genkey
   openssl req -new -key server_key.pem -x509 -nodes -days 365 -out server.pem 
   ```

   You can also add -nodes (short for no DES) if you don’t want to protect your private key with a passphrase. Otherwise, it will prompt you for “at least a 4 character” password.

   The **days** parameter (365) you can replace with any number to affect the expiration date. It will then prompt you for things like “Country Name”, but you can just hit Enter and accept the defaults.

   Add -subj `/CN=localhost` to suppress questions about the contents of the certificate (replace localhost with your desired domain).

   Self-signed certificates are not validated with any third party unless you import them to the browsers previously. If you need more security, you should use a certificate signed by a certificate authority (CA).

   Make sure the certificate files are reachable by ThingsBoard process:

   * Linux: use `/etc/thingsboard/conf` folder. Make sure the files - server_key.pem & server.pem have same permissions as thingsboard.conf; Use relative file path, e.g. keystore.p12;

1. 复制 `server.pem` 关重命名为 `main/mqtt_thingsboard_server_cert.pem`;

1. 获取 Access token

   `Login in ThingsBoard CE/PE as tenant` --> `Devices` --> 单击选择我的设备 --> `Copy access token`.

   ![image](../../.docs/images//copy-access-token/copy-access-token-1.png)

1. 设定 Target (optional)

   在项目 configuration 与 build 之前, 请务必使用设置正确的芯片目标:

   ```bash
   idf.py set-target <chip_name>
   ```

1. 编译配置 menuconfig

   项目 configuration:

   ```bash
   idf.py menuconfig
   ```

   配置以下选项 ThingsBoard MQTT URI, access token, Wi-Fi SSID, password:

   ```menuconfig
   Example Configuration  --->
      (mqtt://MyThingsboardServerIP) Broker URL
      (8883) Port
      (MyDeviceToken) Access Token
      [*] Skip any validation of server certificate CN field
   Example Connection Configuration  --->
      [*] connect using WiFi interface
      (MySSID) WiFi SSID 
      (MyPassword) WiFi Password                  
   ```

1. 编译与运行 build, flash and monitor

   运行 `idf.py -p PORT flash monitor` 来编译、烧录、监控项目.

   (如果要退出串口监控，请输入 ``Ctrl-]``.)

   有关配置和使用 ESP-IDF 构建项目的完整步骤，请参阅 [入门指南](https://idf.espressif.com/)。


## 日志输出

```none
...
0x400811a8: call_start_cpu1 at C:/Espressif/frameworks/esp-idf-v4.4.3/components/esp_system/port/cpu_start.c:148

I (0) cpu_start: App cpu up.
I (466) cpu_start: Pro cpu start user code
I (466) cpu_start: cpu freq: 160000000
I (466) cpu_start: Application information:
I (470) cpu_start: Project name:     access_token_w_onewayssl
I (477) cpu_start: App version:      1429df0-dirty
I (482) cpu_start: Compile time:     Jan  2 2023 11:12:17
I (488) cpu_start: ELF file SHA256:  ce05d9a4ee6520d9...
I (494) cpu_start: ESP-IDF:          v4.4.3-dirty
I (500) heap_init: Initializing. RAM available for dynamic allocation:
I (507) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (513) heap_init: At 3FFB7650 len 000289B0 (162 KiB): DRAM
I (519) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (526) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (532) heap_init: At 40094A04 len 0000B5FC (45 KiB): IRAM
I (540) spi_flash: detected chip: generic
I (543) spi_flash: flash io: dio
W (547) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (561) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (571) EXAM_ACCESS_TOKEN_W_SSL: [APP] Startup..
I (581) EXAM_ACCESS_TOKEN_W_SSL: [APP] Free memory: 275980 bytes
I (581) EXAM_ACCESS_TOKEN_W_SSL: [APP] IDF version: v4.4.3-dirty
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
I (3971) wifi:state: init -> auth (b0)
I (3981) wifi:state: auth -> assoc (0)
I (3981) wifi:state: assoc -> run (10)
W (3991) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (4011) wifi:connected with Duoman, aid = 4, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (4011) wifi:security: WPA2-PSK, phy: bgn, rssi: -43
I (4011) wifi:pm start, type: 1

I (4031) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5611) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6111) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (6111) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (6121) example_connect: Connected to example_connect: sta
I (6121) example_connect: - IPv4 address: 192.168.0.124
I (6131) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6141) EXAM_ACCESS_TOKEN_W_SSL: Init tbcmh ...
I (6151) EXAM_ACCESS_TOKEN_W_SSL: Connect tbcmh ...
I (6151) tb_mqtt_client_helper: connecting to mqtts://192.168.0.210:8883 ...
I (6161) EXAM_ACCESS_TOKEN_W_SSL: connect tbcmh ...
I (6161) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (6171) EXAM_ACCESS_TOKEN_W_SSL: Still NOT connected to server!
I (7301) EXAM_ACCESS_TOKEN_W_SSL: Still NOT connected to server!
I (8411) EXAM_ACCESS_TOKEN_W_SSL: Still NOT connected to server!
I (9411) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (9411) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc5d04
I (9411) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (9411) tb_mqtt_client_helper: before call on_connected()...
I (9421) EXAM_ACCESS_TOKEN_W_SSL: Connected to thingsboard server!
I (9431) tb_mqtt_client_helper: after call on_connected()
I (10531) EXAM_ACCESS_TOKEN_W_SSL: Send telemetry: temprature, humidity
I (10531) EXAM_ACCESS_TOKEN_W_SSL: Get temperature (a time-series data)
I (10531) EXAM_ACCESS_TOKEN_W_SSL: Get humidity (a time-series data)
I (10541) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":25,"humidity":26}
I (11551) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=36584
I (15951) EXAM_ACCESS_TOKEN_W_SSL: Send telemetry: temprature, humidity
I (15951) EXAM_ACCESS_TOKEN_W_SSL: Get temperature (a time-series data)
I (15951) EXAM_ACCESS_TOKEN_W_SSL: Get humidity (a time-series data)
I (15961) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":25.5,"humidity":27}
I (16971) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=10466
I (21371) EXAM_ACCESS_TOKEN_W_SSL: Send telemetry: temprature, humidity
I (21371) EXAM_ACCESS_TOKEN_W_SSL: Get temperature (a time-series data)
I (21371) EXAM_ACCESS_TOKEN_W_SSL: Get humidity (a time-series data)
I (21381) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":26,"humidity":28}
I (22391) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=61812
I (26791) EXAM_ACCESS_TOKEN_W_SSL: Send telemetry: temprature, humidity
I (26791) EXAM_ACCESS_TOKEN_W_SSL: Get temperature (a time-series data)
I (26791) EXAM_ACCESS_TOKEN_W_SSL: Get humidity (a time-series data)
I (26801) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":26.5,"humidity":29}
I (27811) EXAM_ACCESS_TOKEN_W_SSL: Disconnect tbcmh ...
I (27811) tb_mqtt_client_helper: disconnecting from mqtts://192.168.0.210:8883 ...
I (27811) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=43973
I (27921) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (27931) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (27931) MQTT_CLIENT: Client asked to stop, but was not started
I (28031) EXAM_ACCESS_TOKEN_W_SSL: Destroy tbcmh ...
I (28031) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
```

## ThingsBoard 输出

* 在 ThingsBoard 上查看最新的 telemetry data.

   Login in ThingsBoard CE/PE --> `Devices` --> 单击选择我的设备 --> `Latest tememetry`, 你能发现 `humidity` 和 `temprature`. 在本示例运行时，这两个值会随时变化。

   ![image](../../.docs/images/check-latest-telemetry/check-latest-telemetry-1.png)

## 故障排除

如有任何技术问题，请打开 [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues)。 我们会尽快回复您。
