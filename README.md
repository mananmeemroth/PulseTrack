# PulseTrack — FreeRTOS-Based Embedded Monitoring System

PulseTrack is a small embedded monitoring project built around the ESP32 and FreeRTOS. The purpose of the system is to measure temperature and vibration from a machine or equipment, detect abnormal conditions, and display the result locally on an OLED screen. The design is intentionally simple so it is easy to understand, easy to explain in an interview, and realistic as a beginner-friendly embedded systems project.

This project stays local to the embedded hardware, which is a better fit for understanding RTOS design, sensor interfacing, GPIO interrupts, queues, and embedded debugging.

---

## 1. Why this project exists

Imagine a small motor, fan, pump, or machine that needs monitoring. If it gets too hot or vibrates too much, it may be developing a fault. This project uses local sensing and simple alert logic to make a decision in real time.

The system does three core things:

- reads temperature from DHT11
- reads vibration from ADXL335
- shows the status on an OLED display and via serial output

This is a realistic embedded systems use case because the device is making decisions locally, with real-time constraints and hardware interaction.

---

## 2. Project goal

The goal is to provide a clean demonstration of:

- ESP32 programming
- FreeRTOS task design
- sensor acquisition
- ADC usage
- GPIO interrupt handling
- binary semaphore signaling
- queue-based communication
- I2C OLED communication
- UART debugging

The project is intentionally simple so that every important concept can be understood from reading the code and README.

---

## 3. What the system does

The system runs continuously and does the following:

1. SensorTask reads temperature from DHT11.
2. SensorTask reads X, Y, and Z vibration values from the ADXL335 using ADC.
3. It calculates a simple motion change by comparing current and previous values.
4. It classifies vibration as LOW or HIGH.
5. It sends the latest sensor data through a FreeRTOS queue.
6. DisplayTask receives the data and updates the OLED display.
7. A button press triggers an interrupt, which wakes MonitorTask using a binary semaphore.
8. MonitorTask toggles the system mode and prints status information over UART.

---

## 4. Hardware used

- ESP32 development board
- DHT11 temperature sensor
- ADXL335 accelerometer
- 0.96-inch I2C OLED display (128x64)
- Push button
- Built-in LED (optional but useful for visible status)

This project is built to be small, reliable, and easy to debug on a bench.

---

## 5. Hardware wiring summary

### Sensor connections

- DHT11 data pin → GPIO 32
- ADXL335 X output → ADC input A0
- ADXL335 Y output → ADC input A3
- ADXL335 Z output → ADC input A6

### Button and LED

- Push button → GPIO 19
- Built-in LED → GPIO 2

### OLED display

- OLED SDA → ESP32 SDA
- OLED SCL → ESP32 SCL
- OLED VCC → 3.3V or 5V as supported by your module
- OLED GND → GND

> The exact pin definitions are kept in one place in config.h so the project is easy to adjust.

---

## 6. Pin configuration

All important pin mappings are centralized in:

- `parcel_tracking_rtos/config.h`

Important definitions:

- `DHT_PIN` = 32
- `AXIS_X_PIN` = A0
- `AXIS_Y_PIN` = A3
- `AXIS_Z_PIN` = A6
- `BUTTON_PIN` = 19
- `BUILTIN_LED_PIN` = 2

The project also contains thresholds and timing values in the same configuration file:

- `TEMP_ALERT_THRESHOLD` = 32.0
- `VIBRATION_ALERT_THRESHOLD` = 60
- `SENSOR_TASK_PERIOD_MS` = 2000
- `BUTTON_DEBOUNCE_MS` = 50

This keeps the project beginner-friendly and makes tuning easier.

---

## 7. Software architecture

The project is organized in a very small and clear structure:

```text
parcel_tracking_rtos/
├── parcel_tracking_rtos.ino
├── config.h
├── sensor_task.h
├── sensor_task.cpp
├── display_task.h
├── display_task.cpp
├── monitor_task.h
├── monitor_task.cpp
```

### File-by-file explanation

#### `parcel_tracking_rtos.ino`
This is the main Arduino entry point. It initializes UART, starts the RTOS tasks, and runs the main loop.

