# Distributed real-time enegry grid monitor: KSE REST API endpoint

This application is part of a distributed real-time system for monitoring Polish power grid data correlated with weather information. **KSE API Handler** is responsible for:

- Periodically fetching relevant data from https://api.raporty.pse.pl/
- Sending processed data to a database endpoint via MQTT

The application is designed for **Linux with PREEMPT_RT** (soft real‑time) and is intended to run continuously on a server with a public network access.

---

## Requirements

### Hardware / OS
- Linux with **PREEMPT_RT kernel**.
- Network connectivity to MQTT broker.

### Software Dependencies
- **C++17** compiler (g++ ≥ 9 or clang ≥ 10).
- **CMake** ≥ 3.15.
- **Boost** (for ASIO and MQTT5) – header‑only.
- **nlohmann/json** – for JSON parsing.

---

## Communication Architecture

The four stations communicate via a central **MQTT broker**. All stations connect to this broker over a secure virtual network provided by **Hamachi**. This section explains how to set up the network and connect your station to the broker.

### Overview

- **Broker**: Mosquitto MQTT broker
- **VPN**: Hamachi creates a virtual LAN; all stations join the same Hamachi network
- **Protocol**: MQTT over TCP (port 1883) with username/password authentication
- **Topics**: Predefined hierarchy

### Prerequisites

- Install **Hamachi** ([LogMeIn Hamachi](https://vpn.net/)) on your development machine.
- Install **Mosquitto client tools** (for testing) – on Linux: `sudo apt install mosquitto-clients`, on Windows: download from [mosquitto.org](https://mosquitto.org/download/).

### Step 1: Join the Hamachi Network

1. Open Hamachi and log in with your **personal LogMeIn account**.
2. Join this network with provided credentials.
3. After joining, you will see a list of online peers. The broker (Station C4) appears with a **green** or **blue** icon. Its Hamachi IP (e.g., `25.xxx.xxx.xxx`) is shown – **this IP is your MQTT broker address**.

### Step 2: Obtain MQTT Credentials

The broker requires authentication. The username and password are shared securely with the team.

### Step 3: Test the Connection

Use the Mosquitto command-line tools to verify connectivity:

```bash
# Replace placeholders with actual values
BROKER_IP="25.xxx.xxx.xxx"          # Hamachi IP of Station C4
MQTT_USER="your_username"            # shared credentials
MQTT_PASS="your_password"

# Subscribe to a test topic (run in one terminal)
mosquitto_sub -h $BROKER_IP -t "testtopic/test" -u $MQTT_USER -P $MQTT_PASS -v

# Publish a test message (run in another terminal)
mosquitto_pub -h $BROKER_IP -t "testtopic/test" -u $MQTT_USER -P $MQTT_PASS -m "Hello from my station"
```

---
