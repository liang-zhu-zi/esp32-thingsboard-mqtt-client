# 示例

* [English Version](README.md)

1. 没有使用本库的 API -- (仅供测试组件编译)

   * Hello world: [g00_hello_world](./getstarted/g00_hello_world)

2. 使用助手API (Helper API) 示例 -- [tbc_mqtt_helper.h](./../components/tbcmh/include/tbc_mqtt_helper.h)：参考 [这里](https://thingsboard.io/docs/reference/mqtt-api/)

   * Telemetry upload API: [h10_telemetry_upload](./helper/h10_telemetry_upload)
   * Attributes API
     * Publish attribute update to the server: [h20_attributes_update](./helper/h20_attributes_update)
     * Subscribe to attribute updates from the server: [h21_attributes_subscribe](./helper/h21_attributes_subscribe)
     * Request attribute values from the server: [h22_attributes_request](./helper/h22_attributes_request)
   * RPC API
     * Server-side RPC: [h30_server_side_rpc](./helper/h30_server_side_rpc)
     * Client-side RPC: [h31_client_side_rpc](./helper/h31_client_side_rpc)
   * Claiming devices: [h40_claiming_device](./helper/h40_claiming_device)
   * Firmware API: [h50_fw_update](./helper/h50_fw_update)

3. 使用扩展API (Extension API) 示例 -- [tbc_extension.h](../components/tbcmh/include/tbc_extension.h)
   * Time-series data API: [e10_timeseries_data](./extension/e10_timeseries_data)
   * Client-side attributes API: [e20_client_attributes](./extension/e20_client_attributes)
   * Shared attributes API: [e30_shared_attributes](./extension/e30_shared_attributes)

4. [安全认证 (Security authentication)](https://thingsboard.io/docs/user-guide/device-credentials/) 示例：参考 [这里](https://thingsboard.io/docs/user-guide/mqtt-over-ssl/)

   * [访问令牌 (Access Tokens)](https://thingsboard.io/docs/user-guide/access-token/)

     * Plain MQTT (without SSL): [a11_access_token_wo_ssl](./authentication/a11_access_token_wo_ssl)
     * MQTTS (MQTT over SSL): [a12_access_token_w_onewayssl](./authentication/a12_access_token_w_onewayssl)

   * [基本的 MQTT 凭证 (Basic MQTT Credentials)](https://thingsboard.io/docs/user-guide/basic-mqtt/)

     * Authentication based on Client ID only: [a21_basic_mqtt_credential_c_wo_ssl](./authentication/a21_basic_mqtt_credential_c_wo_ssl)
     * Authentication based on Username and Password: [a22_basic_mqtt_credential_up_wo_ssl](./authentication/a22_basic_mqtt_credential_up_wo_ssl) only for esp-idf v5.x
     * Authentication based on Client ID, Username and Password: [a23_basic_mqtt_credential_cup_wo_ssl](authentication/a23_basic_mqtt_credential_cup_wo_ssl)
     * MQTTS (MQTT over TLS): [a24_basic_mqtt_credential_cup_w_onewayssl](./authentication/a24_basic_mqtt_credential_cup_w_onewayssl)
 
   * [X.509 证书 (X.509 Certificates)](https://thingsboard.io/docs/user-guide/certificates/)
     * X.509 Certificates: [a31_x.509_ceritificate_w_twowayssl](./authentication/a31_x.509_ceritificate_w_twowayssl)

    本节示例仅 MQTT 参数有差异。这些参数的对比类似:

    | Device authentication options          |                                                          | token                | clientId            | username                  | password                  |  | ca_certs=<br>"mqttserver<br>.pub.pem" | certfile=<br>"mqtt<br>_thingsboard<br>_server<br>_cert.pem" | keyfile=<br>"key<br>.pem" |  | Default Port |
    |----------------------------------------|----------------------------------------------------------|----------------------|---------------------|---------------------------|---------------------------|--|-------------------------------|---------------------|-------------------|--|--------------|
    | Access Token                           | Plain MQTT (without SSL)                                 | -u <br>"YOUR<br>_ACCESS<br>_TOKEN" |                     |                           |                           |  |                               |                     |                   |  | -p "1883"    |
    |                                        | MQTTS (MQTT over SSL)                                    | -u <br>"YOUR<br>_ACCESS<br>_TOKEN" |                     |                           |                           |  | --cafile <br>tb-server-chain.pem  |                     |                   |  | -p "8883"    |
    | Basic MQTT authentication              | Authentication based on Client ID only                   |                      | -i <br>"YOUR<br>_CLIENT<br>_ID" |                           |                           |  |                               |                     |                   |  | -p "1883"    |
    |                                        | Authentication based on Username and Password            |                      |                     | -u <br>"YOUR<br>_CLIENT<br>_USERNAME" | -P <br>"YOUR<br>_CLIENT<br>_PASSWORD" |  |                               |                     |                   |  | -p "1883"    |
    |                                        | Authentication based on Client ID, Username and Password |                      | -i <br>"YOUR<br>_CLIENT<br>_ID" | -u <br>"YOUR<br>_CLIENT<br>_USERNAME" | -P <br>"YOUR<br>_CLIENT<br>_PASSWORD" |  |                               |                     |                   |  | -p "1883"    |
    |                                        | MQTTS (MQTT over TLS)                                    |                      | -i <br>"YOUR<br>_CLIENT<br>_ID" | -u <br>"YOUR<br>_CLIENT<br>_USERNAME" | -P <br>"YOUR<br>_CLIENT<br>_PASSWORD" |  | --cafile <br>tb-server-chain.pem  |                     |                   |  | -p "8883"    |
    | X.509 Certificate Based Authentication | (two-way SSL)                                            |                      |                     |                           |                           |  | --cafile <br>tb-server-chain.pem  | --cert <br>mqtt<br>_thingsboard<br>_server<br>_cert.pem     | --key <br>key<br>.pem     |  | -p "8883"    |
    |                                        |                                                          |                      |                     |                           |                           |  |                               |                     |                   |  |              |
    |                                        | -i: client ID                                            |                      |                     |                           |                           |  |                               |                     |                   |  |              |
    |                                        | -u: user name                                            |                      |                     |                           |                           |  |                               |                     |                   |  |              |
    |                                        | -P: password                                             |

5. [设备配置 (Provisioning Devices)](https://thingsboard.io/docs/user-guide/device-provisioning/) 示例： 参考 [这里](https://thingsboard.io/docs/reference/mqtt-api/#device-provisioning)

   * 凭证由服务端产生 (Credentials generated by the ThingsBoard server)
     * Credentials generated by the ThingsBoard server without SSL: [ps10_srv_gen_credentials_wo_ssl](./provision/ps10_srv_gen_credentials_wo_ssl)
     * Credentials generated by the ThingsBoard server with one-way SSL: [ps20_srv_gen_credentials_w_onewayssl](./provision/ps20_srv_gen_credentials_w_onewayssl)
   * 设备提供访问凭证 (Devices supplies Access Token)
     * Devices supplies Access Token without SSL: [pd11_dev_sup_access_token_wo_ssl](./provision/pd11_dev_sup_access_token_wo_ssl)
     * Devices supplies Access Token with one-way SSL: [pd12_dev_sup_access_token_w_onewayssl](./provision/pd12_dev_sup_access_token_w_onewayssl)
   * 设备提供基本的 MQTT 凭证 (Devices supplies Basic MQTT Credentials)
     * Devices supplies Basic MQTT Credentials - Client ID without SSL: [pd21_dev_sup_basic_mqtt_credential_c_wo_ssl](./provision/pd21_dev_sup_basic_mqtt_credential_c_wo_ssl)
     * Devices supplies Basic MQTT Credentials - User name & Password without SSL: [pd22_dev_sup_basic_mqtt_credential_up_wo_ssl](./provision/pd22_dev_sup_basic_mqtt_credential_up_wo_ssl)
     * Devices supplies Basic MQTT Credentials - client ID, User name & Password without SSL: [pd23_dev_sup_basic_mqtt_credential_cup_wo_ssl](./provision/pd23_dev_sup_basic_mqtt_credential_cup_wo_ssl)
     * Devices supplies Basic MQTT Credentials - client ID, User name & Password with one-way SSL: [pd24_dev_sup_basic_mqtt_credential_cup_w_onewayssl](./provision/pd24_dev_sup_basic_mqtt_credential_cup_w_onewayssl)
   * 设备提供X.509证书 (Devices supplies X.509 Certificate)
     * Devices supplies X.509 Certificate: [pd31_dev_sup_x.509_ceritificate_w_twowayssl](./provision/pd31_dev_sup_x.509_ceritificate_w_twowayssl)
   * 所有的 Provisioning 选项在 Menuconfig 中确定 (All options of device provisioning are defined in menuconfig)
     * All options of device provisioning are defined in menuconfig: [px00_device_provisioning](./provision/px00_device_provisioning)

