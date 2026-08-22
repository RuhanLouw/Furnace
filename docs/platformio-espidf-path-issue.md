# PlatformIO ESP-IDF Path Issue

## Error Seen

PlatformIO failed during ESP-IDF project creation with:

```text
Error: Detected a whitespace character in project paths.
```

The attempted project path was:

```text
C:\Users\Ruhan Louw\OneDrive\Documents\Furnace\Dev\ProdV2\FurnaceController
```

This path contains whitespace in:

- `Ruhan Louw`
- `OneDrive\Documents` is fine, but the user profile name is not.

The existing Arduino project path also contains whitespace:

```text
Dev\ProdV1\Furnace Controller
```

## What Happened

The ESP-IDF project was partially created in:

```text
Dev\ProdV2\FurnaceController
```

with:

```ini
framework = espidf
```

but PlatformIO/ESP-IDF refused to finish processing because the full absolute path contains a space.

This is an ESP-IDF tooling/path restriction, not a furnace firmware design problem.

## Recommendation

For this project, keep the current Arduino ESP32 project for the first prototype and use FreeRTOS from inside Arduino.

That gives:

- no ESP-IDF path fight inside the current OneDrive user path.
- faster start with TFT, buttons, serial, and thermocouple libraries.
- access to FreeRTOS tasks, queues, mutexes, and timers.

The current Arduino project is:

```text
Dev\ProdV1\Furnace Controller
```

## If ESP-IDF Is Still Needed

Create or move the ESP-IDF project to a path with no spaces anywhere in the full absolute path.

Examples:

```text
C:\dev\furnace-controller
C:\furnace\controller
D:\dev\furnace-controller
```

Avoid:

```text
C:\Users\Ruhan Louw\...
...\Furnace Controller\...
```

Then create the project there with PlatformIO or move the project and rebuild.

## Clean Recovery Options

Option 1: continue with Arduino + FreeRTOS.

- Keep `Dev\ProdV1\Furnace Controller`.
- Use `framework = arduino`.
- Implement FreeRTOS task structure in `src\main.cpp`.

Option 2: use ESP-IDF in a clean no-space path.

- Create a new folder such as `C:\dev\furnace-controller`.
- Create the PlatformIO ESP-IDF project there.
- Avoid putting it under the current user profile path.

Option 3: keep the repo here but place only the ESP-IDF firmware elsewhere.

- Main docs/BOM stay in this repo.
- ESP-IDF firmware lives at `C:\dev\furnace-controller`.
- Later, use Git or a copy step to bring it back if needed.

## Current Advice

Use Arduino + FreeRTOS for the first furnace controller prototype.

Move to ESP-IDF only if the project later needs deeper ESP-IDF features such as advanced watchdog configuration, lower-level networking, custom partition/security handling, or a stricter production firmware structure.
