# Example: ColirOne Command Receive (with Arduino Ground Station)

This example demonstrates how to use the ColirOne class on STM32 to receive command packets sent from a ground station. The ground station is implemented using an Arduino with an nRF24L01 module.

---

## Project Structure
receive_colirone_cmd/ 
├── ground_station/ 
│ └── nrf24_tx_cmd.ino # Arduino code for ground station transmitter 
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

## How It Works

- **Ground Station (Arduino):**
  - Uses `nrf24_tx_cmd.ino` to send command packets (`colirone_payload_cmd_t`) via nRF24L01.
- **Rocket (STM32):**
  - Receives the command packets using the ColirOne library and nRF24L01.
  - Parses and prints the received command fields via UART.

---

## How to Build and Flash (STM32)

1. **Configure Paths (if needed):**
   - Edit the following lines in the `Makefile` if your toolchain is in a different location:
     ```
     DRIVER_PATH = /home/dat/Documents/colir_one/rocket_code
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
Open ground_station/nrf24_tx_cmd.ino in the Arduino IDE.
Connect your nRF24L01 module to the Arduino as specified in the code.
Upload the sketch to your Arduino.
2. Flash STM32 Rocket Code
Build and flash the STM32 code as described above.
3. Run and Observe
Power both the Arduino ground station and the STM32 rocket board.
The Arduino will periodically send command packets.
The STM32 will receive and print the command fields via UART (e.g., using a serial terminal).

## Example Output (STM32 UART)
Lighter Launch Number: 4
Open Shutes: 1
Start Logs: 0
Write Logs: 0
Reset Altitude: 1
Remove Logs: 0