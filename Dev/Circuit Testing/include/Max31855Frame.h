#pragma once

#include <stdint.h>

namespace Max31855 {

enum class Status : uint8_t { Waiting, Ok, ThermocoupleFault, InvalidFrame, Stale, Unavailable };

struct Reading {
    uint32_t raw = 0;
    float celsius = 0;
    float coldJunctionCelsius = 0;
    uint8_t faults = 0;
    Status status = Status::Waiting;
};

inline Reading decode(uint32_t raw) {
    Reading reading{};
    reading.raw = raw;
    reading.faults = static_cast<uint8_t>(raw & 7);
    const bool faultFlag = (raw & 0x00010000UL) != 0;
    if ((raw & 0x00020008UL) != 0 || faultFlag != (reading.faults != 0)) {
        reading.status = Status::InvalidFrame;
        return reading;
    }
    int32_t thermocouple = static_cast<int32_t>((raw >> 18) & 0x3FFF);
    int32_t coldJunction = static_cast<int32_t>((raw >> 4) & 0x0FFF);
    if ((thermocouple & 0x2000) != 0) {
        thermocouple -= 0x4000;
    }
    if ((coldJunction & 0x0800) != 0) {
        coldJunction -= 0x1000;
    }
    reading.celsius = thermocouple * 0.25f;
    reading.coldJunctionCelsius = coldJunction * 0.0625f;
    reading.status = faultFlag ? Status::ThermocoupleFault : Status::Ok;
    return reading;
}

inline const char *statusText(Status status) {
    switch (status) {
        case Status::Waiting: return "Waiting for first reading";
        case Status::Ok: return "OK";
        case Status::ThermocoupleFault: return "Thermocouple fault";
        case Status::InvalidFrame: return "Invalid SPI frame / check wiring";
        case Status::Stale: return "Stale reading";
        case Status::Unavailable: return "Sensor task unavailable";
    }
    return "Unknown sensor status";
}

}
