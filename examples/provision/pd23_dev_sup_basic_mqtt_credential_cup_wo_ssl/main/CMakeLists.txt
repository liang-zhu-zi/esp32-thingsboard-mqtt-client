#set(COMPONENT_REQUIRES json)

idf_component_register(SRCS "dev_sup_basic_mqtt_credential_cup_wo_ssl_main.c"
							"tbc_front_connection.c"
							"tbc_normal_connection.c"
                            "tbc_transport_credentials_memory.c"
                    INCLUDE_DIRS "")

                    
