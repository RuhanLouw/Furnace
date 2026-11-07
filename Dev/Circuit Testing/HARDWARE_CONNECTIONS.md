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

Start with the SSR load/output wiring disconnected from the heater supply. This first test checks the 5 V control circuit and SSR input indicator.

1. Upload; GPIO4 is commanded LOW/OFF at startup.
2. Open the serial monitor at **115200 baud**. Commands must end with Enter/newline.
3. Send `help` to display the command list.
4. Send `on` or `1`: GPIO4 goes HIGH and energizes the SSR input through the NPN.
5. Send `off` or `0`: GPIO4 goes LOW immediately on receipt of the complete command.
6. An ON activation automatically ends after **5 seconds**, even without a connected serial monitor. Repeating `on` while already on does not extend that interval.
7. Send `status` to view `ssr_command` and remaining time. These are command values, not physical output feedback. Teleplot can also plot the `>name:value` lines sent each second.

The initial state remains OFF after reset; there is no automatic repeating cycle. An unknown or overlong command also commands OFF. A firmware timeout is a bench convenience, not an independent thermal cutoff.

## Hardware connections

### Onboard LED

The firmware drives the onboard addressable RGB LED; no external LED or LED resistor is needed.

| Indication | Meaning |
|---|---|
| Brief blue pulse every 2 seconds | Firmware heartbeat while SSR command is OFF |
| Steady green | SSR commanded ON; not measured load current |
| Amber for 2 seconds | Five-second ON timeout has switched the SSR command OFF |
| Red for 2 seconds | Invalid/overlong command has switched the SSR command OFF |

The LED is assumed to be on **GPIO48**. Some DevKitC revisions use **GPIO38**; change `StatusLedPin` in `include/TestConfig.h` if that matches your board. `StatusLedBrightness` controls intensity (0-255), default 16. A fixed power LED cannot be controlled by firmware. The RGB indication runs without adding blink delays to the switching loop.

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

The input indicator is useful for checking drive but does not prove the output can carry load current. The five-second ON timeout still applies.

Do not use continuity mode as a pass/fail test for the D48100A output. Its AC output needs a suitable AC load test within its ratings; a meter's small DC test signal does not reproduce that condition. Do not use resistance or continuity mode on an energized load circuit. A 5 V or 12 V DC LED test load is unsuitable for validating this AC output.

For an output test, use an enclosed, fused AC resistive test load and suitable isolated AC source, with the output voltage within the SSR's specified range. Minimum load current is not given in the listing, so a very small load can give misleading results. Confirm the manufacturer's full data before choosing the load.

For the furnace, determine heater current from its supply voltage and power, then size the SSR heatsink and protection from verified thermal data. The printed 100 A rating alone is insufficient. The SSR can leak current when off and can fail conducting; retain the independent heater shutdown path. This bring-up firmware has no furnace safety inputs.

## Reference

[Espressif ESP32-S3-WROOM-1 / WROOM-1U datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf): module memory variants and GPIO restrictions.

[D48100A supplier specifications](https://www.robotics.org.za/D48100A).

[onsemi BC547 datasheet](https://www.onsemi.com/pdf/datasheet/bc546-d.pdf).
