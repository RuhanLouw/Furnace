#pragma once

#include <Arduino.h>

namespace SsrDriver {

void begin();
void turnOn(uint32_t nowMs);
void turnOff();
bool service(uint32_t nowMs);
bool isCommandedOn();
uint32_t remainingOnMs(uint32_t nowMs);

}
