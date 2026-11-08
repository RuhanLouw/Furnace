#include "RotaryInput.h"

#include "TestConfig.h"

namespace {

uint8_t previousState = 0;
int8_t accumulatedEdges = 0;
bool rawPressed = false;
bool stablePressed = false;
uint32_t lastButtonChangeMs = 0;
uint32_t presses = 0;

uint8_t readState() {
    return static_cast<uint8_t>((digitalRead(TestConfig::EncoderAPin) << 1) |
                                digitalRead(TestConfig::EncoderBPin));
}

int8_t decodeTransition(uint8_t previous, uint8_t current) {
    switch ((previous << 2) | current) {
        case 0b0001:
        case 0b0111:
        case 0b1110:
        case 0b1000:
            return 1;
        case 0b0010:
        case 0b1011:
        case 0b1101:
        case 0b0100:
            return -1;
        default:
            return 0;
    }
}

}

namespace RotaryInput {

void begin() {
    static_assert(TestConfig::EncoderEdgesPerDetent == 2 || TestConfig::EncoderEdgesPerDetent == 4,
                  "Use two or four quadrature edges per detent");
    pinMode(TestConfig::EncoderAPin, INPUT_PULLUP);
    pinMode(TestConfig::EncoderBPin, INPUT_PULLUP);
    pinMode(TestConfig::EncoderButtonPin, INPUT_PULLUP);
    previousState = readState();
    accumulatedEdges = 0;
    rawPressed = digitalRead(TestConfig::EncoderButtonPin) == LOW;
    stablePressed = rawPressed;
    lastButtonChangeMs = millis();
    presses = 0;
}

Events service(uint32_t nowMs) {
    Events events{};
    const uint8_t current = readState();
    if ((previousState ^ current) == 0b11) {
        accumulatedEdges = 0;
    } else {
        accumulatedEdges += decodeTransition(previousState, current);
        const int8_t edgesPerDetent = TestConfig::EncoderEdgesPerDetent;
        if (accumulatedEdges >= edgesPerDetent) {
            events.detents = 1;
            accumulatedEdges -= edgesPerDetent;
        } else if (accumulatedEdges <= -edgesPerDetent) {
            events.detents = -1;
            accumulatedEdges += edgesPerDetent;
        }
    }
    previousState = current;

    const bool pressedNow = digitalRead(TestConfig::EncoderButtonPin) == LOW;
    if (pressedNow != rawPressed) {
        rawPressed = pressedNow;
        lastButtonChangeMs = nowMs;
    }
    if (nowMs - lastButtonChangeMs >= TestConfig::ButtonDebounceMs && stablePressed != rawPressed) {
        stablePressed = rawPressed;
        if (stablePressed) {
            ++presses;
            events.pressed = true;
        }
    }
    return events;
}

bool isButtonPressed() {
    return stablePressed;
}

uint32_t pressCount() {
    return presses;
}

}
