# Local HMI Design

This document defines the furnace-mounted operator interface for the ESP32-WROOM-32E controller.

## HMI Direction

Use a non-touch display with physical controls.

Recommended control set:

- color TFT display.
- rotary encoder with push button.
- dedicated Start button.
- dedicated Stop button.
- Back/Cancel button.
- fault acknowledge button, or acknowledge through encoder push if panel space is limited.
- Running/Heating status LED.
- Fault status LED or buzzer.

This is better than a touchscreen for this furnace because physical controls are easier to operate near heat, dust, gloves, and workshop conditions. It also gives Stop a fixed physical location.

## Display Recommendation

Start with a 2.8 inch or 3.2 inch SPI TFT.

Recommended minimum:

- 320 x 240 resolution.
- SPI interface.
- 3.3 V logic.
- readable backlight.
- simple rectangular active area for status-heavy screens.

Likely controller families:

- ILI9341.
- ST7789.
- ILI9488 if a larger display is needed.

For the first prototype, prefer a common SPI TFT module with existing ESP32 library support. Avoid a large high-resolution display unless there is a strong reason because the ESP32-WROOM-32E has no PSRAM.

## Physical Layout

Suggested front panel layout:

```text
+------------------------------------------------+
|                 TFT DISPLAY                    |
|                                                |
| Runtime        Setpoint        TC1 / TC2       |
| State          Output %        Fault summary   |
+------------------------------------------------+
|                                                |
|  [START]   [STOP]   [BACK]     ( ROTARY )      |
|                              push = confirm    |
+------------------------------------------------+
|  RUN LED       HEAT LED        FAULT LED        |
+------------------------------------------------+
```

Control placement:

- Stop should be physically obvious and easy to reach.
- Start and Stop should not depend on menu focus.
- Rotary encoder should handle menu navigation and value editing.
- Back should always leave the current page or cancel an edit.
- Fault indication should be visible without reading small text.

## Page Structure

Recommended local pages:

- Home.
- Profile Select.
- Profile Edit.
- Run.
- Manual Output.
- Faults.
- Settings.

Home page:

- controller state.
- selected profile.
- TC1 and TC2.
- current setpoint.
- output percentage.
- active fault summary.

Profile Select page:

- list stored profiles.
- select active profile.
- show profile summary.

Profile Edit page:

- edit basic ramp and hold values.
- confirm before saving.
- reject impossible values.

Run page:

- runtime.
- active segment.
- setpoint.
- TC1.
- TC2.
- output percentage.
- mode, AUTO or MANUAL.
- interlock state.

Manual Output page:

- manual output percentage.
- clear warning state in UI.
- output limited by firmware.
- interlocks still override output.

Faults page:

- active fault list.
- latched fault list.
- acknowledge action.
- show whether heater output is blocked.

Settings page:

- display brightness.
- units if needed.
- communication status.
- PID parameters if local tuning is allowed.
- firmware version.

## Button Behaviour

Dedicated buttons:

- Start: request run start from `IDLE` or `READY`.
- Stop: request controlled stop from any non-fault run state.
- Back: cancel edit, leave page, or return home.
- Fault Ack: acknowledge latchable faults when the fault condition is no longer present.

Rotary encoder:

- rotate: move selection or change value.
- press: confirm selection or enter edit mode.
- long press: optional shortcut to Home or Settings.

Stop behaviour:

- Stop must not depend on the current screen.
- Stop must request heater output OFF immediately.
- Stop is separate from emergency stop. Emergency stop should be a hardwired safety device, not only an ESP32 input.

## Interaction Rules

- The local HMI and PC dashboard must use the same ESP32 command handler.
- If a run is active, profile editing is locked unless pause/edit/resume is explicitly implemented.
- PC commands should be visible on the local HMI.
- Local Stop should always be accepted.
- Safety faults override every screen and every mode.
- Manual output should require confirmation before entry.
- Returning from a fault must require deliberate acknowledgement.

## Firmware UI Approach

Recommended for first prototype:

- Use a simple page/state UI, not a full desktop-style framework.
- Use fixed-layout screens with large numeric fields.
- Redraw at a modest rate, for example 2 to 5 Hz for live values.
- Keep the control loop independent of display redraw timing.

Suitable library direction:

- `TFT_eSPI` or equivalent for SPI TFT rendering.
- Small menu/state-machine layer written in firmware.
- Avoid heavy UI frameworks unless memory use is proven acceptable on ESP32-WROOM-32E.

## Initial ESP32 I/O Need

Approximate local HMI I/O:

- SPI SCLK.
- SPI MOSI.
- TFT chip select.
- TFT data/command.
- TFT reset.
- TFT backlight control.
- encoder A.
- encoder B.
- encoder push.
- Start button.
- Stop button.
- Back button.
- status LED outputs.
- optional buzzer.

This is compatible with the ESP32-WROOM-32E, but pin assignment must avoid flash pins and problematic boot strapping behaviour.

## Prototype Build

First HMI prototype:

- ESP32-WROOM-32E development board.
- 2.8 inch or 3.2 inch SPI TFT.
- rotary encoder with push button.
- Start, Stop, and Back pushbuttons.
- three LEDs: Run, Heat, Fault.
- USB serial link to PC.
- simulated thermocouple values and simulated heater output.

Second HMI prototype:

- add two thermocouple interface modules.
- add real profile execution.
- add local profile storage.
- add PC dashboard telemetry.
- keep heater output simulated until interlock logic is tested.
