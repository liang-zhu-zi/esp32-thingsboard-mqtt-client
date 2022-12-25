#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

#COMPONENT_REQUIRES := json

# embed files from the "certs" directory as binary data symbols
# in the app
COMPONENT_EMBED_TXTFILES := mqtt_thingsboard_server_cert.pem
# COMPONENT_EMBED_TXTFILES += client.crt
# COMPONENT_EMBED_TXTFILES += client.key
