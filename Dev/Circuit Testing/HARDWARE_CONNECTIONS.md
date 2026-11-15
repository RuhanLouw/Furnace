# Furnace SSR circuit bring-up

This project tests the furnace's NPN-driven SSR input and monitors two MAX31855 thermocouple boards using the ESP32-S3-WROOM-1 N16R8 development board: 16 MB flash and 8 MB octal PSRAM. It is a separate hardware test beside `../ProdV1/Furnace Controller`; it does not implement PID, profiles, or safety interlocks. Temperature readings are monitoring only: a probe fault does not disable the manual SSR output.

The older controller targets ESP32-WROOM-32E and assigns GPIO26 to its SSR. Do not reuse that assignment on the S3: its memory wiring and available pins differ. This test assumes **GPIO4** drives the transistor. Match your wiring to this pin or change `SsrDrivePin` in `include/TestConfig.h` before uploading.

## Build and upload

Open this folder as a PlatformIO project. Use the USB-C port labelled UART/COM for the default `ssr_uart` environment. It uses the USB-to-serial bridge.

From this project folder in PowerShell:

```powershell
& 'C:\PlatformIO\penv\Scripts\platformio.exe' run -e ssr_uart
& 'C:\PlatformIO\penv\Scripts\platformio.exe' run -e ssr_uart -t upload
& 'C:\PlatformIO\penv\Scripts\platformio.exe' device monitor -b 115200
```

If multiple COM ports exist, add `--upload-port COMx` when uploading and `--port COMx` when opening the monitor, using the actual port number. Close the monitor before uploading.

For the port labelled USB/OTG, select `ssr_native_usb` instead. That build enables native USB CDC for Serial. The COM port can change when firmware starts. If needed, hold BOOT, tap RESET, release BOOT, and retry upload. Boot button/port details depend on the actual board revision.

## Switching test

### Web dashboard quick start

The web page is stored on the ESP itself. It provides enable/OFF buttons, a 0-100% duty slider, confirmed controller state, connection status, and TC1/TC2 temperatures in degrees Celsius. Each sensor also shows its cold-junction temperature, sample age, and fault status. There are no external web assets or cloud services.

1. Upload `ssr_uart` or `ssr_native_usb` as above.
2. Open serial at 115200 baud. Send `wifi` + Enter to display the connection information and password again.
3. With no configuration file, join the ESP's **Furnace-Test** Wi-Fi. Its static password is **adminpassword**.
4. Open **http://192.168.4.1** (or the AP address printed in serial).
5. Log in as **admin**, password **adminpassword** by default. A separate dashboard password can be configured below.
6. Choose a duty using the slider, then press **Enable output**. Press **OFF** to disable. Setting duty alone does not enable output.

The page confirms a command only after the control task has applied it. Controls wait for each command to finish. If connection is lost, the page marks output as unknown; closing the page or losing Wi-Fi does not turn the output off. The local encoder remains usable. No automatic timeout has been added.

For an existing Wi-Fi network, copy `include/NetworkSecrets.example.h` to `include/NetworkSecrets.h` and enter the Wi-Fi SSID/password. Optionally set `DashboardPassword` to an 8-63 character password. Rebuild/upload. The local configuration file is Git-ignored; do not commit router credentials. An empty dashboard password uses the fixed bench default `adminpassword`. Changing the dashboard password does not change the fallback Wi-Fi password, which remains `adminpassword` across reboots.

The ESP prints its LAN IP once connected. If connection fails for 20 seconds, it also starts the fallback AP. A phone connected to the fallback AP may report no internet; stay connected for the local dashboard.

This build has **no direct Tailscale client**. The page currently works on the same LAN or ESP access point. Remote access needs the later VPN integration or an existing subnet router. It uses password-authenticated HTTP, not HTTPS; keep it on a trusted LAN/VPN and do not port-forward it to the public internet.

### Serial and encoder test

Start with the SSR load/output wiring disconnected from the heater supply. This first test checks the 5 V control circuit and SSR input indicator.

