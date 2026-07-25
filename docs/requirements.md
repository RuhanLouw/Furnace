# Furnace Electronics Requirements

This document captures the supplied project brief from:

- `Furnace Specs_260616_125326.pdf`
- `Furnace Project Specification_260708_203945.pdf`
- `furnace.jpeg`

## System Purpose

Design electronics and control software for a muffle furnace with programmable temperature profiles up to 1200 C. The system must heat a resistive coil according to a user-defined ramp/soak programme, monitor temperature with thermocouples, and shut heating down safely on user stop or fault conditions.

## User Interface Requirements

### Laptop Interface

The laptop interface should provide two main screens.

Input screen:

- Enter target temperatures and segment times before running a programme.
- Support ramp and hold stages, for example:
  - Ramp to Temp1 over time1.
  - Hold Temp1 for time2.
  - Ramp to Temp2 over time3.
  - Hold Temp2 for time4.
  - Ramp down or cooldown over time5.
- Confirm with the user before starting runtime.

Monitoring screen:

- Display current runtime since programme start.
- Display current setpoint temperature.
- Display current process values from TC1 and TC2.
- Display controller output percentage.
- Provide AUTO and MANUAL modes.
- Confirm with the user before switching AUTO/MANUAL mode.
- In AUTO, enable PID control.
- In MANUAL, allow direct user adjustment of output percentage.
- Show a live graph of furnace behaviour.

### Furnace Display

The physical furnace display should show:

- Current runtime.
- Current target stage, such as Temp1 or Temp2.
- Current calculated setpoint.
- Current process values from the thermocouples.

## Control Requirements

- Use a microcontroller-based controller.
- Use a temperature programme to calculate the active setpoint over time.
- Compare the active setpoint against measured thermocouple temperature.
- Use PID control in AUTO mode.
- Allow PID tuning by the user or commissioning engineer.
- Drive the heating element through a solid-state relay.
- Controller output should be represented as output percentage or duty cycle.
- Reset the programme after completion or emergency stop so a new programme can be entered.

## Sensor Requirements

- Use two thermocouples, labelled TC1 and TC2.
- Use thermocouple signal conditioning appropriate for the selected thermocouple type and maximum temperature.
- Detect thermocouple faults where possible, including open-circuit, implausible readings, and disagreement between TC1 and TC2.

## Heater And Power Requirements

- Heat using a resistive coil element.
- Provide a power supply for the heating coil.
- Provide a separate electronics supply or an isolated auxiliary supply for low-voltage electronics.
- Include voltage and current measurement or indication for power usage.
- Include over-current protection with a fuse or suitable protective device.

## Safety Requirements

The brief explicitly requests:

- Emergency stop switch.
- Fuse for over-current protection.
- Door or lid limit switch.
- Heater shutdown when the door or lid is open.
- Running/Off LED indicator.

Additional safety requirements that should be treated as mandatory for design review:

- Heater output must default OFF during reset, firmware crash, sensor fault, communication loss, and startup.
- Use galvanic isolation between mains/heater circuitry and user-accessible electronics.
- Use an independent over-temperature cutoff that does not rely only on firmware.
- Use correctly rated SSR, heatsink, wiring, terminals, fuses, and enclosure materials.
- Use a hardware heater-enable chain so emergency stop, door switch, and over-temperature faults can remove heater power independently of software.
- Add watchdog handling in firmware.
- Add fault latching for serious faults until manually acknowledged.
- Define creepage, clearance, earthing, strain relief, and protective bonding requirements before PCB layout.

## Mechanical And Enclosure Requirements

- Electronics should fit in a 3D printed enclosure.
- The furnace reference image shows a top-opening insulated furnace with an external frame and lid/door mechanism.
- Door/lid interlock placement must be mechanically reliable and protected from heat.
- The electronics enclosure must be thermally separated from hot furnace surfaces.

## Open Questions

- What mains input voltage and frequency will be used?
- What is the heater power rating, coil resistance, and maximum current?
- Is the heater AC mains, low-voltage AC, or DC?
- What thermocouple type is required, for example K, N, R, or S?
- What accuracy and repeatability are required?
- Should TC1, TC2, average temperature, maximum temperature, or a selected channel drive the PID loop?
- What is the required temperature ramp-rate range?
- What is the acceptable overshoot at 1200 C?
- What microcontroller or controller platform is preferred?
- How should the laptop communicate with the controller: USB, serial, Ethernet, Wi-Fi, or Bluetooth?
- What physical display is required on the furnace?
- Is manual output mode allowed while the door is open, or must all heater output remain blocked?
- What regulatory standard, site safety rule, or certification target applies?
- What ambient temperature will the electronics enclosure experience?
- Is the furnace intended for supervised lab use only or unattended operation?
