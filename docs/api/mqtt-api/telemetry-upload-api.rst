Telemetry upload API
===========================

## 前提

### tb是什么

### tb的架构

### xxx是什么

## flow chart

### chart

### description

### note

## msg

### note xxx

### example

## tips

* key的长度
* msg的周期
* 可以有几个telemetry消息

## Next steps

## Relation documents

### Client API


所有功能/通信协的描述，都采用下面的模板:

1. 功能描述 Function description
1. 流程图 Flow chart
1. 编程接口参考 API reference
1. 评论 Review
1. 操作流程 Operation
1. 客户端示例 Client examples

   1. Python example Scripts
   1. TBMQTH example

1. 参考 References







Flow Chart
--------------------

.. uml::

   title  Telemetry upload

   participant "Device" as TBDev order 10
   participant "ThingsBoard Server"  as TBSrv order 20 

   TBDev  ->  TBSrv: Telemetry upload (**MQTT, PUBLISH**) \nTopic: **v1/devices/me/telemetry** \nPayload: {"key1":"value1", "key2":"value2"} or \nPayload: [{"key1":"value1"}, {"key2":"value2"}] or \nPayload: {"ts":1451649600512, "values":{"key1":"value1", "key2":"value2"}}


In order to publish telemetry data to ThingsBoard server node, send PUBLISH message to the following topic::

   v1/devices/me/telemetry

The simplest supported data formats are:

.. code:: json

   {"key1":"value1", "key2":"value2"}

or

.. code:: json

   [{"key1":"value1"}, {"key2":"value2"}]

**Please note** that in this case, the server-side timestamp will be assigned to uploaded data!

In case your device is able to get the client-side timestamp, you can use following format:

.. code:: json

   {"ts":1451649600512, "values":{"key1":"value1", "key2":"value2"}}

In the example above, we assume that “1451649600512” is a `unix timestamp`__ with milliseconds precision. For example, the value ‘1451649600512’ corresponds to ‘Fri, 01 Jan 2016 12:00:00.512 GMT’

.. __: https://en.wikipedia.org/wiki/Unix_time


Example
*******

+----------------+----------------------------+------------------------------------+
| Client library | Shell file                 | JSON file                          |
+================+============================+====================================+
| **Mosquitto**  | `mosquitto-telemetry.sh`_  | - `telemetry-data-as-object.json`_ |
+----------------+----------------------------+ - `telemetry-data-as-array.json`_  |
| **MQTT.js**    | `mqtt-js-telemetry.sh`_    | - `telemetry-data-with-ts.json`_   |
+----------------+----------------------------+------------------------------------+

mosquitto-telemetry.sh
++++++++++++++++++++++

.. code:: bash

   # Publish data as an object without timestamp (server-side timestamp will be used)
   mosquitto_pub -d -h "127.0.0.1" -t "v1/devices/me/telemetry" -u "$ACCESS_TOKEN" -f "telemetry-data-as-object.json"
   # Publish data as an array of objects without timestamp (server-side timestamp will be used)
   mosquitto_pub -d -h "127.0.0.1" -t "v1/devices/me/telemetry" -u "$ACCESS_TOKEN" -f "telemetry-data-as-array.json"
   # Publish data as an object with timestamp (server-side timestamp will be used)
   mosquitto_pub -d -h "127.0.0.1" -t "v1/devices/me/telemetry" -u "$ACCESS_TOKEN" -f "telemetry-data-with-ts.json"


mqtt-js-telemetry.sh
++++++++++++++++++++

.. code:: bash

   # Publish data as an object without timestamp (server-side timestamp will be used)
   cat telemetry-data-as-object.json | mqtt pub -v -h "127.0.0.1" -t "v1/devices/me/telemetry" -u '$ACCESS_TOKEN' -s
   # Publish data as an array of objects without timestamp (server-side timestamp will be used)
   cat telemetry-data-as-array.json | mqtt pub -v -h "127.0.0.1" -t "v1/devices/me/telemetry" -u '$ACCESS_TOKEN' -s
   # Publish data as an object with timestamp (server-side timestamp will be used)
   cat telemetry-data-with-ts.json | mqtt pub -v -h "127.0.0.1" -t "v1/devices/me/telemetry" -u '$ACCESS_TOKEN' -s


telemetry-data-as-object.json
+++++++++++++++++++++++++++++

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


telemetry-data-as-array.json
++++++++++++++++++++++++++++

.. code:: json
   
   [{"key1":"value1"}, {"key2":true}]


telemetry-data-with-ts.json
+++++++++++++++++++++++++++

.. code:: json
   
   {
      "ts": 1451649600512,
      "values": {
         "stringKey": "value1",
         "booleanKey": true,
         "doubleKey": 42.0,
         "longKey": 73,
         "jsonKey": {
            "someNumber": 42,
            "someArray": [1, 2, 3],
            "someNestedObject": {
            "key": "value"
            }
         }
      }
   }