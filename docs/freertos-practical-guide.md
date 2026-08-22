# FreeRTOS Practical Guide

This guide explains how FreeRTOS works and how to apply it to the ESP32 furnace controller.

## What FreeRTOS Does

FreeRTOS lets firmware run multiple independent tasks instead of putting every job into one large Arduino `loop()`.

Without an RTOS:

```text
loop()
  read buttons
  update display
  read thermocouples
  run PID
  update SSR
  handle PC messages
```

This becomes fragile when one section blocks. For example, slow display drawing or serial parsing could delay heater safety checks.

With FreeRTOS:

```text
Safety task
Sensor task
Control task
SSR timing task
Local HMI task
PC communication task
System task
```

FreeRTOS switches between these tasks so each part of the firmware runs at its own rate.

## ESP32 And Arduino

The ESP32 Arduino framework already runs on FreeRTOS. In this project, `platformio.ini` currently uses:

```ini
framework = arduino
```

That is fine. We can keep Arduino and still use FreeRTOS calls such as:

```cpp
xTaskCreatePinnedToCore(...)
vTaskDelay(...)
vTaskDelayUntil(...)
xQueueCreate(...)
xQueueSend(...)
xQueueReceive(...)
xSemaphoreCreateMutex(...)
xSemaphoreTake(...)
xSemaphoreGive(...)
```

Do not add a second scheduler. Use the FreeRTOS primitives already available on ESP32.

## Core Concepts

### Tasks

A task is a function that runs independently.

Example:

```cpp
void safetyTask(void *parameter) {
    while (true) {
        readSafetyInputs();
        updateSafetyState();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
```

`vTaskDelay()` pauses only this task. Other tasks continue running.

### Scheduler

The scheduler decides which task gets CPU time. Higher-priority ready tasks run before lower-priority ready tasks.

Suggested priority order:

```text
highest: safety
high:    SSR timing
high:    control/PID
medium:  sensors
low:     local HMI, PC communication, diagnostics
```

Do not make every task high priority.

### Fixed-Rate Tasks

Use `vTaskDelayUntil()` when a task must run at a steady interval.

```cpp
void controlTask(void *parameter) {
    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        runControlCycle();
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1000));
    }
}
```

This is better than a loose delay for PID timing.

### Queues

A queue passes messages safely between tasks.

Use queues for commands:

```text
Local HMI task ----+
                   +--> command queue --> command handler
PC comms task -----+
```

Example command:

```cpp
enum class CommandType {
    StartRun,
    StopRun,
    SetManualOutput,
    AcknowledgeFault
};

struct Command {
    CommandType type;
    float value;
};
```

The local HMI and PC dashboard should send commands into a queue. They should not directly change heater output variables.

### Mutexes

A mutex is a lock for shared data. "Mutex" means mutual exclusion.

Use a mutex when more than one task reads or writes the same shared state.

Example shared state:

```cpp
struct SensorState {
    float tc1C;
    float tc2C;
    bool tc1Fault;
    bool tc2Fault;
};

SensorState sensorState;
SemaphoreHandle_t sensorMutex;
```

Sensor task writes:

```cpp
xSemaphoreTake(sensorMutex, portMAX_DELAY);
sensorState.tc1C = readTc1();
sensorState.tc2C = readTc2();
sensorState.tc1Fault = false;
sensorState.tc2Fault = false;
xSemaphoreGive(sensorMutex);
```

Control task reads a copy:

```cpp
SensorState localSensors;

xSemaphoreTake(sensorMutex, portMAX_DELAY);
localSensors = sensorState;
xSemaphoreGive(sensorMutex);

runControl(localSensors);
```

Important mutex rule:

```text
lock -> copy quickly -> unlock -> do slow work
```

Do not hold a mutex while drawing the display, waiting for serial data, or reading a slow sensor.

### Semaphores

A semaphore is a signal between tasks.

Examples:

- a button interrupt tells the HMI task something changed.
- a timer tells a task that a cycle is due.
- a task signals that new telemetry is available.

For this furnace firmware, start with queues and mutexes. Add semaphores only where they make the design clearer.

### Watchdog

A watchdog detects firmware that has stopped running correctly.

For this furnace, the watchdog should prove that critical tasks are alive:

- safety task.
- control task.
- SSR timing task.

On a serious watchdog fault or reset, heater output must default OFF.

## Furnace Task Design

### Safety Task

Runs every 10 ms to 50 ms.

Owns:

- emergency stop feedback.
- door/lid switch feedback.
- independent over-temperature feedback.
- safety fault latch.
- firmware heater permission.

