*************
Provision
*************

* 一台 Device 能否多次 Provision?

* `MQTT Device API Reference - Device provisioning`_
* `Provision Device APIs - MQTT Device APIs`_

.. _MQTT Device API Reference - Device provisioning: https://thingsboard.io/docs/reference/mqtt-api/#device-provisioning
.. _Provision Device APIs - MQTT Device APIs: https://thingsboard.io/docs/user-guide/device-provisioning/?mqttprovisioning=access-token#mqtt-device-apis

delay 9 times : 0, 1, 2, 4, 8, 16, 32, 64, 128 senconds

.. uml::

    title  Provision

    state "checkCredentials" as checkCredentials <<choice>>
    state "Front Connection" as FrontConn {
        state "Connecting" as Connecting
        state "CheckRetryConnTimes" as CheckRetryConnTimes <<choice>>
        state "Connected" as Connected {
            state "ProvisionRequest" as ProvisionRequest
            state "CheckRetryRequestTimes" as CheckRetryRequestTimes <<choice>>
            [*] --> ProvisionRequest
        }
        [*] --> Connecting
    }
    state "Normal Connection" as NormalConn

    [*] --> checkCredentials : tbc_transport_credentials_memory_get()

    checkCredentials --> FrontConn : [no credentials]
    checkCredentials --> NormalConn : [has credentials]

    FrontConn : [Entry]/ tbc_transport_credentials_memory_clean()

        Connecting: connect_retry_times = 0;
        Connecting: [Entry]/ _provision_connect()
        Connecting --> Connected: [Connection succeeded]/
        Connecting --> CheckRetryConnTimes: [Connection failed or Timeout]/

        CheckRetryConnTimes --> Connecting : [connect_retry_times++<=9]/
        CheckRetryConnTimes --> [*] : [connect_retry_times++>9]/

        Connected : [Entry]/ subscribeResponse(), sendRequest()
        Connected : [Exit]/ _provision_disconnect()

            ProvisionRequest: request_retry_times = 0;
            ProvisionRequest: [Entry]/ sendRequest()
            ProvisionRequest --> NormalConn : [response successed]/ tbc_transport_credentials_memory_save()
            ProvisionRequest --> CheckRetryRequestTimes: [response failed or Timeout]/

            CheckRetryRequestTimes --> ProvisionRequest : [request_retry_times++<=9]/
            CheckRetryRequestTimes --> [*] : [request_retry_times++>9]/

    NormalConn : [Entry]/ tbc_transport_credentials_memory_get()
    NormalConn --> [*]

