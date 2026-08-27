#ifndef MONITOR_TASK_H
#define MONITOR_TASK_H

#include <Arduino.h>
#include "hardware_config.h"
#include "sensor_acquisition.h"

extern SemaphoreHandle_t monitorSemaphore;
extern bool systemArmed;

void startMonitorTask();
void toggleSystemMode();
void printStatusReport();
void handleSerialCommand();

#endif
