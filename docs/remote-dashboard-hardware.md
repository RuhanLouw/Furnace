# Remote Dashboard Hardware

This document captures recommended hardware paths for remote dashboard and control of the furnace controller.

## Design Rule

Remote dashboard hardware must not be part of the safety chain.

The ESP32 controller and hardwired safety chain must remain responsible for:

- emergency stop.
- door/lid interlock.
- independent over-temperature trip.
- heater output disable.
- thermocouple fault handling.
- profile execution once a run has started.

Remote hardware may send commands and display state, but it must not directly control the SSR pin.

## Recommended Development Path

### Stage 1: PC Dashboard Over USB Serial

Use the existing ESP32-WROOM-32E board connected to a PC by USB.

Hardware:

- ESP32-WROOM-32E development board.
- PC or laptop running Node-RED.
- USB cable.

Protocol:

- newline-delimited JSON over USB serial.

Use this first because it is simple, debuggable, and keeps the first firmware change focused on the command/telemetry contract.

Limitations:

- PC must stay near the furnace if USB is the only dashboard link.
- USB cable length is limited.
- USB is not ideal near noisy high-power switching hardware.

### Stage 2: Local Network Dashboard Gateway

Run Node-RED on a small local computer near the furnace.

Recommended hardware:

- Raspberry Pi 4 or Raspberry Pi 5.
- industrial/fanless mini PC if the environment is hot, dusty, or reliability-critical.

Connection from gateway to ESP32:

- USB serial for early prototype.
- RS-485 for a robust final wired link.
- Ethernet or Wi-Fi if the ESP32 hardware supports it reliably.

Connection from user to dashboard:

- browser on LAN.
- VPN for off-site access.

Do not expose the dashboard directly to the public internet.

### Stage 3: ESP32 Network Interface

Once the command protocol is stable, add direct network capability to the controller.

Options:

- ESP32 Wi-Fi with MQTT.
- ESP32 plus W5500 SPI Ethernet module.
- ESP32 Ethernet/PoE board such as Olimex ESP32-POE-ISO.

Preferred final wired options:

- RS-485 from ESP32 controller to dashboard gateway.
- Ethernet from ESP32 controller to dashboard gateway or LAN.

Wi-Fi is acceptable for monitoring and non-critical commands, but should not be required for safe heater operation.

## Hardware Options

### Option A: Existing ESP32-WROOM-32E + USB Serial

Use for:

- first dashboard prototype.
- firmware command/telemetry testing.
- Node-RED dashboard development.

Pros:

- already available.
- no extra network hardware.
- easiest to debug.

Cons:

- PC/gateway must remain physically connected.
- not ideal for final installed furnace wiring.

Recommendation:

- use this first.

### Option B: Existing ESP32-WROOM-32E + Wi-Fi MQTT

Use for:

- wireless dashboard.
- remote monitoring on local network.
- development without USB tether.

Pros:

- no extra controller hardware.
- ESP32 already includes Wi-Fi.
- works well with Node-RED and MQTT.

Cons:

- Wi-Fi can drop.
- workshop/lab RF conditions can be poor.
- credentials and network setup add firmware complexity.

Recommendation:

- good second step after USB JSON works.
- do not make Wi-Fi required for safety.

### Option C: Existing ESP32-WROOM-32E + W5500 Ethernet

Use for:

- wired network dashboard.
- MQTT or TCP over Ethernet.

Hardware:

- W5500 SPI Ethernet module.
- RJ45 Ethernet connection.
- extra SPI chip select and interrupt pins.

Pros:

- more stable than Wi-Fi.
- keeps current ESP32-WROOM-32E board.
- common Arduino library support.

Cons:

- consumes SPI pins.
- shares bus planning with TFT/thermocouple devices.
- needs careful 3.3 V compatibility check.

Recommendation:

- good path if wired network is required but the current ESP32 board must be kept.