#### `config.h`
This holds all central hardware definitions, thresholds, and FreeRTOS priorities. This is the place to adjust pins and behavior.

#### `sensor_task.h` / `sensor_task.cpp`
This contains the SensorTask and the `SensorData` structure. It reads temperature and vibration and publishes the result.

#### `display_task.h` / `display_task.cpp`
This contains the OLED display logic. It waits for data from the queue and shows the latest values on the screen.

#### `monitor_task.h` / `monitor_task.cpp`
This contains the button logic, ISR, semaphore handling, and UART command processing.

---

## 8. RTOS design

The project uses a very small FreeRTOS architecture:

### SensorTask

This task:

- reads temperature once every 2 seconds
- reads ADC values from the ADXL335
- computes vibration by comparing current and previous samples
- sends the sensor result through a queue

Why this is important:

- sensor reading is a continuous hardware task
- it should not block the display task or button handling
- FreeRTOS makes this separation clean and predictable

### DisplayTask

This task:

- waits on the queue
- receives the sensor data structure
- updates the OLED display

Why this is important:

- the display is a consumer of the sensor data
- the display logic should not directly read the hardware sensors
- this creates a clean producer-consumer relationship

### MonitorTask

This task:

- waits for a semaphore signal
- handles the button event
- toggles the system mode
- prints debug/status messages

Why this is important:

- button activity is asynchronous and event-driven
- it should not run inside the interrupt routine
- the interrupt should only wake the task

---

## 9. Why the queue is used

A queue is used to pass data from SensorTask to DisplayTask.

This matters because:

- SensorTask generates the readings
- DisplayTask uses the readings
- the two tasks run independently and at different times
- the queue provides safe communication between tasks without direct shared-variable problems

The message sent through the queue is a `SensorData` structure containing:

- temperature
- vibration X
- vibration Y
- vibration Z
- vibrationHigh flag

This is a simple and realistic RTOS communication pattern.

---

## 10. Why the semaphore and interrupt are used

The push button is connected to a GPIO pin and configured with an interrupt.

The interrupt handler does only one thing:

- it calls `xSemaphoreGiveFromISR()`

This is the correct embedded pattern because interrupts must remain short. If the ISR did full processing, it could delay other system actions and cause timing problems.

Then MonitorTask does the actual work:

- takes the semaphore
- debounces button input
- toggles mode / LED / status

This separation is important in embedded systems design:

- ISR = fast wake-up signal
- task = actual logic handling

---

## 11. How temperature and vibration are read

### Temperature measurement (DHT11)

The DHT11 sensor is digital and provides temperature values in Celsius. The code uses the DHT library and calls `dht.readTemperature()`.

If the reading fails, the code prints a fault message and sets a safe fallback value.

### Vibration measurement (ADXL335)

The ADXL335 is a 3-axis analog accelerometer. The ESP32 ADC reads the X, Y, and Z outputs. The code compares each current ADC reading with the previous ADC reading.

```cpp
vibrationX = abs(currentX - lastX);
vibrationY = abs(currentY - lastY);
vibrationZ = abs(currentZ - lastZ);
```

Then the code adds the absolute differences together and compares the sum with a threshold.

If the sum is large enough, the system labels vibration as HIGH.

This is intentionally simple and reliable. It is not advanced signal processing, but it strongly demonstrates embedded sensor processing and threshold logic.

---

## 12. Why ADC is used

The ADXL335 produces analog voltage outputs. The ESP32 ADC converts those analog voltages into digital values that the microcontroller can process.

This is a key embedded concept:

- sensors often produce analog values
- ADC converts them into digital values for software analysis

In this project, ADC is used for vibration monitoring.

---

## 13. Why I2C is used

The OLED display communicates over I2C. This is a common embedded protocol for small display modules and sensors.

The project uses:

- `Wire.begin();`
- `Adafruit_SSD1306`

This keeps the OLED logic simple and reliable.

---

## 14. UART / Serial debugging

The ESP32 UART interface is used for debugging and simple command input.

The system prints messages such as:

```text
[SensorTask] Temperature: 27.4 C | Vibration: LOW
[MonitorTask] Button event received.
```

It also accepts simple commands:

- `STATUS`
- `LED ON`
- `LED OFF`

Example response:

