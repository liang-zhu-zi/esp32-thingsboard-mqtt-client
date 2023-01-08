| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-H2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# Device provisioning - Devices supplies X.509 Certificate - MQTT over SSL (with SSL)

* [中文版](./README_CN.md)

- [Device provisioning - Devices supplies X.509 Certificate - MQTT over SSL (with SSL)](#device-provisioning---devices-supplies-x509-certificate---mqtt-over-ssl-with-ssl)
  - [Introduction](#introduction)
  - [Hardware Required](#hardware-required)
  - [Use Case 1 - Allowing creating new devices with **device name**](#use-case-1---allowing-creating-new-devices-with-device-name)
    - [How to Use Example](#how-to-use-example)
    - [Example Output](#example-output)
    - [ThingsBoard CE/PE Data](#thingsboard-cepe-data)
  - [Use Case 2 - Checking pre-provisioned devices with **device name**](#use-case-2---checking-pre-provisioned-devices-with-device-name)
    - [How to Use Example](#how-to-use-example-1)
    - [Example Output](#example-output-1)
    - [ThingsBoard CE/PE Data](#thingsboard-cepe-data-1)
  - [Troubleshooting](#troubleshooting)

## Introduction

This example implements the following functions:

* Device provisioning - Devices supplies X.509 Certificate - MQTT over SSL (with SSL)
  * Use Case 1 - Allowing creating new devices with **device name**. [here](#use-case-1---allowing-creating-new-devices-with-device-name)
  * Use Case 2 - Checking pre-provisioned devices with **device name**. [here](#use-case-2---checking-pre-provisioned-devices-with-device-name)
* Publish telemetry: temprature, humidity
  * Publish: `{"temprature":25,"humidity":26}`

Refer [here](https://thingsboard.io/docs/user-guide/device-provisioning/) & [here](https://thingsboard.io/docs/user-guide/certificates/).

## Hardware Required

* A development board with ESP32/ESP32-C3/ESP32-H2/ESP32-C2/ESP32-S3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

## Use Case 1 - Allowing creating new devices with **device name**

### How to Use Example

1. ThingsBoard CE/PE SSL configuration using PEM certificates file

   See [MQTT over SSL - using Self-signed PEM certificates file](../../.docs/mqtt-over-ssl-ssl-configuration-using-pem-certificates-file.md)

1. Generate Client certificate

   See [here](../../.docs/generate-client-certificate.md).

1. **ThingsBoard CE/PE**: add or modify a device profile for device provisioning - Allow to create new devices.

   See [here](../../.docs/add-or-modify-device-profile-for-device-provisioning-that-allow-to-create-new-devices.md)

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
   Example ThingsBoard MQTT Configuration  ---> 
      Transport server address  --->
         (MyThingsboardServerIP) Hostname, to set ipv4 pass it as string
         (8883) Port
      Provisioning config  --->
         (MY_DEVICE_NAME) Device name (Optional)
         (MY_PROVISION_KEY) Device key
         (MY_PROVISION_SECRET) Device secret
      Transport verification / Server certificate  --->
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

### Example Output

```none
...
0x400811a8: call_start_cpu1 at C:/Espressif/frameworks/esp-idf-v4.4.3/components/esp_system/port/cpu_start.c:148

I (0) cpu_start: App cpu up.
I (467) cpu_start: Pro cpu start user code
I (467) cpu_start: cpu freq: 160000000
I (467) cpu_start: Application information:
I (472) cpu_start: Project name:     dev_sup_x.509_ceritificate_w_tw
I (479) cpu_start: App version:      c1d3e3d-dirty
I (484) cpu_start: Compile time:     Jan  8 2023 18:15:46
I (490) cpu_start: ELF file SHA256:  9d85b82e7af79d4e...
I (496) cpu_start: ESP-IDF:          v4.4.3-dirty
I (502) heap_init: Initializing. RAM available for dynamic allocation:
I (509) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (515) heap_init: At 3FFB76A0 len 00028960 (162 KiB): DRAM
I (521) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (528) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (534) heap_init: At 40094A04 len 0000B5FC (45 KiB): IRAM
I (542) spi_flash: detected chip: generic
I (545) spi_flash: flash io: dio
W (549) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (563) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (573) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: [APP] Startup..
I (583) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: [APP] Free memory: 275900 bytes
I (583) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: [APP] IDF version: v4.4.3-dirty
I (623) wifi:wifi driver task: 3ffc0024, prio:23, stack:6656, core=0
I (623) system_api: Base MAC address is not set
I (623) system_api: read default base MAC address from EFUSE
I (633) wifi:wifi firmware version: 8cb87ff
I (633) wifi:wifi certification version: v7.0
I (633) wifi:config NVS flash: enabled
I (633) wifi:config nano formating: disabled
I (643) wifi:Init data frame dynamic rx buffer num: 32
I (643) wifi:Init management frame dynamic rx buffer num: 32
I (653) wifi:Init management short buffer num: 32
I (653) wifi:Init dynamic tx buffer num: 32
I (663) wifi:Init static rx buffer size: 1600
I (663) wifi:Init static rx buffer num: 10
I (663) wifi:Init dynamic rx buffer num: 32
I (673) wifi_init: rx ba win: 6
I (673) wifi_init: tcpip mbox: 32
I (683) wifi_init: udp mbox: 6
I (683) wifi_init: tcp mbox: 6
I (683) wifi_init: tcp tx win: 5744
I (693) wifi_init: tcp rx win: 5744
I (693) wifi_init: tcp mss: 1440
I (703) wifi_init: WiFi IRAM OP enabled
I (703) wifi_init: WiFi RX IRAM OP enabled
I (713) example_connect: Connecting to Duoman...
I (713) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (823) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (823) wifi:enable tsf
I (823) example_connect: Waiting for IP(s)
I (3233) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3983) wifi:state: init -> auth (b0)
I (3983) wifi:state: auth -> assoc (0)
I (3993) wifi:state: assoc -> run (10)
W (3993) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (4013) wifi:connected with Duoman, aid = 4, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (4013) wifi:security: WPA2-PSK, phy: bgn, rssi: -27
I (4013) wifi:pm start, type: 1

I (4033) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5613) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6113) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (6113) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (6123) example_connect: Connected to example_connect: sta
I (6123) example_connect: - IPv4 address: 192.168.0.124
I (6133) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6143) FRONT-CONN: Init tbcmh ...
I (6143) FRONT-CONN: Connect tbcmh ...
I (6153) tb_mqtt_client_helper: connecting to mqtts://192.168.0.210:8883 ...
I (6163) tb_mqtt_wapper: src_event->event_id=7
I (6163) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (6173) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Still NOT connected to server!
I (8123) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Still NOT connected to server!
I (9123) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (9123) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc5d70
I (9123) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (9123) tb_mqtt_client_helper: before call on_connected()...
I (9133) FRONT-CONN: Connected to thingsboard server!
I (10143) provision: sent subscribe successful, msg_id=53441, topic=/provision/response
I (10143) tb_mqtt_wapper: [Provision][Tx] request_id=1 {"deviceName":"MY_DEVICE_NAME","provisionDeviceKey":"MY_PROVISION_KEY","provisionDeviceSecret":"MY_PROVISION_SECRET","credentialsType":"X509_CERTIFICATE","hash":"-----BEGIN CERTIFICATE-----\nMIIFkTCCA3mgAwIBAgIUDB8KdXy7Dm7XwFBY4EFkeQOHqwcwDQYJKoZIhvcNAQEL\nBQAwWDELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\nGEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDERMA8GA1UEAwwIRVNQMzItVEIwHhcN\nMjMwMTAyMTExNjI4WhcNMjQwMTAyMTExNjI4WjBYMQswCQYDVQQGEwJBVTETMBEG\nA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkg\nTHRkMREwDwYDVQQDDAhFU1AzMi1UQjCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCC\nAgoCggIBAMufZE1rgd56Tu1XAzr0Xa1dvcovi9v2Xw72wS90d2ZD6mYMTpRxNenL\nrW+uC2xrVdYX08bsWuv4uVXKxcXfLCwE8MpNh/3XbxMYcgKIEBmp6QsuNhxP315M\nUtsLsCr5Ej9J260TimRJESJDsXR3Cu3YMyjkZ7DQ2JYWNDuLeJRZGFRdXvsVgeae\nUfIN9gLJ/2Ak7E0djLVvFIBeWlWu8iHNRudWY1SbaPArGcVixxlFm6eqsKdtxIXK\ndm0XhezKyup+yhZf0+h2BeqO1TBDr7ZS8qdFswWmZV7lod6ILk6CxjgW3D4UM8ur\nbPwjz6obPWAs01cm8sndJXU4qphuw6p6yGxD6IQA0kfqgr+rTJON586txBdGeIs9\nJTT1ZX5rkE14rw6/Vt3sP7Et+K67OHA7uJq+4vHcH51RchMij6seGkhgwP3qLWYq\nz3xnwFroD+buqN8lT0Qa0lLrSDgPg204gA8KnEl9NYdRqadA8WZXB2XMQF84qqLw\nehcRlamOBF8wTWcUAeyAhMjf5Wl6ruii2rPlwSLofn6ru3RzBtv0hGLJekT2zg5h\nE8bFOt3TT5i3S0OTWXq+BCgwS4odNpYsq4qIaf2+nFQ9/RqGh451ZzPJlIvLaPNk\nC3eqfYUEImLBqQzVkYhbFG99TSB0zvThddgJUf8VO+EpJEBwm4XDAgMBAAGjUzBR\nMB0GA1UdDgQWBBQIBxeoyudDqGqtHWnjl1VE6qfHuDAfBgNVHSMEGDAWgBQIBxeo\nyudDqGqtHWnjl1VE6qfHuDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUA\nA4ICAQDK+gbpWOcvHt9HoMqHFcuqg8K5Jg5dXE5ejCg68LKM5QcoaNxWunxCUnJq\nVrrXBCJSdZavx1/eW9ncedxNgmwTbDC8EkkPxDegaURCRtiDHEjENT2IHqUCmJjw\nu2lKTZHXfbC5Ch4Ouxu0KaAOjTh+/h2uXkSwAwBbCaorirdCIpP0jveX4xqZNnwh\n04NRdH2dxyowklET7cJ/Kbc+RfhKlnTg7R23h+sD0y5lnUPvKfZkqjUL5LTIFxwz\nmokgh8n9+7+HqrMV1T1egspgwVRM/3V4/bA6sQDWR2j+4A4ksEdcYI5lZhdXrbpT\nVbr85nrEMgqZZ3TvfNjfEaMAfLF1DCOvwbqJI1/vSs2U+7V7erTTKX8KRPKpK9F0\nXoUYEKy2Li+I5C82gB/UvNKW67G050KF1X2EclTAAmR755Kx0bhaf8WcOLPkPL4y\ncQ72jSXhMHROL0kZdmebl+DLf/MCfj5hmJ762h1UhZLUE2o3H843OrK0K/ZHiq9v\nYcgJ8YC0IFu0yfyzMk1V4UoYN+3v8+BpmRLxmZgcS4FGaU38j8Z2Yebzs5kAkoa/\nFGJPIrMK2I57M2w4kUcMmCz1cJ9GoLqjdyMK7fNeJRqeUpJClxz9muUlG2wyV2vw\nlL4lFwTUETRwl6eJ1ivjEHzzGz3pHtqgjDDJeWop09ySo7L05g==\n-----END CERTIFICATE-----\n"}
I (10353) tb_mqtt_client_helper: after call on_connected()
I (10493) tb_mqtt_wapper: src_event->event_id=5
I (10503) tb_mqtt_wapper: [Provision][Rx] topic_type=6, payload_len=1987 {"credentialsValue":"MIIFkTCCA3mgAwIBAgIUDB8KdXy7Dm7XwFBY4EFkeQOHqwcwDQYJKoZIhvcNAQELBQAwWDELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDERMA8GA1UEAwwIRVNQMzItVEIwHhcNMjMwMTAyMTExNjI4WhcNMjQwMTAyMTExNjI4WjBYMQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMREwDwYDVQQDDAhFU1AzMi1UQjCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAMufZE1rgd56Tu1XAzr0Xa1dvcovi9v2Xw72wS90d2ZD6mYMTpRxNenLrW+uC2xrVdYX08bsWuv4uVXKxcXfLCwE8MpNh/3XbxMYcgKIEBmp6QsuNhxP315MUtsLsCr5Ej9J260TimRJESJDsXR3Cu3YMyjkZ7DQ2JYWNDuLeJRZGFRdXvsVgeaeUfIN9gLJ/2Ak7E0djLVvFIBeWlWu8iHNRudWY1SbaPArGcVixxlFm6eqsKdtxIXKdm0XhezKyup+yhZf0+h2BeqO1TBDr7ZS8qdFswWmZV7lod6ILk6CxjgW3D4UM8urbPwjz6obPWAs01cm8sndJXU4qphuw6p6yGxD6IQA0kfqgr+rTJON586txBdGeIs9JTT1ZX5rkE14rw6/Vt3sP7Et+K67OHA7uJq+4vHcH51RchMij6seGkhgwP3qLWYqz3xnwFroD+buqN8lT0Qa0lLrSDgPg204gA8KnEl9NYdRqadA8WZXB2XMQF84qqLwehcRlamOBF8wTWcUAeyAhMjf5Wl6ruii2rPlwSLofn6ru3RzBtv0hGLJekT2zg5hE8bFOt3TT5i3S0OTWXq+BCgwS4odNpYsq4qIaf2+nFQ9/RqGh451ZzPJlIvLaPNkC3eqfYUEImLBqQzVkYhbFG99TSB0zvThddgJUf8VO+EpJEBwm4XDAgMBAAGjUzBRMB0GA1UdDgQWBBQIBxeoyudDqGqtHWnjl1VE6qfHuDAfBgNVHSMEGDAWgBQIBxeoyudDqGqtHWnjl1VE6qfHuDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4ICAQDK+gbpWOcvHt9HoMqHFcuqg8K5Jg5dXE5ejCg68LKM5QcoaNxWunxCUnJqVrrXBCJSdZavx1/eW9ncedxNgmwTbDC8EkkPxDegaURCRtiDHEjENT2IHqUCmJjwu2lKTZHXfbC5Ch4Ouxu0KaAOjTh+/h2uXkSwAwBbCaorirdCIpP0jveX4xqZNnwh04NRdH2dxyowklET7cJ/Kbc+RfhKlnTg7R23h+sD0y5lnUPvKfZkqjUL5LTIFxwzmokgh8n9+7+HqrMV1T1egspgwVRM/3V4/bA6sQDWR2j+4A4ksEdcYI5lZhdXrbpTVbr85nrEMgqZZ3TvfNjfEaMAfLF1DCOvwbqJI1/vSs2U+7V7erTTKX8KRPKpK9F0XoUYEKy2Li+I5C82gB/UvNKW67G050KF1X2EclTAAmR755Kx0bhaf8WcOLPkPL4ycQ72jSXhMHROL0kZdmebl+DLf/MCfj5hmJ762h1UhZLUE2o3H843OrK0K/ZHiq9vYcgJ8YC0IFu0yfyzMk1V4UoYN+3v8+BpmRLxmZgcS4FGaU38j8Z2Yebzs5kAkoa/FGJPIrMK2I57M2w4kUcMmCz1cJ9GoLqjdyMK7fNeJRqeUpJClxz9muUlG2wyV2vwlL4lFwTUETRwl6eJ1ivjEHzzGz3pHtqgjDDJeWop09ySo7L05g==","credentialsType":"X509_CERTIFICATE","status":"SUCCESS"}
I (11353) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=24676
I (11353) tb_mqtt_client_helper: TBCM_EVENT_DATA
I (11353) provision: sent unsubscribe successful, msg_id=48472, topic=/provision/response
I (11363) FRONT-CONN: Provision successful and the device will work!
I (11363) tb_mqtt_client_helper: disconnecting from mqtts://192.168.0.210:8883 ...
I (11473) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (11483) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (11483) MQTT_CLIENT: Client asked to stop, but was not started
I (11583) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
I (11583) NORMAL-CONN: Init tbcmh ...
I (11583) NORMAL-CONN: Connect tbcmh ...
I (11583) tb_mqtt_client_helper: connecting to mqtts://192.168.0.210:8883 ...
I (11593) tb_mqtt_wapper: src_event->event_id=7
I (11593) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Still NOT connected to server!
I (13143) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (13143) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Still NOT connected to server!
I (14253) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Still NOT connected to server!
I (15353) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Still NOT connected to server!
I (16353) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (16353) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffcb1c8
I (16353) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (16353) tb_mqtt_client_helper: before call on_connected()...
I (16363) NORMAL-CONN: Connected to thingsboard server!
I (16373) NORMAL-CONN: Send telemetry: temprature, humidity
I (16373) NORMAL-CONN: Get temperature (a time-series data)
I (16383) NORMAL-CONN: Get humidity (a time-series data)
I (16393) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":25,"humidity":26}
I (16403) tb_mqtt_client_helper: after call on_connected()
I (16413) tb_mqtt_wapper: src_event->event_id=5
I (17403) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=16523
I (52403) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Disconnect tbcmh ...
I (52403) tb_mqtt_client_helper: disconnecting from mqtts://192.168.0.210:8883 ...
I (52503) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (52513) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (52513) MQTT_CLIENT: Client asked to stop, but was not started
I (52613) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Destroy tbcmh ...
I (52613) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
```

### ThingsBoard CE/PE Data

1. Device provisiong results.

   Refer [here](../../.docs/device-provisioning-results-that-allow-to-create-new-devices.md).

1. **Delete new provisioned device**. 

   In order to ensure that the example runs successfully next time, the newly added device needs to be deleted.

   See [here](../../.docs/delete-provisioned-device.md).


## Use Case 2 - Checking pre-provisioned devices with **device name**

### How to Use Example

1. ThingsBoard CE/PE SSL configuration using PEM certificates file

   See [MQTT over SSL - using Self-signed PEM certificates file](../../.docs/mqtt-over-ssl-ssl-configuration-using-pem-certificates-file.md)

1. Generate Client certificate

   See [here](../../.docs/generate-client-certificate.md).

1. **ThingsBoard CE/PE**: add or modify a device profile for device provisioning - checking pre-provisioned devices. 

   See [here](../../.docs/add-or-modify-device-profile-for-device-provisioning-using-pre-provisioned-devices.md)

1. **ThingsBoard CE/PE**: pre-provisioning device with basic MQTT credentials -  Client ID, Username and Password

   See [here](../../.docs/pre-provisioning-device-with-x509-ceritificate.md)

1. **ThingsBoard CE/PE**: device provisioning results using pre-provisioned devices. 

   See [here](../../.docs/pre-provisioning-device-status.md)

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
   Example ThingsBoard MQTT Configuration  ---> 
      Transport server address  --->
         (MyThingsboardServerIP) Hostname, to set ipv4 pass it as string
         (8883) Port
      Provisioning config  --->
         (MY_DEVICE_NAME) Device name (Optional)
         (MY_PROVISION_KEY) Device key
         (MY_PROVISION_SECRET) Device secret
      Transport verification / Server certificate  --->
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

### Example Output

```none
...
0x400811a8: call_start_cpu1 at C:/Espressif/frameworks/esp-idf-v4.4.3/components/esp_system/port/cpu_start.c:148

I (0) cpu_start: App cpu up.
I (467) cpu_start: Pro cpu start user code
I (467) cpu_start: cpu freq: 160000000
I (467) cpu_start: Application information:
I (472) cpu_start: Project name:     dev_sup_x.509_ceritificate_w_tw
I (479) cpu_start: App version:      c1d3e3d-dirty
I (484) cpu_start: Compile time:     Jan  8 2023 18:15:46
I (490) cpu_start: ELF file SHA256:  9d85b82e7af79d4e...
I (496) cpu_start: ESP-IDF:          v4.4.3-dirty
I (502) heap_init: Initializing. RAM available for dynamic allocation:
I (509) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (515) heap_init: At 3FFB76A0 len 00028960 (162 KiB): DRAM
I (521) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (528) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (534) heap_init: At 40094A04 len 0000B5FC (45 KiB): IRAM
I (542) spi_flash: detected chip: generic
I (545) spi_flash: flash io: dio
W (549) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (563) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (573) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: [APP] Startup..
I (583) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: [APP] Free memory: 275900 bytes
I (583) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: [APP] IDF version: v4.4.3-dirty
I (623) wifi:wifi driver task: 3ffc0024, prio:23, stack:6656, core=0
I (623) system_api: Base MAC address is not set
I (623) system_api: read default base MAC address from EFUSE
I (633) wifi:wifi firmware version: 8cb87ff
I (633) wifi:wifi certification version: v7.0
I (633) wifi:config NVS flash: enabled
I (633) wifi:config nano formating: disabled
I (643) wifi:Init data frame dynamic rx buffer num: 32
I (643) wifi:Init management frame dynamic rx buffer num: 32
I (653) wifi:Init management short buffer num: 32
I (653) wifi:Init dynamic tx buffer num: 32
I (663) wifi:Init static rx buffer size: 1600
I (663) wifi:Init static rx buffer num: 10
I (663) wifi:Init dynamic rx buffer num: 32
I (673) wifi_init: rx ba win: 6
I (673) wifi_init: tcpip mbox: 32
I (683) wifi_init: udp mbox: 6
I (683) wifi_init: tcp mbox: 6
I (683) wifi_init: tcp tx win: 5744
I (693) wifi_init: tcp rx win: 5744
I (693) wifi_init: tcp mss: 1440
I (703) wifi_init: WiFi IRAM OP enabled
I (703) wifi_init: WiFi RX IRAM OP enabled
I (713) example_connect: Connecting to Duoman...
I (713) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (823) wifi:mode : sta (bc:dd:c2:d1:be:b0)
I (823) wifi:enable tsf
I (823) example_connect: Waiting for IP(s)
I (3233) wifi:new:<2,0>, old:<1,0>, ap:<255,255>, sta:<2,0>, prof:1
I (3973) wifi:state: init -> auth (b0)
I (3983) wifi:state: auth -> assoc (0)
I (3993) wifi:state: assoc -> run (10)
W (4003) wifi:<ba-add>idx:0 (ifx:0, d8:0d:17:00:5b:13), tid:0, ssn:0, winSize:64
I (4023) wifi:connected with Duoman, aid = 4, channel 2, BW20, bssid = d8:0d:17:00:5b:13
I (4023) wifi:security: WPA2-PSK, phy: bgn, rssi: -27
I (4023) wifi:pm start, type: 1

I (4083) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5613) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6113) esp_netif_handlers: example_connect: sta ip: 192.168.0.124, mask: 255.255.255.0, gw: 192.168.0.1
I (6113) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.124
I (6123) example_connect: Connected to example_connect: sta
I (6123) example_connect: - IPv4 address: 192.168.0.124
I (6133) example_connect: - IPv6 address: fe80:0000:0000:0000:bedd:c2ff:fed1:beb0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6143) FRONT-CONN: Init tbcmh ...
I (6143) FRONT-CONN: Connect tbcmh ...
I (6153) tb_mqtt_client_helper: connecting to mqtts://192.168.0.210:8883 ...
I (6163) tb_mqtt_wapper: src_event->event_id=7
I (6163) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (6173) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Still NOT connected to server!
I (8073) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Still NOT connected to server!
I (9073) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (9073) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffc5d5c
I (9073) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (9073) tb_mqtt_client_helper: before call on_connected()...
I (9083) FRONT-CONN: Connected to thingsboard server!
I (10093) provision: sent subscribe successful, msg_id=17815, topic=/provision/response
I (10093) tb_mqtt_wapper: [Provision][Tx] request_id=1 {"deviceName":"MY_DEVICE_NAME","provisionDeviceKey":"MY_PROVISION_KEY","provisionDeviceSecret":"MY_PROVISION_SECRET","credentialsType":"X509_CERTIFICATE","hash":"-----BEGIN CERTIFICATE-----\nMIIFkTCCA3mgAwIBAgIUDB8KdXy7Dm7XwFBY4EFkeQOHqwcwDQYJKoZIhvcNAQEL\nBQAwWDELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\nGEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDERMA8GA1UEAwwIRVNQMzItVEIwHhcN\nMjMwMTAyMTExNjI4WhcNMjQwMTAyMTExNjI4WjBYMQswCQYDVQQGEwJBVTETMBEG\nA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkg\nTHRkMREwDwYDVQQDDAhFU1AzMi1UQjCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCC\nAgoCggIBAMufZE1rgd56Tu1XAzr0Xa1dvcovi9v2Xw72wS90d2ZD6mYMTpRxNenL\nrW+uC2xrVdYX08bsWuv4uVXKxcXfLCwE8MpNh/3XbxMYcgKIEBmp6QsuNhxP315M\nUtsLsCr5Ej9J260TimRJESJDsXR3Cu3YMyjkZ7DQ2JYWNDuLeJRZGFRdXvsVgeae\nUfIN9gLJ/2Ak7E0djLVvFIBeWlWu8iHNRudWY1SbaPArGcVixxlFm6eqsKdtxIXK\ndm0XhezKyup+yhZf0+h2BeqO1TBDr7ZS8qdFswWmZV7lod6ILk6CxjgW3D4UM8ur\nbPwjz6obPWAs01cm8sndJXU4qphuw6p6yGxD6IQA0kfqgr+rTJON586txBdGeIs9\nJTT1ZX5rkE14rw6/Vt3sP7Et+K67OHA7uJq+4vHcH51RchMij6seGkhgwP3qLWYq\nz3xnwFroD+buqN8lT0Qa0lLrSDgPg204gA8KnEl9NYdRqadA8WZXB2XMQF84qqLw\nehcRlamOBF8wTWcUAeyAhMjf5Wl6ruii2rPlwSLofn6ru3RzBtv0hGLJekT2zg5h\nE8bFOt3TT5i3S0OTWXq+BCgwS4odNpYsq4qIaf2+nFQ9/RqGh451ZzPJlIvLaPNk\nC3eqfYUEImLBqQzVkYhbFG99TSB0zvThddgJUf8VO+EpJEBwm4XDAgMBAAGjUzBR\nMB0GA1UdDgQWBBQIBxeoyudDqGqtHWnjl1VE6qfHuDAfBgNVHSMEGDAWgBQIBxeo\nyudDqGqtHWnjl1VE6qfHuDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUA\nA4ICAQDK+gbpWOcvHt9HoMqHFcuqg8K5Jg5dXE5ejCg68LKM5QcoaNxWunxCUnJq\nVrrXBCJSdZavx1/eW9ncedxNgmwTbDC8EkkPxDegaURCRtiDHEjENT2IHqUCmJjw\nu2lKTZHXfbC5Ch4Ouxu0KaAOjTh+/h2uXkSwAwBbCaorirdCIpP0jveX4xqZNnwh\n04NRdH2dxyowklET7cJ/Kbc+RfhKlnTg7R23h+sD0y5lnUPvKfZkqjUL5LTIFxwz\nmokgh8n9+7+HqrMV1T1egspgwVRM/3V4/bA6sQDWR2j+4A4ksEdcYI5lZhdXrbpT\nVbr85nrEMgqZZ3TvfNjfEaMAfLF1DCOvwbqJI1/vSs2U+7V7erTTKX8KRPKpK9F0\nXoUYEKy2Li+I5C82gB/UvNKW67G050KF1X2EclTAAmR755Kx0bhaf8WcOLPkPL4y\ncQ72jSXhMHROL0kZdmebl+DLf/MCfj5hmJ762h1UhZLUE2o3H843OrK0K/ZHiq9v\nYcgJ8YC0IFu0yfyzMk1V4UoYN+3v8+BpmRLxmZgcS4FGaU38j8Z2Yebzs5kAkoa/\nFGJPIrMK2I57M2w4kUcMmCz1cJ9GoLqjdyMK7fNeJRqeUpJClxz9muUlG2wyV2vw\nlL4lFwTUETRwl6eJ1ivjEHzzGz3pHtqgjDDJeWop09ySo7L05g==\n-----END CERTIFICATE-----\n"}
I (10303) tb_mqtt_client_helper: after call on_connected()
I (10433) tb_mqtt_wapper: src_event->event_id=5
I (10443) tb_mqtt_wapper: [Provision][Rx] topic_type=6, payload_len=1987 {"credentialsValue":"MIIFkTCCA3mgAwIBAgIUDB8KdXy7Dm7XwFBY4EFkeQOHqwcwDQYJKoZIhvcNAQELBQAwWDELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDERMA8GA1UEAwwIRVNQMzItVEIwHhcNMjMwMTAyMTExNjI4WhcNMjQwMTAyMTExNjI4WjBYMQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMREwDwYDVQQDDAhFU1AzMi1UQjCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAMufZE1rgd56Tu1XAzr0Xa1dvcovi9v2Xw72wS90d2ZD6mYMTpRxNenLrW+uC2xrVdYX08bsWuv4uVXKxcXfLCwE8MpNh/3XbxMYcgKIEBmp6QsuNhxP315MUtsLsCr5Ej9J260TimRJESJDsXR3Cu3YMyjkZ7DQ2JYWNDuLeJRZGFRdXvsVgeaeUfIN9gLJ/2Ak7E0djLVvFIBeWlWu8iHNRudWY1SbaPArGcVixxlFm6eqsKdtxIXKdm0XhezKyup+yhZf0+h2BeqO1TBDr7ZS8qdFswWmZV7lod6ILk6CxjgW3D4UM8urbPwjz6obPWAs01cm8sndJXU4qphuw6p6yGxD6IQA0kfqgr+rTJON586txBdGeIs9JTT1ZX5rkE14rw6/Vt3sP7Et+K67OHA7uJq+4vHcH51RchMij6seGkhgwP3qLWYqz3xnwFroD+buqN8lT0Qa0lLrSDgPg204gA8KnEl9NYdRqadA8WZXB2XMQF84qqLwehcRlamOBF8wTWcUAeyAhMjf5Wl6ruii2rPlwSLofn6ru3RzBtv0hGLJekT2zg5hE8bFOt3TT5i3S0OTWXq+BCgwS4odNpYsq4qIaf2+nFQ9/RqGh451ZzPJlIvLaPNkC3eqfYUEImLBqQzVkYhbFG99TSB0zvThddgJUf8VO+EpJEBwm4XDAgMBAAGjUzBRMB0GA1UdDgQWBBQIBxeoyudDqGqtHWnjl1VE6qfHuDAfBgNVHSMEGDAWgBQIBxeoyudDqGqtHWnjl1VE6qfHuDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4ICAQDK+gbpWOcvHt9HoMqHFcuqg8K5Jg5dXE5ejCg68LKM5QcoaNxWunxCUnJqVrrXBCJSdZavx1/eW9ncedxNgmwTbDC8EkkPxDegaURCRtiDHEjENT2IHqUCmJjwu2lKTZHXfbC5Ch4Ouxu0KaAOjTh+/h2uXkSwAwBbCaorirdCIpP0jveX4xqZNnwh04NRdH2dxyowklET7cJ/Kbc+RfhKlnTg7R23h+sD0y5lnUPvKfZkqjUL5LTIFxwzmokgh8n9+7+HqrMV1T1egspgwVRM/3V4/bA6sQDWR2j+4A4ksEdcYI5lZhdXrbpTVbr85nrEMgqZZ3TvfNjfEaMAfLF1DCOvwbqJI1/vSs2U+7V7erTTKX8KRPKpK9F0XoUYEKy2Li+I5C82gB/UvNKW67G050KF1X2EclTAAmR755Kx0bhaf8WcOLPkPL4ycQ72jSXhMHROL0kZdmebl+DLf/MCfj5hmJ762h1UhZLUE2o3H843OrK0K/ZHiq9vYcgJ8YC0IFu0yfyzMk1V4UoYN+3v8+BpmRLxmZgcS4FGaU38j8Z2Yebzs5kAkoa/FGJPIrMK2I57M2w4kUcMmCz1cJ9GoLqjdyMK7fNeJRqeUpJClxz9muUlG2wyV2vwlL4lFwTUETRwl6eJ1ivjEHzzGz3pHtqgjDDJeWop09ySo7L05g==","credentialsType":"X509_CERTIFICATE","status":"SUCCESS"}
I (11303) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=47756
I (11303) tb_mqtt_client_helper: TBCM_EVENT_DATA
I (11303) provision: sent unsubscribe successful, msg_id=22355, topic=/provision/response
I (11313) FRONT-CONN: Provision successful and the device will work!
I (11313) tb_mqtt_client_helper: disconnecting from mqtts://192.168.0.210:8883 ...
I (11423) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (11433) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (11433) MQTT_CLIENT: Client asked to stop, but was not started
I (11533) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
I (11533) NORMAL-CONN: Init tbcmh ...
I (11533) NORMAL-CONN: Connect tbcmh ...
I (11533) tb_mqtt_client_helper: connecting to mqtts://192.168.0.210:8883 ...
I (11543) tb_mqtt_wapper: src_event->event_id=7
I (11543) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Still NOT connected to server!
I (12553) tb_mqtt_client_helper: TBCM_EVENT_BEFORE_CONNECT, msg_id=64319
I (12553) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Still NOT connected to server!
I (14413) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Still NOT connected to server!
I (16333) tb_mqtt_client_helper: TBCM_EVENT_CONNECTED
I (16333) tb_mqtt_client_helper: client->tbmqttclient = 0x3ffcb474
I (16333) tb_mqtt_client_helper: Connected to thingsboard MQTT server!
I (16343) tb_mqtt_client_helper: before call on_connected()...
I (16343) NORMAL-CONN: Connected to thingsboard server!
I (16353) NORMAL-CONN: Send telemetry: temprature, humidity
I (16353) NORMAL-CONN: Get temperature (a time-series data)
I (16363) NORMAL-CONN: Get humidity (a time-series data)
I (16373) tb_mqtt_wapper: [Telemetry][Tx] {"temprature":25,"humidity":26}
I (16383) tb_mqtt_client_helper: after call on_connected()
I (16393) tb_mqtt_wapper: src_event->event_id=5
I (17383) tb_mqtt_client_helper: TBCM_EVENT_PUBLISHED, msg_id=13716
I (53483) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Disconnect tbcmh ...
I (53483) tb_mqtt_client_helper: disconnecting from mqtts://192.168.0.210:8883 ...
I (53583) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_stop()...
I (53593) tb_mqtt_wapper: tbcm_disconnect(): call esp_mqtt_client_destroy()...
W (53593) MQTT_CLIENT: Client asked to stop, but was not started
I (53693) DEV_SUP_X509_CERT_W_TWOWAYSSL_MAIN: Destroy tbcmh ...
I (53693) tb_mqtt_client_helper: It already disconnected from thingsboard MQTT server!
```

### ThingsBoard CE/PE Data

1. Device provisiong results.

   Refer [here](../../.docs/device-provisioning-results-using-pre-provisioned-devices.md).

1. **Delete new provisioned device**. 

   In order to ensure that the example runs successfully next time, the newly added device needs to be deleted.

   See [here](../../.docs/delete-provisioned-device.md).

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client/issues) on GitHub. We will get back to you soon.
