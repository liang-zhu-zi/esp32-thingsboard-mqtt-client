Attributes API
--------------

ThingsBoard attributes API allows devices to

* Request `client-side`__ and `shared`__ device attributes from the server.
* Upload `client-side`__ device attributes to the server.
* Subscribe to `shared`__ device attributes from the server.

.. __: https://thingsboard.io/docs/user-guide/attributes/#attribute-types
.. __: https://thingsboard.io/docs/user-guide/attributes/#attribute-types
.. __: https://thingsboard.io/docs/user-guide/attributes/#attribute-types
.. __: https://thingsboard.io/docs/user-guide/attributes/#attribute-types


Request attribute values from the server
****************************************

.. uml::

   title  Request attribute values from the server

   participant "Device" as TBDev order 10
   participant "ThingsBoard Server"  as TBSrv order 20 

   == Subscribe to client-side and shared attribute response from the server ==
   TBDev  ->  TBSrv: subscribe to attribute response (**MQTT, SUBSCRIBE**) \nTopic: **v1/devices/me/attributes/response/+**

   == Request client-side and shared attributes from the server ==
   TBDev  ->  TBSrv: request attribute values from the server (**MQTT, PUBLISH**) \nTopic: **v1/devices/me/attributes/request/$request_id** \nPayload: {"clientKeys":"attribute1,attribute2", "sharedKeys":"shared1,shared2"}
   
   TBDev <--  TBSrv: receive response (**MQTT, PUBLISH**) \nTopic: **v1/devices/me/attributes/response/$request_id** \nPayload: {"client":{"attribute1":"value1","attribute2":"value2"},\n"shared":{"shared1":"value1","shared1":"value2"}}

Before sending PUBLISH message with the attributes request, client need to **subscribe** to::

   v1/devices/me/attributes/response/+

Once subscribed, the client may request client-side or shared device attributes to ThingsBoard server node, send **PUBLISH** message to the following topic::

   v1/devices/me/attributes/request/$request_id

where **$request_id** is your integer request identifier. 

The client should receive the response to the following topic::

   v1/devices/me/attributes/response/$request_id


Example
+++++++

The following example is written in javascript and is based on mqtt.js. Pure command-line examples are not available because subscribe and publish need to happen in the same mqtt session.

+----------------+-----------------------------------+------------------------------------+------------------------------+
| Client library | Shell file                        | JavaScript file                    |  Result (JSON file)          |
+================+===================================+====================================+==============================+
| **MQTT.js**    | `mqtt-js-attributes-request.sh`_  | `mqtt-js-attributes-request.js`_   | `attributes-response.json`_  |
+----------------+-----------------------------------+------------------------------------+------------------------------+

mqtt-js-attributes-request.sh
:::::::::::::::::::::::::::::

.. code:: bash

   export TOKEN=$ACCESS_TOKEN
   node mqtt-js-attributes-request.js


mqtt-js-attributes-request.js
:::::::::::::::::::::::::::::

.. code:: javascript

   var mqtt = require('mqtt')
   var client  = mqtt.connect('mqtt://127.0.0.1',{
      username: process.env.TOKEN
   })

   client.on('connect', function () {
      console.log('connected')
      client.subscribe('v1/devices/me/attributes/response/+')
      client.publish('v1/devices/me/attributes/request/1', '{"clientKeys":"attribute1,attribute2", "sharedKeys":"shared1,shared2"}')
   })

   client.on('message', function (topic, message) {
      console.log('response.topic: ' + topic)
      console.log('response.body: ' + message.toString())
      client.end()
   })


attributes-response.json
::::::::::::::::::::::::

.. code:: json

   {"key1":"value1"}


**Please note**, the intersection of client-side and shared device attribute keys is a **bad** practice! However, it is still possible to have same keys for client, shared or even server-side attributes.


Publish attribute update to the server
**************************************

.. uml::

   title  Publish attribute update to the server

   participant "Device" as TBDev order 10
   participant "ThingsBoard Server"  as TBSrv order 20 

   TBDev  ->  TBSrv: publish client-side attributes update to the server (**MQTT, PUBLISH**) \nTopic: **v1/devices/me/attributes** \nPayload: {"attribute1":"value1","attribute2":true}


In order to publish client-side device attributes to ThingsBoard server node, send **PUBLISH** message to the following topic::

   v1/devices/me/attributes

Example
+++++++

+----------------+-------------------------------------+------------------------------------+
| Client library | Shell file                          | JSON file                          |
+================+=====================================+====================================+
| **Mosquitto**  | `mosquitto-attributes-publish.sh`_  | `new-attributes-values.json`_      |
+----------------+-------------------------------------+                                    |
| **MQTT.js**    | `mqtt-js-attributes-publish.sh`_    |                                    |
+----------------+-------------------------------------+------------------------------------+

mosquitto-attributes-publish.sh
:::::::::::::::::::::::::::::::

.. code:: bash

   # Publish client-side attributes update
   mosquitto_pub -d -h "127.0.0.1" -t "v1/devices/me/attributes" -u "$ACCESS_TOKEN" -f "new-attributes-values.json"


mqtt-js-attributes-publish.sh
:::::::::::::::::::::::::::::

.. code:: bash
   
   # Publish client-side attributes update
   cat new-attributes-values.json | mqtt pub -d -h "127.0.0.1" -t "v1/devices/me/attributes" -u '$ACCESS_TOKEN' -s


new-attributes-values.json
::::::::::::::::::::::::::

.. code:: json
   
   {
      "stringKey": "value1",
      "booleanKey": true,
      "doubleKey": 42.0,
      "longKey": 73,
      "jsonKey": {
         "someNumber": 42,
         "someArray": [1,2,3],
         "someNestedObject": {"key": "value"}
      }
   }


Subscribe to attribute updates from the server
**********************************************

.. uml::

   title  Subscribe to attribute updates from the server

   participant "Device" as TBDev order 10
   participant "ThingsBoard Server"  as TBSrv order 20 

   == Subscribe to attribute updates from the server ==
   TBDev  ->  TBSrv: subscribe to attribute response (**MQTT, SUBSCRIBE**) \nTopic: **v1/devices/me/attributes**

   == Receive the attribute update from the server ==
   TBDev  <-  TBSrv: receive attribute update from the server (**MQTT, PUBLISH**) \nTopic: **v1/devices/me/attributes** \nPayload: {"attribute1":"value1","attribute2":"value2"}


In order to subscribe to shared device attribute changes, send **SUBSCRIBE** message to the following topic::

   v1/devices/me/attributes

When a shared attribute is changed by one of the server-side components (such as the REST API or the Rule Chain), the client will **receive** the following update:

.. code:: json

   {"key1":"value1"}


Example
+++++++

+----------------+---------------------------------------+
| Client library | Shell file                            |
+================+=======================================+
| **Mosquitto**  | `mosquitto-attributes-subscribe.sh`_  |
+----------------+---------------------------------------+
| **MQTT.js**    | `mqtt-js-attributes-subscribe.sh`_    |
+----------------+---------------------------------------+

mosquitto-attributes-subscribe.sh
:::::::::::::::::::::::::::::::::

.. code:: bash

   # Subscribes to attribute updates
   mosquitto_sub -d -h "127.0.0.1" -t "v1/devices/me/attributes" -u "$ACCESS_TOKEN"

mqtt-js-attributes-subscribe.sh
:::::::::::::::::::::::::::::::

.. code:: bash
   
   # Subscribes to attribute updates
   mqtt sub -v "127.0.0.1" -t "v1/devices/me/attributes" -u '$ACCESS_TOKEN'
