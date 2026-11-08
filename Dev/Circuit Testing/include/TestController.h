#pragma once

#include <Arduino.h>

namespace TestController {

enum class CommandType : uint8_t { Enable, Disable, SetDuty, ErrorStop };

struct Snapshot {
    bool enabled;
    bool ssrOn;
    uint8_t dutyPercent;
    bool buttonPressed;
    uint32_t buttonPressCount;
    uint32_t lastCommandId;
};

bool begin();
bool submit(CommandType type, uint8_t value = 0, uint32_t commandId = 0);
Snapshot snapshot();

}
