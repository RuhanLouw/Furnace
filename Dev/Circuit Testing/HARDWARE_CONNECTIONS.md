# Furnace SSR circuit bring-up

This project tests the furnace's NPN-driven SSR input using the newly identified ESP32-S3-WROOM-1 N16R8 development board: 16 MB flash and 8 MB octal PSRAM. It is a separate hardware test beside `../ProdV1/Furnace Controller`; it does not implement temperature measurement, PID, profiles, or interlocks.

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

The web page is stored on the ESP itself. It provides enable/OFF buttons, a 0-100% duty slider, confirmed controller state, and connection status. There are no external web assets or cloud services.

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
7. Send `status` + Enter on the serial/debug port to view `ssr_command`, `output_enabled`, `duty_percent`, `button_pressed`, and `button_press_count` once. Serial telemetry is strictly request-only: there is no periodic stream and no streaming command. These are software values, not physical output feedback. The web dashboard's HTTP status polling is independent of serial output.

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

[Espressif ESP32-S3-WROOM-1 / WROOM-1U datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf): module memory variants and GPIO restrictions.

[D48100A supplier specifications](https://www.robotics.org.za/D48100A).

[onsemi BC547 datasheet](https://www.onsemi.com/pdf/datasheet/bc546-d.pdf).
