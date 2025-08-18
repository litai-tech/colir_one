#include "main.h"
#include "../../ColirOne/Inc/colir_one.h"
#include "string.h"

#define LOG_FOLDER_NAME "COLIR_ONE_LOG_EXAMPLE"
#define LOG_FILE_NAME "log.txt"

colirone_payload_cmd_t commandData = {
    .lighterLaunchNumber = 3,
    .closeShutes = true,
    .openShutes = false,
    .startLogs = true,
    .writeLogs = false,
    .resetAltitude = true,
    .removeLogs = false
};

colirone_payload_sensor_t sensorData = {
    .acceleration = {0.0f, 0.0f, 9.81f},
    .gyroscope = {0.0f, 0.0f, 0.0f},
    .orientation = {0.0f, 0.0f, 1.0f},
    .quaternion = {1.0f, 0.0f, 0.0f, 0.0f},
    .barometer = {1013.25f, 25.0f, 100.0f},
    .gps = {37.7749, -122.4194, 5},
    .verticalVelocity = 1.5f
};

int main(void){
  ColirOne colirOne;
  // if not defined, it will use the default log folder and file name: "ColirOne_logs/colirone_logs.txt"
  colirone_err_t ret = colirOne.init();
  printf("ColirOne Init Status: %d\r\n", ret);

  colirOne.logger.startLogging();
  printf("Storing logs...\n");
  colirOne.logger.storeLog((uint8_t*)"Hello, ColirOne!\n", 17);
  colirOne.logger.storeSensorLog(&sensorData);
  colirOne.logger.storeCommandLog(&commandData);

  for(int i = 0; i < 100; i++) {
    char logBuffer[64];
    sprintf(logBuffer, "Log entry %d\n", i + 1);
    colirOne.logger.storeLog((uint8_t*)logBuffer, strlen(logBuffer));
  }
  colirOne.logger.stopLogging();

  printf("Start store logs to SD card\n");
  colirOne.logger.initLogFile(LOG_FOLDER_NAME, LOG_FILE_NAME);
  colirOne.logger.writeAllLogsFile(LOG_FOLDER_NAME, LOG_FILE_NAME);
  uint32_t logFileSize = colirOne.logger.getLogFileSize(LOG_FOLDER_NAME, LOG_FILE_NAME);
  printf("Log file size: %lu bytes\n", logFileSize);
  while(1){}
	return 0;
}