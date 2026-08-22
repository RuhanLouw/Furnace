#pragma once

#include <Arduino.h>

namespace AppConfig {

constexpr uint32_t SerialBaud = 115200;

constexpr uint8_t SsrOutputPin = 26;
constexpr uint8_t RunLedPin = 2;
constexpr uint8_t FaultLedPin = 27;

constexpr uint8_t EstopFeedbackPin = 32;
constexpr uint8_t DoorSwitchPin = 33;
constexpr uint8_t OverTempFeedbackPin = 25;

constexpr uint32_t SafetyPeriodMs = 20;
constexpr uint32_t SensorPeriodMs = 500;
constexpr uint32_t ControlPeriodMs = 1000;
constexpr uint32_t SsrPeriodMs = 100;
constexpr uint32_t HmiPeriodMs = 20;
constexpr uint32_t TelemetryPeriodMs = 1000;
constexpr uint32_t SystemPeriodMs = 1000;

constexpr uint32_t SsrWindowMs = 10000;

}
