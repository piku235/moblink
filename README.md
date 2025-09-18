# 🔗 moblink

A lightweight bridge that links the **Mobilus Cosmo GTW** and a target MQTT broker. It subscribes to MQTT topics on the **Cosmo GTW** and publishes them into a standard MQTT structure, making integration with external systems (e.g. Loxberry, Home Automation platforms) much easier.

## Requirements

* C++17 compatible compiler (GCC 8+, Clang 7+)
* OpenSSL v3
* Mosquitto v2.x (v1.x optionally supported)

## Overview

The **moblink** operates on 4 MQTT topics.

**Subscription topics:**

* `devices/{deviceId}/state` — current device state (emitted after a command is executed)
* `devices/{deviceId}/error`— errors occurred during command execution
* `devices/{deviceId}/pending-command` — received commands that are pending execution

**Publish topics:**

* `devices/{deviceId}/command` — for sending commands to a specific device

> {deviceId} is an integer assinged by the Cosmo GTW

## Usage

```bash
MOBILUS_DSN="mqtts://192.168.1.1" MOBILUS_USERNAME=admin MOBILUS_PASSWORD=admin TARGET_DSN="mqtt://127.0.0.1:1883" ./moblink
```

## Build

This project uses **CMake** as its build system.

To build the project:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```
