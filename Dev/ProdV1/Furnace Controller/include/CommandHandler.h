#pragma once

#include "AppTypes.h"

const char *controllerStateName(ControllerState state);
void handleCommand(const Command &command);
