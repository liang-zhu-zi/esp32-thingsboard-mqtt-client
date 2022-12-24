# 示例

* 没有使用本库的API -- (仅调试用)
  * Hello world: [00_hello_world](./helper/00_hello_world)

* 使用 Helper API -- tbc_mqtt_helper.h
  * Telemetry upload API: [10_telemetry_upload](./helper/10_telemetry_upload)
  * Attributes API
    * Publish attribute update to the server: [20_attributes_update](./helper/20_attributes_update)
    * Subscribe to attribute updates from the server: [21_attributes_subscribe](./helper/21_attributes_subscribe)
    * Request attribute values from the server: [22_attributes_request](./helper/22_attributes_request)
  * RPC API
    * Server-side RPC: [30_server_side_rpc](./helper/30_server_side_rpc)
    * Client-side RPC: [31_client_side_rpc](./helper/31_client_side_rpc)
  * Claiming devices: [40_claiming_device](./helper/40_claiming_device)
  * Firmware API: [50_fw_update](./helper/50_fw_update)

* 使用 Extension API -- tbc_extension.h
  * Time-series data API: [10_timeseries_data](./extension/10_timeseries_data)
  * Client-side attributes API: [20_client_attributes](./extension/20_client_attributes)
  * Shared attributes API: [30_shared_attributes](./extension/30_shared_attributes)

* 安全认证示例 <https://thingsboard.io/docs/user-guide/device-credentials/>
  * Access Tokens
    * Plain MQTT (without SSL): [s11_access_token_without_ssl](./security_auth/s11_access_token_without_ssl)
    * MQTTS (MQTT over SSL): [s12_access_token_with_onewayssl](./security_auth/s12_access_token_with_onewayssl)
  * Basic MQTT Credentials
    * Authentication based on Client ID only: [s21_basic_mqtt_credential_c_without_ssl](./security_auth/s21_basic_mqtt_credential_c_without_ssl)
    * Authentication based on Username and Password: [s22_basic_mqtt_credential_u_p_without_ssl](./security_auth/s22_basic_mqtt_credential_u_p_without_ssl)
    * Authentication based on Client ID, Username and Password: [s23_basic_mqtt_credential_c_u_p_without_ssl](security_auth/s23_basic_mqtt_credential_c_u_p_without_ssl)
    * MQTTS (MQTT over TLS): [s24_basic_mqtt_credential_c_u_p_with_onewayssl](./security_auth/s24_basic_mqtt_credential_c_u_p_with_onewayssl)
  * X.509 Certificates: [s31_x.509_ceritificate_with_twowayssl](./security_auth/s31_x.509_ceritificate_with_twowayssl)

    本节示例仅 MQTT 参数有差异。这些参数的对比类似：

    | Device authentication options          |                                                          | token                | clientId            | username                  | password                  |  | ca_certs="mqttserver.pub.pem" | certfile="cert.pem" | keyfile="key.pem" |  | Default Port |
    |----------------------------------------|----------------------------------------------------------|----------------------|---------------------|---------------------------|---------------------------|--|-------------------------------|---------------------|-------------------|--|--------------|
    | Access Token                           | Plain MQTT (without SSL)                                 | -u YOUR_ACCESS_TOKEN |                     |                           |                           |  |                               |                     |                   |  | -p "1883"    |
    |                                        | MQTTS (MQTT over SSL)                                    | -u YOUR_ACCESS_TOKEN |                     |                           |                           |  | --cafile tb-server-chain.pem  |                     |                   |  | -p "8883"    |
    | Basic MQTT authentication              | Authentication based on Client ID only                   |                      | -i "YOUR_CLIENT_ID" |                           |                           |  |                               |                     |                   |  | -p "1883"    |
    |                                        | Authentication based on Username and Password            |                      |                     | -u "YOUR_CLIENT_USERNAME" | -P "YOUR_CLIENT_PASSWORD" |  |                               |                     |                   |  | -p "1883"    |
    |                                        | Authentication based on Client ID, Username and Password |                      | -i "YOUR_CLIENT_ID" | -u "YOUR_CLIENT_USERNAME" | -P "YOUR_CLIENT_PASSWORD" |  |                               |                     |                   |  | -p "1883"    |
    |                                        | MQTTS (MQTT over TLS)                                    |                      | -i "YOUR_CLIENT_ID" | -u "YOUR_CLIENT_USERNAME" | -P "YOUR_CLIENT_PASSWORD" |  | --cafile tb-server-chain.pem  |                     |                   |  | -p "8883"    |
    | X.509 Certificate Based Authentication | (two-way SSL)                                            |                      |                     |                           |                           |  | --cafile tb-server-chain.pem  | --cert cert.pem     | --key key.pem     |  | -p "8883"    |
    |                                        |                                                          |                      |                     |                           |                           |  |                               |                     |                   |  |              |
    |                                        | -i: client ID                                            |                      |                     |                           |                           |  |                               |                     |                   |  |              |
    |                                        | -u: user name                                            |                      |                     |                           |                           |  |                               |                     |                   |  |              |
    |                                        | -P: password                                             |

* Provision 示例
  * Device provisioning: [60_device_provisioning](./helper/60_device_provisioning)
