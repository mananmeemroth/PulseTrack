#include "oled_display.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initOLED() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("[DisplayTask] OLED initialization failed.");
    for (;;) {}
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("PulseTrack");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(600));
  display.clearDisplay();
}

void displaySensorData(const SensorData &data) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(data.temperatureC, 1);
  display.println(" C");

  display.setCursor(0, 18);
  display.print("Vib: ");
  display.println(data.vibrationHigh ? "HIGH" : "LOW");

  display.setCursor(0, 36);
  display.print("System: ");
  display.println((data.temperatureC > TEMP_ALERT_THRESHOLD || data.vibrationHigh) ? "ALERT" : "OK");

  display.display();
}

void displayTaskFunction(void *parameter) {
  (void)parameter;

  SensorData data;

  while (true) {
    if (xQueueReceive(sensorQueue, &data, portMAX_DELAY) == pdTRUE) {
      // The display task waits for sensor updates and refreshes the OLED with the latest values.
      displaySensorData(data);
    }
  }
}

void startDisplayTask() {
  Wire.begin();
  initOLED();

  xTaskCreatePinnedToCore(
    displayTaskFunction,
    "DisplayTask",
    4096,
    NULL,
    DISPLAY_TASK_PRIORITY,
    NULL,
    app_cpu
  );
}
