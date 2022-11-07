PRC API
------------

Server-side RPC
***************

.. uml::

   title  Server-side RPC

   participant "Device" as TBDev order 10
   participant "ThingsBoard Server"  as TBSrv order 20 

   == Subscribe to sever-side RPC request from the server ==
   TBDev  ->  TBSrv: subscribe to sever-side RPC request (**MQTT, SUBSCRIBE**) \nTopic: **v1/devices/me/rpc/request/+**

   == Receive two-way sever-side RPC request from the server ==
   TBDev  <-  TBSrv: receive server-side RPC request from the server (**MQTT, PUBLISH**) \nTopic: **v1/devices/me/rpc/request/$request_id** \nPayload: {"method":"remoteOTA","params":"http://192.168.xx.xxx/abc.bin"}
   
   TBDev -->  TBSrv: send response (**MQTT, PUBLISH**) \nTopic: **v1/devices/me/rpc/response/$request_id** \nPayload: {"method":"remoteOTA","results":{"result":"success"}}

   == Receive one-way sever-side RPC request from the server ==
   TBDev  <-  TBSrv: receive server-side RPC request from the server (**MQTT, PUBLISH**) \nTopic: **v1/devices/me/rpc/request/$request_id** \nPayload: {"method":"setSpValue","params":14.5}


In order to subscribe to RPC commands from the server, send **SUBSCRIBE** message to the following topic::

   v1/devices/me/rpc/request/+

Once subscribed, the client will receive individual commands as a **PUBLISH** message to the corresponding topic::

   v1/devices/me/rpc/request/$request_id

where **$request_id** is an integer request identifier.

The client should publish the response to the following topic::

   v1/devices/me/rpc/response/$request_id


Example
+++++++

The following example is written in javascript and is based on mqtt.js. Pure command-line examples are not available because subscribe and publish need to happen in the same mqtt session.

+----------------+-----------------------------------+------------------------------------+
| Client library | Shell file                        | JavaScript file                    |
+================+===================================+====================================+
| **MQTT.js**    | `mqtt-js-rpc-from-server.sh`_     | `mqtt-js-rpc-from-server.js`_      |
+----------------+-----------------------------------+------------------------------------+

mqtt-js-rpc-from-server.sh
::::::::::::::::::::::::::

.. code:: bash

   export TOKEN=$ACCESS_TOKEN
   node mqtt-js-rpc-from-server.js

mqtt-js-rpc-from-server.js
:::::::::::::::::::::::::::::::

.. code:: javascript
   
   var mqtt = require('mqtt');
   var client  = mqtt.connect('mqtt://127.0.0.1',{
      username: process.env.TOKEN
   });

   client.on('connect', function () {
      console.log('connected');
      client.subscribe('v1/devices/me/rpc/request/+')
   });

   client.on('message', function (topic, message) {
      console.log('request.topic: ' + topic);
      console.log('request.body: ' + message.toString());
      var requestId = topic.slice('v1/devices/me/rpc/request/'.length);
      //client acts as an echo service
      client.publish('v1/devices/me/rpc/response/' + requestId, message);
   });


Client-side RPC
***************

.. uml::

   title  Client-side RPC

   participant "Device" as TBDev order 10
   participant "ThingsBoard Server"  as TBSrv order 20 

   == Subscribe to client-side RPC response from the server ==
   TBDev  ->  TBSrv: subscribe to client-side RPC response (**MQTT, SUBSCRIBE**) \nTopic: **v1/devices/me/rpc/response/+**

   == Publish client-side RPC request ==
   TBDev  ->  TBSrv: publish client-side RPC request (**MQTT, PUBLISH**) \nTopic: **v1/devices/me/rpc/request/$request_id** \nPayload: {"method":"getTime","params":{}}
   
   TBDev <--  TBSrv: receive response (**MQTT, PUBLISH**) \nTopic: **v1/devices/me/rpc/response/$request_id** \n{"method":"getTime","results":{"utcDateime":"2020-06-18T09:16:59Z"}}


In order to subscribe to client-side RPC response from the server, send **SUBSCRIBE** message to the following topic::

   v1/devices/me/rpc/response/+

Once subscribed, the client may send **PUBLISH** message to the following topic::

   v1/devices/me/rpc/request/$request_id

where **$request_id** is an integer request identifier. The response from server will be published to the following topic::

   v1/devices/me/rpc/response/$request_id


Example
+++++++

The following example is written in javascript and is based on mqtt.js. Pure command-line examples are not available because subscribe and publish need to happen in the same mqtt session.

+----------------+-----------------------------------+------------------------------------+
| Client library | Shell file                        | JavaScript file                    |
+================+===================================+====================================+
| **MQTT.js**    | `mqtt-js-rpc-from-client.sh`_     | `mqtt-js-rpc-from-client.js`_      |
+----------------+-----------------------------------+------------------------------------+


mqtt-js-rpc-from-client.sh
::::::::::::::::::::::::::

.. code:: bash

   export TOKEN=$ACCESS_TOKEN
   node mqtt-js-rpc-from-client.js


mqtt-js-rpc-from-client.js
::::::::::::::::::::::::::

.. code:: javascript
   
   var mqtt = require('mqtt');
   var client = mqtt.connect('mqtt://127.0.0.1', {
      username: process.env.TOKEN
   });

   client.on('connect', function () {
      console.log('connected');
      client.subscribe('v1/devices/me/rpc/response/+');
      var requestId = 1;
      var request = {
         "method": "getTime",
         "params": {}
      };
      client.publish('v1/devices/me/rpc/request/' + requestId, JSON.stringify(request));
   });

   client.on('message', function (topic, message) {
      console.log('response.topic: ' + topic);
      console.log('response.body: ' + message.toString());
   });

