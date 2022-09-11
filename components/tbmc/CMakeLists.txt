set(srcs "src/tb_mqtt_client.c")
set(includes "include")
set(priv_includes "private_include")
set(priv_requires "mqtt")
set(requires "mqtt")

if(CONFIG_TB_MQTT_CLIENT_HELPER_ENABLE)
    list(APPEND srcs "src/helper/tb_mqtt_client_helper.c"
                    "src/helper/attributes_request_observer.c"
                    "src/helper/client_attribute_helper.c"
                    "src/helper/client_rpc_observer.c"
                    "src/helper/fw_update_observer.c"
                    "src/helper/server_rpc_observer.c"
                    "src/helper/shared_attribute_observer.c"
                    "src/helper/timeseries_data_helper.c")

    list(APPEND priv_requires "json")
    list(APPEND requires "json")
endif()

idf_component_register(SRCS ${srcs}
                INCLUDE_DIRS ${includes}
                PRIV_INCLUDE_DIRS ${priv_includes}
                PRIV_REQUIRES ${priv_requires} )
#REQUIRES ${requires}
# coap

# Silence warning: cast between incompatible function types from 'tbmc_on_response_t' {aka 'void (*)(void *, int,  const char *, int)'} 
# to 'void (*)(void *, int,  int,  const char *, int)' [-Wcast-function-type]
set_source_files_properties(src/tb_mqtt_client.c PROPERTIES COMPILE_FLAGS -Wcast-function-type)