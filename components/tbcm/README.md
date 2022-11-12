# Core Components - ESP32 ThingsBoard MQTT Client library

[中文](./README_CN.md)

<https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client>

ESP32 ThingsBoard MQTT Client library for C/C++ developers.

It is packaged into an ESP-IDF component, which can be used under ESP-IDF and ESP-ADF, and can also be easily ported to Arduino-ESP32.

This library provides some simple API to communicate with ThingsBoard platform using MQTT APIs.

Current client version is based on ESP-IDF-v4.4.1, and is compatible with ThingsBoard starting from version 3.4.0.

This component template is based on `esp-idf-v4.4.1\components\esp_ipc` and `esp-idf-v4.4.1\components\esp_lcd`.

## supported esp-mqtt API

| esp-mqtt API                      | User context<br/>(User task) | MQTT event handler<br/>(Internal MQTT task) | Memo                                                                                                                                                                                                                                   |
|-----------------------------------|------------------------------|---------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| esp_mqtt_client_init()            | √                            |                                             |                                                                                                                                                                                                                                        |
| esp_mqtt_client_set_uri()         | √                            | *                                           |                                                                                                                                                                                                                                        |
| esp_mqtt_client_start()           | √                            |                                             |                                                                                                                                                                                                                                        |
| esp_mqtt_client_reconnect()       | √                            |                                             |                                                                                                                                                                                                                                        |
| esp_mqtt_client_disconnect()      | √                            |                                             |                                                                                                                                                                                                                                        |
| esp_mqtt_client_stop()            | √                            | X                                           |                                                                                                                                                                                                                                        |
| esp_mqtt_client_subscribe()       | √                            | √                                           | Client must be connected to send subscribe message                                                                                                                                                                                     |
| esp_mqtt_client_unsubscribe()     | √                            | √                                           | Client must be connected to send unsubscribe message                                                                                                                                                                                   |
| esp_mqtt_client_publish()         | √                            | √                                           | This API might block for several seconds, either due to network timeout (10s) or if publishing payloads longer than internal buffer (due to message fragmentation)                                                                     |
| esp_mqtt_client_enqueue()         | √                            | √                                           | This API generates and stores the publish message into the internal outbox and the actual sending to the network is performed in the mqtt-task context. Thus, it could be used as a non blocking version of esp_mqtt_client_publish(). |
| esp_mqtt_client_destroy()         | √                            | X                                           |                                                                                                                                                                                                                                        |
| esp_mqtt_set_config()             | √                            | *                                           |                                                                                                                                                                                                                                        |
| esp_mqtt_client_register_event()  | √                            |                                             |                                                                                                                                                                                                                                        |
| esp_mqtt_client_get_outbox_size() | √                            | *                                           |                                                                                                                                                                                                                                        |
|                                   |                              |                                             | √:MUST, *:SHOULD, X:MUST NOT                                                                                                                                                                                                           |

## Firmware/Software OTA updates

* On connected:
   1. Subscribe to `v1/devices/me/attributes/response/+`
   1. Subscribe to `v1/devices/me/attributes`
   1. Subscribe to `v2/fw/response/+`
   1. Send telemetry: *current firmware info*
      * Topic: `v1/devices/me/telemetry`
      * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0"}`

      Replace `Initial` and `v0` with your F/W title and version.

   1. Send attributes request: *request firmware info*
      * Topic: `v1/devices/me/attributes/request/{request_id}`
      * Payload: `{"sharedKeys": "fw_checksum,fw_checksum_algorithm,fw_size,fw_title,fw_version"}`

* If receiving *fw_title* & *fw_version* in `v1/devices/me/attributes` or `v1/devices/me/attributes/response/{request_id}`, and they are not the same as *current_fw_title* & *current_fw_version*, then
   1. chunk_id = 0;
   2. Send telemetry: *current firmware info*
      * Topic: `v1/devices/me/telemetry`
      * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0","fw_state":"DOWNLOADING"}`

      Replace `Initial` and `v0` with your F/W title and version.

   3. Send f/w request: *getting firmware*
      * Topic: `v2/fw/request/{request_id}/chunk/{chunk_id}`
      * Payload: `chunk_size`

      Replace `{chunk_id}` with your chunk_id -- `0`

      Replace `chunk_size` with your chunk size, eg: `16384`

