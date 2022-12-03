set(includes "include")

set(priv_includes "private_include"
                  "src/transport"
                  "src/wapper"
                  "src/helper"
                  "src/extension")

#set(priv_requires "mqtt")

set(requires "mqtt" "json" "app_update")

set(srcs "src/transport/tbc_transport_config.c"
         "src/transport/tbc_transport_storage.c"
         "src/wapper/tbc_mqtt_wapper.c"
         "src/wapper/tbc_mqtt_payload_buffer.c"
         "src/helper/tbc_mqtt_helper.c"
         "src/helper/timeseries_data.c"
         "src/helper/attributes_request.c"
         "src/helper/client_rpc.c"
         "src/helper/device_provision.c"
         "src/helper/ota_update.c"
         "src/helper/ota_fwupdate.c"
         "src/helper/server_rpc.c"
         "src/helper/claiming_device.c"
         "src/extension/tbc_extension_telemetry.c"
         "src/extension/client_attribute.c"
         "src/extension/shared_attribute.c")

#list(APPEND priv_requires "json")
#list(APPEND requires "json")

idf_component_register(SRCS ${srcs}
                INCLUDE_DIRS ${includes}
                PRIV_INCLUDE_DIRS ${priv_includes}
                REQUIRES ${requires} )

                #PRIV_REQUIRES ${priv_requires} 
