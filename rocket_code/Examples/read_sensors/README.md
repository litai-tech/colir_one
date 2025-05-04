## Example: Read Sensors
Here is an example of how to use the ColirOne class to retrieve sensor data, including IMU, Barometer, and GPS.

## How to modify and build new code
change this line in Makefile:

DRIVER_PATH = /home/dat/Documents/colir_one/rocket_code
OPENOCD_PATH = /home/dat/.config/Code/User/globalStorage/litaitech.stm32-colir-one/@xpack-dev-tools/openocd/0.12.0-4.1/.content/bin/openocd
GCC_PATH = /home/dat/.config/Code/User/globalStorage/litaitech.stm32-colir-one/@xpack-dev-tools/arm-none-eabi-gcc/13.3.1-1.1.1/.content/bin

With DRIVER_PATH being the path to the folder containing the rocket_code directory, OPENOCD_PATH the path to OpenOCD, and GCC_PATH the path to GCC. If you are using the STM32 Colir One VSCode plugin, there is no need to modify these paths, as everything will be automatically updated based on the environment paths.

Using cmd: `make` to build code
Using cmd: `make flash` to flash this code to your rocket after build success.
Using cmd: `make clean` to clean build folder
