set(srcs "src/tb_mqtt_client.c")

set(includes "include")
# set(priv_requires "interface")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${includes})
#                      PRIV_REQUIRES ${priv_requires}
