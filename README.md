# PulseTrack

I built this project as a compact ESP32 + FreeRTOS monitoring system for tracking temperature and vibration in a machine or equipment setup. The board reads sensor values, checks for abnormal conditions, and shows the current status on an OLED display.

This is my personal build for learning embedded systems design, task scheduling, hardware interfacing, and real-time monitoring.

---

## What the system does

The device continuously:

- reads temperature from a DHT11 sensor
- reads vibration data from an ADXL335 accelerometer using the ESP32 ADC
- compares current and previous readings to detect motion changes
- sends the latest values through a FreeRTOS queue
- updates the OLED display with the current status
- listens for a button press and toggles the monitoring mode
- prints status information over the serial monitor

If the temperature rises above the configured threshold or the vibration level becomes high, the system marks the status as an alert.

---

## Hardware

- ESP32 development board
- DHT11 temperature sensor
- ADXL335 accelerometer
- 0.96-inch I2C OLED display
- Push button
- Built-in LED

### Wiring summary

- DHT11 data → GPIO 32
- ADXL335 X → ADC input A0
- ADXL335 Y → ADC input A3
- ADXL335 Z → ADC input A6
- Push button → GPIO 19
- Built-in LED → GPIO 2
- OLED SDA → ESP32 SDA
- OLED SCL → ESP32 SCL
- OLED VCC → 3.3V / 5V as supported by the module
- OLED GND → GND

The pin layout and thresholds are stored in [pulse_track/hardware_config.h](pulse_track/hardware_config.h).

---

## Project structure

```text
pulse_track/
├── pulse_track_main.ino
├── hardware_config.h
├── sensor_acquisition.h
├── sensor_acquisition.cpp
├── oled_display.h
├── oled_display.cpp
├── system_monitor.h
├── system_monitor.cpp
```

### Files

- [pulse_track/pulse_track_main.ino](pulse_track/pulse_track_main.ino): main Arduino sketch; initializes UART and starts the RTOS tasks
- [pulse_track/hardware_config.h](pulse_track/hardware_config.h): pin definitions, thresholds, task priorities, and timing values
- [pulse_track/sensor_acquisition.cpp](pulse_track/sensor_acquisition.cpp): reads temperature and vibration and publishes sensor data
- [pulse_track/oled_display.cpp](pulse_track/oled_display.cpp): refreshes the OLED display with the latest readings
- [pulse_track/system_monitor.cpp](pulse_track/system_monitor.cpp): handles the interrupt-driven button input and monitoring mode

---

## RTOS design

This project uses a very small FreeRTOS setup:

- SensorTask reads hardware inputs and sends data to a queue
- DisplayTask waits for sensor data and updates the screen
- MonitorTask waits on a semaphore triggered by the button interrupt

The queue keeps sensor data moving from the producer task to the display task without direct shared-variable access, and the semaphore lets the button event be handled safely in a task instead of inside the interrupt routine.

---

## How the logic works

1. The ESP32 boots and initializes UART and the OLED.
2. SensorTask reads temperature and motion values from the connected hardware.
3. The measured vibration is calculated by comparing the current and previous ADC values.
4. The latest data is placed in a queue.
5. DisplayTask receives the data and shows it on the screen.
6. Pressing the button triggers an ISR, which gives a binary semaphore.
7. MonitorTask wakes up, debounces the event, and toggles the system mode.
8. Serial commands such as STATUS, LED ON, and LED OFF can be used to inspect and control the device.

---

## Build and run

1. Open the project in the Arduino IDE.
2. Install the ESP32 board support if needed.
3. Install the required libraries:
   - DHT Sensor Library by Adafruit
   - Adafruit SSD1306
   - Adafruit GFX
4. Open [pulse_track/pulse_track_main.ino](pulse_track/pulse_track_main.ino).
5. Select the correct ESP32 board and COM port.
6. Compile and upload the sketch.
7. Open the Serial Monitor at 115200 baud.
8. Use commands like STATUS, LED ON, and LED OFF.


---

## Notes

I kept the design intentionally simple so the code stays readable and the embedded concepts are easy to follow. It is a practical project for learning how sensors, interrupts, queues, and task scheduling come together in a real-time embedded system.

