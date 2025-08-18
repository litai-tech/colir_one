# Example: Transmit Normal Data

This example demonstrates how to use the ColirOne class on STM32 to transmit simple data (such as a string or test message) wirelessly to a ground station using nRF24L01 modules. The ground station is implemented with an Arduino, which receives and prints the data.

---

## Project Structure

transmit_norma/ 
├── ground_station/ 
│ └── nrf24.ino # Arduino code for ground station transmitter 
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
Open ground_station/nrf24.ino in the Arduino IDE.
Connect your nRF24L01 module to the Arduino as specified in the code.
Upload the sketch to your Arduino.
2. Flash STM32 Rocket Code
Build and flash the STM32 code as described above.
3. Run and Observe
Power both the Arduino ground station and the STM32 rocket board.
The STM32 will periodically send a test message (e.g., "Hello World!").
The Arduino will receive and print the message via Serial Monitor.

---

## Notes
Ensure both nRF24L01 modules (STM32 and Arduino) use the same address and data rate.
If you modify the transmitted data, update both STM32 and Arduino code to match.
Use a serial terminal to view the output from the Arduino ground station.
This example is suitable for verifying basic wireless communication before moving to more complex sensor data transmission.
