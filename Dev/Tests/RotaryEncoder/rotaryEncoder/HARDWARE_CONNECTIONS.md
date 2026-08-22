# Rotary Encoder Teleplot Test Hardware Connections

This test reads a bare mechanical rotary encoder with a push button and sends rotation/button telemetry to the VS Code Teleplot extension.

The firmware uses FreeRTOS:

- `encoderTask` polls the rotary A/B contacts every 1 ms.
- `buttonTask` is woken by a GPIO interrupt on the encoder push button, then debounces in task context.
- `telemetryTask` sends Teleplot values every 100 ms.
- shared state is protected with a mutex.

## Test Project

```text
Dev/Tests/RotaryEncoder/rotaryEncoder
```

## Clock And Timing Settings

Project-level ESP32 clock settings are in `platformio.ini`:

```ini
board_build.f_cpu = 240000000L
board_build.f_flash = 40000000L
board_build.flash_mode = dio
monitor_speed = 115200
```

Firmware timing constants are at the top of `src/main.cpp`:

```cpp
SerialBaud = 115200
PwmLedFrequencyHz = 5000
PwmLedResolutionBits = 8
EncoderPollPeriodMs = 1
ButtonDebounceMs = 30
TelemetryPeriodMs = 100
```

The FreeRTOS task delays use these constants through `vTaskDelay()` and `vTaskDelayUntil()`.

## ESP32 Pins Used

| Function | ESP32 pin | Direction | Notes |
|---|---:|---|---|
| Encoder A | GPIO32 | input pullup | Rotary quadrature contact A |
| Encoder B | GPIO33 | input pullup | Rotary quadrature contact B |
| Encoder button | GPIO25 | input pullup interrupt | Active low push button |
| PWM LED | GPIO26 | PWM output | External LED brightness output |
| Activity LED | GPIO2 | output | Onboard LED on many ESP32 dev boards |
| Ground | GND | reference | Common ground for encoder |

The sketch uses `INPUT_PULLUP`, so each input idles high and becomes low when connected to ground.

## Bare Encoder Wiring

A bare rotary encoder usually has 5 pins:

- 3 pins for the rotary contacts.
- 2 pins for the push button.

The rotary side normally has:

```text
A   C   B
```

where `C` is the common pin.

Wire it like this:

| Encoder pin | Connect to |
|---|---|
| A | ESP32 GPIO32 |
| B | ESP32 GPIO33 |
| C / common | ESP32 GND |
| Button pin 1 | ESP32 GPIO25 |
| Button pin 2 | ESP32 GND |

If turning clockwise decreases the value, swap A and B.

## External PWM LED Wiring

Add one external LED for the brightness test.

Wire it like this:

```text
ESP32 GPIO26 -> resistor -> LED anode
LED cathode -> ESP32 GND
```

Use a current-limiting resistor. Start with:

- `330 ohm` for a typical red/green LED.
- `470 ohm` or `1 kOhm` if you want it dimmer.

Do not connect an LED directly to an ESP32 GPIO without a resistor.

The encoder controls the LED:

- rotate clockwise: increase brightness.
- rotate counter-clockwise: decrease brightness.
- press encoder button: toggle LED on/off.

The button interrupt does not directly control the LED. It only wakes the button task. Debounce, state changes, PWM updates, and Teleplot output happen outside the interrupt.

## KY-040 Breakout Reference

If you later use a KY-040 breakout board, the names are usually:

| KY-040 pin | Connect to |
|---|---|
| CLK | ESP32 GPIO32 |
| DT | ESP32 GPIO33 |
| SW | ESP32 GPIO25 |
| + | 3.3 V |
| GND | GND |

For the bare encoder test, do not connect 3.3 V to the encoder contacts directly. The ESP32 internal pullups provide the pullup path.

## Voltage Notes

- ESP32 GPIO is 3.3 V only.
- Do not connect 5 V to any ESP32 input.
- A bare mechanical encoder does not need a supply voltage when using internal pullups.
- If powering the ESP32 board through its `5V`/`VIN` pin, use a regulated 5 V supply and connect supply ground to ESP32 ground.
- The ESP32 board's `3V3` pin is normally powered from the onboard regulator when USB or `5V`/`VIN` is supplied.
- Only use the `3V3` pin for small loads such as sensor modules or pullups. Do not power high-current loads from it.
- Keep wires short during the test. Long wires can pick up noise and cause false counts.

## Teleplot Setup

Install/open the VS Code Teleplot extension.

Use:

- baud rate: `115200`
- serial port: the ESP32 USB serial port

The sketch sends lines such as:

```text
>encoder_position:3
>encoder_delta:1
>button_pressed:0
>button_press_count:2
>led_enabled:1
>led_brightness_percent:45
```

Teleplot plots serial messages formatted as:

```text
>variable_name:value
```

Normal startup text appears in the Teleplot console.

## Expected Behaviour

- Rotate clockwise: `encoder_position` should increase.
- Rotate counter-clockwise: `encoder_position` should decrease.
- Press button: `button_pressed` goes to `1`.
- Release button: `button_pressed` goes to `0`.
- Each press increments `button_press_count`.
- The external LED on GPIO26 changes brightness as the encoder turns.
- Pressing the encoder button toggles the external LED on/off.
- The onboard LED toggles on accepted rotation detents and button presses.

## Troubleshooting

If rotation direction is reversed:

- swap the A and B wires.

If position jumps erratically:

- shorten wires.
- confirm common pin is connected to GND.
- confirm only GPIO32/GPIO33/GPIO25 are connected to encoder contacts.
- add external 10 kOhm pullups to 3.3 V if internal pullups are too weak.
- add small hardware debounce capacitors only after confirming the raw wiring.

If the button is always pressed:

- check that GPIO25 is not permanently shorted to GND.
- check the button pair with a multimeter; the two button pins should only connect while pressed.

If Teleplot shows nothing:

- confirm baud rate is `115200`.
- close other serial monitors using the same COM port.
- confirm the sketch uploaded to the ESP32.
