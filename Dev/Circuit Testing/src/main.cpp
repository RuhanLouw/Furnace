#include <Arduino.h>
#include <cctype>
#include <cstring>

#include "TestConfig.h"
#include "TestController.h"
#include "ThermocoupleInput.h"
#include "WebDashboard.h"

namespace {

char commandBuffer[TestConfig::CommandBufferSize]{};
size_t commandLength = 0;
bool discardLine = false;
bool ready = false;

void printHelp() {
    Serial.println("Furnace SSR test: GPIO4, rotary GPIO5/6, button GPIO7");
    Serial.println("Commands + Enter: on (or 1), off (or 0), status, temps, wifi, help");
    Serial.println("MAX31855: shared CLK=GPIO12, DO=GPIO13; TC1 CS=GPIO10, TC2 CS=GPIO11.");
    Serial.println("Serial telemetry is request-only: status + Enter.");
    Serial.println("Rotate or use web slider: duty 0-100%. Push or web buttons: enable/disable.");
    Serial.println("1-second switching window. Starts disabled at 0%. No timeout.");
    Serial.println("RGB: blue=disabled heartbeat, green=SSR ON, cyan=enabled OFF phase, red=bad command.");
}

void printStatus() {
    const TestController::Snapshot state = TestController::snapshot();
    Serial.printf(">ssr_command:%d\n>output_enabled:%d\n>duty_percent:%u\n>button_pressed:%d\n>button_press_count:%lu\n",
                  state.ssrOn ? 1 : 0, state.enabled ? 1 : 0,
                  static_cast<unsigned int>(state.dutyPercent), state.buttonPressed ? 1 : 0,
                  static_cast<unsigned long>(state.buttonPressCount));
    ThermocoupleInput::printStatus();
}

void handleCommand(const char *command) {
    auto type = TestController::CommandType::ErrorStop;
    if (strcmp(command, "off") == 0 || strcmp(command, "0") == 0) {
        type = TestController::CommandType::Disable;
    } else if (strcmp(command, "on") == 0 || strcmp(command, "1") == 0) {
        type = TestController::CommandType::Enable;
    } else if (strcmp(command, "status") == 0) {
        printStatus();
        return;
    } else if (strcmp(command, "temps") == 0) {
        ThermocoupleInput::printStatus();
        return;
    } else if (strcmp(command, "help") == 0) {
        printHelp();
        return;
    } else if (strcmp(command, "wifi") == 0) {
        WebDashboard::printInfo();
        return;
    } else {
        Serial.println("Unknown command: requesting output OFF. Type help + Enter.");
    }
    Serial.println(TestController::submit(type) ? "Command queued" : "Command rejected: queue full");
}

void receiveCommands() {
    for (size_t processed = 0; processed < TestConfig::CommandBufferSize && Serial.available() > 0; ++processed) {
        const char incoming = static_cast<char>(Serial.read());
        if (incoming == '\r' || incoming == '\n') {
            if (!discardLine && commandLength > 0) {
                commandBuffer[commandLength] = '\0';
                char *command = commandBuffer;
                while (*command != '\0' && std::isspace(static_cast<unsigned char>(*command))) {
                    ++command;
                }
                char *end = command + strlen(command);
                while (end > command && std::isspace(static_cast<unsigned char>(end[-1]))) {
                    *--end = '\0';
                }
                for (char *character = command; *character != '\0'; ++character) {
                    *character = static_cast<char>(std::tolower(static_cast<unsigned char>(*character)));
                }
                if (*command != '\0') {
                    handleCommand(command);
                }
            }
            commandLength = 0;
            discardLine = false;
        } else if ((incoming == '\b' || incoming == 127) && !discardLine) {
            if (commandLength > 0) {
                --commandLength;
            }
        } else if (!discardLine) {
            if (commandLength < sizeof(commandBuffer) - 1) {
                commandBuffer[commandLength++] = incoming;
            } else {
                TestController::submit(TestController::CommandType::ErrorStop);
                discardLine = true;
                Serial.println("Command too long: stop requested. Send Enter to clear.");
            }
        }
    }
}

}

void setup() {
    Serial.begin(TestConfig::SerialBaud);
    ready = TestController::begin();
    if (!ready) {
        Serial.println("Controller initialization failed; output remains OFF");
        return;
    }
    printHelp();
    if (!ThermocoupleInput::begin()) {
        Serial.println("Thermocouple task failed; dashboard will show unavailable. Manual SSR control is unchanged.");
    }
    WebDashboard::begin();
}

void loop() {
    if (ready) {
        receiveCommands();
        WebDashboard::service();
    }
    vTaskDelay(pdMS_TO_TICKS(1));
}
