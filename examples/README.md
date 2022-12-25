# Examples

* Without the API -- (Only for debug)
  * Hello world: [g00_hello_world](./getstarted/g00_hello_world)

* Using Helper API -- tbc_mqtt_helper.h
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

* Using Extension API -- tbc_extension.h
  * Time-series data API: [e10_timeseries_data](./extension/e10_timeseries_data)
  * Client-side attributes API: [e20_client_attributes](./extension/e20_client_attributes)
  * Shared attributes API: [e30_shared_attributes](./extension/e30_shared_attributes)

* Security authentication <https://thingsboard.io/docs/user-guide/device-credentials/>
  * Access Tokens
    * Plain MQTT (without SSL): [a11_access_token_wo_ssl](./authentication/a11_access_token_wo_ssl)
    * MQTTS (MQTT over SSL): [a12_access_token_w_onewayssl](./authentication/a12_access_token_w_onewayssl)
  * Basic MQTT Credentials
    * Authentication based on Client ID only: [a21_basic_mqtt_credential_c_wo_ssl](./authentication/a21_basic_mqtt_credential_c_wo_ssl)
    * Authentication based on Username and Password: [a22_basic_mqtt_credential_up_wo_ssl](./authentication/a22_basic_mqtt_credential_up_wo_ssl) only for esp-idf v5.x
    * Authentication based on Client ID, Username and Password: [a23_basic_mqtt_credential_cup_wo_ssl](authentication/a23_basic_mqtt_credential_cup_wo_ssl)
    * MQTTS (MQTT over TLS): [a24_basic_mqtt_credential_cup_w_onewayssl](./authentication/a24_basic_mqtt_credential_cup_w_onewayssl)
  * X.509 Certificates: [a31_x.509_ceritificate_w_twowayssl](./authentication/a31_x.509_ceritificate_w_twowayssl)

    These examples in this section differ only in the MQTT parameters. The comparison of these parameters is similar to:

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

* Provision
  * Credentials generated by the ThingsBoard server without SSL: [ps10_srv_gen_credentials_wo_ssl](./provison/ps10_srv_gen_credentials_wo_ssl)
  * Credentials generated by the ThingsBoard server with one-way SSL: [ps20_srv_gen_credentials_w_onewayssl](./provison/ps20_srv_gen_credentials_w_onewayssl)
  * Devices supplies Access Token without SSL: [pd11_dev_sup_access_token_wo_ssl](./provison/pd11_dev_sup_access_token_wo_ssl)
  * Devices supplies Access Token with one-way SSL: [pd12_dev_sup_access_token_w_onewayssl](./provison/pd12_dev_sup_access_token_w_onewayssl)
  * Devices supplies Basic MQTT Credentials - Client ID without SSL: [pd21_dev_sup_basic_mqtt_credential_c_wo_ssl](./provison/pd21_dev_sup_basic_mqtt_credential_c_wo_ssl)
  * Devices supplies Basic MQTT Credentials - User name & Password without SSL: [pd22_dev_sup_basic_mqtt_credential_up_wo_ssl](./provison/pd22_dev_sup_basic_mqtt_credential_up_wo_ssl)
  * Devices supplies Basic MQTT Credentials - client ID, User name & Password without SSL: [pd23_dev_sup_basic_mqtt_credential_cup_wo_ssl](./provison/pd23_dev_sup_basic_mqtt_credential_cup_wo_ssl)
  * Devices supplies Basic MQTT Credentials - client ID, User name & Password with one-way SSL: [pd24_dev_sup_basic_mqtt_credential_cup_w_onewayssl](./provison/pd24_dev_sup_basic_mqtt_credential_cup_w_onewayssl)
  * Devices supplies X.509 Certificate: [pd31_dev_sup_x.509_ceritificate_w_twowayssl](./provison/pd31_dev_sup_x.509_ceritificate_w_twowayssl)
  * Device provisioning with all options in menuconfig: [px00_device_provisioning](./provison/px00_device_provisioning)
