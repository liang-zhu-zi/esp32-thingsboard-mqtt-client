ThingsBoard MQTT API reference
###############################################


Introduction
============

See `ThingsBoard API reference`__.

.. __: https://thingsboard.io/docs/api/


ThingsBoard API consists of two main parts: **device API** and **server-side API**.


.. uml::

   interface "MQTT API" as MQTTAPI
   interface "CoAP API" as CoAPAPI
   interface "HTTP API" as HTTPAPI

   /'interface "Device API" as DeviceAPI'/
   interface "Websocket API" as WebsocketAPI
   interface "REST API" as RestAPI
   
   node "\nThingsBoard Server\n" as TBSrv {
   }

   node "Device" as TBDev {
   }

   node "Server-side Application" as TBApp {
      node "Web UI" as WebUI {
      }
   }

   /'TBSrv -left- DeviceAPI'/
   TBSrv -down-  MQTTAPI
   TBSrv -down-  CoAPAPI
   TBSrv -down-  HTTPAPI
   TBSrv -down- RestAPI
   TBSrv -down-  WebsocketAPI

   /'TBDev .up.> DeviceAPI'/
   TBDev .up.> MQTTAPI : **Device API**
   TBDev .up.> CoAPAPI : **Device API**
   TBDev .up.> HTTPAPI : **Device API**

   TBApp -up-> RestAPI : **Server-side API**
   WebUI -up-> WebsocketAPI : **Server-side API**

   note bottom of TBDev
      Switch one in MQTT API, 
      CoAP API and HTTP API
   end note


* **Server-side API** is available as *REST API* and *Websocket API*:
   * *REST API*:
      * `Administration REST API`__ - The server-side core APIs.
      * `Attributes query API`__ - The server-side APIs provided by `Telemetry Service`__.
      * `Timeseries query API`__ - The server-side APIs provided by `Telemetry Service`__.
      * `RPC API`__ - The server-side APIs provided by `RPC Service`__.
      * `REST Client`__ 
   * *Websocket API*:
      * `Websocket API`__ duplicates REST API functionality and provides the ability to subscribe to device data changes. 

.. __: https://thingsboard.io/docs/reference/rest-api
.. __: https://thingsboard.io/docs/user-guide/attributes/#data-query-api
.. __: https://thingsboard.io/docs/user-guide/attributes/
.. __: https://thingsboard.io/docs/user-guide/telemetry/#data-query-api
.. __: https://thingsboard.io/docs/user-guide/telemetry/
.. __: https://thingsboard.io/docs/user-guide/rpc/#server-side-rpc-api
.. __: https://thingsboard.io/docs/user-guide/rpc/
.. __: https://thingsboard.io/docs/reference/rest-client

.. __: https://thingsboard.io/docs/user-guide/telemetry/#websocket-api


* **Device API** is grouped by supported communication protocols:
   * `MQTT API`__
   * `CoAP API`__
   * `HTTP API`__

.. __: https://thingsboard.io/docs/reference/mqtt-api
.. __: https://thingsboard.io/docs/reference/coap-api
.. __: https://thingsboard.io/docs/reference/http-api


MQTT API 
========

See `MQTT Device API Reference`__.

.. __: https://thingsboard.io/docs/reference/mqtt-api/


Getting started
---------------

MQTT basics
***********

`MQTT`__ is a lightweight publish-subscribe messaging protocol which probably makes it the most suitable for various IoT devices. You can find more information about MQTT `here`__.

ThingsBoard server nodes act as an MQTT Broker that supports QoS levels 0 (at most once) and 1 (at least once) and a set of predefined topics.

.. __: https://en.wikipedia.org/wiki/MQTT
.. __: http://mqtt.org/


Client libraries setup
**********************

You can find a large number of MQTT client libraries on the web. Examples in this article will be based on Mosquitto and MQTT.js. In order to setup one of those tools, you can use instructions in our `Hello World`__ guide.

.. __: https://thingsboard.io/docs/getting-started-guides/helloworld/

MQTT Connect
************

We will use access token device credentials in this article and they will be referred to later as **$ACCESS_TOKEN**. The application needs to send MQTT CONNECT message with username that contains **$ACCESS_TOKEN**. Possible return codes and their reasons during connect sequence:

* **0x00 Connected** - Successfully connected to ThingsBoard MQTT server.
* **0x04 Connection Refused, bad user name or password** - Username is empty.
* **0x05 Connection Refused, not authorized** - Username contains invalid **$ACCESS_TOKEN**.

Key-value format
----------------

By default, ThingsBoard supports key-value content in **JSON**. Key is always a string, while value can be either string, boolean, double, long or JSON. Using custom binary format or some serialization framework is also possible. See `Protocol customization`_ for more details. For example:

.. code:: json

   {
      "stringKey":"value1", 
      "booleanKey":true, 
      "doubleKey":42.0, 
      "longKey":73, 
      "jsonKey": {
         "someNumber": 42,
         "someArray": [1,2,3],
         "someNestedObject": {"key": "value"}
      }
   }

JSON value support
--------------------

TODO: ...






Protocol customization
----------------------

MQTT transport can be fully customized for specific use-case by changing the corresponding `module`__.

.. __: https://github.com/thingsboard/thingsboard/tree/master/transport/mqtt



Device MQTT Topic 
-----------------

.. role:: strike
    :class: strike

.. list-table:: Device MQTT Topic 
   :widths: auto
   :header-rows: 1

   * - Function \ Topic
     - Subscribe
     - Tx
     - Rx

   * - Telemetry
     - 
     - ① v1/devices/me/telemetry
     - 

   * - 
     - 
     - 
     - 
   * - Request attributes
     - ① v1/devices/me/attributes/response/+
     - ② v1/devices/me/attributes/request/$request_id
     - ③ v1/devices/me/attributes/response/$request_id
   * - Publish attributes
     - 
     - ① v1/devices/me/attributes
     - 
   * - Subscribe attributes update
     - ① v1/devices/me/attributes
     - 
     - ② v1/devices/me/attributes

   * - 
     - 
     - 
     - 
   * - Server-Side RPC
     - ① v1/devices/me/rpc/request/+
     - ③ v1/devices/me/rpc/response/$request_id
     - ② v1/devices/me/rpc/request/$request_id
   * - Client-Side RPC
     - ① v1/devices/me/rpc/response/+
     - ② v1/devices/me/rpc/request/$request_id
     - ③ v1/devices/me/rpc/response/$request_id

   * - 
     - 
     - 
     - 
   * - Claiming device
     - 
     - :strike:`① v1/devices/me/claim`
     - 


**Note**: ①②③ The order in which topics are performed.
