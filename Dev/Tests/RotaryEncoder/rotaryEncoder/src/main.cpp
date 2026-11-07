#include <Arduino.h>

namespace {

constexpr uint32_t SerialBaud = 115200;

constexpr uint8_t EncoderAPin = 32;
constexpr uint8_t EncoderBPin = 33;
constexpr uint8_t EncoderButtonPin = 25;
constexpr uint8_t ActivityLedPin = 2;
constexpr uint8_t PwmLedPin = 26;

constexpr uint8_t PwmLedChannel = 0;
constexpr uint32_t PwmLedFrequencyHz = 5000;
constexpr uint8_t PwmLedResolutionBits = 8;
constexpr int32_t BrightnessStep = 5;

constexpr uint32_t EncoderPollPeriodMs = 1;
constexpr uint32_t ButtonDebounceMs = 30;
constexpr uint32_t TelemetryPeriodMs = 100;

struct TestState {
    int32_t encoderPosition;
    int32_t encoderStepCount;
    int32_t encoderDeltaSinceTelemetry;
    int32_t buttonPressCount;
    int32_t ledBrightnessPercent;
    bool buttonPressed;
    bool pwmLedEnabled;
};

TestState testState{};
SemaphoreHandle_t stateMutex = nullptr;
TaskHandle_t buttonTaskHandle = nullptr;
uint8_t previousEncoderState = 0;

uint8_t readEncoderState() {
    const uint8_t a = digitalRead(EncoderAPin) == HIGH ? 1 : 0;
    const uint8_t b = digitalRead(EncoderBPin) == HIGH ? 1 : 0;
    return static_cast<uint8_t>((a << 1) | b);
}

int8_t decodeTransition(uint8_t previousState, uint8_t currentState) {
    const uint8_t transition = static_cast<uint8_t>((previousState << 2) | currentState);

    switch (transition) {
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

void sendTeleplotValue(const char *name, int32_t value) {
    Serial.print('>');
    Serial.print(name);
    Serial.print(':');
    Serial.println(value);
}

void sendTeleplotValue(const char *name, bool value) {
    sendTeleplotValue(name, value ? 1 : 0);
}

void applyPwmLedOutput(bool enabled, int32_t brightnessPercent) {
    const int32_t clampedBrightness = constrain(brightnessPercent, 0, 100);
    const uint32_t duty = enabled
        ? static_cast<uint32_t>(map(clampedBrightness, 0, 100, 0, 255))
        : 0;

    ledcWrite(PwmLedChannel, duty);
}  

void toggleActivityLed() {
    digitalWrite(ActivityLedPin, !digitalRead(ActivityLedPin));
}

void IRAM_ATTR buttonInterrupt() {
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    if (buttonTaskHandle != nullptr) {
        vTaskNotifyGiveFromISR(buttonTaskHandle, &higherPriorityTaskWoken);

        if (higherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR();
        }
    }
}

void encoderTask(void *parameter) {
    (void)parameter;
    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        const uint8_t currentState = readEncoderState();
        const int8_t transitionDelta = decodeTransition(previousEncoderState, currentState);

        if (transitionDelta != 0) {
            bool acceptedDetent = false;
            bool ledEnabled = false;
            int32_t ledBrightnessPercent = 0;

            xSemaphoreTake(stateMutex, portMAX_DELAY);
            testState.encoderStepCount += transitionDelta;

            if (testState.encoderStepCount >= 4) {
                testState.encoderPosition++;
                testState.encoderDeltaSinceTelemetry++;
                testState.encoderStepCount = 0;
                testState.ledBrightnessPercent = constrain(testState.ledBrightnessPercent + BrightnessStep, 0, 100);
                acceptedDetent = true;
            } else if (testState.encoderStepCount <= -4) {
                testState.encoderPosition--;
                testState.encoderDeltaSinceTelemetry--;
                testState.encoderStepCount = 0;
                testState.ledBrightnessPercent = constrain(testState.ledBrightnessPercent - BrightnessStep, 0, 100);
                acceptedDetent = true;
            }

            ledEnabled = testState.pwmLedEnabled;
            ledBrightnessPercent = testState.ledBrightnessPercent;
            xSemaphoreGive(stateMutex);

            if (acceptedDetent) {
                applyPwmLedOutput(ledEnabled, ledBrightnessPercent);
                toggleActivityLed();
            }
        }

        previousEncoderState = currentState;
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(EncoderPollPeriodMs));
    }
}

void buttonTask(void *parameter) {
    (void)parameter;

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(ButtonDebounceMs));