1. Upload; GPIO4 is commanded LOW/OFF at startup, with output disabled and duty at 0%.
2. Open the serial monitor at **115200 baud**. Commands must end with Enter/newline.
3. Send `help` to display the command list.
4. Rotate the encoder to select 0-100% duty, in 5% steps. Clockwise/counterclockwise direction depends on A/B wiring.
5. Press and release the encoder button to enable output. Press again to disable. Holding it does not repeatedly toggle. Serial `on` or `1` also enables the selected duty; it does not set duty to 100%.
6. Send `off` or `0`: GPIO4 goes LOW on receipt of the complete command. The selected duty is retained for the next enable.
7. Send `status` + Enter on the serial/debug port to view SSR/encoder state plus both temperature channels once. Send `temps` + Enter for just the sensor diagnostics: Celsius, cold-junction Celsius, valid flag, fault bits, raw 32-bit frame, and sample age. Invalid temperatures are omitted rather than reported as zero. Serial telemetry is strictly request-only: there is no periodic stream and no streaming command. SSR values are software commands, not physical output feedback. The web dashboard's HTTP status polling is independent of serial output.

The PlatformIO monitor enables local echo and LF line endings. Type a command and press Enter; Backspace/Delete editing, uppercase commands, and surrounding whitespace are accepted. Firmware still waits for a newline to execute a command. Restart the serial monitor after changing its settings, and re-upload for the new quiet-default firmware. Close Teleplot or other terminals before opening the same COM port in PlatformIO.

**There is no automatic timeout.** Once enabled, output continues at the selected duty until disabled or reset, including if the PC disconnects. An unknown or overlong command disables output. Startup/reset never restores an enabled output.

## Heater duty control

This is slow time-proportional control, not high-frequency GPIO PWM. Each 1000 ms window commands GPIO4 HIGH for the selected percentage and LOW for the remainder:

| Duty | ON time | OFF time |
|---|---|---|
| 0% | 0 ms | 1000 ms |
| 25% | 250 ms | 750 ms |
| 50% | 500 ms | 500 ms |
| 75% | 750 ms | 250 ms |
| 100% | Continuously ON while enabled | 0 ms |

`SsrWindowMs` and `DutyStepPercent` are configured in `include/TestConfig.h`. Duty changes take effect in the current window without restarting it. At 0% no power is commanded, but output can remain enabled; increasing duty then begins switching. Disable with the button or `off` before adjusting if you want it to stay off.

Actual AC load conduction is affected by the SSR switching characteristics and AC waveform. The advertised D48100A listing does not confirm zero-cross switching. This bring-up program has no temperature regulation or thermal shutdown inputs.

## Hardware connections

### Two MAX31855 thermocouple boards

These assignments are now in `include/TestConfig.h`. Use the GPIO numbers printed on the ESP32 board, not the ordinal position in the header. The SSR remains on GPIO4, the encoder on GPIO5/6/7, and the RGB LED on GPIO48.

| MAX31855 pin | TC1 board to ESP32-S3 | TC2 board to ESP32-S3 |
|---|---|---|
| Vin | 3V3 | 3V3 |
| 3VO | Leave unconnected | Leave unconnected |
| GND | GND | GND |
| DO / SO | GPIO13, shared | GPIO13, shared |
| CS | GPIO10 | GPIO11 |
| CLK / SCK | GPIO12, shared | GPIO12, shared |
| Thermocouple + / T+ terminal | Probe 1 positive lead | Probe 2 positive lead |
| Thermocouple - / T- terminal | Probe 1 negative lead | Probe 2 negative lead |

Leave GPIO14 unconnected and reserved for the SPI driver's unused MOSI signal. In the installed Arduino-ESP32 2.0.17 core, passing -1 as MOSI still attaches the default FSPI MOSI pin, GPIO11. That conflicts with TC2 CS. The firmware therefore explicitly routes unused MOSI to GPIO14; neither MAX31855 board needs a MOSI wire. A null SPI bus is also checked before starting the sensor task, rather than accepting failed transfers as zero readings.

