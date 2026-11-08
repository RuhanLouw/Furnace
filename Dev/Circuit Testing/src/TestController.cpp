#include "TestController.h"

#include "RotaryInput.h"
#include "SsrDriver.h"
#include "StatusLed.h"
#include "TestConfig.h"

namespace {

struct Command {
    TestController::CommandType type;
    uint8_t value;
    uint32_t id;
};

constexpr size_t QueueLength = 8;
QueueHandle_t commands = nullptr;
SemaphoreHandle_t stateMutex = nullptr;
TestController::Snapshot state{};

void controlTask(void *parameter) {
    (void)parameter;
    TickType_t lastWake = xTaskGetTickCount();
    uint32_t lastCommandId = 0;
    while (true) {
        const uint32_t nowMs = millis();
        const RotaryInput::Events events = RotaryInput::service(nowMs);
        if (events.detents != 0) {
            const int nextDuty = static_cast<int>(SsrDriver::dutyPercent()) +
                                 events.detents * TestConfig::DutyStepPercent;
            SsrDriver::setDutyPercent(static_cast<uint8_t>(constrain(nextDuty, 0, 100)));
        }
        if (events.pressed) {
            if (SsrDriver::isEnabled()) {
                SsrDriver::turnOff();
            } else {
                SsrDriver::turnOn(nowMs);
            }
            StatusLed::clearNotice();
        }
        Command command{};
        for (size_t processed = 0; processed < QueueLength && xQueueReceive(commands, &command, 0) == pdTRUE; ++processed) {
            switch (command.type) {
                case TestController::CommandType::Enable:
                    SsrDriver::turnOn(nowMs);
                    StatusLed::clearNotice();
                    break;
                case TestController::CommandType::Disable:
                    SsrDriver::turnOff();
                    StatusLed::clearNotice();
                    break;
                case TestController::CommandType::SetDuty:
                    SsrDriver::setDutyPercent(command.value);
                    break;
                case TestController::CommandType::ErrorStop:
                    SsrDriver::turnOff();
                    StatusLed::showCommandError(nowMs);
                    break;
            }
            if (command.id != 0) {
                lastCommandId = command.id;
            }
        }
        SsrDriver::service(nowMs);
        StatusLed::service(nowMs, SsrDriver::isCommandedOn(), SsrDriver::isEnabled());
        const TestController::Snapshot next{
            SsrDriver::isEnabled(), SsrDriver::isCommandedOn(), SsrDriver::dutyPercent(),
            RotaryInput::isButtonPressed(), RotaryInput::pressCount(), lastCommandId
        };
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        state = next;
        xSemaphoreGive(stateMutex);
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1));
    }
}

}

namespace TestController {

bool begin() {
    SsrDriver::begin();
    RotaryInput::begin();
    StatusLed::begin();
    commands = xQueueCreate(QueueLength, sizeof(Command));
    stateMutex = xSemaphoreCreateMutex();
    if (commands == nullptr || stateMutex == nullptr) {
        return false;
    }
    return xTaskCreatePinnedToCore(controlTask, "SSRControl", 4096, nullptr, 3, nullptr, 1) == pdPASS;
}

bool submit(CommandType type, uint8_t value, uint32_t commandId) {
    if (commands == nullptr || (type == CommandType::SetDuty && value > 100)) {
        return false;
    }
    const Command command{type, value, commandId};
    return xQueueSend(commands, &command, 0) == pdTRUE;
}

Snapshot snapshot() {
    Snapshot copy{};
    if (stateMutex != nullptr) {
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        copy = state;
        xSemaphoreGive(stateMutex);
    }
    return copy;
}

}
