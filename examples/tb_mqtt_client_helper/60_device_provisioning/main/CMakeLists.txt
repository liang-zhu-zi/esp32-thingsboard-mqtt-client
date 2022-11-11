#set(COMPONENT_REQUIRES json)

idf_component_register(SRCS "device_provisioning_main.c"
							"tbc_front_connection.c"
							"tbc_normal_connection.c"
                            "tbc_transport_credentials_memory.c"
                    INCLUDE_DIRS ""
                    EMBED_TXTFILES cert.pem client.crt client.key)

                    
