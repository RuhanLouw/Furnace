#pragma once

#include <Arduino.h>

#include "AppTypes.h"

void initializeStateStore();

void publishSafetyState(const SafetyState &next);
void publishSensorState(const SensorState &next);
void publishControlState(const ControlState &next);

SafetyState readSafetyState();
SensorState readSensorState();
ControlState readControlState();

bool sendCommand(const Command &command, TickType_t timeoutTicks = 0);
bool receiveCommand(Command &command, TickType_t timeoutTicks);
