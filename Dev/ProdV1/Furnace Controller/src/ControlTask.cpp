#include "Tasks.h"

#include <Arduino.h>

#include "AppConfig.h"
#include "StateStore.h"

void controlTask(void *parameter) {
    (void)parameter;
    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        ControlState next = readControlState();
        const SafetyState safety = readSafetyState();
        const SensorState sensors = readSensorState();

        next.processValueC = sensors.tc1C;

        if (!safety.heaterAllowed || sensors.tc1Fault || sensors.tc2Fault) {
            next.requestedOutputPercent = 0.0f;
            next.faultLatched = true;
            next.controllerState = safety.estopActive ? ControllerState::Estop : ControllerState::Fault;
        } else if (next.controllerState == ControllerState::Running) {
            next.runtimeSeconds += AppConfig::ControlPeriodMs / 1000;

            if (next.mode == ControlMode::Auto) {
                const float errorC = next.setpointC - next.processValueC;
                next.requestedOutputPercent = constrain(errorC * 2.0f, 0.0f, 100.0f);
            }
        } else {
            next.requestedOutputPercent = 0.0f;
        }

        publishControlState(next);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(AppConfig::ControlPeriodMs));
    }
}
