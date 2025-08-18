# ColirOne Rocket Code

## Introduction
This project provides the source code for customizing and controlling your rocket using the `ColirOne` class. It is designed to work with various sensors such as IMU, GPS, barometer, and components like servos and lighters. The project is modular, making it easy to extend and adapt to your specific requirements.

---

## Code Structure

The project is organized into the following directories:
rocket_code/ 
├── ColirOne/ 
│ ├── Inc/ 
│ │ ├── colir_one.h # Defines the ColirOne class and its components 
│ │ ├── imu.h # Defines the IMU class and its methods for handling IMU sensors 
│ │ ├── gps.h # Defines the GPS class and its methods for handling GPS data 
│ │ ├── barometer.h # Defines the Barometer class for handling pressure sensors 
│ │ ├── servo.h # Defines the Servo class for controlling servo motors 
│ │ ├── lighter.h # Defines the Lighter class for controlling the ignition system 
│ ├── Src/ 
│ │ ├── colir_one.cpp # Implements the ColirOne class 
│ │ ├── imu.cpp # Implements the IMU class 
│ │ ├── gps.cpp # Implements the GPS class 
│ │ ├── barometer.cpp # Implements the Barometer class 
│ │ ├── servo.cpp # Implements the Servo class 
│ │ ├── lighter.cpp # Implements the Lighter class 
├── Examples/ 
│ ├── read_sensors/ 
│ │ ├── Src/ 
│ │ │ ├── main.cpp # Example demonstrating how to use ColirOne to read sensor data 
│ │ ├── Makefile # Makefile for building the example 
├── Module/ 
│ ├── Inc/ 
│ │ ├── init.h # Declares initialization functions for STM32 
│ ├── Src/ 
│ │ ├── init.c # Implements initialization functions for STM32 
├── README.md # Project documentation

---

## Directory Descriptions

### 1. **`ColirOne/`**
This directory contains the core implementation of the `ColirOne` class and its related components:
- **`Inc/`**: Contains header files that define the classes and their methods.
- **`Src/`**: Contains source files that implement the functionality of the classes.

### 2. **`Examples/`**
This directory contains example projects demonstrating how to use the `ColirOne` class:
- **`read_sensors/`**: An example project that shows how to read data from sensors like IMU, GPS, and barometer.

### 3. **`Module/`**
This directory contains utility files for initializing and configuring STM32 peripherals:
- **`Inc/`**: Contains header files for initialization functions.
- **`Src/`**: Contains source files that implement the initialization functions.

---

## How to Use

### 1. **Build the Project**
- Navigate to the example directory:
  ```bash
  cd Examples/read_sensors

  Build the project using the provided Makefile: `make`
### 2. **Run the Example**
Flash the compiled binary to your STM32 microcontroller using a suitable tool (e.g., OpenOCD, Dap-link).
Run the program and monitor the sensor data via UART or a debugging interface.

### 3. **Example Output**
The example program reads data from the IMU, GPS, and barometer sensors and prints the results to the console. Example output:

Acceleration: X: 0.12, Y: -0.34, Z: 9.81
Gyroscope: X: 0.01, Y: 0.02, Z: 0.03
Orientation: X: 45.00, Y: 0.00, Z: 90.00

## Features
IMU Support: Read acceleration, gyroscope, and orientation data.
GPS Support: Retrieve GPS coordinates and other location data.
Barometer Support: Measure atmospheric pressure and altitude.
Servo Control: Control servo motors for rocket stabilization.
Ignition System: Manage the ignition system for rocket launch.
