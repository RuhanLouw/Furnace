#include "Tasks.h"

#include <Arduino.h>

#include "AppTypes.h"
#include "StateStore.h"

void pcCommsTask(void *parameter) {
    (void)parameter;
    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        if (Serial.available() > 0) {
            const char input = static_cast<char>(Serial.read());
            Command command{};
            bool commandReady = true;

            switch (input) {
                case 's':
                    command.type = CommandType::StartRun;
                    break;
                case 'x':
                    command.type = CommandType::StopRun;
                    break;
                case 'a':
                    command.type = CommandType::SetAuto;
                    break;
                case 'm':
                    command.type = CommandType::SetManual;
                    break;
                case 'f':
                    command.type = CommandType::AcknowledgeFault;
                    break;
                default:
                    commandReady = false;
                    break;
            }

            if (commandReady) {
                sendCommand(command);
            }
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(20));
    }
}
