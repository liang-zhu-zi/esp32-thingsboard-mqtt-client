Claiming Devices API
-------------------------------------------

Please see the corresponding article to get more information about the `Claiming devices`__ feature.

.. __: https://thingsboard.io/docs/user-guide/claiming-devices

In order to initiate claiming device, send PUBLISH message to the following topic::

   v1/devices/me/claim

The supported data format is:

.. code:: json
   
   {"secretKey":"value", "durationMs":60000}

**Please note** that the above fields are optional. In case the **secretKey** is not specified, the empty string as a default value is used. In case the **durationMs** is not specified, the system parameter **device.claim.duration** is used (in the file **/etc/thingsboard/conf/thingsboard.yml** ).

