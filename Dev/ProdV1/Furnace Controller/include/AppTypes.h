#pragma once

#include <Arduino.h>

enum class ControllerState : uint8_t {
    Boot,
    Idle,
    Ready,
    Running,
    Paused,
    Complete,
    Fault,
    Estop
};

enum class ControlMode : uint8_t {
    Auto,
    Manual
};

enum class CommandType : uint8_t {
    StartRun,
    StopRun,
    PauseRun,
    ResumeRun,
    SetAuto,
    SetManual,
    SetManualOutput,
    AcknowledgeFault
};

struct Command {
    CommandType type;
    float value;
};

struct SafetyState {
    bool estopActive;
    bool doorOpen;
    bool overTemp;
    bool heaterAllowed;
};

struct SensorState {
    float tc1C;
    float tc2C;
    bool tc1Fault;
    bool tc2Fault;
};

struct ControlState {
    ControllerState controllerState;
    ControlMode mode;
    uint32_t runtimeSeconds;
    float setpointC;
    float processValueC;
    float requestedOutputPercent;
    bool runRequested;
    bool faultLatched;
};
