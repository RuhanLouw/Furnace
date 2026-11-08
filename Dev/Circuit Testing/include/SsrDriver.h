#pragma once

#include <Arduino.h>

namespace SsrDriver {

void begin();
void turnOn(uint32_t nowMs);
void turnOff();
void setDutyPercent(uint8_t percent);
uint8_t dutyPercent();
bool isEnabled();
void service(uint32_t nowMs);
bool isCommandedOn();

}
