#include "main.h"
#include "../../ColirOne/Inc/colir_one.h"

const uint8_t RxAddress[] = {0xEE,0xDD,0xCC,0xBB,0xAA};
const uint8_t TxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xAA};

const char* LOG_FOLDER_NAME = "COLIR-ONE_LOGS";

typedef struct __attribute__((packed)) {
    uint8_t type; // type != 0 for custom data
    uint8_t is_rocket_rx;
} colirone_common_packet_t;

int main(void){
  ColirOne colirOne;
  colirone_err_t ret = colirOne.init();
  if(ret != COLIRONE_OK) {
    printf("Initialization failed with error code: %d\n", ret);
    return -1;
  }
  else{
    printf("ColirOne initialized successfully.\n");
  }
  colirone_payload_sensor_t sensorData;
  colirone_common_packet_t commonPacket;
  colirone_payload_cmd_t colironePayloadCmd = {};
  commonPacket.type = 1;
  float altitude = 0.0f;
  float lastAltitude = 0.0f;
  float verticalVelocity = 0.0f;
  uint32_t logFileIndex = 0;

  uint32_t lastTimestamp = colirOne.getTimeStamp();
  uint32_t lastRxMode = colirOne.getTimeStamp();
  uint32_t lastTxMode = colirOne.getTimeStamp();
  uint32_t txPacketCount = 0;
  uint32_t maxTxPackets = 5;

  // get storage info
  printf("Storage Info:\n");
  colirone_storage_info_t storageInfo = colirOne.logger.getStorageInfo();
  printf("Used Space: %ld bytes\n", colirOne.logger.getUsedSpace());
  printf("Free Space: %ld bytes\n", colirOne.logger.getFreeSpace());

  // Erase all logs if needed
  // colirOne.logger.eraseAllLogs();
  // Erase log file index if needed
  // colirOne.logger.resetLogFileIndex(); // Reset log file index to 0
  // printf("Used Space after erase: %ld bytes\n", colirOne.logger.getUsedSpace());
  // printf("Free Space after erase: %ld bytes\n", colirOne.logger.getFreeSpace());

  // max: 4.294.967.295 files
  char newLogFileName[32];
  if(colirOne.logger.getUsedSpace() > 0){
    colirOne.logger.updateLogFileIndex();
    colirOne.logger.getLogFileIndex(&logFileIndex);
    snprintf(newLogFileName, sizeof(newLogFileName), "log_%ld.txt", logFileIndex);
    colirOne.logger.initLogFile(LOG_FOLDER_NAME, newLogFileName);
    printf("Write old logs to file: %s\n", newLogFileName);
    colirOne.logger.writeAllLogsFile(LOG_FOLDER_NAME, newLogFileName);
  }
  // Set up RF communication
  colirOne.rf.setTxRxAdress((uint8_t*)TxAddress, (uint8_t*)RxAddress);
  colirOne.rf.setTxMode();
  // colirOne.logger.startLogging(); // start logging by default
	while(1){
        uint32_t timestamp = colirOne.getTimeStamp();
        XYZ_t accel = colirOne.imu.getAcceleration();
        XYZ_t gyro = colirOne.imu.getGyroscope();
        XYZ_t orientation = colirOne.imu.getOrientation();
        Quaternion_t quaternion = colirOne.imu.getQuaternion();
        float temp = colirOne.barometer.getTemperature();
        float pressure = colirOne.barometer.getPressure();

        if(pressure > 10000){
            altitude = colirOne.barometer.calculateSeaLevelAltitude(pressure);
            float deltaTime = (timestamp - lastTimestamp) / 1000.0f; // Convert to seconds
            if(deltaTime > 0){
                verticalVelocity = (altitude - lastAltitude) / deltaTime; // m/s
            }
            lastAltitude = altitude;
            lastTimestamp = timestamp;
        }

        colirOne.gps.run();
        double longitude = colirOne.gps.getLongitude();
        double latitude = colirOne.gps.getLatitude();
        int visibleSatellites = colirOne.gps.getVisibleSatellites();

        if(colirOne.logger.checkEnableWriteLogs()){
          colirOne.logger.storeSensorLog(&sensorData);
        }
        if(!colirOne.rf.isRxMode()){
            sensorData.acceleration = accel;
            sensorData.gyroscope = gyro;
            sensorData.orientation = orientation;
            sensorData.quaternion = quaternion;
            sensorData.barometer.temperature = temp;
            sensorData.barometer.pressure = pressure;
            sensorData.barometer.altitude = altitude;
            sensorData.verticalVelocity = verticalVelocity;
            sensorData.gps.longitude = longitude;
            sensorData.gps.latitude = latitude;
            sensorData.gps.visibleSatellites = visibleSatellites;

            colirone_err_t err =  colirOne.rf.transmitSensorData(&sensorData, timestamp);
            if(err != COLIRONE_OK){
                printf("Error transmitting sensor data: %d\n", err);
            }
            txPacketCount++;
            printf("TX[%lu] ====== SENSOR DATA ======\n", txPacketCount);
            printf("  Accel: X:%.2f Y:%.2f Z:%.2f m/s²\n", 
                   sensorData.acceleration.x, sensorData.acceleration.y, sensorData.acceleration.z);
            printf("  Gyro:  X:%.2f Y:%.2f Z:%.2f °/s\n", 
                   sensorData.gyroscope.x, sensorData.gyroscope.y, sensorData.gyroscope.z);
            printf("  Orient: X:%.1f Y:%.1f Z:%.1f °\n", 
                   sensorData.orientation.x, sensorData.orientation.y, sensorData.orientation.z);
            printf("  Quat: W:%.3f X:%.3f Y:%.3f Z:%.3f\n", 
                   sensorData.quaternion.w, sensorData.quaternion.x, 
                   sensorData.quaternion.y, sensorData.quaternion.z);
            printf("  Baro: Temp:%.1f°C Press:%.1fPa Alt:%.2fm\n", 
                   sensorData.barometer.temperature, sensorData.barometer.pressure, 
                   sensorData.barometer.altitude);
            printf("  VelZ: %.2f m/s\n", sensorData.verticalVelocity);
            printf("  GPS: Lat:%.6f Lon:%.6f Sats:%d\n", 
                   sensorData.gps.latitude, sensorData.gps.longitude, 
                   sensorData.gps.visibleSatellites);
            printf("  Timestamp: %lu ms\n", timestamp);
            printf("=============================\n");
            sensorData = {0};
            if((timestamp - lastTxMode > 50) || (txPacketCount >= maxTxPackets)){
                printf("Switching to RX mode (packets sent: %lu)\n", txPacketCount);
                commonPacket.is_rocket_rx = 1;
                colirOne.rf.transmitData((uint8_t*)&commonPacket, sizeof(commonPacket));
                colirOne.rf.setRxMode();
                lastRxMode = colirOne.getTimeStamp();
                txPacketCount = 0;
                HAL_Delay(2);
            }
        }
        else{
            if(colirOne.rf.hasReceivedData()){
                colironePayloadCmd = {0};
                colirOne.rf.readColirOneCommand();
                colironePayloadCmd.lighterLaunchNumber = colirOne.rf.getLighterLaunchNumber();
                colironePayloadCmd.openShutes = colirOne.rf.getOpenShutes();
                colironePayloadCmd.closeShutes = colirOne.rf.getCloseShutes();
                colironePayloadCmd.startLogs = colirOne.rf.getStartLogs();
                colironePayloadCmd.writeLogs = colirOne.rf.getWriteLogs();
                colironePayloadCmd.resetAltitude = colirOne.rf.getResetAltitude();
                colironePayloadCmd.removeLogs = colirOne.rf.getRemoveLogs();
                printf("RX: Launch Number: %d, Open Shutes: %d, Close Shutes: %d, Start Logs: %d, Write Logs: %d, Reset Altitude: %d, Remove Logs: %d\n",
                       colironePayloadCmd.lighterLaunchNumber, 
                       colironePayloadCmd.openShutes, 
                       colironePayloadCmd.closeShutes, 
                       colironePayloadCmd.startLogs, 
                       colironePayloadCmd.writeLogs, 
                       colironePayloadCmd.resetAltitude, 
                       colironePayloadCmd.removeLogs);
                if(colirOne.logger.checkEnableWriteLogs()){
                    colirOne.logger.storeCommandLog(&colironePayloadCmd);
                }
                // process commands
                if(colironePayloadCmd.openShutes){
                  printf("Opening shutes\n");
                  colirOne.servo.setServoAngle(1, 40);
                }
                else if(colironePayloadCmd.closeShutes){
                  printf("Closing shutes\n");
                  colirOne.servo.setServoAngle(1, 80);
                }
                else if(colironePayloadCmd.lighterLaunchNumber > 0){
                  printf("Fire lighter %d\n", colironePayloadCmd.lighterLaunchNumber);
                  colirOne.lighter.fireLighter(colironePayloadCmd.lighterLaunchNumber);
                }
                else if(colironePayloadCmd.startLogs){
                  printf("Start write log\n");
                  colirOne.logger.startLogging();
                }
                else if(colironePayloadCmd.writeLogs){
                  printf("Write All logs to SD card\n");
                  colirOne.logger.updateLogFileIndex();
                  colirOne.logger.getLogFileIndex(&logFileIndex);
                  snprintf(newLogFileName, sizeof(newLogFileName), "log_%ld.txt", logFileIndex);
                  colirOne.logger.initLogFile(LOG_FOLDER_NAME, newLogFileName);
                  if(colirOne.logger.getUsedSpace() > 0){
                    colirOne.logger.writeAllLogsFile(LOG_FOLDER_NAME, newLogFileName);
                  }
                }
                else if(colironePayloadCmd.resetAltitude){
                    printf("Reset altitude and vertical velocity\n");
                    altitude = 0.0f;
                    lastAltitude = 0.0f;
                    verticalVelocity = 0.0f;
                }
                else if(colironePayloadCmd.removeLogs){
                    printf("Erase all logs\n");
                    colirOne.logger.eraseAllLogs();
                }

                printf("Command received, switching to TX mode immediately\n");
                colirOne.rf.setTxMode();
                HAL_Delay(2);
                commonPacket.is_rocket_rx = 0;
                colirOne.rf.transmitData((uint8_t*)&commonPacket, sizeof(commonPacket));
                lastTxMode = colirOne.getTimeStamp();
            }
            else if(timestamp - lastRxMode > 200){
                printf("RX timeout, switching to TX mode\n");
                colirOne.rf.setTxMode();
                HAL_Delay(2);
                commonPacket.is_rocket_rx = 0;
                colirOne.rf.transmitData((uint8_t*)&commonPacket, sizeof(commonPacket));
                lastTxMode = colirOne.getTimeStamp();
            }
        }
        HAL_Delay(1);
  }
	return 0;
}