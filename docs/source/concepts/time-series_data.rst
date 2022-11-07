Telemetry - Time-series data
===============================================



Working with telemetry data
---------------------------

See `Working with telemetry data`__.

.. __: https://thingsboard.io/docs/user-guide/telemetry/


.. uml::

   title  Telemetry data

   participant "Device" as TBDev order 10
   participant "ThingsBoard Server"  as TBSrv order 20 
   box "Server-side Application" #LightBlue
   participant "Server-side Application" as TBApp  order 30
   participant "Web UI" as WebUI  order 40
   end box

   == Telemetry Upload ==

   TBDev  ->  TBSrv: Telemetry upload API (**Device API**)

   == Timeseries data Query ==

   TBSrv  <-  TBApp: Timeseries data keys API (**REST API**)
   TBSrv  <-  TBApp: Timeseries data values API (**REST API**)
   ... other **Timeseries data Query API** ...

   == or Timeseries data Subscription ==

   TBSrv  <-  WebUI: subscription commands (**Websocket API**)
   TBSrv  ->  WebUI: subscription updates (**Websocket API**)



Data points
-----------------
