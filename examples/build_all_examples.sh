set echo off

echo getstarted/g00_hello_world
cd getstarted/g00_hello_world
idf.py build > log.txt
cd ../..

echo ================================================

echo helper/h10_telemetry_upload
cd helper/h10_telemetry_upload
idf.py build > log.txt
cd ../..

echo helper/h20_attributes_update
cd helper/h20_attributes_update
idf.py build > log.txt
cd ../..

echo helper/h21_attributes_subscribe
cd helper/h21_attributes_subscribe
idf.py build > log.txt
cd ../..

echo helper/h22_attributes_request
cd helper/h22_attributes_request
idf.py build > log.txt
cd ../..

echo helper/h30_server_side_rpc
cd helper/h30_server_side_rpc
idf.py build > log.txt
cd ../..

echo helper/h31_client_side_rpc
cd helper/h31_client_side_rpc
idf.py build > log.txt
cd ../..

echo helper/h40_claiming_device
cd helper/h40_claiming_device
idf.py build > log.txt
cd ../..

echo helper/h50_fw_update
cd helper/h50_fw_update
idf.py build > log.txt
cd ../..

echo helper/h60_device_provisioning
cd helper/h60_device_provisioning
idf.py build > log.txt
cd ../..

echo ================================================

echo extension/e10_timeseries_data
cd extension/e10_timeseries_data
idf.py build > log.txt
cd ../..


echo extension/e20_client_attributes
cd extension/e20_client_attributes
idf.py build > log.txt
cd ../..


echo extension/e30_shared_attributes
cd extension/e30_shared_attributes
idf.py build > log.txt
cd ../..


echo ================================================

echo authentication/a11_access_token_without_ssl
cd authentication/a11_access_token_without_ssl
idf.py build > log.txt
cd ..\..

echo authentication/a12_access_token_with_onewayssl
cd authentication/a12_access_token_with_onewayssl
idf.py build > log.txt
cd ..\..

echo authentication/a21_basic_mqtt_credential_c_without_ssl
cd authentication/a21_basic_mqtt_credential_c_without_ssl
idf.py build > log.txt
cd ..\..

echo authentication/a22_basic_mqtt_credential_u_p_without_ssl
cd authentication/a22_basic_mqtt_credential_u_p_without_ssl
idf.py build > log.txt
cd ..\..

echo authentication/a23_basic_mqtt_credential_c_u_p_without_ssl
cd authentication/a23_basic_mqtt_credential_c_u_p_without_ssl
idf.py build > log.txt
cd ..\..

echo authentication/a24_basic_mqtt_credential_c_u_p_with_onewayssl
cd authentication/a24_basic_mqtt_credential_c_u_p_with_onewayssl
idf.py build > log.txt
cd ..\..

echo authentication/a31_x.509_ceritificate_with_twowayssl
cd authentication/a31_x.509_ceritificate_with_twowayssl
idf.py build > log.txt
cd ..\..

echo ================================================


set echo on