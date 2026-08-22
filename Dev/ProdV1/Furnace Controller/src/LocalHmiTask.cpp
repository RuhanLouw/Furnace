#include "Tasks.h"

#include <Arduino.h>

#include "AppConfig.h"

void localHmiTask(void *parameter) {
    (void)parameter;
    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        // Placeholder for rotary encoder, buttons, and TFT page handling.
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(AppConfig::HmiPeriodMs));
    }
}
