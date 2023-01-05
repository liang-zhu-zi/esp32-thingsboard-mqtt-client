ThingsBoard Overview
################################

TODO: IoT/ThingsBoard 整体架构；简单介绍各个业务功能；简单介绍各组API;两者的对应；协议的层次及图示。

Reprinted articles:  https://thingsboard.io/docs/

What is ThingsBoard?
=============================

See `What is ThingsBoard?`__

.. __: https://thingsboard.io/docs/getting-started-guides/what-is-thingsboard/


Architecture Overview
===========================

.. uml::

   node "\nThingsBoard Server\n" as TBSrv {
   }

   node "\nDevice\n" as TBDev {
   }

   node "\nServer-side Application\n" as TBApp {
   }

   TBSrv <-down-> TBDev : Device API
   TBSrv <-down-> TBApp : REST API, Websocket API


Key concepts & Features
==================================

* Time-series data
* Attribures
* Remote commands to devices

* Claiming Devices
* Provisoin
  * Device provisioning
  * Bulk Provisioning???
* OTA updates

Data Visualization
==================

ThingsBoard allows you to configure customizable IoT dashboards. Each IoT Dashboard may contain multiple dashboard widgets that visualize data from multiple IoT devices. Once IoT Dashboard is created, you may assign it to one of the customers of you IoT project.

IoT Dashboards are light-weight and you may have millions of dashboards. For example, you may automatically create a dashboard for each new customer based on data from registered customer IoT devices. Or you may modify dashboard via script when a new device is assigned to a customer. All these actions may be done manually or automated via REST API.

You can find useful links to get started below:

* `Dashboards`__
* `Widgets Library`__
    * **Digital** and **analog** gauges for latest real-time values visualization
    * Highly customizable Bar and Line **charts** for visualization of historical and sliding-window data points
    * **Map** widgets for tracking movement and latest positions of IoT devices on Google or OpenStreet maps.
    * **GPIO** control widgets that allow sending GPIO toggle commands to devices.
    * **Card** widgets to enhance your dashboards with flexible HTML labels based on static content or latest telemetry values from IoT devices.

.. __: https://thingsboard.io/docs/user-guide/ui/dashboards/
.. __: https://thingsboard.io/docs/user-guide/ui/widget-library/


Getting Started Guides
======================

These guides provide quick overview of main ThingsBoard features. Designed to be completed in 15-30 minutes.

* `Hello world`__ : Learn how to collect IoT device data using MQTT, HTTP or CoAP and visualize it on a simple dashboard. Provides variety of sample scripts that you can run on your PC or laptop to simulate the device.
* `End user IoT dashboards`__ : Learn how to perform basic operations over Devices, Customers, and Dashboards.
* `Device data management`__ : Learn how to perform basic operations over device attributes to implement practical device management use cases.

.. __: https://thingsboard.io/docs/getting-started-guides/helloworld/
.. __: https://thingsboard.io/docs/iot-video-tutorials/#working-with-users-devices-and-dashboards
.. __: https://thingsboard.io/docs/iot-video-tutorials/#device-data-management-using-thingsboard