```text
=== PulseTrack Status ===
System: RUNNING
Mode: NORMAL
Temperature: 27.4 C
Vibration: LOW
LED: ON
```

This is a clean way to inspect the system without adding a complex command parser.

---

## 15. Task priorities

The project defines the RTOS priorities in config.h.

```cpp
#define SENSOR_TASK_PRIORITY 2
#define DISPLAY_TASK_PRIORITY 1
#define MONITOR_TASK_PRIORITY 2
```

Why these values:

- SensorTask is the most important because it reads data and continuously updates system state.
- MonitorTask also runs at a higher priority because the button event should be handled quickly.
- DisplayTask is lower priority because it only updates the OLED screen.

This is a simple, realistic priority assignment for a small embedded system.

---

## 16. System behavior and expected output

### Normal operation

OLED display:

```text
Temp: 27.4 C
Vib: LOW
System: OK
```

Serial output:

```text
[SensorTask] Temperature: 27.4 C | Vibration: LOW
```

### Alert condition

If temperature exceeds threshold or vibration becomes high, the OLED may show:

```text
Temp: 35.2 C
Vib: HIGH
System: ALERT
```

This is the practical detection logic for machine health monitoring.

---

## 17. How the software actually flows

Here is the simplest way to understand the whole program:

1. Power on ESP32.
2. Setup UART and OLED.
3. Start SensorTask.
4. Start DisplayTask.
5. Start MonitorTask.
6. SensorTask continuously reads sensor values.
7. SensorTask sends data to queue.
8. DisplayTask wakes up and updates the OLED.
9. User presses the button.
10. GPIO interrupt triggers a semaphore.
11. MonitorTask wakes, toggles mode, and prints status.
12. User can type `STATUS` over Serial to inspect the current state.

This is the entire program logic in one cycle.

---

## 18. Practical interview explanation

If asked in an interview, you can explain the project like this:

> This project is an embedded monitoring system for detecting abnormal temperature and vibration in a machine. It runs on an ESP32 using FreeRTOS, with one task reading sensors, one task updating the OLED, and another task handling button-triggered control. The sensor task sends data through a FreeRTOS queue to the display task, and the button press uses a GPIO interrupt and binary semaphore to trigger a system event without blocking the ISR. The system is simple, local, and suitable for troubleshooting machine health in real time.

That is a good concise technical answer.

---

## 19. How to build and upload

1. Open the Arduino IDE.
2. Install the ESP32 core for Arduino if it is not already installed.
3. Install these libraries:
   - DHT Sensor Library by Adafruit
   - Adafruit SSD1306
   - Adafruit GFX
4. Open `parcel_tracking_rtos/parcel_tracking_rtos.ino`.
5. Select the correct ESP32 board and COM port.
6. Compile and upload.
7. Open the Serial Monitor at 115200 baud.
8. Type `STATUS`, `LED ON`, or `LED OFF`.

---

## 20. Expected serial monitor output

```text
PulseTrack booting...
Type: STATUS, LED ON, or LED OFF
[SensorTask] Temperature: 27.4 C | Vibration: LOW
[SensorTask] Temperature: 28.1 C | Vibration: LOW
[MonitorTask] Button event received.
[MonitorTask] Monitoring mode: ACTIVE
[SensorTask] Temperature: 35.2 C | Vibration: HIGH
=== PulseTrack Status ===
System: RUNNING
Mode: NORMAL
Temperature: 27.4 C
Vibration: LOW
LED: ON
```

---

## 21. Future improvements

This project is intentionally kept small. If you wanted to extend it later, some reasonable next steps would be:

- better sensor filtering for noise reduction
- a proper state machine for fault states
- buzzer alarm output
- threshold calibration via UART
- more robust watchdog handling

But these are optional and not required for the current design.

---

## 22. License

This project is provided under the MIT license.

---

## 23. Summary

PulseTrack is a simple but strong embedded systems project because it demonstrates the following in a small and understandable way:

- ESP32 hardware
- FreeRTOS tasks
- queues
- binary semaphore
- GPIO interrupt
- ADC sensor reading
- I2C OLED output
- UART debugging
- embedded monitoring logic

If you can explain this project clearly, you can explain many real embedded software concepts that interviewers care about.
