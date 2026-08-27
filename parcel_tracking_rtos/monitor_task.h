#ifndef MONITOR_TASK_H
#define MONITOR_TASK_H

#include <Arduino.h>
#include "config.h"
#include "sensor_task.h"

extern SemaphoreHandle_t monitorSemaphore;
extern bool systemArmed;

void startMonitorTask();
void toggleSystemMode();
void printStatusReport();
void handleSerialCommand();

#endif
