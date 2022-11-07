RPC
========


Using RPC capabilities
----------------------

See `Using RPC capabilities`__.

.. __: https://thingsboard.io/docs/user-guide/rpc/


Thinsboard RPC feature can be divided into two types based on originator: device-originated and server-originated RPC calls. In order to use more familiar names, we will name device-originated RPC calls as a **client-side RPC** calls and server-originated RPC calls as **server-side RPC** calls.


Client-side RPC
***************

.. uml::

   title  Client-side RPC

   participant "Device" as TBDev order 10
   participant "ThingsBoard Server"  as TBSrv order 20 
   participant "Server-side Application" as TBApp  order 30

   TBDev   ->  TBSrv: Client-side RPC Request (**Device API**)
   TBSrv   ->  TBApp: Client-side RPC Request API (**REST API**)
   TBApp  -->  TBSrv: Client-side RPC response API (**REST API**)
   TBSrv  -->  TBDev: Client-side RPC Response (**Device API**)



Server-side RPC
***************

Server-side RPC calls can be divided into one-way and two-way:

* **One-way server-side RPC** request is sent to the device without delivery confirmation and obviously, does not provide any response from the device. RPC call may fail only if there is no active connection with the target device within a configurable timeout period.

   .. uml::

      title  One-way server-side RPC

      participant "Device" as TBDev order 10
      participant "ThingsBoard Server"  as TBSrv order 20 
      participant "Server-side Application" as TBApp  order 30

      TBSrv   <-  TBApp: Server-side RPC Request API (**REST API**)
      TBDev  <-  TBSrv: Server-side RPC Request (**Device API**)
      TBSrv  -->  TBApp: Server-side RPC **Empty** Response (**REST API**)


* **Two-way server-side RPC** request is sent to the device and expects to receive a response from the device within the certain timeout. The Server-side request is blocked until the target device replies to the request.

   .. uml::

      title  Two-way server-side RPC

      participant "Device" as TBDev order 10
      participant "ThingsBoard Server"  as TBSrv order 20 
      participant "Server-side Application" as TBApp  order 30

      TBSrv   <-  TBApp: Server-side RPC Request API (**REST API**)
      TBDev   <-  TBSrv: Server-side RPC Request (**Device API**)
      TBDev  -->  TBSrv: Server-side RPC response (**Device API**)
      TBSrv  -->  TBApp: Server-side RPC Response (**REST API**)
