#include "system_monitor.h"

SemaphoreHandle_t monitorSemaphore;
bool systemArmed = true;

void printStatusReport() {
  Serial.println("=== PulseTrack Status ===");
  Serial.print("System: ");
  Serial.println(systemArmed ? "RUNNING" : "STANDBY");
  Serial.print("Mode: ");
  Serial.println(systemArmed ? "NORMAL" : "STANDBY");
  Serial.print("Temperature: ");
  Serial.print(latestSensorData.temperatureC, 1);
  Serial.println(" C");
  Serial.print("Vibration: ");
  Serial.println(latestSensorData.vibrationHigh ? "HIGH" : "LOW");
  Serial.print("LED: ");
  Serial.println(digitalRead(BUILTIN_LED_PIN) == HIGH ? "ON" : "OFF");
}

void handleSerialCommand() {
  if (Serial.available() <= 0) {
    return;
  }

  String command = Serial.readStringUntil('\n');
  command.trim();
  command.toUpperCase();

  if (command == "STATUS") {
    printStatusReport();
  } else if (command == "LED ON") {
    digitalWrite(BUILTIN_LED_PIN, HIGH);
    Serial.println("[UART] LED ON");
  } else if (command == "LED OFF") {
    digitalWrite(BUILTIN_LED_PIN, LOW);
    Serial.println("[UART] LED OFF");
  } else if (command.length() > 0) {
    Serial.println("[UART] Unknown command. Try: STATUS, LED ON, LED OFF");
  }
}

void IRAM_ATTR buttonISR() {
  BaseType_t higherPriorityTaskWoken = pdFALSE;

  // The ISR should only wake the handler task. It must not do any heavy processing.
  xSemaphoreGiveFromISR(monitorSemaphore, &higherPriorityTaskWoken);

  if (higherPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

void toggleSystemMode() {
  systemArmed = !systemArmed;
  digitalWrite(BUILTIN_LED_PIN, systemArmed ? HIGH : LOW);
  Serial.print("[MonitorTask] Monitoring mode: ");
  Serial.println(systemArmed ? "ACTIVE" : "STANDBY");
}

void monitorTaskFunction(void *parameter) {
  (void)parameter;

  pinMode(BUILTIN_LED_PIN, OUTPUT);
  digitalWrite(BUILTIN_LED_PIN, HIGH);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

  while (true) {
    if (xSemaphoreTake(monitorSemaphore, portMAX_DELAY) == pdTRUE) {
      // The semaphore wakes MonitorTask only after the button interrupt, so the button
      // event is processed in a task rather than directly in the ISR.
      vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS));
      if (digitalRead(BUTTON_PIN) == LOW) {
        toggleSystemMode();
        Serial.println("[MonitorTask] Button event received.");
      }
    }
  }
}

void startMonitorTask() {
  monitorSemaphore = xSemaphoreCreateBinary();

  if (monitorSemaphore == NULL) {
    Serial.println("[Init] Semaphore creation failed.");
    return;
  }

  xTaskCreatePinnedToCore(
    monitorTaskFunction,
    "MonitorTask",
    4096,
    NULL,
    MONITOR_TASK_PRIORITY,
    NULL,
    app_cpu
  );
}
