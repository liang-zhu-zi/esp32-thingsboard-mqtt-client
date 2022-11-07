# OTA updates - ThingsBoard MQTT device API

[MQTT Device API Reference - Firmware API](https://thingsboard.io/docs/reference/mqtt-api/#firmware-api)

**Tips**: In this article, **client** refers to **device**.

## Firmware OTA updates

**Tips**: Firmware attributes are some shared attributes.

### Request firmware attributes values from the server

```plantuml
...

```

1. Prerequisites
    * Control Packet type: SUBSCRIBE (Client subscribe request)
    * Direction of flow:        Client to Server
    * Payload-Topic Filter:     `???`

1. Send request of firmware attributes

1. Handle response of firmware attributes

### Subscribe to firmware attributes updates from the server

```plantuml
...
A --> B
```

1. Prerequisites
    * Control Packet type: SUBSCRIBE (Client subscribe request)
    * Direction of flow:        Client to Server
    * Payload-Topic Filter:     `firmware attributes updates`

1. Handle PUBLISH message of firmware attributes updates
    * Control Packet type: SUBSCRIBE (Client subscribe request)
    * Direction of flow:        Server to Client
    * Payload-Topic Filter:     `firmware attributes updates`

### Request firmware chunk from the server

1. Prerequisites
    * Control Packet type: SUBSCRIBE (Client subscribe request)
    * Direction of flow:        Client to Server
    * Payload-Topic Filter:     `???`

1. Send request of firmware chunk

1. Handle response of firmware chunk

1. Send telemetry of current firmware status

## Software OTA updates

所有应用于Firmware的操作，也可以应用于 Software。相关的协议，把 `fw_...` 替换成 `sw_...` 即可。

## ThingsBoard Operation

* Device 的 OTA updates 覆盖 Device profile 的 OTA updates.

## References

1. [**Firmware API**](https://thingsboard.io/docs/reference/mqtt-api/#firmware-api) - ThingsBoard.
