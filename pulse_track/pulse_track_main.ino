#include <Arduino.h>
#include <Wire.h>
#include "hardware_config.h"
#include "sensor_acquisition.h"
#include "oled_display.h"
#include "system_monitor.h"

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
