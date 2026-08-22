#include "HardwareOutput.h"

#include <Arduino.h>

#include "AppConfig.h"

void configureOutputPins() {
    pinMode(AppConfig::SsrOutputPin, OUTPUT);
    pinMode(AppConfig::RunLedPin, OUTPUT);
    pinMode(AppConfig::FaultLedPin, OUTPUT);
    setHeaterOutput(false);
    setFaultLed(false);
}

void setHeaterOutput(bool enabled) {
    digitalWrite(AppConfig::SsrOutputPin, enabled ? HIGH : LOW);
    digitalWrite(AppConfig::RunLedPin, enabled ? HIGH : LOW);
}

void setFaultLed(bool enabled) {
    digitalWrite(AppConfig::FaultLedPin, enabled ? HIGH : LOW);
}
