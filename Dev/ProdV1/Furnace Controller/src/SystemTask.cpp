#include "Tasks.h"

#include <Arduino.h>

#include "AppConfig.h"
#include "HardwareOutput.h"
#include "StateStore.h"

void systemTask(void *parameter) {
    (void)parameter;
    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        const ControlState control = readControlState();
        setFaultLed(control.faultLatched);
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(AppConfig::SystemPeriodMs));
    }
}