* If receiving `v2/fw/response/{request_id}/chunk/{chunk_id}`, then
   1. Saves *msg.payload*.
   2. chunk_id++;
   3. If *accumulated received firmware data length* is less than `fw_size`, then
      1. Send f/w request: *getting firmware*
         * Topic: `v2/fw/request/{request_id}/chunk/{chunk_id}`
         * Payload: `chunk_size`

         Replace `{chunk_id}` with your chunk id;

         Replace `chunk_size` with your chunk size, eg: `16384`

   4. Else *processing firmware*:
      1. Send telemetry: *current firmware info*
         * Topic: `v1/devices/me/telemetry`
         * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0","fw_state":"DOWNLOADED"}`
      2. *Verify checksum*
      3. If verification failure, then
         1. Send telemetry: *current firmware info -- failed*
            * Topic: `v1/devices/me/telemetry`
            * Payload: `{"fw_state":"FAILED","fw_error":"Checksum verification failed!"}`

            You may replace `Checksum verification failed!` with your text.

         2. End.
      4. Else
         1. Send telemetry: *current firmware info*
            * Topic: `v1/devices/me/telemetry`
            * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0","fw_state":"VERIFIED"}`
         2. Do upgrade...
         3. Send telemetry: *current firmware info*
            * Topic: `v1/devices/me/telemetry`
            * Payload: `{"current_fw_title":"Initial","current_fw_version":"v0","fw_state":"UPDATING"}`
         4. If upgrade failue, then
            1. Send telemetry: *current firmware info -- failed*
               * Topic: `v1/devices/me/telemetry`
               * Payload: `{"fw_state":"FAILED","fw_error":"Update failed!"}`

               You may replace `Update failed!` with your text.

            2. End.
         5. Else
            1. send telemetry: *current firmware info*
               * Topic: `v1/devices/me/telemetry`
               * Payload:`{"current_fw_title":"new_fw_title","current_fw_version":"new_fw_version","fw_state":"UPDATED"}`

               Replace `new_fw_title` and `new_fw_version` with your new F/W title and version.

            2. End.

| Provisioning request  | Parameter              | Description                                                                    | Credentials generated by <br/>the ThingsBoard server | Devices supplies<br/>Access Token | Devices supplies<br/>Basic MQTT Credentials | Devices supplies<br/>X.509 Certificate |
|-----------------------|------------------------|--------------------------------------------------------------------------------|------------------------------------------------------|-----------------------------------|---------------------------------------------|----------------------------------------|
|                       | deviceName             | Device name in ThingsBoard.                                                    | (O) DEVICE_NAME                                      | (O) DEVICE_NAME                   | (O) DEVICE_NAME                             | (O) DEVICE_NAME                        |
|                       | provisionDeviceKey     | Provisioning device key, you should take it from configured device profile.    | (M) PUT_PROVISION_KEY_HERE                           | (M) PUT_PROVISION_KEY_HERE        | (M) PUT_PROVISION_KEY_HERE                  | (M) PUT_PROVISION_KEY_HERE             |
|                       | provisionDeviceSecret  | Provisioning device secret, you should take it from configured device profile. | (M) PUT_PROVISION_SECRET_HERE                        | (M) PUT_PROVISION_SECRET_HERE     | (M) PUT_PROVISION_SECRET_HERE               | (M) PUT_PROVISION_SECRET_HERE          |
|                       | credentialsType        | Credentials type parameter.                                                    |                                                      | (M) ACCESS_TOKEN                  | (M) MQTT_BASIC                              | (M) X509_CERTIFICATE                   |
|                       | token                  | Access token for device in ThingsBoard.                                        |                                                      | (M) DEVICE_ACCESS_TOKEN           |                                             |                                        |
|                       | clientId               | Client id for device in ThingsBoard.                                           |                                                      |                                   | (M) DEVICE_CLIENT_ID_HERE                   |                                        |
|                       | username               | Username for device in ThingsBoard.                                            |                                                      |                                   | (M) DEVICE_USERNAME_HERE                    |                                        |
|                       | password               | Password for device in ThingsBoard.                                            |                                                      |                                   | (M) DEVICE_PASSWORD_HERE                    |                                        |
|                       | hash                   | Public key X509 hash for device in ThingsBoard.                                |                                                      |                                   |                                             | (M) MIIB……..AQAB                       |
|                       | (O) Optional, (M) Must |