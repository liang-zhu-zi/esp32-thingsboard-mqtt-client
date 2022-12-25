#set(COMPONENT_REQUIRES json)

idf_component_register(SRCS "srv_gen_credentials_w_onewayssl_main.c"
							"tbc_front_connection.c"
							"tbc_normal_connection.c"
                            "tbc_transport_credentials_memory.c"
                    INCLUDE_DIRS ""
                    EMBED_TXTFILES mqtt_thingsboard_server_cert.pem)

                    
