#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

#COMPONENT_REQUIRES := json
COMPONENT_EMBED_TXTFILES := cert.pem
COMPONENT_EMBED_TXTFILES += client.crt
COMPONENT_EMBED_TXTFILES += client.key