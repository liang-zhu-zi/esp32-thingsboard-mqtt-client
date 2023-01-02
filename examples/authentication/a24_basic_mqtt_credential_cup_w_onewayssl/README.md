| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Basic MQTT authentication - Authentication based on Client ID, Username and Password, MQTTS (MQTT over TLS)

* [中文版](./README_CN.md)

This example is based on [`$ESP-IDF\examples\protocols\mqtt\tcp`](https://github.com/espressif/esp-idf/tree/master/examples/protocols/mqtt/tcp).

This example implements some functions:

* Basic MQTT authentication - Authentication based on Client ID, Username and Password, MQTTS (MQTT over TLS)
* Publish telemetry: temprature, humidity

Refer [here](https://thingsboard.io/docs/user-guide/basic-mqtt/#mqtts-mqtt-over-tls).

## Hardware Required

* A development board with ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

## How to Use Example

1. ThingsBoard CE/PE SSL configuration using PEM certificates file
   Refer [here](https://thingsboard.io/docs/user-guide/mqtt-over-ssl/#ssl-configuration-using-pem-certificates-file)

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

   See [PEM certificate file](https://thingsboard.io/docs/user-guide/mqtt-over-ssl/#pem-certificate-file)

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

1. Copy `server.pem` and rename it to `main\mqtt_thingsboard_server_cert.pem`;

1. Get a device token

   `Login in ThingsBoard CE/PE` --> `Devices` --> Click my device --> `Details` --> `Copy Access Token`.

   ![image](./basic_mqtt_credential_cup_w_onewayssl_1.png)

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
      (8883) Port
      (MY_CLIENT_ID) Client ID
      (MY_USER_NAME) User name
      (MY_PASSWORD) Password
      [*] Skip any validation of server certificate CN field
   Example Connection Configuration  --->
       [*] connect using WiFi interface
       (MySSID) WiFi SSID 
       (MyPassword) WiFi Password                  
   ```

1. build, flash and monitor

   Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

   (To exit the serial monitor, type ``Ctrl-]``.)

   See the [Getting Started Guide](https://idf.espressif.com/) for full steps to configure and use ESP-IDF to build projects.

1. Check out the latest Telemetry data on ThingsBoard

   * `Login in ThingsBoard CE/PE` --> `Devices` --> click my device --> `Latest telemetry` --> Check out the latest Telemetry data.

   ![image](./basic_mqtt_credential_cup_w_onewayssl_2.png)

## Example Output

```none
...
0x400811a8: call_start_cpu1 at C:/Espressif/frameworks/esp-idf-v4.4.3/components/esp_system/port/cpu_start.c:148

I (0) cpu_start: App cpu up.
I (466) cpu_start: Pro cpu start user code
I (466) cpu_start: cpu freq: 160000000
I (466) cpu_start: Application information:
I (471) cpu_start: Project name:     basic_mqtt_credential_cup_w_one
I (478) cpu_start: App version:      6c8f292-dirty
I (483) cpu_start: Compile time:     Jan  2 2023 18:57:55
I (489) cpu_start: ELF file SHA256:  018fb30883514c13...
I (495) cpu_start: ESP-IDF:          v4.4.3-dirty
I (501) heap_init: Initializing. RAM available for dynamic allocation:
I (508) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (514) heap_init: At 3FFB7650 len 000289B0 (162 KiB): DRAM
I (520) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (526) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (533) heap_init: At 40094A04 len 0000B5FC (45 KiB): IRAM
I (540) spi_flash: detected chip: generic
I (544) spi_flash: flash io: dio
W (548) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (562) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (572) EXAM_BASIC_MQTT_CUP_W_SSL: [APP] Startup..
I (582) EXAM_BASIC_MQTT_CUP_W_SSL: [APP] Free memory: 275980 bytes
I (582) EXAM_BASIC_MQTT_CUP_W_SSL: [APP] IDF version: v4.4.3-dirty
I (622) wifi:wifi driver task: 3ffbffcc, prio:23, stack:6656, core=0
I (622) system_api: Base MAC address is not set
I (622) system_api: read default base MAC address from EFUSE
I (632) wifi:wifi firmware version: 8cb87ff
I (632) wifi:wifi certification version: v7.0
I (632) wifi:config NVS flash: enabled
I (632) wifi:config nano formating: disabled
I (642) wifi:Init data frame dynamic rx buffer num: 32
I (642) wifi:Init management frame dynamic rx buffer num: 32
I (652) wifi:Init management short buffer num: 32
I (652) wifi:Init dynamic tx buffer num: 32
I (662) wifi:Init static rx buffer size: 1600
I (662) wifi:Init static rx buffer num: 10
I (662) wifi:Init dynamic rx buffer num: 32
I (672) wifi_init: rx ba win: 6
I (672) wifi_init: tcpip mbox: 32
I (682) wifi_init: udp mbox: 6
I (682) wifi_init: tcp mbox: 6
I (682) wifi_init: tcp tx win: 5744
I (692) wifi_init: tcp rx win: 5744
I (692) wifi_init: tcp mss: 1440
I (702) wifi_init: WiFi IRAM OP enabled
I (702) wifi_init: WiFi RX IRAM OP enabled
I (712) example_connect: Connecting to Duoman...
I (712) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (822) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (822) wifi:enable tsf
I (822) example_connect: Waiting for IP(s)
I (3232) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3972) wifi:state: init -> auth (b0)
I (3982) wifi:state: auth -> assoc (0)
I (3982) wifi:state: assoc -> run (10)
W (3992) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (4012) wifi:connected with Duoman, aid = 4, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (4012) wifi:security: WPA2-PSK, phy: bgn, rssi: -39
I (4012) wifi:pm start, type: 1

I (4062) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5612) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6112) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (6112) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (6122) example_connect: Connected to example_connect: sta
I (6122) example_connect: - IPv4 address: 192.168.0.124
I (6132) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6142) EXAM_BASIC_MQTT_CUP_W_SSL: Init tbcmh ...
I (6152) EXAM_BASIC_MQTT_CUP_W_SSL: Connect tbcmh ...
I (6152) tb_mqtt_client_helper: connecting to mqtts://192.168.0.210:8883 ...
I (6162) EXAM_BASIC_MQTT_CUP_W_SSL: connect tbcmh ...
I (6172) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (6172) EXAM_BASIC_MQTT_CUP_W_SSL: Still NOT connected to server!
I (7712) EXAM_BASIC_MQTT_CUP_W_SSL: Still NOT connected to server!
I (8712) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (8712) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc5d04
I (8712) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (8712) tb_mqtt_client_helper: before call on_connected()...
I (8722) EXAM_BASIC_MQTT_CUP_W_SSL: Connected to thingsboard server!
I (8732) tb_mqtt_client_helper: after call on_connected()
I (10932) EXAM_BASIC_MQTT_CUP_W_SSL: Send telemetry: temprature, humidity
I (10932) EXAM_BASIC_MQTT_CUP_W_SSL: Get temperature (a time-series data)
I (10932) EXAM_BASIC_MQTT_CUP_W_SSL: Get humidity (a time-series data)
I (10942) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":25,"humidity":26}
I (11952) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=64406
I (16352) EXAM_BASIC_MQTT_CUP_W_SSL: Send telemetry: temprature, humidity
I (16352) EXAM_BASIC_MQTT_CUP_W_SSL: Get temperature (a time-series data)
I (16352) EXAM_BASIC_MQTT_CUP_W_SSL: Get humidity (a time-series data)
I (16362) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":25.5,"humidity":27}
I (17372) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=54044
I (21772) EXAM_BASIC_MQTT_CUP_W_SSL: Send telemetry: temprature, humidity
I (21772) EXAM_BASIC_MQTT_CUP_W_SSL: Get temperature (a time-series data)
I (21772) EXAM_BASIC_MQTT_CUP_W_SSL: Get humidity (a time-series data)
I (21782) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":26,"humidity":28}
I (22792) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=22455
I (27192) EXAM_BASIC_MQTT_CUP_W_SSL: Send telemetry: temprature, humidity
I (27192) EXAM_BASIC_MQTT_CUP_W_SSL: Get temperature (a time-series data)
I (27192) EXAM_BASIC_MQTT_CUP_W_SSL: Get humidity (a time-series data)
I (27202) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":26.5,"humidity":29}
I (28212) EXAM_BASIC_MQTT_CUP_W_SSL: Disconnect tbcmh ...
I (28212) tb_mqtt_client_helper: disconnecting from mqtts://192.168.0.210:8883 ...
I (28212) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=31245
I (28322) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (28332) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (28332) MQTT_CLIENT: Client asked to stop, but was not started
I (28442) EXAM_BASIC_MQTT_CUP_W_SSL: Destroy tbcmh ...
I (28442) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
```

## ThingsBoard Data

* `Login in ThingsBoard CE/PE` --> `Devices` --> Click my device --> `Attributes` --> `Latest tememetry`, your can find `humidity` and `temprature`. Their values change over time.

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues) on GitHub. We will get back to you soon.
