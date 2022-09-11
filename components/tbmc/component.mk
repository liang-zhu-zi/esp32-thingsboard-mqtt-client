#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := include
COMPONENT_PRIV_INCLUDEDIRS := private_include
COMPONENT_SRCDIRS := src
ifdef CONFIG_TB_MQTT_CLIENT_HELPER_ENABLE
    COMPONENT_SRCDIRS += src/helper
endif

# Silence warning: cast between incompatible function types from 'tbmc_on_response_t' {aka 'void (*)(void *, int,  const char *, int)'} 
# to 'void (*)(void *, int,  int,  const char *, int)' [-Wcast-function-type]
src/tb_mqtt_client..o: CFLAGS += -Wcast-function-type