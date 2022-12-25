#set(COMPONENT_REQUIRES json)

idf_component_register(SRCS "x.509_ceritificate_w_twowayssl_main.c"
                    INCLUDE_DIRS ""
                    EMBED_TXTFILES mqtt_thingsboard_server_cert.pem client.crt client.key)

                    
