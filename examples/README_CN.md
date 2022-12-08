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
  * Device provisioning: [60_device_provisioning](./helper/60_device_provisioning)

* 使用 Extension API -- tbc_extension.h
  * Time-series data API: [10_timeseries_data](./extension/10_timeseries_data)
  * Client-side attributes API: [20_client_attributes](./extension/20_client_attributes)
  * Shared attributes API: [30_shared_attributes](./extension/30_shared_attributes)
