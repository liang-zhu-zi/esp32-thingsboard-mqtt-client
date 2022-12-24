#set(COMPONENT_REQUIRES json)

idf_component_register(SRCS "x.509_ceritificate_with_twowayssl_main.c"
                    INCLUDE_DIRS ""
                    EMBED_TXTFILES cert.pem client.crt client.key)

                    
