#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include <Arduino.h>
#include "hardware_config.h"

struct SensorData {
  float temperatureC;
  int vibrationX;
  int vibrationY;
  int vibrationZ;
  bool vibrationHigh;
};

extern QueueHandle_t sensorQueue;
extern SensorData latestSensorData;

void startSensorTask();

#endif
