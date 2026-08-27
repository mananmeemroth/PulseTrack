#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Sensor and system pins
#define DHT_PIN 32
#define AXIS_X_PIN A0
#define AXIS_Y_PIN A3
#define AXIS_Z_PIN A6
#define BUTTON_PIN 19
#define BUILTIN_LED_PIN 2

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET -1

// Thresholds
#define TEMP_ALERT_THRESHOLD 32.0f
#define VIBRATION_ALERT_THRESHOLD 60

// Task timing
#define SENSOR_TASK_PERIOD_MS 2000
#define DISPLAY_TASK_PERIOD_MS 500
#define BUTTON_DEBOUNCE_MS 50

// UART
#define SERIAL_BAUD 115200

// FreeRTOS priorities
// SensorTask is given a slightly higher priority than DisplayTask because it reads
// hardware inputs and publishes fresh data. MonitorTask also runs at a high priority
// so the button interrupt can be handled promptly without delaying the sensor loop.
#define SENSOR_TASK_PRIORITY 2
#define DISPLAY_TASK_PRIORITY 1
#define MONITOR_TASK_PRIORITY 2

// RTOS settings
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

#endif
