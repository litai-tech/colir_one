#include "main.h"
#include "../../ColirOne/Inc/colir_one.h"

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
  colirone_err_t ret =  colirOne.init();
  if(ret != COLIRONE_OK) {
    printf("Initialization failed with error code: %d\n", ret);
    return -1;
  }
  printf("Storage Info:\n");
  colirone_storage_info_t storageInfo = colirOne.logger.getStorageInfo();
  printf("Page Size: %d bytes\n", storageInfo.pageSize);
  printf("Page Count: %ld\n", storageInfo.pageCount);
  printf("Sector Size: %ld bytes\n", storageInfo.sectorSize);
  printf("Sector Count: %ld\n", storageInfo.sectorCount);
  printf("Block Size: %ld bytes\n", storageInfo.blockSize);
  printf("Block Count: %ld\n", storageInfo.blockCount);
  printf("Capacity: %ld KB\n", storageInfo.capacityInKiloByte);
  printf("Used Space: %ld bytes\n", colirOne.logger.getUsedSpace());
  printf("Free Space: %ld bytes\n", colirOne.logger.getFreeSpace());
  printf("Has enough space for 100 bytes: %s\n", colirOne.logger.hasEnoughSpace(100) ? "Yes" : "No");

  printf("Erase all logs...\n");
  colirOne.logger.eraseAllLogs();
  printf("Used Space after erase: %ld bytes\n", colirOne.logger.getUsedSpace());
  printf("Free Space after erase: %ld bytes\n", colirOne.logger.getFreeSpace());
  printf("Checking if write logs is enabled: %s\n", colirOne.logger.checkEnableWriteLogs() ? "Yes" : "No");

  colirOne.logger.startLogging();
  printf("Storing logs...\n");
  colirOne.logger.storeLog((uint8_t*)"Hello, ColirOne!\n", 17);
  colirOne.logger.storeSensorLog(&sensorData);
  colirOne.logger.storeCommandLog(&commandData);
  printf("Read logs after storing:\n");
  colirOne.logger.readLogs();
  printf("Used Space after storing logs: %ld bytes\n", colirOne.logger.getUsedSpace());
  printf("Free Space after storing logs: %ld bytes\n", colirOne.logger.getFreeSpace());
	while(1){}
	return 0;
}