#pragma once

void createTasks();

void safetyTask(void *parameter);
void sensorTask(void *parameter);
void controlTask(void *parameter);
void ssrTimingTask(void *parameter);
void commandTask(void *parameter);
void localHmiTask(void *parameter);
void pcCommsTask(void *parameter);
void telemetryTask(void *parameter);
void systemTask(void *parameter);
