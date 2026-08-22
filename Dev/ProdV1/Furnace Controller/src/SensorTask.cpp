#include "Tasks.h"

#include <Arduino.h>

#include "AppConfig.h"
#include "StateStore.h"

void sensorTask(void *parameter) {
    (void)parameter;
    TickType_t lastWake = xTaskGetTickCount();
    float simulatedTemperatureC = 25.0f;

    while (true) {
        const ControlState control = readControlState();

        if (control.controllerState == ControllerState::Running) {
            simulatedTemperatureC += 0.4f;
        } else if (simulatedTemperatureC > 25.0f) {
            simulatedTemperatureC -= 0.1f;
        }

        SensorState next{};
        next.tc1C = simulatedTemperatureC;
        next.tc2C = simulatedTemperatureC + 0.8f;
        next.tc1Fault = false;
        next.tc2Fault = false;
        publishSensorState(next);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(AppConfig::SensorPeriodMs));
    }
}
