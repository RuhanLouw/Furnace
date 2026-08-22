# Firmware RTOS Architecture

This document defines the recommended firmware architecture for the ESP32-WROOM-32E furnace controller.

## Recommendation

Use FreeRTOS.

The current PlatformIO project uses:

```ini
framework = arduino
board = esp32doit-devkit-v1
```

On ESP32, the Arduino framework already runs on top of FreeRTOS. That means the project can keep Arduino/PlatformIO convenience while using FreeRTOS tasks, queues, mutexes, timers, and watchdog handling where needed.

Do not add a second scheduler or a separate RTOS layer. Use the ESP32 FreeRTOS primitives that are already available through the Arduino ESP32 environment.

## Why RTOS Makes Sense

The furnace controller has several activities with different timing needs:

- heater control and safety checks.
- thermocouple acquisition.
- PID calculation.
- local HMI screen updates and button handling.
- PC communication.
- telemetry and logging.
- profile runtime state.

A single blocking `loop()` will become fragile once the screen, serial protocol, sensors, and control logic are added. FreeRTOS gives clean separation without turning the firmware into a blocking state machine.

## Design Rule

The control and safety path must not depend on the HMI or PC tasks.

If the display freezes, USB serial disconnects, Wi-Fi drops, or graphing stalls, the controller must still:

- read safety inputs.
- detect sensor faults.
- force heater output off when required.
- maintain or stop the active temperature programme according to the defined fault rules.

## Proposed Tasks

### Safety Task

Purpose:

- read emergency stop feedback.
- read door/lid interlock feedback.
- read independent over-temperature feedback.
- check watchdog/fault latch conditions.
- publish safety state.
- force heater enable false on unsafe conditions.

Suggested period:

- 10 ms to 50 ms.

Priority:

- highest application priority.

Notes:

- Hardwired safety must still remove heater power independently of firmware.
- This task supervises and reports safety state; it is not the only safety mechanism.

### Sensor Task

Purpose:

- read TC1.
- read TC2.
- check thermocouple interface faults.
- filter readings if needed.
- publish latest temperature state.

Suggested period:

- 250 ms to 1000 ms, depending on thermocouple interface conversion time.

Priority:

- medium-high.

Notes:

- Furnace temperature changes slowly, so this does not need high-frequency sampling.
- Fault detection matters more than high sample rate.

### Control Task

Purpose:

- calculate active setpoint from the profile.
- select process value from TC1/TC2.
- run PID in AUTO.
- apply manual output in MANUAL.
- apply output limits and fault overrides.
- update heater output command.

Suggested period:

- 500 ms to 1000 ms.

Priority:

- high, below safety.

Notes:

- Furnace thermal response is slow.
- PID should run at a fixed interval.
- Do not run PID from display or communication code.

### SSR Timing Task

Purpose:

- convert output percentage to slow time-proportional SSR control.
- manage SSR on/off state inside a fixed control window.

Suggested period:

- 100 ms to 250 ms.

Priority:

- high, below safety.

Notes:

- For a mains AC resistive heater with zero-cross SSR, this is slow time-proportional control, not high-frequency PWM.
- Example: 30 percent output in a 10 second window means 3 seconds ON and 7 seconds OFF.
- Safety override must always force SSR OFF.

### Local HMI Task

Purpose:

- scan buttons and rotary encoder.
- update local UI state.
- render display pages.
- create commands for the central command handler.

Suggested period:

- input scan: 10 ms to 20 ms.
- display redraw: 200 ms to 500 ms, or only on value/page changes.

Priority:

- medium-low.

Notes:

- Display drawing must not block control or safety tasks.
- HMI sends commands; it does not directly manipulate heater output.

### PC Communication Task

Purpose:

- handle USB serial messages.
- later handle MQTT, Wi-Fi, or Ethernet if added.
- parse commands.
- send telemetry.
- report command responses.

Suggested period:

- event-driven or 20 ms to 100 ms polling.

Priority:

- medium-low.

Notes:

- PC communication is optional.
- Loss of PC communication must not crash the active run.

### System Task

Purpose:

- heartbeat.
- firmware version reporting.
- uptime.
- non-volatile settings/profile save requests.
- lower priority diagnostics.

Suggested period:

- 1000 ms or slower.

Priority:

- low.

## Shared Data Model

Use shared state snapshots rather than letting every task modify everything.

Recommended data groups:

- `SafetyState`
- `SensorState`
- `ControlState`
- `ProfileState`
- `HmiState`
- `CommsState`
- `FaultState`

Use queues for commands:

- local HMI commands.
- PC commands.
- system commands.

Use mutexes or critical sections for shared state snapshots.

Avoid long mutex holds. A task should copy the data it needs, release the lock, then do its work.

## Command Handler

Use one central command handler.

Inputs:

- local HMI task.
- PC communication task.
- internal system events.

Outputs:

- accepted/rejected command response.
- updated controller state.
- updated fault state.

This prevents local HMI and PC dashboard from becoming two different control systems.

## Core Assignment

ESP32 has two cores. A practical starting point:

- Core 0: communication and background system tasks.
- Core 1: safety, sensors, control, SSR timing, and local HMI.

Do not depend too heavily on a specific core split early. Get the task ownership right first, then pin tasks only where it reduces jitter or avoids conflicts.

## Watchdog Strategy

Use watchdog supervision for critical tasks.

Minimum watchdog expectations:

- safety task must run.
- control task must run.
- SSR timing task must run.
- firmware must force heater output OFF during startup, reset, watchdog timeout, and fatal faults.

The watchdog should not be fed from a task that only proves the scheduler is alive. It should prove the critical application tasks are still cycling.

## Fault Behaviour

Faults that should force heater OFF:

- emergency stop active.
- door/lid open.
- independent over-temperature trip.
- TC1/TC2 fault according to selected control strategy.
- thermocouple disagreement beyond allowed limit.
- SSR command fault if detectable.
- control task timeout.
- safety task timeout.
- invalid active profile.
- watchdog reset.

Some faults should latch until acknowledged. Acknowledgement should only be accepted after the underlying fault condition is gone.

## What Not To Do

- Do not put heater control in the display code.
- Do not let the PC dashboard directly toggle the SSR pin.
- Do not use high-frequency PWM for a zero-cross mains SSR.
- Do not block critical tasks waiting for serial input, Wi-Fi, display drawing, or file writes.
- Do not make every module a task. Use tasks only where timing, blocking behaviour, or ownership justifies it.

## First Firmware Milestone

Before connecting real heater hardware, build this:

- FreeRTOS tasks created from `setup()`.
- `loop()` left empty or used only for a low-priority heartbeat.
- simulated TC1/TC2 readings.
- simulated SSR output on an LED.
- local HMI button/encoder input.
- USB serial telemetry.
- simple profile run state.
- safety inputs simulated by buttons or jumpers.

Once this is stable, add thermocouple modules and then the real isolated output hardware.
