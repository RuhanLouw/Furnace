#include "Tasks.h"

#include <Arduino.h>

#include "AppConfig.h"
#include "CommandHandler.h"
#include "StateStore.h"

void telemetryTask(void *parameter) {
    (void)parameter;
    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        const SafetyState safety = readSafetyState();
        const SensorState sensors = readSensorState();
        const ControlState control = readControlState();

        Serial.print("state=");
        Serial.print(controllerStateName(control.controllerState));
        Serial.print(", mode=");
        Serial.print(control.mode == ControlMode::Auto ? "AUTO" : "MANUAL");
        Serial.print(", runtime=");
        Serial.print(control.runtimeSeconds);
        Serial.print(", sp=");
        Serial.print(control.setpointC, 1);
        Serial.print(", tc1=");
        Serial.print(sensors.tc1C, 1);
        Serial.print(", tc2=");
        Serial.print(sensors.tc2C, 1);
        Serial.print(", out=");
        Serial.print(control.requestedOutputPercent, 1);
        Serial.print(", allowed=");
        Serial.print(safety.heaterAllowed ? "true" : "false");
        Serial.print(", estop=");
        Serial.print(safety.estopActive ? "true" : "false");
        Serial.print(", door=");
        Serial.print(safety.doorOpen ? "open" : "closed");
        Serial.print(", overtemp=");
        Serial.println(safety.overTemp ? "true" : "false");

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(AppConfig::TelemetryPeriodMs));
    }
}
