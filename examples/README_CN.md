# 示例

* Hello world: [00_hello_world](./tb_mqtt_client_helper/00_hello_world)
* 使用底层 API -- tb_mqtt_client.h
  * *TODO: ...*
* 采用高层 API -- tb_mqtt_client_helper.h
  * Telemetry upload API: [01_telemetry_and_client_attr](./tb_mqtt_client_helper/01_telemetry_and_client_attr)
  * Attributes API
    * Publish attribute update to the server: [01_telemetry_and_client_attr](./tb_mqtt_client_helper/01_telemetry_and_client_attr)
    * Subscribe to attribute updates from the server: [02_shared_attr](./tb_mqtt_client_helper/02_shared_attr)
    * Request attribute values from the server: [03_attr_request](./tb_mqtt_client_helper/03_attr_request)
  * RPC API
    * Server-side RPC: [04_server_side_rpc](./tb_mqtt_client_helper/04_server_side_rpc)
    * Client-side RPC: [05_client_side_rpc](./tb_mqtt_client_helper/05_client_side_rpc)
  * Claiming devices: *API 仍未实现*
  * Device provisioning: *API 仍未实现*
  * Firmware API: [08_ota_update](./tb_mqtt_client_helper/08_ota_update)
  * All in one: [09_all_in_one](./tb_mqtt_client_helper/09_all_in_one)