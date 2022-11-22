set(includes "include")

set(priv_includes "private_include"
                  "src/transport"
                  "src/mqtt"
                  "src/helper")

#set(priv_requires "mqtt")

set(requires "mqtt" "json")

set(srcs "src/transport/tbc_transport_config.c"
         "src/transport/tbc_transport_storage.c"
         "src/mqtt/tbc_mqtt.c"
         "src/mqtt/tbc_mqtt_payload_buffer.c")

if(CONFIG_TB_MQTT_CLIENT_HELPER_ENABLE)
    list(APPEND srcs "src/helper/tbc_mqtt_helper.c"
                     "src/helper/attributes_request.c"
                     "src/helper/client_attribute.c"
                     "src/helper/client_rpc.c"
                     "src/helper/device_provision.c"
                     "src/helper/ota_update.c"
                     "src/helper/server_rpc.c"
                     "src/helper/shared_attribute.c"
                     "src/helper/timeseries_data.c"
                     "src/helper/claiming_device.c")

    #list(APPEND priv_requires "json")
    #list(APPEND requires "json")
endif()

idf_component_register(SRCS ${srcs}
                INCLUDE_DIRS ${includes}
                PRIV_INCLUDE_DIRS ${priv_includes}
                REQUIRES ${requires} )

                #PRIV_REQUIRES ${priv_requires} 
#
# coap
