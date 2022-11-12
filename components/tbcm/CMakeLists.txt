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
                     "src/helper/attributes_request_observer.c"
                     "src/helper/client_attribute_helper.c"
                     "src/helper/client_rpc_observer.c"
                     "src/helper/provision_observer.c"
                     "src/helper/ota_update_observer.c"
                     "src/helper/server_rpc_observer.c"
                     "src/helper/shared_attribute_observer.c"
                     "src/helper/timeseries_data_helper.c")

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