### Option D: ESP32 Ethernet/PoE Board

Use for:

- cleaner final networked controller.
- single-cable Ethernet plus power where appropriate.

Candidate:

- Olimex ESP32-POE-ISO or equivalent isolated Ethernet/PoE ESP32 board.

Pros:

- integrated Ethernet.
- PoE option.
- isolated PoE variant improves network power isolation.

Cons:

- different board and pinout.
- may require firmware pin reassignment.
- still needs proper isolation from heater/mains circuits.

Recommendation:

- strong final candidate if the furnace controller must be permanently networked.

### Option E: RS-485 Link To A Gateway

Use for:

- robust industrial-style connection between furnace controller and Node-RED gateway.

Hardware:

- isolated RS-485 transceiver.
- twisted-pair cable.
- Raspberry Pi or PC gateway with USB-RS485 adapter.

Protocol:

- Modbus RTU.
- or simple framed JSON/binary protocol if Modbus is not needed.

Pros:

- robust over longer cable runs.
- good noise immunity.
- easy isolation.
- common industrial pattern.

Cons:

- needs an adapter/gateway.
- less plug-and-play than USB.

Recommendation:

- best final wired control link if the dashboard computer sits away from the controller.

## Dashboard Computer

### PC Or Laptop

Use for:

- development.
- commissioning.
- lab testing.

Pros:

- already available.
- easy VS Code, PlatformIO, Node-RED, logs.

Cons:

- not ideal as permanent furnace hardware.
- user may take it away.

### Raspberry Pi

Use for:

- dedicated local dashboard appliance.
- Node-RED host.
- MQTT broker.
- local logging.
- browser dashboard on LAN.

Recommended:

- Raspberry Pi 4 or 5.
- official or good-quality power supply.
- Ethernet connection where possible.
- case mounted away from furnace heat.

Pros:

- small and cheap.
- can stay with furnace.
- Ethernet and Wi-Fi available.
- good Node-RED support.

Cons:

- SD cards can corrupt if power is bad.
- needs proper shutdown/power design.
- not industrial by default.

### Fanless Mini PC

Use for:

- more robust permanent HMI/gateway.
- Windows or Linux Node-RED host.

Pros:

- better storage options than SD card.
- more durable for continuous operation.
- easier remote desktop/service tooling.

Cons:

- more expensive.
- physically larger.

Recommendation:

- use a Raspberry Pi for development/permanent low-cost gateway.
- use a fanless mini PC if reliability and maintainability matter more than cost.

## Preferred Project Recommendation

Start with:

```text
PC Node-RED dashboard
    |
USB serial JSON
    |
ESP32-WROOM-32E controller
```

Then move to:

```text
Raspberry Pi Node-RED + MQTT broker
    |
USB serial or Wi-Fi MQTT
    |
ESP32-WROOM-32E controller
```

Final robust option:

```text
Browser dashboard
    |
Raspberry Pi or fanless mini PC running Node-RED
    |
RS-485 or Ethernet
    |
ESP32 controller
```

## BOM Items To Consider Adding

Dashboard and networking:

- Raspberry Pi 4 or Raspberry Pi 5.
- Raspberry Pi official power supply.
- microSD card or SSD boot storage.
- Ethernet cable.
- small DIN-rail or panel-mount Ethernet switch.
- USB-RS485 adapter.
- isolated RS-485 transceiver module.
- W5500 SPI Ethernet module.
- ESP32-POE-ISO board if replacing the current ESP32 board.

Electrical robustness:

- isolated 24 V to 5 V DC-DC converter.
- USB isolator for development near powered heater hardware.
- shielded/twisted pair cable for RS-485.
- DIN-rail terminal blocks.
- panel-mount RJ45 or USB service connector.

Operational:

- local HMI remains mandatory even with a remote dashboard.
- physical Stop and emergency stop remain mandatory.
- dashboard access should be LAN/VPN only.
