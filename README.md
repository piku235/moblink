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

By default, if no ENV vars are set, **moblink** connects to the Cosmo GTW Mosquitto broker and publishes translated messages to the corresponding topics. 
You can change this behavior by setting the appropriate ENV vars.

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

Adjust config parameters in `/etc/moblink.conf`

Restart:

```bash
sudo systemctl restart moblink
```

And verify that it's running:

```bash
sudo systemctl status moblink
```

### Automated (Cosmo GTW)

Installation is performed on the Cosmo GTW via SSH. How to enable SSH access, you'll find [here](https://forum.arturhome.pl/t/aktywacja-ssh-dla-mobilus-cosmo-gtw/15325).

Once you login via SSH, you need to download and install the [**runtime**](https://github.com/piku235/mobgtw-runtime) by running:

```bash
wget --no-check-certificate -qO- https://raw.githubusercontent.com/piku235/mobgtw-runtime/main/install.sh | sh
```

Then, you can install the **moblink** with this simple command:

```bash
/opt/jungi/bin/pkg install moblink
```

Adjust config parameters in `/opt/jungi/etc/moblink.conf` and start the service:

```bash
/etc/init.d/moblink start
```

You can inspect its logs by running:

```bash
logread -e moblink
```

### Manual (other with systemd)

Download **moblink-Linux.tar.gz** from the **Releases** page and copy the files inside the package on your filesystem.

```bash
tar -xzvf moblink-Linux.tar.gz
cd moblink-Linux
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
