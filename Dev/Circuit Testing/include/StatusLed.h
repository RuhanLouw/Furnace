#pragma once

#include <Arduino.h>

namespace StatusLed {

void begin();
void showTimeout(uint32_t nowMs);
void showCommandError(uint32_t nowMs);
void clearNotice();
void service(uint32_t nowMs, bool ssrCommandedOn);

}
