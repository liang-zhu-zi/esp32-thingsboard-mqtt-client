set(includes "include")

set(priv_includes "src/transport"
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
         "src/helper/telemetry_upload.c"
         "src/helper/attributes_update.c"
         "src/helper/attributes_request.c"
         "src/helper/attributes_subscribe.c"
         "src/helper/client_rpc.c"
         "src/helper/provision_request.c"
         "src/helper/ota_update.c"
         "src/helper/ota_fwupdate.c"
         "src/helper/server_rpc.c"
         "src/helper/claiming_device.c"
         "src/extension/tbc_extension_timeseriesdata.c"
         "src/extension/tbc_extension_clientattributes.c"
         "src/extension/tbc_extension_sharedattributes.c")

#list(APPEND priv_requires "json")
#list(APPEND requires "json")

idf_component_register(SRCS ${srcs}
                INCLUDE_DIRS ${includes}
                PRIV_INCLUDE_DIRS ${priv_includes}
                REQUIRES ${requires} )

                #PRIV_REQUIRES ${priv_requires} 
