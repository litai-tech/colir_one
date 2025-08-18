#include "main.h"
#include "../../ColirOne/Inc/colir_one.h"
#include "string.h"

#define LOG_FOLDER_NAME "TEST_LOG"
#define LOG_FILE_PATH_1 "TEST_LOG/test1.txt"
#define LOG_FILE_PATH_2 "TEST_LOG/test2.txt"

int main(void){
  ColirOne colirOne;
  colirone_err_t ret = colirOne.init();
  printf("ColirOne Init Status: %d\r\n", ret);
  //require mounting the SD card before using it
  colirOne.sdcard.mount();
  // If you want to work with folders and files, you need to create the folder first
  if(colirOne.sdcard.checkExistsFolder(LOG_FOLDER_NAME) == COLIRONE_ERROR) {
    printf("Folder %s does not exist, creating...\r\n", LOG_FOLDER_NAME);
    colirOne.sdcard.createFolder(LOG_FOLDER_NAME);
  } else {
    printf("Folder %s already exists.\r\n", LOG_FOLDER_NAME);
  }
  // Create two files in the created folder
  if(colirOne.sdcard.checkExistsFile(LOG_FILE_PATH_1) != COLIRONE_OK) {
    printf("File %s does not exist, creating...\r\n", LOG_FILE_PATH_1);
    colirOne.sdcard.createFile(LOG_FILE_PATH_1);
  }
  if(colirOne.sdcard.checkExistsFile(LOG_FILE_PATH_2) != COLIRONE_OK) {
    printf("File %s does not exist, creating...\r\n", LOG_FILE_PATH_2);
    colirOne.sdcard.createFile(LOG_FILE_PATH_2);
  }

  // ------------------------------------------------Write to File 1 in folder TEST_LOG------------------------------------------------

  // Open the file and write some data (fist time), if the file already exists, it will be overwritten
  colirOne.sdcard.openFile(LOG_FILE_PATH_1);
  char data[256];
  strcpy(data, "Hello, ColirOne!\r\n");
  colirOne.sdcard.writeData(LOG_FILE_PATH_1, data, strlen(data));
  strcpy(data, "Colir One is the flight controller which has built in:\r\n");
  colirOne.sdcard.writeData(LOG_FILE_PATH_1, data, strlen(data));
  strcpy(data, "STMicroelectronics STM32F405 MCU\r\n");
  colirOne.sdcard.writeData(LOG_FILE_PATH_1, data, strlen(data));
  strcpy(data, "Bosch Sensortec BNO055 9-axis absolute orientation sensor (IMU)\r\n");
  colirOne.sdcard.writeData(LOG_FILE_PATH_1, data, strlen(data));
  colirOne.sdcard.closeFile();

  
  // Append more data to the existing file
  colirOne.sdcard.openFile(LOG_FILE_PATH_1);
  strcpy(data, "Pressure sensor for altitude and vertical velocity estimation\r\n");
  colirOne.sdcard.appendToFile(LOG_FILE_PATH_1, data, strlen(data));
  strcpy(data, "Pins to connect your favourite GPS\r\n");
  colirOne.sdcard.appendToFile(LOG_FILE_PATH_1, data, strlen(data));
  colirOne.sdcard.closeFile();

  // ------------------------------------------------Write to File 2 in folder TEST_LOG------------------------------------------------
  // If the file is empty, user can use append to write directly, but the speed will be slower. It’s recommended to use writeData for the first write.
  colirOne.sdcard.openFile(LOG_FILE_PATH_2);
  strcpy(data, "8 outputs for servo (5V, up to 2A current)\r\n");
  colirOne.sdcard.appendToFile(LOG_FILE_PATH_2, data, strlen(data));
  strcpy(data, "6 outputs for e-igniters (5V, up to 3A)\r\n");
  colirOne.sdcard.appendToFile(LOG_FILE_PATH_2, data, strlen(data));
  strcpy(data, "Nordic Semiconductor® nRF24L01 - for data telemetry\r\n");
  colirOne.sdcard.appendToFile(LOG_FILE_PATH_2, data, strlen(data));
  strcpy(data, "8MB flash memory for logs\r\n");
  colirOne.sdcard.appendToFile(LOG_FILE_PATH_2, data, strlen(data));
  colirOne.sdcard.closeFile();

  // Browse files in the created folder
  colirOne.sdcard.browseFiles(LOG_FOLDER_NAME);
  // get free space and total size of the SD card
  uint32_t freeSpace = colirOne.sdcard.getFreeSpace();
  uint32_t totalSize = colirOne.sdcard.getTotalSize();
  uint32_t fileSize1 = colirOne.sdcard.getFileSize(LOG_FILE_PATH_1);
  uint32_t fileSize2 = colirOne.sdcard.getFileSize(LOG_FILE_PATH_2);
  uint32_t folderSize = colirOne.sdcard.getFolderSize(LOG_FOLDER_NAME);
  printf("Folder %s size: %lu bytes\r\n", LOG_FOLDER_NAME, folderSize);
  printf("File %s size: %lu bytes\r\n", LOG_FILE_PATH_1, fileSize1);
  printf("File %s size: %lu bytes\r\n", LOG_FILE_PATH_2, fileSize2);
  printf("SD Card Info:\r\n");
  printf("Free Space: %lu bytes\n", freeSpace);
  printf("Total Size: %lu bytes\n", totalSize);

  // Unmount the SD card when done
  colirOne.sdcard.unmount();
  while(1){}
	return 0;
}