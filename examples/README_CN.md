# 示例

* Hello world: [00_hello_world](./tb_mqtt_client_helper/00_hello_world)
* 使用底层 API -- tb_mqtt_client.h
  * *TODO: ...*
* 采用高层 API -- tb_mqtt_client_helper.h
  * Telemetry upload API: [10_telemetry_and_client_attr](./tb_mqtt_client_helper/10_telemetry_and_client_attr)
  * Attributes API
    * Publish attribute update to the server: [10_telemetry_and_client_attr](./tb_mqtt_client_helper/10_telemetry_and_client_attr)
    * Subscribe to attribute updates from the server: [21_shared_attr](./tb_mqtt_client_helper/21_shared_attr)
    * Request attribute values from the server: [22_attr_request](./tb_mqtt_client_helper/22_attr_request)
  * RPC API
    * Server-side RPC: [30_server_side_rpc](./tb_mqtt_client_helper/30_server_side_rpc)
    * Client-side RPC: [31_client_side_rpc](./tb_mqtt_client_helper/31_client_side_rpc)
  * Claiming devices: [40_claiming_device](./tb_mqtt_client_helper/40_claiming_device)
  * Device provisioning: *API 仍未实现*
  * Firmware API: [60_ota_update](./tb_mqtt_client_helper/60_ota_update)
  * All in one: [09_all_in_one](./tb_mqtt_client_helper/09_all_in_one)