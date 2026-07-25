# HMI And Control Plan

This document defines where the furnace electronics project should begin.

## Design Starting Point

Start from the operator interface and work backward:

1. Define what the operator must see and do.
2. Define the ESP32 command and state model that supports those actions.
3. Define communication between the PC and ESP32.
4. Define the local display and button behaviour.
5. Select thermocouple, SSR, interlock, and power hardware around the controller I/O needs.

## Baseline Decision

Use:

- PC dashboard for rich HMI, graphing, profile editing, and logging.
- ESP32-WROOM-32E as the autonomous furnace controller.
- ESP32-attached non-touch display, rotary encoder, and buttons for operation without the PC.

The furnace must remain controllable and safe if the PC is disconnected after a run starts.

## Controller Baseline

The available controller is an ESP32-WROOM-32E module.

This is suitable for the first prototype because it provides:

- dual-core MCU performance for HMI, communication, timing, and control tasks.
- Wi-Fi and Bluetooth capability for later network operation.
- enough GPIO for thermocouple interfaces, local display, buttons, interlocks, and SSR command.
- SPI, I2C, UART, and PWM/timer peripherals.

Design constraints:

- Treat all ESP32 I/O as 3.3 V logic.
- Do not use GPIO6 to GPIO11 because they are used by the module flash.
- Avoid using boot strapping pins for signals that may be pulled to the wrong level during reset.
- Do not rely on Wi-Fi connectivity for heater safety.
- Keep the ESP32 and local HMI thermally isolated from the furnace body.
- Use proper isolation between ESP32 circuitry and mains/heater circuitry.

## HMI Screens

### PC Dashboard

Minimum screens:

- Profile editor.
- Runtime monitor.
- Alarm and fault log.
- Settings and tuning.

Profile editor should support:

- Segment list.
- Ramp target temperature.
- Ramp duration.
- Hold duration.
- Profile validation before upload.
- Upload profile to ESP32.
- Save and load profiles on the PC.

Runtime monitor should show:

- Run state.
- Runtime.
- Active segment.
- Current setpoint.
- TC1.
- TC2.
- Control process value.
- Output percentage.
- Interlock state.
- Fault state.
- Live trend graph.

### Local HMI

Minimum local pages:

- Home/status page.
- Profile select page.
- Profile edit page for basic values.
- Run page.
- Fault page.
- Settings page.

Minimum controls:

- Start.
- Stop.
- Back/cancel.
- Confirm/enter.
- Up/down or rotary navigation.
- Fault acknowledge.

The preferred physical interface is a non-touch TFT display with a rotary encoder, dedicated Start and Stop buttons, Back/Cancel, and clear status indicators. See `docs/local-hmi-design.md`.

The local HMI does not need to be as comfortable as the PC dashboard, but it must allow basic operation without a PC.

## ESP32 State Model

Recommended top-level states:

- `BOOT`
- `IDLE`
- `PROFILE_EDIT`
- `READY`
- `RUNNING`
- `PAUSED`
- `COMPLETE`
- `STOPPING`
- `FAULT`
- `ESTOP`

Recommended mode states:

- `AUTO`
- `MANUAL`

Recommended authority states:

- `LOCAL`
- `PC`
- `LOCKED`
- `NONE`

Safety states override mode and authority.

## Command Model

Commands accepted from PC and local HMI:

- `profile_upload`
- `profile_select`
- `profile_validate`
- `run_start`
- `run_stop`
- `run_pause`
- `run_resume`
- `mode_set_auto`
- `mode_set_manual`
- `manual_output_set`
- `fault_ack`
- `settings_update`
- `status_request`

Every command should return:

- accepted or rejected.
- reason if rejected.
- current state.
- active faults.

## Telemetry Model

ESP32 should report:

- firmware version.
- controller state.
- active authority.
- runtime seconds.
- active segment index.
- current setpoint.
- TC1 temperature.
- TC2 temperature.
- selected control temperature.
- output percentage.
- SSR enabled state.
- door/lid switch state.
- emergency stop state.
- over-temperature state.
- thermocouple fault state.
- communication status.
- last command result.

## Communication Starting Point

Start with USB serial because it is simple, reliable, and easy to debug.

Recommended first protocol:

- newline-delimited JSON messages.
- PC sends commands.
- ESP32 sends telemetry at a fixed interval.
- ESP32 sends immediate event messages for faults, state changes, and command responses.

Later options:

- MQTT over Wi-Fi or Ethernet for network dashboards.
- Modbus RTU over RS-485 for a more industrial interface.
- WebSocket if the ESP32 hosts its own web UI.

## First Hardware Decisions

Decide these before schematics:

- ESP32-WROOM-32E development board or carrier board.
- Local display type and size.
- Button and rotary encoder input method.
- PC communication method for prototype.
- Thermocouple type.
- Thermocouple interface IC.
- SSR type and control voltage.
- Heater voltage, current, and power.
- Door/lid switch type.
- E-stop and contactor/interlock architecture.
- Electronics supply voltage.

## Recommended First Prototype

Prototype the control stack before high-power heater hardware:

- ESP32-WROOM-32E development board.
- Small non-touch TFT display.
- Rotary encoder plus start/stop buttons.
- Two thermocouple interface modules.
- Low-voltage lamp or LED load in place of SSR/heater.
- PC running Node-RED or a simple serial dashboard.

This lets the HMI, profile execution, telemetry, and fault logic be tested without mains heater risk.

## Initial I/O Allocation

This allocation is functional rather than final. Exact GPIO numbers should be assigned after the specific ESP32 development board and display module are selected.

Core I/O:

- USB serial: PC dashboard and firmware upload.
- SPI bus: local TFT display.
- SPI or I2C bus: thermocouple interface modules.
- Digital output: SSR command.
- Digital output: run/status LED.
- Digital input: door/lid switch.
- Digital input: emergency stop feedback.
- Digital input: independent over-temperature trip feedback.
- Digital input: rotary encoder A.
- Digital input: rotary encoder B.
- Digital input: encoder push button or confirm.
- Digital input: local Stop button.
- Digital input: local Back/Cancel button.

Optional I/O:

- buzzer output for alarms.
- current sensor input or digital metering interface.
- voltage measurement interface.
- second communication port for RS-485 or service diagnostics.

Suggested bus split:

- Use one SPI bus for the display.
- Use a separate SPI bus or I2C bus for thermocouple interfaces if the chosen thermocouple IC supports it.
- Keep interlocks on simple digital inputs.
- Keep SSR command on a dedicated digital output with a known OFF state during boot.
