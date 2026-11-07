#include "StatusLed.h"

#include "esp32-hal-rgb-led.h"
#include "TestConfig.h"

namespace {

enum class Notice { None, Timeout, CommandError };

Notice notice = Notice::None;
uint32_t noticeStartedMs = 0;
uint32_t previousColor = 0xFFFFFFFF;

void setColor(uint8_t red, uint8_t green, uint8_t blue) {
    const uint32_t color = (static_cast<uint32_t>(red) << 16) |
                           (static_cast<uint32_t>(green) << 8) | blue;
    if (color != previousColor) {
        neopixelWrite(TestConfig::StatusLedPin, red, green, blue);
        previousColor = color;
    }
}

}

namespace StatusLed {

void begin() {
    notice = Notice::None;
    setColor(0, 0, 0);
}

void showTimeout(uint32_t nowMs) {
    notice = Notice::Timeout;
    noticeStartedMs = nowMs;
}

void showCommandError(uint32_t nowMs) {
    notice = Notice::CommandError;
    noticeStartedMs = nowMs;
}

void clearNotice() {
    notice = Notice::None;
}

void service(uint32_t nowMs, bool ssrCommandedOn) {
    if (notice != Notice::None && nowMs - noticeStartedMs >= TestConfig::LedNoticeMs) {
        clearNotice();
    }

    const uint8_t brightness = TestConfig::StatusLedBrightness;
    if (ssrCommandedOn) {
        setColor(0, brightness, 0);
    } else if (notice == Notice::CommandError) {
        setColor(brightness, 0, 0);
    } else if (notice == Notice::Timeout) {
        setColor(brightness, brightness / 2, 0);
    } else {
        const bool heartbeatOn = nowMs % TestConfig::HeartbeatPeriodMs < TestConfig::HeartbeatOnMs;
        setColor(0, 0, heartbeatOn ? brightness : 0);
    }
}

}
