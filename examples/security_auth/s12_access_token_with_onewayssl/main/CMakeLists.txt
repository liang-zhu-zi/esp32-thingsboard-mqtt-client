#set(COMPONENT_REQUIRES json)

idf_component_register(SRCS "access_token_with_onewayssl_main.c"
                    INCLUDE_DIRS ""
                    EMBED_TXTFILES cert.pem)

                    
