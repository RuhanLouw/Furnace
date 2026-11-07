#pragma once

#include <Arduino.h>

namespace TestConfig {

constexpr uint8_t SsrDrivePin = 4;
constexpr uint8_t StatusLedPin = 48;
constexpr uint8_t StatusLedBrightness = 16;
constexpr uint32_t HeartbeatPeriodMs = 2000;
constexpr uint32_t HeartbeatOnMs = 100;
constexpr uint32_t LedNoticeMs = 2000;
constexpr uint32_t SerialBaud = 115200;
constexpr uint32_t MaximumOnMs = 5000;
constexpr uint32_t StatusPeriodMs = 1000;
constexpr size_t CommandBufferSize = 32;

}
