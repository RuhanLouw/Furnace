#include "Tasks.h"

#include <Arduino.h>

#include "AppConfig.h"
#include "HardwareOutput.h"
#include "StateStore.h"

void safetyTask(void *parameter) {
    (void)parameter;
    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        SafetyState next{};
        next.estopActive = digitalRead(AppConfig::EstopFeedbackPin) == LOW;
        next.doorOpen = digitalRead(AppConfig::DoorSwitchPin) == LOW;
        next.overTemp = digitalRead(AppConfig::OverTempFeedbackPin) == LOW;
        next.heaterAllowed = !next.estopActive && !next.doorOpen && !next.overTemp;

        publishSafetyState(next);

        if (!next.heaterAllowed) {
            setHeaterOutput(false);
            ControlState control = readControlState();
            control.requestedOutputPercent = 0.0f;
            control.runRequested = false;
            control.faultLatched = true;
            control.controllerState = next.estopActive ? ControllerState::Estop : ControllerState::Fault;
            publishControlState(control);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(AppConfig::SafetyPeriodMs));
    }
}
