# 🔗 moblink

A lightweight bridge that links the **Mobilus Cosmo GTW** and a target MQTT broker. It subscribes to MQTT topics on the **Cosmo GTW** and publishes them into a standard MQTT structure, making integration with external systems (e.g. Loxberry, Home Automation platforms) much easier.

## Requirements

* OpenSSL v3
* Mosquitto v2.x (v1.x optionally supported)

## Overview

The **moblink** operates on 4 MQTT topics.

**Subscription topics:**

* `mobilus/devices/{deviceId}/state` — current device state (emitted after a command is executed)
* `mobilus/devices/{deviceId}/error`— errors occurred during command execution
* `mobilus/devices/{deviceId}/pending-command` — received commands that are pending execution

**Publish topics:**

* `mobilus/devices/{deviceId}/command` — for sending commands to a specific device

> {deviceId} is an integer assinged by the Cosmo GTW

You can change the default root topic: `mobilus` to anything else by providing env `ROOT_TOPIC`.

## Basic usage

```bash
MOBILUS_DSN="mqtts://192.168.1.1:8883?verify=false" \
MOBILUS_USERNAME=admin \
MOBILUS_PASSWORD=admin \
TARGET_DSN="mqtt://127.0.0.1:1883" \
ROOT_TOPIC=mobilus
./moblink
```

### Systemd

You can easily set up **moblink** to start automatically using **systemd**, which is available on most Linux distributions.

```bash
sudo install -m 644 ./etc/moblink.conf /etc/
sudo install -m 644 ./systemd/moblink.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable moblink
sudo systemctl start moblink
```

## Build

This project uses **CMake** as its build system.

To build the project:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```
