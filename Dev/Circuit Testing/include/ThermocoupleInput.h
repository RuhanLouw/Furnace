#pragma once

#include "Max31855Frame.h"

namespace ThermocoupleInput {

constexpr uint8_t ChannelCount = 2;

struct Snapshot {
    Max31855::Reading channels[ChannelCount];
    uint32_t sampledAtMs = 0;
    bool sampled = false;
};

bool begin();
Snapshot snapshot();
void printStatus();

}
