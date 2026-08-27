#ifndef DISPLAY_TASK_H
#define DISPLAY_TASK_H

#include <Arduino.h>
#include "sensor_task.h"

void startDisplayTask();
void initOLED();
void displaySensorData(const SensorData &data);

#endif
