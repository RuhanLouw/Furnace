# Initial Electronics Architecture

This is a first-pass architecture based on the supplied brief. Values, parts, and ratings must be selected only after the heater voltage, heater current, thermocouple type, enclosure constraints, and applicable safety standard are confirmed.

## Proposed Block Diagram

```text
PC dashboard
   |
   | USB serial / Ethernet / Wi-Fi
   |
ESP32 controller + command/state machine
   |
   +-- Local furnace HMI
   |     +-- Display
   |     +-- Buttons / rotary encoder
   |
   +-- Thermocouple interface TC1
   +-- Thermocouple interface TC2
   |
   +-- Status LED: Running/Off
   |
   +-- Heater command output
          |
          v
Hardware heater-enable chain
   |
   +-- Emergency stop contact
   +-- Door/lid limit switch
   +-- Independent over-temperature cutoff
   +-- Controller enable relay/contactor path
          |
          v
Solid-state relay
          |
          v
Resistive heating element
```

## Recommended Starting Architecture

Begin with a PC dashboard plus ESP32-WROOM-32E controller.

The ESP32-WROOM-32E should be the primary machine controller. It must be able to run a full temperature programme without the PC connected. The PC should be treated as an optional operator interface for setup, monitoring, logging, graphing, and easier profile editing.

The local furnace HMI should be connected directly to the ESP32 and should provide enough control to operate the furnace when the PC is removed.

Recommended role split:

- ESP32-WROOM-32E: profile storage, runtime state, setpoint generation, PID, SSR control, thermocouple reads, interlocks, faults, watchdog, and local HMI.
- PC dashboard: profile editor, start/stop commands if enabled, live graph, logs, diagnostics, PID tuning, export.
- Local HMI: select profile, edit basic profile values, start, stop, acknowledge faults, view runtime, setpoint, process values, output, and fault state.

The first prototype can use USB serial between the PC and ESP32. Network communication can be added later once the command model is stable.

## HMI Baseline

Use two HMIs that talk to the same ESP32 command/state machine:

- PC HMI: Node-RED with FlowFuse Dashboard, a custom web app, or another PC dashboard.
- Local HMI: ESP32-attached non-touch display, rotary encoder, and buttons.

Both HMIs should send commands through the same internal command handler. Do not create separate control paths for PC and local operation.

Operator authority rules:

- Safety stop, emergency stop, door open, and over-temperature faults override both HMIs.
- Local Stop must always be accepted, even if the run was started from the PC.
- Manual output mode must still be blocked by hardware and firmware safety limits.
- Profile editing should be locked while a run is active unless a deliberate pause/edit/resume feature is later specified.
- Only one profile source should be active at a time.
- The ESP32 should report the current authority state, such as local, PC, running, idle, faulted, or locked.

## Control Approach

The temperature profile should be implemented as a segment table. Each segment defines a target temperature and duration. The controller calculates the current setpoint from elapsed runtime and the active segment.

Recommended segment types:

- Ramp from current temperature or previous target to next target over a defined time.
- Hold at target temperature for a defined time.
- Cooldown or ramp-down segment where heater output is limited or disabled depending on process requirements.

AUTO mode:

- PID controller calculates output percentage from setpoint and measured temperature.
- Output percentage is converted into time-proportional SSR control.
- PID gains, output limits, and ramp-rate limits should be configurable.

MANUAL mode:

- User directly sets output percentage.
- Safety interlocks and fault shutdowns must still override manual output.
- Mode changes should require user confirmation.

## SSR Drive Recommendation

For an AC resistive heater, use time-proportional control with a zero-cross SSR unless the process specifically requires phase-angle control. This means the firmware switches the SSR on for a percentage of a fixed control window, for example 2 seconds on in a 10 second window for 20 percent output.

High-frequency PWM is usually not appropriate for a mains zero-cross SSR. If fast power modulation is required, that becomes a different power electronics design with additional EMI, isolation, and thermal implications.

## Safety Chain

The heater should require both a software command and a hardware enable path before power reaches the heating element.

Minimum hardware shutdown inputs:

- Emergency stop.
- Door/lid open switch.
- Independent over-temperature cutoff.
- Over-current protection.

The hardware chain should be designed so a broken wire, failed switch, or unpowered controller tends toward heater OFF where practical.

## Measurement

Temperature:

- Use two thermocouple channels rated for the selected thermocouple type and 1200 C operation.
- Provide cold-junction compensation.
- Add fault detection for open thermocouple and out-of-range readings.

Power indication:

- Measure heater current with an isolated current sensor or current transformer.
- Measure voltage only with a properly isolated sensing circuit.
- Calculate apparent power or energy if useful for the monitoring screen.

## Firmware Responsibilities

- Parse and validate temperature profiles from the PC dashboard and local HMI.
- Store validated profiles locally so a run can continue without the PC connected.
- Store active runtime state.
- Calculate setpoint from the active profile segment.
- Read TC1 and TC2.
- Detect sensor faults and interlock faults.
- Run PID in AUTO mode.
- Apply manual output in MANUAL mode.
- Convert output percentage into SSR control.
- Report runtime, setpoint, PV values, output percentage, and fault state to the PC dashboard and local HMI.
- Reset to idle after programme completion or stopped run.

## Hardware Design Packages To Create

- System block diagram.
- Electrical safety concept.
- I/O list.
- Power budget.
- Heater power stage schematic.
- Thermocouple input schematic.
- Microcontroller and display schematic.
- Interlock and emergency stop schematic.
- PCB layout rules and isolation notes.
- Wiring diagram.
- Bill of materials.
- Commissioning and test procedure.

## Immediate Design Decisions Needed

The next design step depends on these decisions:

- Heater supply voltage and maximum current.
- Heater element resistance and power.
- Thermocouple type and mounting method.
- Required controller hardware platform.
- Laptop UI technology.
- Display type and enclosure size.
- Whether the system must satisfy a formal safety or compliance standard.
