# -NeuroGuard-Embedded-Parkinson-s-Detection-System
This project uses an mbed microcontroller to detect Parkinson's tremors with a gyroscope sensor. It initializes gyroscope settings, configures interrupts, and processes data for 15 seconds. Results are displayed on an LCD in real-time, showing tremor detection.
# Introduction
This project is designed to detect Parkinson's tremors using a gyroscope sensor connected to an mbed microcontroller. The system collects gyroscope data for a period of 15 seconds, processes the data to calculate angular velocity and frequency, and displays the results on an LCD screen.

# Features
Initialization of LCD, SPI, and InterruptIn objects
Real-time data processing and display
Detection of Parkinson's tremors
LED indication for visual feedback
We have used STM32 mic

## Code Overview
# Header Files and Definitions
The code includes necessary header files for mbed, math operations, LCD control, and input/output operations. Constants and configurations for gyroscope control registers, flags, scaling, and filtering are defined.

# EventFlags and Callback Functions
EventFlags object: Declared to handle various events.
Callback functions: Defined for handling SPI events, data readiness, button presses, and timer interrupts.
# Hardware Initialization
Initialization of LCD, SPI, and InterruptIn objects.
Setting up SPI format and frequency.
Writing configuration data to gyroscope control registers using SPI transfers.
# Main Loop
Initializes variables and arrays for data processing.
Runs indefinitely until Parkinson’s tremors are detected or until stopped.
# Data Recording Loop
Waits for data readiness and reads gyroscope data through SPI for 15 seconds.
Processes raw gyroscope data and scales it to radians per second.
Records processed data in arrays and uses velocity to calculate frequency.
Displays processed data and measurements on an LCD screen.
# LCD Display
Updates real-time information such as frequency of all axes, Parkinson’s detection status, and intensity of detected tremors.
# Timer for 15 Seconds
A timer is used to record data for exactly 15 seconds.
# Data Aggregation and Final LCD Update
Updates the average angular velocity after the 15-second recording period.
Displays the final results, including frequency of each axis and detection result with intensity, on the LCD screen.
# LCD and LED Output
The LCD screen displays real-time and final results of the recorded data.
An LED provides visual indication if any tremors are detected.
