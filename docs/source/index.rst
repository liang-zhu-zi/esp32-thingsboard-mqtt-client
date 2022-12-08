.. ESP32-ThingsBoard-MQTT-Client documentation master file, created by
   sphinx-quickstart on Sun Oct 23 21:46:24 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

欢迎使用 ESP32-ThingsBoard-MQTT-Client SDK
=======================================================


`ESP32-ThingsBoard-MQTT-Client SDK`__ 适用于 C/C++ 开发人员。 此库提供了一些简单的 API 来使用 MQTT API 与 ThingsBoard 平台进行通信。

它被封装成一个ESP-IDF组件，既可以在ESP-IDF和ESP-ADF下使用，也可以很简单移植到Arduino-ESP32。

当前客户端版本基于 ESP-IDF-v4.4.1，兼容 ThingsBoard 3.4.0 及更新版本。

.. __: https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client


Contents
--------

.. toctree::
   :maxdepth: 2
   :caption: First steps

   /intro/usage

..   Getting Started with ESP32-ThingsBoard-MQTT-Client SDK


.. toctree::
   :maxdepth: 2
   :caption: ThingsBoard Concepts & Features

   /concepts/overview
   /concepts/time-series_data
   /concepts/attributes
   /concepts/rpc
   /concepts/claiming_devices
   /concepts/otaupdate
   /concepts/provision


.. .. toctree::
..    :maxdepth: 2
..    :caption: Device Connectivity APIs/Protocols

..    MQTT Device API   </api/mqtt-api/...>
..    MQTT Gateway API  </api/gateway-mqtt-api/...>
..    CoAP Device API   </api/coap-api/...>
..    HTTP Device API   </api/http-api/...>

.. toctree::
   :maxdepth: 2
   :caption: MQTT Device Connectivity API (Protocol)

   /api/mqtt-api/thingsboard-mqtt-api
   /api/mqtt-api/telemetry-upload-api
   /api/mqtt-api/attributes-api
   /api/mqtt-api/rpc-api
   /api/mqtt-api/claiming-devices-api
   /api/mqtt-api/firmware-api
   /api/mqtt-api/provisioning-api
   /api/mqtt-api/security


.. .. toctree::
..    :maxdepth: 2
..    :caption: Software Development Kits

..    ESP32 ThingsBoard MQTT Client SDK   </sdk/esp32-thingsboard-mqtt-client/...>
..       Funcitons
..       Examples
..       Opreation
..    Python Client SDK                   </sdk/python-client-sdk/...>



.. .. toctree::
..    :maxdepth: 2
..    :caption: Improvement

..    How to improve   </improve/...>


.. toctree::
   :maxdepth: 2
   :caption: About

   /about/copyright





.. .. toctree::
..    :maxdepth: 2
..    :caption: First steps

..    /intro/get-started
..    /intro/add-ta65-to-thingsboard
..    /intro/connect-ta65-to-thingsboard


.. .. toctree::
..    :maxdepth: 3
..    :caption: Usage

..    ThingsBoard </usage/thingsboard-usage>
..    TA65-FC Wi-Fi Thermostat  </usage/ta65-fc-manual>
..    TA65-FH Wi-Fi Thermostat  </usage/ta65-fh-manual>
..    Avantec Thermostat Dashboard </usage/avantec-dashboard-usage>


.. .. toctree::
..    :maxdepth: 2
..    :caption: Customization

..    /customize/customization


.. .. toctree::
..    :maxdepth: 4
..    :caption: Protocol

..    ThingsBoard MQTT API </protocol/thingsboard-mqtt-api>
..    TA65-TBMQTT API </protocol/ta65-mqtt-api>


.. .. toctree::
..    :maxdepth: 2
..    :caption: About

..    copyright


.. Indices and tables
.. ==================

.. * :ref:`genindex`
.. * :ref:`modindex`
.. * :ref:`search`

.. `Install sphinx-quickstart <https://github.com/avantec-iot/avantec-thingsboard/blob/master/README.md>`_

.. `Read the Docs tutorial <https://docs.readthedocs.io/en/stable/tutorial/>`_


.. note::

   This project is under active development.