# Example: Read Sensors

This example demonstrates how to use the ColirOne class on STM32 to read and print sensor data from IMU, Barometer, and GPS modules. The output is sent via UART for easy monitoring and debugging.

---

## Project Structure
read_sensors/ 
├── Inc/ 
│ ├── config.h 
│ └── main.h 
├── Src/ 
│ └── main.cpp # STM32 main application 
├── Makefile # Build script for STM32 project 
├── README.md # This file 
├── STM32F407VETX_FLASH.ld # Linker script 
├── openocd.cfg # OpenOCD config for flashing 
└── startup_stm32f407xx.s # Startup assembly

# How to Build and Flash (STM32)

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

# How to Use
1. Flash STM32 Rocket Code
    - Build and flash the STM32 code as described above.
2. Run and Observe
    - Power the STM32 board.
    - The board will periodically read sensor data (IMU, Barometer, GPS) and print the results via UART.
    - Use a serial terminal (e.g., PuTTY, Tera Term) to view the output.

---

# Example Output (UART)
Acceleration: X: 0.12, Y: -0.34, Z: 9.81
Gyroscope: X: 0.01, Y: 0.02, Z: 0.03
Orientation: X: 45.00, Y: 0.00, Z: 90.00
Quaternion: W: 1.00, X: 0.00, Y: 0.00, Z: 0.00
Temperature: 25.00
Pressure: 101325.00
Altitude: 12.34
GPS Longitude: 105.123456, Latitude: 21.123456, Visible Satellites: 7