#include "sensor_task.h"

#include <DHT.h>

#define DHTTYPE DHT11

DHT dht(DHT_PIN, DHTTYPE);
QueueHandle_t sensorQueue;
SensorData latestSensorData = {0.0f, 0, 0, 0, false};

static int readAxisValue(uint8_t analogPin) {
  // The ADXL335 produces analog voltage proportional to acceleration.
  // The ESP32 ADC reads this voltage and we compare successive values.
  return analogRead(analogPin);
}

static bool isVibrationHigh(int deltaX, int deltaY, int deltaZ) {
  int totalChange = abs(deltaX) + abs(deltaY) + abs(deltaZ);
  return totalChange > VIBRATION_ALERT_THRESHOLD;
}

void sensorTaskFunction(void *parameter) {
  (void)parameter;

  int lastX = readAxisValue(AXIS_X_PIN);
  int lastY = readAxisValue(AXIS_Y_PIN);
  int lastZ = readAxisValue(AXIS_Z_PIN);

  while (true) {
    SensorData data;
    data.temperatureC = dht.readTemperature();

    int currentX = readAxisValue(AXIS_X_PIN);
    int currentY = readAxisValue(AXIS_Y_PIN);
    int currentZ = readAxisValue(AXIS_Z_PIN);

    data.vibrationX = abs(currentX - lastX);
    data.vibrationY = abs(currentY - lastY);
    data.vibrationZ = abs(currentZ - lastZ);
    data.vibrationHigh = isVibrationHigh(data.vibrationX, data.vibrationY, data.vibrationZ);

    if (isnan(data.temperatureC)) {
      Serial.println("[SensorTask] Failed to read DHT11.");
      data.temperatureC = 0.0f;
    }

    latestSensorData = data;

    Serial.print("[SensorTask] Temperature: ");
    Serial.print(data.temperatureC);
    Serial.print(" C | Vibration: ");
    Serial.println(data.vibrationHigh ? "HIGH" : "LOW");

    // The queue provides a clean handoff from the sensor producer to the display consumer.
    xQueueSend(sensorQueue, &data, portMAX_DELAY);

    lastX = currentX;
    lastY = currentY;
    lastZ = currentZ;

    // Sensor sampling is periodic, so sleeping here prevents the task from busy-waiting
    // and keeps the system simple and predictable.
    vTaskDelay(pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS));
  }
}

void startSensorTask() {
  sensorQueue = xQueueCreate(5, sizeof(SensorData));

  if (sensorQueue == NULL) {
    Serial.println("[Init] Queue creation failed.");
    return;
  }

  dht.begin();

  xTaskCreatePinnedToCore(
    sensorTaskFunction,
    "SensorTask",
    4096,
    NULL,
    SENSOR_TASK_PRIORITY,
    NULL,
    app_cpu
  );
}
