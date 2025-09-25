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

Run **moblink** by setting environment variables inline:

```bash
[ENV_VAR=value ...] ./moblink
```

| Variable           | Description |
|--------------------|-------------|
| `MOBILUS_DSN`      | DSN for Cosmo GTW (`mqtt://` or `mqtts://`) |
| `MOBILUS_USERNAME` | Username for Cosmo GTW |
| `MOBILUS_PASSWORD` | Password for Cosmo GTW |
| `TARGET_DSN`       | DSN for MQTT broker where messages should be published from Cosmo GTW |
| `ROOT_TOPIC`       | Root MQTT topic that is appended for all topics |

#### Example

```bash
MOBILUS_DSN="mqtts://mobilus:8883" \
MOBILUS_USERNAME="admin" \
MOBILUS_PASSWORD="admin" \
TARGET_DSN="mqtt://127.0.0.1:1883" \
ROOT_TOPIC="mobilus" \
./moblink
```

## Installation

### Automated (Debian)

Download the latest **moblink-*.deb** from the **Releases** and install it:

```bash
sudo apt install ./moblink-*.deb
```

Adjust config parameters in `/etc/moblink.conf`.

Restart:

```bash
sudo systemctl restart moblink
```

And verify that it's running:

```bash
sudo systemctl status moblink
```

### Manual (other with systemd)

For other Linux distro supporting **systemd** you can setup **moblink** manually by downloading **moblink-linux.tar.gz** in **Releases** and copy:

```bash
sudo cp moblink.conf /etc/moblink.conf
sudo cp moblink.service /lib/systemd/system/moblink.service
sudo cp moblink /usr/bin/moblink
```

Then, enable the **moblink** service and start it:

```bash
sudo systemctl daemon-reload
sudo systemctl enable moblink
sudo systemctl start moblink
```

And verify that it's running:

```bash
sudo systemctl status moblink
```

## Build

This project uses **CMake** as its build system.

To build the project:

```bash
cmake -B build
cmake --build build
```
