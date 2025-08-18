# Example: Transmit and Receive ColirOne Data

This example demonstrates how to use the ColirOne class on STM32 to both transmit sensor data and receive command packets wirelessly using nRF24L01 modules. The ground station is implemented with an Arduino, which can receive sensor data from the STM32 and send command packets back.

---

## Project Structure

transmit_receive_colir_one/ 
├── ground_station/ 
│ └── nrf24_colir_one.ino # Arduino code for ground station transmitter 
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
    - Open ground_station/nrf24_colir_one.ino in the Arduino IDE.
    - Connect your nRF24L01 module to the Arduino as specified in the code.
    - Upload the sketch to your Arduino.
2. Flash STM32 Rocket Code
    - Build and flash the STM32 code as described above.
3. Run and Observe
    - Power both the Arduino ground station and the STM32 rocket board.
    - The STM32 will periodically send sensor data packets and listen for command packets.
    - The Arduino will receive and print sensor data via Serial Monitor, and can send commands (such as open shutes, launch, etc.) back to the STM32.

---

## Notes
Ensure both nRF24L01 modules (STM32 and Arduino) use the same address and data rate.
The sensor and command structs must be packed and use only fixed-width types (e.g., uint8_t, float), not bool.
If you modify the struct, update both Arduino and STM32 code to match.
Use a serial terminal to view the output from the Arduino ground station.
This example is suitable for verifying two-way wireless communication before moving to more complex logic.
