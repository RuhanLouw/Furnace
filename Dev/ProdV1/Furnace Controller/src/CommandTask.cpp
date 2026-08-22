#include "Tasks.h"

#include <Arduino.h>

#include "AppTypes.h"
#include "CommandHandler.h"
#include "StateStore.h"

void commandTask(void *parameter) {
    (void)parameter;
    Command command{};

    while (true) {
        if (receiveCommand(command, portMAX_DELAY)) {
            handleCommand(command);
        }
    }
}
