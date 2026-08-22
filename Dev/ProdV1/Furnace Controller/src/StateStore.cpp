#include "StateStore.h"

namespace {

SafetyState safetyState{};
SensorState sensorState{};
ControlState controlState{};

SemaphoreHandle_t safetyMutex = nullptr;
SemaphoreHandle_t sensorMutex = nullptr;
SemaphoreHandle_t controlMutex = nullptr;
QueueHandle_t commandQueue = nullptr;

}

void initializeStateStore() {
    safetyMutex = xSemaphoreCreateMutex();
    sensorMutex = xSemaphoreCreateMutex();
    controlMutex = xSemaphoreCreateMutex();
    commandQueue = xQueueCreate(10, sizeof(Command));

    publishSafetyState(SafetyState{false, false, false, true});
    publishSensorState(SensorState{25.0f, 25.0f, false, false});
    publishControlState(ControlState{
        ControllerState::Idle,
        ControlMode::Auto,
        0,
        150.0f,
        25.0f,
        0.0f,
        false,
        false
    });
}

void publishSafetyState(const SafetyState &next) {
    xSemaphoreTake(safetyMutex, portMAX_DELAY);
    safetyState = next;
    xSemaphoreGive(safetyMutex);
}

void publishSensorState(const SensorState &next) {
    xSemaphoreTake(sensorMutex, portMAX_DELAY);
    sensorState = next;
    xSemaphoreGive(sensorMutex);
}

void publishControlState(const ControlState &next) {
    xSemaphoreTake(controlMutex, portMAX_DELAY);
    controlState = next;
    xSemaphoreGive(controlMutex);
}

SafetyState readSafetyState() {
    SafetyState snapshot{};
    xSemaphoreTake(safetyMutex, portMAX_DELAY);
    snapshot = safetyState;
    xSemaphoreGive(safetyMutex);
    return snapshot;
}

SensorState readSensorState() {
    SensorState snapshot{};
    xSemaphoreTake(sensorMutex, portMAX_DELAY);
    snapshot = sensorState;
    xSemaphoreGive(sensorMutex);
    return snapshot;
}

ControlState readControlState() {
    ControlState snapshot{};
    xSemaphoreTake(controlMutex, portMAX_DELAY);
    snapshot = controlState;
    xSemaphoreGive(controlMutex);
    return snapshot;
}

bool sendCommand(const Command &command, TickType_t timeoutTicks) {
    return xQueueSend(commandQueue, &command, timeoutTicks) == pdTRUE;
}

bool receiveCommand(Command &command, TickType_t timeoutTicks) {
    return xQueueReceive(commandQueue, &command, timeoutTicks) == pdTRUE;
}
