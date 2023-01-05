*****************
OTA updates 
*****************

Refer to `MQTT Device API Reference - Firmware API <https://thingsboard.io/docs/reference/mqtt-api/#firmware-api>`_.

**Tips**: In this article, **client** refers to **device**.

Firmware OTA updates
=====================

**Tips**: Firmware attributes are some shared attributes.

Request firmware attributes values from the server
----------------------------------------------------

.. uml::

   title  Device attributes

   participant "Device" as TBDev order 10
   participant "ThingsBoard Server"  as TBSrv order 20 

   TBDev  ->  TBSrv: TODO


1. Prerequisites
    * Control Packet type: SUBSCRIBE (Client subscribe request)
    * Direction of flow:        Client to Server
    * Payload-Topic Filter:     `???`

#. Send request of firmware attributes

#. Handle response of firmware attributes

Subscribe to firmware attributes updates from the server
---------------------------------------------------------

.. uml::

   title  Device attributes

   participant "Device" as TBDev order 10
   participant "ThingsBoard Server"  as TBSrv order 20 

   TBDev  ->  TBSrv: TODO


1. Prerequisites
    * Control Packet type: SUBSCRIBE (Client subscribe request)
    * Direction of flow:        Client to Server
    * Payload-Topic Filter:     `firmware attributes updates`

#. Handle PUBLISH message of firmware attributes updates
    * Control Packet type: SUBSCRIBE (Client subscribe request)
    * Direction of flow:        Server to Client
    * Payload-Topic Filter:     `firmware attributes updates`


Request firmware chunk from the server
----------------------------------------

1. Prerequisites
    * Control Packet type: SUBSCRIBE (Client subscribe request)
    * Direction of flow:        Client to Server
    * Payload-Topic Filter:     `???`

#. Send request of firmware chunk

#. Handle response of firmware chunk

#. Send telemetry of current firmware status


Software OTA updates
=====================

所有应用于Firmware的操作, 也可以应用于 Software。相关的协议, 把 `fw_...` 替换成 `sw_...` 即可。

ThingsBoard Operation
========================

* Device 的 OTA updates 覆盖 Device profile 的 OTA updates.

References
===========

1. `Firmware API <https://thingsboard.io/docs/reference/mqtt-api/#firmware-api>`_ - ThingsBoard.
