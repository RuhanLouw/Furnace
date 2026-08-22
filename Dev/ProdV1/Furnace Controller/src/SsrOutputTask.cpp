#include "Tasks.h"

#include <Arduino.h>

#include "AppConfig.h"
#include "HardwareOutput.h"
#include "StateStore.h"

void ssrTimingTask(void *parameter) {
    (void)parameter;
    TickType_t lastWake = xTaskGetTickCount();
    uint32_t windowStartMs = millis();

    while (true) {
        const uint32_t nowMs = millis();
        if (nowMs - windowStartMs >= AppConfig::SsrWindowMs) {
            windowStartMs = nowMs;
        }

        const SafetyState safety = readSafetyState();
        const ControlState control = readControlState();
        const uint32_t onTimeMs = static_cast<uint32_t>(
            AppConfig::SsrWindowMs * constrain(control.requestedOutputPercent, 0.0f, 100.0f) / 100.0f
        );
        const bool inOnWindow = (nowMs - windowStartMs) < onTimeMs;
        const bool outputEnabled = safety.heaterAllowed &&
                                   control.controllerState == ControllerState::Running &&
                                   inOnWindow;

        setHeaterOutput(outputEnabled);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(AppConfig::SsrPeriodMs));
    }
}
