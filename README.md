# ESP32 Real-Time Environmental Monitoring System

Embedded system built with ESP-IDF to monitor temperature, humidity, and light in real time. The system reads sensor data, smooths noisy inputs, displays live values on an I2C LCD, and triggers alerts when environmental conditions cross defined thresholds.

## Features
- Sensor data acquisition using DHT11 and ADC-based light sensor
- Moving average filtering to reduce noise and stabilize readings
- Hysteresis-based alerting to prevent rapid alert toggling near thresholds
- Real-time display output using I2C LCD
- GPIO-based alert output for visual indication
- Hardware/software debugging during sensor and LCD integration

## How It Works
1. Reads temperature and humidity from the DHT11 sensor.
2. Reads light level using an ADC-based sensor.
3. Applies moving average filtering to stabilize sensor values.
4. Uses threshold logic with hysteresis to determine alert state.
5. Displays current readings and alert status on the I2C LCD.

## Tech Stack
- C
- ESP-IDF
- ESP32
- I2C communication
- GPIO
- ADC

## Key Challenges
- Handling noisy sensor input from real-world readings
- Preventing unstable alert behavior near threshold boundaries
- Debugging sensor communication and I2C LCD interfacing issues
- Separating hardware wiring issues from software logic bugs

## What I Learned
- How to structure an embedded application using ESP-IDF
- How to process noisy sensor data in real time
- How hysteresis improves system stability
- How to debug hardware/software integration issues
