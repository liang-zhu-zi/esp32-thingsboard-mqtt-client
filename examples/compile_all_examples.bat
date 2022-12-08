@echo off

echo helper\00_hello_world
cd helper\00_hello_world 
idf.py build > log.txt
cd ..\..

echo "================================================"

echo helper\10_telemetry_upload
cd helper\10_telemetry_upload
idf.py build > log.txt
cd ..\..

echo helper\20_attributes_update
cd helper\20_attributes_update
idf.py build > log.txt
cd ..\..

echo helper\21_attributes_subscribe
cd helper\21_attributes_subscribe
idf.py build > log.txt
cd ..\..

echo helper\22_attributes_request
cd helper\22_attributes_request
idf.py build > log.txt
cd ..\..

echo helper\30_server_side_rpc
cd helper\30_server_side_rpc
idf.py build > log.txt
cd ..\..

echo helper\31_client_side_rpc
cd helper\31_client_side_rpc
idf.py build > log.txt
cd ..\..

echo helper\40_claiming_device
cd helper\40_claiming_device
idf.py build > log.txt
cd ..\..

echo helper\50_fw_update
cd helper\50_fw_update
idf.py build > log.txt
cd ..\..

echo helper\60_device_provisioning
cd helper\60_device_provisioning
idf.py build > log.txt
cd ..\..

echo "================================================"

echo extension\10_timeseries_data
cd extension\10_timeseries_data
idf.py build > log.txt
cd ..\..


echo extension\20_client_attributes
cd extension\20_client_attributes
idf.py build > log.txt
cd ..\..


echo extension\30_shared_attributes
cd extension\30_shared_attributes
idf.py build > log.txt
cd ..\..