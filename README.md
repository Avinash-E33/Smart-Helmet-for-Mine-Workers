# Smart-Helmet-for-Mine-Workers
This Arduino project for the ESP32-S2 creates a versatile monitoring system, integrating sound level detection, basic noise reduction, and comprehensive gas sensing, alongside an emergency alert feature.
Using an INMP441 I2S microphone, it continuously measures ambient sound, calculates decibel levels, and attempts to generate an "anti-wave" signal when a predefined noise threshold is exceeded, aiming to counteract loud sounds. Simultaneously, the system monitors various environmental factors through a suite of sensors: a thermistor for temperature, an MQ-5 for LPG, an MQ-7 for Carbon Monoxide, and an MQ-135 for Alcohol, Benzene, and Ammonia.
All sensor data, along with real-time audio sample values, are output to the serial monitor. Crucially, the system provides immediate alerts for high concentrations of detected gases, enhancing safety. Furthermore, an integrated emergency button allows users to trigger a critical alert by pressing and holding it for five seconds, ensuring quick notification in hazardous situations. This project serves as a robust foundation for environmental monitoring and safety applications.
## Features

* Sound Level Monitoring: Measures ambient sound levels using an INMP441 I2S microphone and calculates the dB level.
* Active Noise Cancellation (Basic): Implements a rudimentary "anti-wave" generation that inverts the audio signal when the sound level exceeds a predefined threshold, aiming to counteract loud noises.
* Temperature Monitoring: Reads temperature using a thermistor.
* Gas Leak Detection:
    * MQ-5 Sensor: Detects and measures LPG concentration.
    * MQ-7 Sensor: Detects and measures Carbon Monoxide (CO) concentration.
    * MQ-135 Sensor: Detects and measures Alcohol, Benzene, and Ammonia concentrations.
* Emergency Alert Button: A dedicated button that triggers an "Emergency Alert" on the serial monitor when pressed and held for 5 seconds.
* Serial Monitor Output: Provides real-time readings of sound levels (raw and processed audio samples) and periodic updates (every minute) of all sensor data, including alerts for high gas concentrations.

## Hardware Requirements

* ESP32-S2 Development Board: The core of the system.
* INMP441 I2S Digital Microphone: For sound level monitoring.
* Thermistor: For temperature measurement.
* MQ-5 Gas Sensor: For LPG detection.
* MQ-7 Gas Sensor: For Carbon Monoxide (CO) detection.
* MQ-135 Gas Sensor: For Alcohol, Benzene, and Ammonia detection.
* Push Button: For the emergency alert feature.
* Jumper Wires
* Breadboard
