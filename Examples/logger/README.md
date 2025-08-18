# Example: Logger

This example demonstrates how to use the ColirOne Logger class to store and retrieve log data using the W25Q64 flash memory chip. The Logger class provides automatic address management, sector erasing, and various logging functions.

---

## Project Structure
logger/ 
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

---

## Features Demonstrated

This example showcases the following Logger functionalities:

1. **Storage Information Retrieval**: Get detailed information about the flash memory
2. **Automatic Address Management**: The logger automatically manages write addresses
3. **Sector Management**: Automatic sector erasing when needed
4. **Multiple Log Types**: Store raw data, sensor logs, and command logs
5. **Memory Usage Monitoring**: Check used/free space and capacity
6. **Log Reading**: Read back stored logs from flash memory

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

# How to Use
1. Flash STM32 Rocket Code
    - Build and flash the STM32 code as described above.
2. Run and Observe
    - Power the STM32 board.
    - The board will periodically read sensor data (IMU, Barometer, GPS) and print the results via UART.
    - Use a serial terminal (e.g., PuTTY, Tera Term) to view the output.

---

# Example Output (UART)
Storage Info:
Page Size: 256 bytes
Page Count: 32768
Sector Size: 4096 bytes
Sector Count: 2048
Block Size: 65536 bytes
Block Count: 128
Capacity: 8192 KB
Used Space: 0 bytes
Free Space: 8388608 bytes
Has enough space for 100 bytes: Yes

Erase all logs...
Waiting for erase to complete...
All logs erased.
Used Space after erase: 0 bytes
Free Space after erase: 8388608 bytes
Checking if write logs is enabled: Yes

Storing logs...
Read logs after storing:
Hello, ColirOne!
I (1250): Acceleration: (0.00, 0.00, 9.81), Gyroscope: (0.00, 0.00, 0.00), Orientation: (0.00, 0.00, 1.00), Quaternion: (1.00, 0.00, 0.00), Temperature: 25.00, Pressure: 1013.25, Altitude: 100.00, GPS: (37.774900, -122.419400, 5), Vertical Velocity: 1.50
I (1267): Lighter Launch Number: 3, Close Shutes: 1, Open Shutes: 0, Start Logs: 1, Write Logs: 0, Reset Altitude: 1, Remove Logs: 0
Used Space after storing logs: 512 bytes
Free Space after storing logs: 8388096 bytes