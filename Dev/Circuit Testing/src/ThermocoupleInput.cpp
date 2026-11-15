#include "ThermocoupleInput.h"

#include <Arduino.h>
#include <SPI.h>

#include "TestConfig.h"

namespace {

SPIClass sensorBus(FSPI);
constexpr uint8_t SelectPins[] = {TestConfig::Thermocouple1SelectPin, TestConfig::Thermocouple2SelectPin};
SemaphoreHandle_t stateMutex = nullptr;
ThermocoupleInput::Snapshot state{};
bool started = false;

uint32_t readFrame(uint8_t selectPin) {
    sensorBus.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(selectPin, LOW);
    delayMicroseconds(1);
    uint32_t raw = 0;
    for (uint8_t byteIndex = 0; byteIndex < 4; ++byteIndex) {
        raw = (raw << 8) | sensorBus.transfer(0);
    }
    digitalWrite(selectPin, HIGH);
    sensorBus.endTransaction();
    return raw;
}

void sensorTask(void *parameter) {
    (void)parameter;
    vTaskDelay(pdMS_TO_TICKS(TestConfig::ThermocouplePeriodMs));
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        ThermocoupleInput::Snapshot next{};
        for (uint8_t channel = 0; channel < ThermocoupleInput::ChannelCount; ++channel) {
            next.channels[channel] = Max31855::decode(readFrame(SelectPins[channel]));
        }
        next.sampledAtMs = millis();
        next.sampled = true;
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        state = next;
        xSemaphoreGive(stateMutex);
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TestConfig::ThermocouplePeriodMs));
    }
}

}

namespace ThermocoupleInput {

bool begin() {
    for (uint8_t selectPin : SelectPins) {
        digitalWrite(selectPin, HIGH);
        pinMode(selectPin, OUTPUT);
    }
    sensorBus.begin(TestConfig::ThermocoupleClockPin, TestConfig::ThermocoupleDataPin,
                    TestConfig::ThermocoupleUnusedMosiPin, -1);
    if (sensorBus.bus() == nullptr) {
        return false;
    }
    pinMode(TestConfig::ThermocoupleDataPin, INPUT_PULLUP);
    stateMutex = xSemaphoreCreateMutex();
    if (stateMutex == nullptr) {
        return false;
    }
    started = xTaskCreatePinnedToCore(sensorTask, "Thermocouples", 3072, nullptr, 1, nullptr, 0) == pdPASS;
    return started;
}

Snapshot snapshot() {
    Snapshot copy{};
    if (!started) {
        for (auto &channel : copy.channels) {
            channel.status = Max31855::Status::Unavailable;
        }
        return copy;
    }
    xSemaphoreTake(stateMutex, portMAX_DELAY);
    copy = state;
    xSemaphoreGive(stateMutex);
    if (copy.sampled && millis() - copy.sampledAtMs > TestConfig::ThermocoupleStaleMs) {
        for (auto &channel : copy.channels) {
            channel.status = Max31855::Status::Stale;
        }
    }
    return copy;
}

void printStatus() {
    const Snapshot current = snapshot();
    for (uint8_t channel = 0; channel < ChannelCount; ++channel) {
        const auto &reading = current.channels[channel];
        const unsigned int number = channel + 1;
        const bool valid = reading.status == Max31855::Status::Ok;
        Serial.printf(">tc%u_valid:%d\n>tc%u_faults:%u\n", number, valid ? 1 : 0,
                      number, static_cast<unsigned int>(reading.faults));
        if (valid) {
            Serial.printf(">tc%u_c:%.2f\n", number, reading.celsius);
        }
        if (valid || reading.status == Max31855::Status::ThermocoupleFault) {
            Serial.printf(">tc%u_cold_junction_c:%.4f\n", number, reading.coldJunctionCelsius);
        }
        Serial.printf("TC%u: %s, raw=0x%08lX", number, Max31855::statusText(reading.status),
                      static_cast<unsigned long>(reading.raw));
        if (current.sampled) {
            Serial.printf(", age=%lu ms", static_cast<unsigned long>(millis() - current.sampledAtMs));
        }
        Serial.println();
        if (reading.status == Max31855::Status::ThermocoupleFault) {
            if ((reading.faults & 1) != 0) Serial.println("  Open thermocouple");
            if ((reading.faults & 2) != 0) Serial.println("  Thermocouple short to GND");
            if ((reading.faults & 4) != 0) Serial.println("  Thermocouple short to VCC");
        }
    }
}

}
