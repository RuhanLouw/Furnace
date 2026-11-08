#pragma once

#include <Arduino.h>

namespace RotaryInput {

struct Events {
    int8_t detents;
    bool pressed;
};

void begin();
Events service(uint32_t nowMs);
bool isButtonPressed();
uint32_t pressCount();

}
