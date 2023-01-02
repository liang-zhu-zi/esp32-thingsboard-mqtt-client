#set(COMPONENT_REQUIRES json)

idf_component_register(SRCS "basic_mqtt_credential_cup_w_onewayssl_main.c"
                    INCLUDE_DIRS ""
                    EMBED_TXTFILES mqtt_thingsboard_server_cert.pem)

                    
