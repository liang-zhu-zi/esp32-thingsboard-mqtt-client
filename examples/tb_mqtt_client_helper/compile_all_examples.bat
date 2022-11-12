@echo off

echo 00_hello_world
cd 00_hello_world 
idf.py build > log.txt
cd ..

echo 10_telemetry_upload
cd 10_telemetry_upload
idf.py build > log.txt
cd ..

echo 20_client_attribute
cd 20_client_attribute
idf.py build > log.txt
cd ..

echo 21_shared_attribute
cd 21_shared_attribute
idf.py build > log.txt
cd ..

echo 22_attribute_request
cd 22_attribute_request
idf.py build > log.txt
cd ..

echo 30_server_side_rpc
cd 30_server_side_rpc
idf.py build > log.txt
cd ..

echo 31_client_side_rpc
cd 31_client_side_rpc
idf.py build > log.txt
cd ..

echo 40_claiming_device
cd 40_claiming_device
idf.py build > log.txt
cd ..

echo 50_fw_update
cd 50_fw_update
idf.py build > log.txt
cd ..

echo 60_device_provisioning
cd 60_device_provisioning
idf.py build > log.txt
cd ..
