# Furnace Electronics

This repo contains the starting information and design notes for a programmable muffle furnace electronics project.

## Source Material

- `Furnace Specs_260616_125326.pdf`
- `Furnace Project Specification_260708_203945.pdf`
- `furnace.jpeg`

## Derived Design Notes

- `docs/requirements.md` captures the supplied functional, control, hardware, and safety requirements.
- `docs/electronics-architecture.md` outlines a first-pass electronics and control architecture.
- `docs/hmi-and-control-plan.md` defines the PC HMI, local HMI, ESP32 state model, command model, and first prototype path.
- `docs/local-hmi-design.md` defines the furnace-mounted screen, rotary encoder, buttons, pages, and interaction rules.
- `docs/remote-dashboard-hardware.md` recommends hardware options for PC/Raspberry Pi dashboards, MQTT, Wi-Fi, Ethernet, and RS-485.
- `docs/firmware-rtos-architecture.md` defines the ESP32 FreeRTOS task split and firmware safety/control ownership.
- `docs/freertos-practical-guide.md` explains RTOS concepts, mutexes, queues, tasks, and how to apply them to this firmware.
- `docs/platformio-espidf-path-issue.md` documents the ESP-IDF project creation failure caused by whitespace in the full project path.
- `bom/furnace_bom.sqlite` stores the modular starter BOM with supplier offers and prices where captured.
- `bom/README.md` explains the BOM schema, views, and rebuild commands.
- `docs/sqlite-bom-intro.md` introduces SQLite and shows how to query, edit, and export the BOM data.

## Current Direction

The system is expected to provide:

- PC-based temperature profile input, live monitoring, graphing, and logging.
- ESP32-WROOM-32E based autonomous furnace control.
- Local furnace HMI with screen and buttons so the furnace can run without the PC connected.
- Dual thermocouple measurement.
- PID-based AUTO mode and direct-output MANUAL mode.
- SSR-controlled resistive heating element.
- Emergency stop, door/lid interlock, over-current protection, and independent heater shutdown behaviour.

The current baseline is PC dashboard plus ESP32-WROOM-32E controller plus local HMI. The PC is optional once a run is started; the ESP32 owns profile execution, PID, sensor handling, SSR control, and faults.

The next engineering step is to prototype the HMI/control stack with the ESP32-WROOM-32E development board, local display/buttons, simulated heater output, and USB serial telemetry before selecting final high-power components.
