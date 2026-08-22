#include "CommandHandler.h"

#include <Arduino.h>

#include "StateStore.h"

const char *controllerStateName(ControllerState state) {
    switch (state) {
        case ControllerState::Boot:
            return "BOOT";
        case ControllerState::Idle:
            return "IDLE";
        case ControllerState::Ready:
            return "READY";
        case ControllerState::Running:
            return "RUNNING";
        case ControllerState::Paused:
            return "PAUSED";
        case ControllerState::Complete:
            return "COMPLETE";
        case ControllerState::Fault:
            return "FAULT";
        case ControllerState::Estop:
            return "ESTOP";
    }

    return "UNKNOWN";
}

void handleCommand(const Command &command) {
    ControlState next = readControlState();
    const SafetyState safety = readSafetyState();

    switch (command.type) {
        case CommandType::StartRun:
            if (safety.heaterAllowed && !next.faultLatched) {
                next.controllerState = ControllerState::Running;
                next.runRequested = true;
                next.runtimeSeconds = 0;
                next.setpointC = 150.0f;
            }
            break;

        case CommandType::StopRun:
            next.controllerState = ControllerState::Idle;
            next.runRequested = false;
            next.requestedOutputPercent = 0.0f;
            break;

        case CommandType::PauseRun:
            if (next.controllerState == ControllerState::Running) {
                next.controllerState = ControllerState::Paused;
                next.requestedOutputPercent = 0.0f;
            }
            break;

        case CommandType::ResumeRun:
            if (next.controllerState == ControllerState::Paused && safety.heaterAllowed) {
                next.controllerState = ControllerState::Running;
            }
            break;

        case CommandType::SetAuto:
            next.mode = ControlMode::Auto;
            break;

        case CommandType::SetManual:
            next.mode = ControlMode::Manual;
            next.requestedOutputPercent = 0.0f;
            break;

        case CommandType::SetManualOutput:
            if (next.mode == ControlMode::Manual) {
                next.requestedOutputPercent = constrain(command.value, 0.0f, 100.0f);
            }
            break;

        case CommandType::AcknowledgeFault:
            if (safety.heaterAllowed) {
                next.faultLatched = false;
                next.controllerState = ControllerState::Idle;
            }
            break;
    }

    publishControlState(next);
}
