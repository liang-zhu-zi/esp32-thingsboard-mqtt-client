Device Attributes
===========================


Working with IoT device attributes
----------------------------------

See `Working with IoT device attributes`__.

.. __: https://thingsboard.io/docs/user-guide/attributes/


Attributes are treated key-value pairs. Flexibility and simplicity of the key-value format allow easy and seamless integration with almost any IoT device on the market.

Device specific attributes are separated into two main groups:

* **client-side attributes** - attributes are reported and managed by the device application. For example current software/firmware version, hardware specification, etc.

* **shared attributes** - attributes are reported and managed by the server-side application. Visible to the device application. For example customer subscription plan, target software/firmware version.

.. uml::

   title  Device attributes

   participant "Device" as TBDev order 10
   participant "ThingsBoard Server"  as TBSrv order 20 
   participant "Server-side Application" as TBApp  order 30

   == Request client-side and shared attributes from the server ==
   TBDev  ->  TBSrv: request attribute values from the server (**Device API**)
   TBDev <--  TBSrv: receive response (**Device API**)

   == Upload client-side attributes to the server ==
   TBDev ->  TBSrv: client-side attributes update to the server(**Device API**)

   == Subscribe to updates of shared attributes from the server ==
   TBDev ->  TBSrv: subscribe to updates of shared attributes (**Device API**)
   TBDev <-  TBSrv: updates of shared attributes (**Device API**)

   == Attribute Data Query ==
   TBSrv  <-  TBApp: Attribute keys API (**REST API**)
   TBSrv  <-  TBApp: Attribute values API (**REST API**)
   ... other **Attribute Data Query API** ...

TODO: move to api!!!!

Publish attribute update to the server
------------------------------------------------

Request attribute values from the server
------------------------------------------------

Subscribe to attribute updates from the server
------------------------------------------------