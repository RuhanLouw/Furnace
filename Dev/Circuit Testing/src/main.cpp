#include <Arduino.h>
#include <cstring>

#include "SsrDriver.h"
#include "StatusLed.h"
#include "TestConfig.h"

namespace {

char commandBuffer[TestConfig::CommandBufferSize]{};
size_t commandLength = 0;
bool discardLine = false;
uint32_t lastStatusMs = 0;

void printHelp() {
    Serial.println("Furnace SSR/NPN bring-up: ESP32-S3 N16R8, GPIO4, active HIGH");
    Serial.println("Commands + Enter: on (or 1), off (or 0), status, help");
    Serial.println("ON times out after 5 seconds. Repeated ON does not extend the timer.");
    Serial.println("Status reports the GPIO command, not measured heater current.");
    Serial.println("RGB LED: blue pulse=idle, green=SSR command ON, amber=timeout, red=bad command");
}

void printStatus() {
    Serial.printf(">ssr_command:%d\n", SsrDriver::isCommandedOn() ? 1 : 0);
    Serial.printf(">remaining_on_ms:%lu\n",
                  static_cast<unsigned long>(SsrDriver::remainingOnMs(millis())));
}

void handleCommand(const char *command) {
    if (strcmp(command, "off") == 0 || strcmp(command, "0") == 0) {
        SsrDriver::turnOff();
        StatusLed::clearNotice();
        Serial.println("SSR command OFF");
    } else if (strcmp(command, "on") == 0 || strcmp(command, "1") == 0) {
        SsrDriver::turnOn(millis());
        StatusLed::clearNotice();
        Serial.println("SSR command ON (maximum 5 seconds per activation)");
    } else if (strcmp(command, "status") == 0) {
        printStatus();
    } else if (strcmp(command, "help") == 0) {
        printHelp();
    } else {
        SsrDriver::turnOff();
        StatusLed::showCommandError(millis());
        Serial.println("Unknown command: output OFF. Type help + Enter.");
    }
}

void receiveCommands() {
    for (size_t processed = 0; processed < TestConfig::CommandBufferSize && Serial.available() > 0; ++processed) {
        const char incoming = static_cast<char>(Serial.read());

        if (incoming == '\r' || incoming == '\n') {
            if (!discardLine && commandLength > 0) {
                commandBuffer[commandLength] = '\0';
                handleCommand(commandBuffer);
            }
            commandLength = 0;
            discardLine = false;
        } else if (!discardLine) {
            if (commandLength < sizeof(commandBuffer) - 1) {
                commandBuffer[commandLength++] = incoming;
            } else {
                SsrDriver::turnOff();
                StatusLed::showCommandError(millis());
                discardLine = true;
                Serial.println("Command too long: output OFF. Send Enter to clear.");
            }
        }
    }
}

}

void setup() {
    SsrDriver::begin();
    StatusLed::begin();
    Serial.begin(TestConfig::SerialBaud);
    printHelp();
    Serial.printf("Flash: %lu bytes, PSRAM: %lu bytes\n",
                  static_cast<unsigned long>(ESP.getFlashChipSize()),
                  static_cast<unsigned long>(ESP.getPsramSize()));
}

void loop() {
    if (SsrDriver::service(millis())) {
        StatusLed::showTimeout(millis());
        Serial.println("5-second timeout: SSR command OFF");
    }

    receiveCommands();

    const uint32_t nowMs = millis();
    StatusLed::service(nowMs, SsrDriver::isCommandedOn());
    if (nowMs - lastStatusMs >= TestConfig::StatusPeriodMs) {
        lastStatusMs = nowMs;
        printStatus();
    }

    vTaskDelay(pdMS_TO_TICKS(1));
}
