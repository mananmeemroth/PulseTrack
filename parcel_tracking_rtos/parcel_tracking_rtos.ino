#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "sensor_task.h"
#include "display_task.h"
#include "monitor_task.h"

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);

  Serial.println("PulseTrack booting...");
  Serial.println("Type: STATUS, LED ON, or LED OFF");

  pinMode(BUILTIN_LED_PIN, OUTPUT);
  digitalWrite(BUILTIN_LED_PIN, HIGH);

  startSensorTask();
  startDisplayTask();
  startMonitorTask();
}

void loop() {
  handleSerialCommand();
  vTaskDelay(pdMS_TO_TICKS(50));
}
