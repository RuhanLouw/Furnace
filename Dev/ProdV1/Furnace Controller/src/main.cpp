#include <Arduino.h>

#include "AppConfig.h"
#include "HardwareOutput.h"
#include "StateStore.h"
#include "Tasks.h"

namespace {

void configureSafetyInputPins() {
    pinMode(AppConfig::EstopFeedbackPin, INPUT_PULLUP);
    pinMode(AppConfig::DoorSwitchPin, INPUT_PULLUP);
    pinMode(AppConfig::OverTempFeedbackPin, INPUT_PULLUP);
}

void printBootBanner() {
    Serial.println("Furnace controller shell booting");
    Serial.println("Serial commands: s=start, x=stop, a=auto, m=manual, f=ack fault");
}

}

void setup() {
    configureOutputPins();
    configureSafetyInputPins();

    Serial.begin(AppConfig::SerialBaud);
    delay(200);
    printBootBanner();

    initializeStateStore();
    createTasks();
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
