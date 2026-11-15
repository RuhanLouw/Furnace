#pragma once

#include <Arduino.h>

namespace TestConfig {

constexpr uint8_t SsrDrivePin = 4;
constexpr uint8_t EncoderAPin = 5;
constexpr uint8_t EncoderBPin = 6;
constexpr uint8_t EncoderButtonPin = 7;
constexpr uint8_t ThermocoupleClockPin = 12;
constexpr uint8_t ThermocoupleDataPin = 13;
constexpr uint8_t ThermocoupleUnusedMosiPin = 14;
constexpr uint8_t Thermocouple1SelectPin = 10;
constexpr uint8_t Thermocouple2SelectPin = 11;
constexpr uint32_t ThermocouplePeriodMs = 250;
constexpr uint32_t ThermocoupleStaleMs = 1500;
static_assert(ThermocoupleUnusedMosiPin != ThermocoupleClockPin &&
              ThermocoupleUnusedMosiPin != ThermocoupleDataPin &&
              ThermocoupleUnusedMosiPin != Thermocouple1SelectPin &&
              ThermocoupleUnusedMosiPin != Thermocouple2SelectPin,
              "Unused SPI MOSI must not overlap sensor wiring");
constexpr uint8_t DutyStepPercent = 5;
constexpr uint8_t EncoderEdgesPerDetent = 4;
constexpr uint32_t ButtonDebounceMs = 30;
constexpr uint32_t SsrWindowMs = 1000;
constexpr uint8_t StatusLedPin = 48;
constexpr uint8_t StatusLedBrightness = 16;
constexpr uint32_t HeartbeatPeriodMs = 2000;
constexpr uint32_t HeartbeatOnMs = 100;
constexpr uint32_t LedNoticeMs = 2000;
constexpr uint32_t SerialBaud = 115200;
constexpr size_t CommandBufferSize = 32;

}
