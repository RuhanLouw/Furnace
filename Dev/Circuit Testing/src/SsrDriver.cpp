#include "SsrDriver.h"

#include "TestConfig.h"

namespace {

bool commandedOn = false;
uint32_t onStartedMs = 0;

}

namespace SsrDriver {

void begin() {
    digitalWrite(TestConfig::SsrDrivePin, LOW);
    pinMode(TestConfig::SsrDrivePin, OUTPUT);
    turnOff();
}

void turnOn(uint32_t nowMs) {
    if (!commandedOn) {
        onStartedMs = nowMs;
        commandedOn = true;
        digitalWrite(TestConfig::SsrDrivePin, HIGH);
    }
}

void turnOff() {
    digitalWrite(TestConfig::SsrDrivePin, LOW);
    commandedOn = false;
}

bool service(uint32_t nowMs) {
    if (commandedOn && nowMs - onStartedMs >= TestConfig::MaximumOnMs) {
        turnOff();
        return true;
    }

    return false;
}

bool isCommandedOn() {
    return commandedOn;
}

uint32_t remainingOnMs(uint32_t nowMs) {
    if (!commandedOn) {
        return 0;
    }

    const uint32_t elapsedMs = nowMs - onStartedMs;
    return elapsedMs >= TestConfig::MaximumOnMs ? 0 : TestConfig::MaximumOnMs - elapsedMs;
}

}
