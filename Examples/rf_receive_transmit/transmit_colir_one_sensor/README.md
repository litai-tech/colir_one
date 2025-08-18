# Example: Transmit ColirOne Sensor Data

This example demonstrates how to use the ColirOne class on STM32 to transmit sensor data (IMU, Barometer, GPS, etc.) wirelessly to a ground station using nRF24L01 modules. The ground station is implemented with an Arduino, which receives and prints the sensor data.

---

## Project Structure

transmit_colir_one_sensor/ 
├── ground_station/ 
│ └── nrf24_rx_sensor.ino # Arduino code for ground station transmitter 
├── Inc/ 
│ └── main.h # STM32 main header 
├── Src/ 
│ └── main.cpp # STM32 main application 
├── Makefile # Build script for STM32 project 
├── README.md # This file 
├── STM32F407VETX_FLASH.ld # Linker script 
├── openocd.cfg # OpenOCD config for flashing 
└── startup_stm32f407xx.s # Startup assembly

---

# How to Build and Flash (STM32)

1. **Configure Paths (if needed):**
   - Edit the following lines in the `Makefile` if your toolchain is in a different location:
     ```
     DRIVER_PATH = /home/dat/Documents/colir_one
     OPENOCD_PATH = <path-to-openocd>
     GCC_PATH = <path-to-arm-none-eabi-gcc>
     ```
   - If you use the STM32 Colir One VSCode plugin, these paths are set automatically.

2. **Build the project:**
   ```sh
   make

3. Flash the firmware to your STM32: make flash
4. Clean the build folder: make clean

---

## How to Use
1. Upload Arduino Ground Station Code

2. Open ground_station/nrf24_rx_sensor.ino in the Arduino IDE.
Connect your nRF24L01 module to the Arduino as specified in the code.
3. Upload the sketch to your Arduino.
4. Flash STM32 Rocket Code

5. Build and flash the STM32 code as described above.
6. Run and Observe

Power both the Arduino ground station and the STM32 rocket board.
The STM32 will periodically send sensor data packets.
The Arduino will receive and print the sensor data via Serial Monitor.

---

## Notes
Ensure both nRF24L01 modules (STM32 and Arduino) use the same address and data rate.
The sensor data is split into packets with an index/type header for reliable transmission.
If you modify the sensor struct, update both STM32 and Arduino code to match.
Use a serial terminal to view the output from the Arduino ground station.

