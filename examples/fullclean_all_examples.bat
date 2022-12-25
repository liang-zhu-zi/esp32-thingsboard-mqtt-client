@echo off

echo ================================================

echo getstarted\g00_hello_world
cd getstarted\g00_hello_world 
idf.py fullclean > log.txt
cd ..\..

echo ================================================

echo helper\h10_telemetry_upload
cd helper\h10_telemetry_upload
idf.py fullclean > log.txt
cd ..\..

echo helper\h20_attributes_update
cd helper\h20_attributes_update
idf.py fullclean > log.txt
cd ..\..

echo helper\h21_attributes_subscribe
cd helper\h21_attributes_subscribe
idf.py fullclean > log.txt
cd ..\..

echo helper\h22_attributes_request
cd helper\h22_attributes_request
idf.py fullclean > log.txt
cd ..\..

echo helper\h30_server_side_rpc
cd helper\h30_server_side_rpc
idf.py fullclean > log.txt
cd ..\..

echo helper\h31_client_side_rpc
cd helper\h31_client_side_rpc
idf.py fullclean > log.txt
cd ..\..

echo helper\h40_claiming_device
cd helper\h40_claiming_device
idf.py fullclean > log.txt
cd ..\..

echo helper\h50_fw_update
cd helper\h50_fw_update
idf.py fullclean > log.txt
cd ..\..

echo ================================================

echo extension\e10_timeseries_data
cd extension\e10_timeseries_data
idf.py fullclean > log.txt
cd ..\..


echo extension\e20_client_attributes
cd extension\e20_client_attributes
idf.py fullclean > log.txt
cd ..\..


echo extension\e30_shared_attributes
cd extension\e30_shared_attributes
idf.py fullclean > log.txt
cd ..\..


echo ================================================

echo authentication\a11_access_token_wo_ssl
cd authentication\a11_access_token_wo_ssl
idf.py fullclean > log.txt
cd ..\..

echo authentication\a12_access_token_w_onewayssl
cd authentication\a12_access_token_w_onewayssl
idf.py fullclean > log.txt
cd ..\..

echo authentication\a21_basic_mqtt_credential_c_wo_ssl
cd authentication\a21_basic_mqtt_credential_c_wo_ssl
idf.py fullclean > log.txt
cd ..\..

echo authentication\a22_basic_mqtt_credential_up_wo_ssl
cd authentication\a22_basic_mqtt_credential_up_wo_ssl
idf.py fullclean > log.txt
cd ..\..

echo authentication\a23_basic_mqtt_credential_cup_wo_ssl
cd authentication\a23_basic_mqtt_credential_cup_wo_ssl
idf.py fullclean > log.txt
cd ..\..

echo authentication\a24_basic_mqtt_credential_cup_w_onewayssl
cd authentication\a24_basic_mqtt_credential_cup_w_onewayssl
idf.py fullclean > log.txt
cd ..\..

echo authentication\a31_x.509_ceritificate_w_twowayssl
cd authentication\a31_x.509_ceritificate_w_twowayssl
idf.py fullclean > log.txt
cd ..\..

echo ================================================

echo provision\pd11_dev_sup_access_token_wo_ssl
cd provision\pd11_dev_sup_access_token_wo_ssl
idf.py fullclean > log.txt
cd ..\..

echo provision\pd12_dev_sup_access_token_w_onewayssl
cd provision\pd12_dev_sup_access_token_w_onewayssl
idf.py fullclean > log.txt
cd ..\..

echo provision\pd21_dev_sup_basic_mqtt_credential_c_wo_ssl
cd provision\pd21_dev_sup_basic_mqtt_credential_c_wo_ssl
idf.py fullclean > log.txt
cd ..\..

echo provision\pd22_dev_sup_basic_mqtt_credential_up_wo_ssl
cd provision\pd22_dev_sup_basic_mqtt_credential_up_wo_ssl
idf.py fullclean > log.txt
cd ..\..

echo provision\pd23_dev_sup_basic_mqtt_credential_cup_wo_ssl
cd provision\pd23_dev_sup_basic_mqtt_credential_cup_wo_ssl
idf.py fullclean > log.txt
cd ..\..

echo provision\pd24_dev_sup_basic_mqtt_credential_cup_w_onewayssl
cd provision\pd24_dev_sup_basic_mqtt_credential_cup_w_onewayssl
idf.py fullclean > log.txt
cd ..\..

echo provision\pd31_dev_sup_x.509_ceritificate_w_twowayssl
cd provision\pd31_dev_sup_x.509_ceritificate_w_twowayssl
idf.py fullclean > log.txt
cd ..\..


echo provision\ps10_srv_gen_credentials_wo_ssl
cd provision\ps10_srv_gen_credentials_wo_ssl
idf.py fullclean > log.txt
cd ..\..

echo provision\ps20_srv_gen_credentials_w_onewayssl
cd provision\ps20_srv_gen_credentials_w_onewayssl
idf.py fullclean > log.txt
cd ..\..

echo provision\px00_device_provisioning
cd provision\px00_device_provisioning
idf.py fullclean > log.txt
cd ..\..

echo ================================================