This task is not a replacement for hardwired safety. The emergency stop, door switch, over-temperature cutoff, fuse/breaker, and contactor/heater-enable chain still need to physically remove heater power.

### Sensor Task

Runs every 250 ms to 1000 ms.

Owns:

- TC1 reading.
- TC2 reading.
- thermocouple fault detection.
- filtered sensor state.

Furnace temperature changes slowly, so fault detection and reliability matter more than high sample rate.

### Control Task

Runs every 500 ms to 1000 ms.

Owns:

- active profile runtime.
- current setpoint.
- PID calculation.
- manual output handling.
- output limits.

The display and PC do not run PID. They only send commands and display state.

### SSR Timing Task

Runs every 100 ms to 250 ms.

Owns slow time-proportional SSR control.

Example:

```text
control window = 10 seconds
PID output = 40 percent
SSR ON for 4 seconds
SSR OFF for 6 seconds
```

This is the correct approach for a zero-cross AC SSR and resistive heater. It is not high-frequency PWM.

The SSR task must always check safety permission before turning the SSR ON.

### Local HMI Task

Runs button/encoder scanning every 10 ms to 20 ms. Updates the display around 2 to 5 times per second.

Owns:

- rotary encoder.
- Start, Stop, Back buttons.
- display page state.
- screen drawing.

Outputs commands to the command queue.

### PC Communication Task

Handles USB serial first. Later it can handle MQTT, Wi-Fi, or Ethernet.

Owns:

- command parsing.
- telemetry output.
- command responses.

PC communication is optional. If the PC disconnects, the ESP32 must continue safely according to the active run rules.

## Data Flow

Recommended data flow:

```text
Buttons/encoder ----+
                    +--> command queue --> command handler --> controller state
PC serial ----------+

Thermocouple task --> sensor state -----+
Safety task --------> safety state -----+--> control task --> requested output
Profile state ------> profile state ----+

requested output + safety permission --> SSR task --> SSR pin

state snapshots --> local display
state snapshots --> PC telemetry
```

The HMI and PC dashboard should never directly toggle the SSR pin.

## Minimal Code Shape

```cpp
#include <Arduino.h>

struct SensorState {
    float tc1C;
    float tc2C;
    bool fault;
};

struct SafetyState {
    bool estopActive;
    bool doorOpen;
    bool overTemp;
    bool heaterAllowed;
};

SensorState sensorState;
SafetyState safetyState;

SemaphoreHandle_t sensorMutex;
SemaphoreHandle_t safetyMutex;

void safetyTask(void *parameter) {
    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        SafetyState next{};
        next.estopActive = false;
        next.doorOpen = false;
        next.overTemp = false;
        next.heaterAllowed = !next.estopActive && !next.doorOpen && !next.overTemp;

        xSemaphoreTake(safetyMutex, portMAX_DELAY);
        safetyState = next;
        xSemaphoreGive(safetyMutex);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(20));
    }
}

void sensorTask(void *parameter) {
    TickType_t lastWake = xTaskGetTickCount();
    float simulatedTemp = 25.0f;

    while (true) {
        simulatedTemp += 0.1f;

        SensorState next{};
        next.tc1C = simulatedTemp;
        next.tc2C = simulatedTemp + 1.0f;
        next.fault = false;

        xSemaphoreTake(sensorMutex, portMAX_DELAY);
        sensorState = next;
        xSemaphoreGive(sensorMutex);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(500));
    }
}

void setup() {
    Serial.begin(115200);

    sensorMutex = xSemaphoreCreateMutex();
    safetyMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(safetyTask, "Safety", 4096, nullptr, 5, nullptr, 1);
    xTaskCreatePinnedToCore(sensorTask, "Sensors", 4096, nullptr, 2, nullptr, 1);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
```

## Implementation Order

1. Create shared state structs.
2. Create mutexes.
3. Create command queue.
4. Add safety task with simulated inputs.
5. Add sensor task with simulated temperatures.
6. Add control task with fixed test setpoint.
7. Add SSR timing task driving an LED.
8. Add local HMI task for buttons/encoder.
9. Add PC serial telemetry.
10. Replace simulated sensors with thermocouple modules.
11. Replace LED output with isolated SSR driver hardware.

## Common Mistakes

- using `delay()` everywhere instead of `vTaskDelay()` or `vTaskDelayUntil()`.
- sharing variables between tasks without a mutex or queue.
- holding mutexes during slow display, serial, or sensor operations.
- letting UI code control heater outputs directly.
- creating too many tasks with unclear ownership.
- treating RTOS as a substitute for hardwired safety.
