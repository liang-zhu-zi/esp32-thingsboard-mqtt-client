set(srcs "src/thingsboard_mqttclient.c")

set(includes "include")
# set(priv_requires "interface")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${includes})
#                      PRIV_REQUIRES ${priv_requires}
