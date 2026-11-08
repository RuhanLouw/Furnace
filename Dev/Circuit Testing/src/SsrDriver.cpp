#include "SsrDriver.h"

#include "TestConfig.h"

namespace {

bool commandedOn = false;
bool enabled = false;
uint8_t requestedDutyPercent = 0;
uint32_t windowStartedMs = 0;

void writeOutput(bool nextOn) {
    digitalWrite(TestConfig::SsrDrivePin, nextOn ? HIGH : LOW);
    commandedOn = nextOn;
}

}

namespace SsrDriver {

void begin() {
    static_assert(TestConfig::SsrWindowMs > 0, "SSR window must be nonzero");
    digitalWrite(TestConfig::SsrDrivePin, LOW);
    pinMode(TestConfig::SsrDrivePin, OUTPUT);
    turnOff();
    requestedDutyPercent = 0;
}

void turnOn(uint32_t nowMs) {
    if (!enabled) {
        windowStartedMs = nowMs;
        enabled = true;
    }
    service(nowMs);
}

void turnOff() {
    enabled = false;
    writeOutput(false);
}

void setDutyPercent(uint8_t percent) {
    requestedDutyPercent = percent > 100 ? 100 : percent;
    if (requestedDutyPercent == 0) {
        writeOutput(false);
    }
}

uint8_t dutyPercent() {
    return requestedDutyPercent;
}

bool isEnabled() {
    return enabled;
}

void service(uint32_t nowMs) {
    if (!enabled) {
        writeOutput(false);
        return;
    }

    const uint32_t elapsedMs = nowMs - windowStartedMs;
    const uint32_t phaseMs = elapsedMs % TestConfig::SsrWindowMs;
    windowStartedMs = nowMs - phaseMs;
    const uint32_t onTimeMs = TestConfig::SsrWindowMs * requestedDutyPercent / 100;
    writeOutput(phaseMs < onTimeMs);
}

bool isCommandedOn() {
    return commandedOn;
}

}
