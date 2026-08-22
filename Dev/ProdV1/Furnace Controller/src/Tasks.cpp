#include "Tasks.h"

#include <Arduino.h>

void createTasks() {
    xTaskCreatePinnedToCore(safetyTask, "Safety", 4096, nullptr, 5, nullptr, 1);
    xTaskCreatePinnedToCore(ssrTimingTask, "SSR", 4096, nullptr, 4, nullptr, 1);
    xTaskCreatePinnedToCore(controlTask, "Control", 4096, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(sensorTask, "Sensors", 4096, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(commandTask, "Commands", 4096, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(localHmiTask, "LocalHMI", 4096, nullptr, 1, nullptr, 1);
    xTaskCreatePinnedToCore(pcCommsTask, "PCComms", 4096, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(telemetryTask, "Telemetry", 4096, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(systemTask, "System", 2048, nullptr, 1, nullptr, 0);
}
