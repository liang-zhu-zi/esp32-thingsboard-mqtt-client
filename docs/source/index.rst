.. ESP32-ThingsBoard-MQTT-Client documentation master file, created by
   sphinx-quickstart on Sun Oct 23 21:46:24 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to ESP32-ThingsBoard-MQTT-Client!
=========================================================

`ESP32-ThingsBoard-MQTT-Client`__  for C/C++ developers. 

It is packaged into an ESP-IDF component, which can be used under ESP-IDF and ESP-ADF, and can also be easily ported to Arduino-ESP32.

This library provides some simple API to communicate with ThingsBoard platform using MQTT APIs.

Current client version is based on ESP-IDF-v4.4.1, and is compatible with ThingsBoard starting from version 3.4.0.

.. __: https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client

.. uml::
   :align: center

   node "\nThingsBoard Server\n" as TBSrv {
   }

   node "\nTA65\n" as TBDev {
   }

   node "\nBrowser\n" as TBApp {
   }

   TBSrv <-down-> TBDev
   TBSrv <-down-> TBApp

**Lumache** (/lu'make/) is a Python library for cooks and food lovers
that creates recipes mixing random ingredients.
It pulls data from the `Open Food Facts database <https://world.openfoodfacts.org/>`_
and offers a *simple* and *intuitive* API.

Check out the :doc:`usage` section for further information, including
how to :ref:`installation` the project.

.. note::

   This project is under active development.


`Install sphinx-quickstart <https://github.com/avantec-iot/avantec-thingsboard/blob/master/README.md>`_

`Read the Docs tutorial <https://docs.readthedocs.io/en/stable/tutorial/>`_


Contents
--------

.. toctree::
   :maxdepth: 2
   :caption: Contents:
   usage
   api


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