The listed Vin/3VO arrangement assumes the common Adafruit-style regulated breakout matching your labels. [Adafruit specifies 3.3-5 V input for that board](https://www.adafruit.com/product/269). Use 3.3 V here so the setup stays in the ESP32 voltage domain. `3VO` is the regulator output, not a GPIO or extra supply input: do not join the two boards' regulator outputs. If yours is a different clone, confirm its Vin rating and check that 3VO is close to 3.3 V (within the MAX31855's 3.0-3.6 V operating range) before proceeding. Do not connect 5 V to 3VO, DO, CS, or CLK.

There is no MOSI/DIN connection: this chip only sends data. DO can be shared because the MAX31855 releases its output while CS is HIGH. The code selects only one board at a time. Optional 10 kOhm resistors from each CS to 3V3 hold the boards deselected while the ESP32 resets. Keep the SPI leads short and route them away from SSR/heater conductors. No INA219 board, shunt modification, bias divider, or ESP32 ADC input is used.

Check that both converter chips are the K-type variant (MAX31855K) if your probes are K-type. This is fixed by the chip variant, not selectable in software. Follow probe documentation for polarity; wire colours differ between standards. Do not connect either thermocouple lead to GND or 3V3. The MAX31855 does not support a junction electrically grounded to the furnace/chassis. Use appropriately isolated probes and keep mains away from all low-voltage wiring. Neither the converter nor this wiring provides galvanic isolation.

Each board performs its own cold-junction compensation. Keep the input terminals and converter at a similar, stable ambient temperature, away from the ESP32 regulator, hot furnace surfaces, and strong airflow. A board's displayed cold-junction value is its local converter temperature, not the furnace temperature or an independent safety sensor.

#### First test with the heater disconnected

1. Power everything off, wire both boards as above, and check for shorts. Attach each probe to its own board's + and - terminals.
2. Power the control electronics with the heater supply disconnected. Verify the converter supply voltage, then upload the firmware.
3. Open the existing web dashboard and look for TC1 and TC2. They should settle near room temperature; the first sample waits at least 250 ms after sensor initialization.
4. Warm one probe tip gently. Only its channel should rise. A decreasing reading suggests reversed polarity. Do not warm the converter PCB for this test.
5. Disconnect one probe: that channel should show `Open probe` and `--`, while the other channel keeps updating. Keep probe leads away from grounded objects. Do not intentionally short a probe to a supply rail as a test.
6. Send `temps` + Enter to inspect raw frames and faults without continuous serial output. Reconnect the probe and check that its valid reading returns automatically.
7. Compare against a known thermometer before using measurements in later control work. The 0.25 degree output step is resolution, not total accuracy; probe tolerance, cold-junction gradients, thermocouple nonlinearity, and the converter's specified temperature range matter.

#### Diagnosing zero readings

Keep the heater supply disconnected. An all-zero frame means both the reported probe and cold-junction temperatures are zero with no fault flags; at room temperature, repeated zero frames are suspicious and do not establish that a probe is working.

1. Use corrected firmware with unused MOSI on GPIO14. The original -1 setting allowed the core to take over GPIO11 (TC2 CS), a confirmed software pin conflict; fixing it does not prove the hardware is correct.
2. Send `temps` + Enter. The raw frame distinguishes an actual decoded zero from a web-page issue. Check cold-junction temperature too: it should be close to the converter board's ambient temperature even when the probe is disconnected.
3. With a DC voltmeter, measure each module's Vin and 3VO against that same module's GND. For this wiring Vin should be about 3.3 V and 3VO must be within the chip's 3.0-3.6 V operating range. If 3VO is too low, stop and identify the regulator/board before changing power connections.
4. Power off before rewiring. Test board 1 alone, removing all connections to board 2, including its DO connection. Retain shared pin assignments CLK12/DO13 and CS10. Never leave an unpowered board attached to the shared data bus for this isolation test.
5. Power up and disconnect only the thermocouple from board 1's terminals. After a fresh conversion it should report an open-probe fault. If raw data is still all zero, concentrate on power, CS/CLK/DO wiring, and the SPI path rather than temperature conversion or probe polarity.
6. Repeat board 2 by itself, using CS11. If each works alone but not together, check the separate CS wires, DO sharing, solder bridges, and both boards' supply voltages.
7. If available, use a logic analyser: CS10 or CS11 should go LOW for one 32-clock burst, CLK12 runs at 1 MHz, and DO13 supplies data. Compare signals at the ESP32 header and converter pins. A meter averages the short clock bursts and cannot validate SPI timing.

No clock at the converter points to the firmware clock route or CLK wiring; clock at the converter but none at its DO points to selection, power, or the converter; data at the module but not the ESP32 points to DO wiring. These are diagnostic clues, not conclusive fault identification without observing the signals.

#### Implementation details

`ThermocoupleInput.cpp` uses the Arduino core's built-in SPI driver at 1 MHz, mode 0. No additional PlatformIO library is needed. A dedicated low-priority FreeRTOS task reads both converters every 250 ms, separate from the existing 1 ms SSR/encoder task. Serial and HTTP read a mutex-protected snapshot; they do not initiate competing SPI transactions.

`Max31855Frame.h` decodes signed thermocouple bits 31:18 at 0.25 degrees/count and signed cold-junction bits 15:4 at 0.0625 degrees/count. The thermocouple field already includes the chip's cold-junction compensation; software must not add the cold-junction temperature a second time. Fault bits 0/1/2 indicate open probe, short to GND, and short to VCC. Reserved-bit or fault-summary inconsistencies are flagged as invalid SPI frames.

`GET /api/status` retains the existing controller fields and adds a `thermocouples` array ordered TC1, TC2. Each object includes `valid`, `celsius`, `cold_junction_c`, `faults`, `status`, `raw`, and `age_ms`. Temperatures are JSON `null` when invalid; no NaN or fabricated zero is sent. Readings older than 1500 ms are marked stale, and browser disconnection clears the displayed temperatures. The page polls every 500 ms.

The DO input has an internal pullup so a completely disconnected bus normally produces an invalid all-ones frame. The MAX31855 has no device-ID or CRC check: some wiring faults, such as DO stuck LOW, can still mimic a valid zero-valued frame. Fault checking is not proof of correct installation. This remains a manual bench controller, not a safe unattended furnace controller; sensor faults and high temperature do not automatically stop the SSR.

### Onboard LED

The firmware drives the onboard addressable RGB LED; no external LED or LED resistor is needed.

| Indication | Meaning |
|---|---|
| Brief blue pulse every 2 seconds | Output disabled, firmware heartbeat |
| Green | SSR commanded ON during the current duty window; steady at 100% |
| Cyan | Output enabled, currently in OFF portion of duty window (also enabled at 0%) |
| Red for 2 seconds | Invalid/overlong command has switched the SSR command OFF |

The LED is assumed to be on **GPIO48**. Some DevKitC revisions use **GPIO38**; change `StatusLedPin` in `include/TestConfig.h` if that matches your board. `StatusLedBrightness` controls intensity (0-255), default 16. A fixed power LED cannot be controlled by firmware. The RGB indication runs without adding blink delays to the switching loop.

### Bare rotary encoder and push button

Use these S3 assignments; the earlier ESP32 rotary test's GPIO32/33/25 assignments do not transfer to this board.

| Encoder contact | ESP32-S3 connection |
|---|---|
| Rotary A | GPIO5 |
| Rotary B | GPIO6 |
| Rotary common C | GND |
| Push button contact 1 | GPIO7 |
| Push button contact 2 | GND |

All three inputs use internal pullups. The bare encoder needs no 5 V or 3.3 V supply connection and no external series resistors. Confirm the three rotary contacts and separate button pair with a meter while disconnected. Many encoders have common in the centre of the three-contact row, but use the actual part's pinout. Swap A/B if direction is reversed. Keep wires short; external pullups can be considered if noise is observed.

`RotaryInput.cpp` samples the inputs in the dedicated `SSRControl` FreeRTOS task (nominally about 1 ms) and debounces the push button for 30 ms. The same task owns SSR timing and applies queued serial/web commands; web serving and serial printing run separately. The encoder uses polling, not interrupts. Very fast rotation can still miss transitions. `EncoderEdgesPerDetent` defaults to 4; if you consistently need two mechanical clicks per 5% change, check whether your encoder needs 2 instead. Invalid two-bit quadrature jumps discard the partial detent.

### SSR input drive

Selected parts: **JGX-1 D48100A SSR** and **BC547 NPN** as a low-side input switch. The [supplier listing](https://www.robotics.org.za/D48100A) specifies 3-32 V DC control, 5-25 mA control current, and a 24-480 V AC output. A 5 V control supply is within that range. The listing does not establish the output semiconductor topology or confirm zero-cross switching.

```text
Regulated +5 V ----------+-------------------- SSR INPUT +
                        |
                        +-------------------- ESP32 board 5V/VIN

ESP32 GPIO4 ---- 1 kOhm ---------------------- BC547 base (B)
                                                 |
                                                10k
                                                 |
Supply GND -------------+-------------------- NPN emitter (E)
                        |
                        +-------------------- ESP32 GND

SSR INPUT - --------------------------------- NPN collector (C)
```

| Connection | Destination |
|---|---|
| Regulated +5 V | Board pin labelled 5V/VIN and SSR input + |
| Supply GND | Board GND and NPN emitter |
| GPIO4 | BC547 base through 1 kOhm series resistor |
| Base-to-emitter pulldown | 10 kOhm resistor from base to emitter/GND |
| NPN collector | SSR input - |

- For the onsemi BC547 TO-92 part, pin 1 is collector, pin 2 base, and pin 3 emitter. Check the package drawing for your manufacturer's part before wiring.
- Use a **1 kOhm base resistor**, never a direct GPIO-to-base connection. With about 0.8 V base-emitter drop, it supplies approximately (3.3 - 0.8) / 1000 = 2.5 mA before the pulldown current. This is a conservative base drive for the listed SSR input current, well below the BC547's 100 mA collector limit. The transistor carries only SSR input current, not heater current.
- The 10 kOhm base-emitter pulldown keeps the NPN off while the ESP32 resets or its pin floats.
- Keep 5 V away from GPIO4 and all other GPIO pins. GPIO HIGH is approximately 3.3 V.
- With an external 5 V supply connected, verify the board's USB power arrangement before also connecting powered USB; some clones can backfeed USB. For a simple bench setup, USB can power the board/control circuit if the combined load is within its supply budget.
- A DC-input SSR is not a mechanical relay coil; do not assume it needs a flyback diode. A mechanical relay coil would require a suitable driver and flyback protection.
- GPIO HIGH should switch the NPN on and pull SSR input - toward ground. GPIO LOW should remove SSR input current. The SSR's output/load terminals are separate from these control connections.

## D48100A measurements

Keep the heater supply disconnected during control-side checks. Use DC voltage mode:

| Measurement | Command OFF | Command ON |
|---|---|---|
| GPIO4 to GND | Approximately 0 V | Approximately 3.3 V |
| BC547 base to emitter | Approximately 0 V | Approximately 0.7-0.9 V |
| BC547 collector to emitter | Not a useful pass/fail value by itself | Normally a few tenths of a volt or less |
| SSR input + to input - | Below activation voltage | Close to 5 V minus transistor saturation voltage |

The input indicator is useful for checking drive but does not prove the output can carry load current. Use 100% duty while enabled for steady DC drive measurements; lower duty makes the readings cycle or average. There is no automatic timeout; send `off` after the measurement.

Do not use continuity mode as a pass/fail test for the D48100A output. Its AC output needs a suitable AC load test within its ratings; a meter's small DC test signal does not reproduce that condition. Do not use resistance or continuity mode on an energized load circuit. A 5 V or 12 V DC LED test load is unsuitable for validating this AC output.

For an output test, use an enclosed, fused AC resistive test load and suitable isolated AC source, with the output voltage within the SSR's specified range. Minimum load current is not given in the listing, so a very small load can give misleading results. Confirm the manufacturer's full data before choosing the load.

For the furnace, determine heater current from its supply voltage and power, then size the SSR heatsink and protection from verified thermal data. The printed 100 A rating alone is insufficient. The SSR can leak current when off and can fail conducting; retain the independent heater shutdown path. This bring-up firmware has no furnace safety inputs.

## Reference

[MAX31855 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/MAX31855.pdf): SPI framing, conversion timing, cold-junction compensation, and fault bits.

[Adafruit MAX31855 guide](https://learn.adafruit.com/thermocouple/arduino-code): grounded-probe restriction and board bring-up.

[Espressif ESP32-S3-WROOM-1 / WROOM-1U datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf): module memory variants and GPIO restrictions.

[D48100A supplier specifications](https://www.robotics.org.za/D48100A).

[onsemi BC547 datasheet](https://www.onsemi.com/pdf/datasheet/bc546-d.pdf).
