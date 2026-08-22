# Furnace Controller ProdV1

This is the first Arduino + FreeRTOS firmware shell for the ESP32-WROOM-32E furnace controller.

## Framework

```ini
framework = arduino
```

The ESP32 Arduino framework already runs on FreeRTOS, so this project uses Arduino for hardware/library convenience and FreeRTOS tasks for firmware structure.

## Current Template

The firmware currently includes:

- safety task.
- sensor simulation task.
- control task.
- SSR timing task.
- command handler task.
- local HMI placeholder task.
- PC serial command task.
- telemetry task.
- system/status task.

The heater output is represented by `SsrOutputPin` and is intended to drive only a test LED or isolated output circuit at this stage.

## Source Layout

```text
include/
  AppConfig.h          pins, periods, baud rate, SSR window
  AppTypes.h           shared enums and state structs
  CommandHandler.h     central command handler API
  HardwareOutput.h     output pin API
  StateStore.h         mutex-protected shared state and command queue
  Tasks.h              FreeRTOS task entry declarations

src/
  main.cpp             setup only: pins, serial, state store, task creation
  StateStore.cpp       mutexes, state snapshots, command queue
  CommandHandler.cpp   command validation and state transitions
  HardwareOutput.cpp   SSR, run LED, and fault LED outputs
  Tasks.cpp            task creation and priority/core assignment
  SafetyTask.cpp       safety input checks and fault forcing
  SensorTask.cpp       simulated thermocouple readings
  ControlTask.cpp      setpoint/process value/output calculation
  SsrOutputTask.cpp    slow time-proportional SSR output
  CommandTask.cpp      command queue consumer
  PcCommsTask.cpp      simple serial command input
  TelemetryTask.cpp    once-per-second serial telemetry
  LocalHmiTask.cpp     placeholder for screen/buttons/encoder
  SystemTask.cpp       low-priority status outputs
```

Keep `main.cpp` small. New furnace behavior should normally go into the relevant module rather than directly into `setup()` or `loop()`.

## Test Serial Commands

Open the serial monitor at `115200` baud.

Commands:

- `s`: start simulated run.
- `x`: stop run.
- `a`: AUTO mode.
- `m`: MANUAL mode.
- `f`: acknowledge fault.

Telemetry is printed once per second.

## Current Pin Defaults

Configured in `include/AppConfig.h`:

- `GPIO26`: simulated SSR output.
- `GPIO2`: run/heater LED.
- `GPIO27`: fault LED.
- `GPIO32`: e-stop feedback input, active low.
- `GPIO33`: door/lid feedback input, active low.
- `GPIO25`: over-temperature feedback input, active low.

Inputs use `INPUT_PULLUP`, so an open input reads safe/high and grounding the pin simulates an active fault. This is only for the firmware shell and bench testing; final safety wiring must use the proper hardwired heater-enable chain.

## Build

From this folder:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run
```

## Next Firmware Steps

1. Replace simulated safety inputs with real bench buttons/jumpers.
2. Replace simulated thermocouples with TC1/TC2 interface modules.
3. Add rotary encoder and local display task implementation.
4. Replace simple serial commands with newline-delimited JSON.
5. Add real profile storage and ramp/hold execution.
6. Keep heater output on an LED or low-voltage test load until safety logic is proven.
