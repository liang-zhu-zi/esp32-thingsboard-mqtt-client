# 示例

* 没有使用本库的API -- (仅调试用)
  * Hello world: [g00_hello_world](./getstarted/g00_hello_world)

* 使用 Helper API -- tbc_mqtt_helper.h
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

* 使用 Extension API -- tbc_extension.h
  * Time-series data API: [e10_timeseries_data](./extension/e10_timeseries_data)
  * Client-side attributes API: [e20_client_attributes](./extension/e20_client_attributes)
  * Shared attributes API: [e30_shared_attributes](./extension/e30_shared_attributes)

* 安全认证示例 <https://thingsboard.io/docs/user-guide/device-credentials/>
  * Access Tokens
    * Plain MQTT (without SSL): [a11_access_token_wo_ssl](./authentication/a11_access_token_wo_ssl)
    * MQTTS (MQTT over SSL): [a12_access_token_w_onewayssl](./authentication/a12_access_token_w_onewayssl)
  * Basic MQTT Credentials
    * Authentication based on Client ID only: [a21_basic_mqtt_credential_c_wo_ssl](./authentication/a21_basic_mqtt_credential_c_wo_ssl)
    * Authentication based on Username and Password: [a22_basic_mqtt_credential_up_wo_ssl](./authentication/a22_basic_mqtt_credential_up_wo_ssl) 仅在 esp-idf v5.x 下可以运行
    * Authentication based on Client ID, Username and Password: [a23_basic_mqtt_credential_cup_wo_ssl](authentication/a23_basic_mqtt_credential_cup_wo_ssl)
    * MQTTS (MQTT over TLS): [a24_basic_mqtt_credential_cup_w_onewayssl](./authentication/a24_basic_mqtt_credential_cup_w_onewayssl)
  * X.509 Certificates: [a31_x.509_ceritificate_w_twowayssl](./authentication/a31_x.509_ceritificate_w_twowayssl)

    本节示例仅 MQTT 参数有差异。这些参数的对比类似：

    | Device authentication options          |                                                          | token                | clientId            | username                  | password                  |  | ca_certs="mqttserver.pub.pem" | certfile="mqtt_thingsboard_server_cert.pem" | keyfile="key.pem" |  | Default Port |
    |----------------------------------------|----------------------------------------------------------|----------------------|---------------------|---------------------------|---------------------------|--|-------------------------------|---------------------|-------------------|--|--------------|
    | Access Token                           | Plain MQTT (without SSL)                                 | -u YOUR_ACCESS_TOKEN |                     |                           |                           |  |                               |                     |                   |  | -p "1883"    |
    |                                        | MQTTS (MQTT over SSL)                                    | -u YOUR_ACCESS_TOKEN |                     |                           |                           |  | --cafile tb-server-chain.pem  |                     |                   |  | -p "8883"    |
    | Basic MQTT authentication              | Authentication based on Client ID only                   |                      | -i "YOUR_CLIENT_ID" |                           |                           |  |                               |                     |                   |  | -p "1883"    |
    |                                        | Authentication based on Username and Password            |                      |                     | -u "YOUR_CLIENT_USERNAME" | -P "YOUR_CLIENT_PASSWORD" |  |                               |                     |                   |  | -p "1883"    |
    |                                        | Authentication based on Client ID, Username and Password |                      | -i "YOUR_CLIENT_ID" | -u "YOUR_CLIENT_USERNAME" | -P "YOUR_CLIENT_PASSWORD" |  |                               |                     |                   |  | -p "1883"    |
    |                                        | MQTTS (MQTT over TLS)                                    |                      | -i "YOUR_CLIENT_ID" | -u "YOUR_CLIENT_USERNAME" | -P "YOUR_CLIENT_PASSWORD" |  | --cafile tb-server-chain.pem  |                     |                   |  | -p "8883"    |
    | X.509 Certificate Based Authentication | (two-way SSL)                                            |                      |                     |                           |                           |  | --cafile tb-server-chain.pem  | --cert mqtt_thingsboard_server_cert.pem     | --key key.pem     |  | -p "8883"    |
    |                                        |                                                          |                      |                     |                           |                           |  |                               |                     |                   |  |              |
    |                                        | -i: client ID                                            |                      |                     |                           |                           |  |                               |                     |                   |  |              |
    |                                        | -u: user name                                            |                      |                     |                           |                           |  |                               |                     |                   |  |              |
    |                                        | -P: password                                             |

* Provision 示例
  * Device provisioning: [px00_device_provisioning](./provision/px00_device_provisioning)