        const bool stableButtonPressed = digitalRead(EncoderButtonPin) == LOW;
        bool buttonAccepted = false;
        bool ledEnabled = false;
        int32_t ledBrightnessPercent = 0;

        xSemaphoreTake(stateMutex, portMAX_DELAY);

        if (stableButtonPressed != testState.buttonPressed) {
            testState.buttonPressed = stableButtonPressed;
            buttonAccepted = true;

            if (stableButtonPressed) {
                testState.buttonPressCount++;
                testState.pwmLedEnabled = !testState.pwmLedEnabled;
            }
        }

        ledEnabled = testState.pwmLedEnabled;
        ledBrightnessPercent = testState.ledBrightnessPercent;
        xSemaphoreGive(stateMutex);

        if (buttonAccepted) {
            applyPwmLedOutput(ledEnabled, ledBrightnessPercent);
            toggleActivityLed();
        }
    }
}

void telemetryTask(void *parameter) {
    (void)parameter;
    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        TestState snapshot{};

        xSemaphoreTake(stateMutex, portMAX_DELAY);
        snapshot = testState;
        testState.encoderDeltaSinceTelemetry = 0;
        xSemaphoreGive(stateMutex);

        sendTeleplotValue("encoder_position", snapshot.encoderPosition);
        sendTeleplotValue("encoder_steps", snapshot.encoderStepCount);
        sendTeleplotValue("encoder_delta", snapshot.encoderDeltaSinceTelemetry);
        sendTeleplotValue("button_pressed", snapshot.buttonPressed);
        sendTeleplotValue("button_press_count", snapshot.buttonPressCount);
        sendTeleplotValue("led_enabled", snapshot.pwmLedEnabled);
        sendTeleplotValue("led_brightness_percent", snapshot.ledBrightnessPercent);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TelemetryPeriodMs));
    }
}

void printStartupInfo() {
    Serial.println("KY-040 / bare rotary encoder FreeRTOS Teleplot test");
    Serial.println("Teleplot serial format: >name:value");
    Serial.println("Pins: A=GPIO32, B=GPIO33, SW=GPIO25 interrupt, PWM LED=GPIO26, GND=GND");
    Serial.println("Use VS Code Teleplot at 115200 baud.");
}

void createTasks() {
    xTaskCreatePinnedToCore(encoderTask, "Encoder", 2048, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(buttonTask, "Button", 2048, nullptr, 3, &buttonTaskHandle, 1);
    xTaskCreatePinnedToCore(telemetryTask, "Telemetry", 4096, nullptr, 1, nullptr, 0);
}

}

void setup() {
    pinMode(EncoderAPin, INPUT_PULLUP);
    pinMode(EncoderBPin, INPUT_PULLUP);
    pinMode(EncoderButtonPin, INPUT_PULLUP);
    pinMode(ActivityLedPin, OUTPUT);
    digitalWrite(ActivityLedPin, LOW);

    ledcSetup(PwmLedChannel, PwmLedFrequencyHz, PwmLedResolutionBits);
    ledcAttachPin(PwmLedPin, PwmLedChannel);
    applyPwmLedOutput(false, 0);

    Serial.begin(SerialBaud);
    delay(200);

    stateMutex = xSemaphoreCreateMutex();
    previousEncoderState = readEncoderState();

    xSemaphoreTake(stateMutex, portMAX_DELAY);
    testState = TestState{
        0,
        0,
        0,
        0,
        0,
        digitalRead(EncoderButtonPin) == LOW,
        false
    };
    xSemaphoreGive(stateMutex);

    printStartupInfo();
    createTasks();
    attachInterrupt(digitalPinToInterrupt(EncoderButtonPin), buttonInterrupt, CHANGE);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
