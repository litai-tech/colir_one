#include "main.h"
#include "../../ColirOne/Inc/colir_one.h"

const uint8_t TxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xAA};
const uint8_t RxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xBB};

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
  colirone_payload_sensor_t sensorData;
  colirone_common_packet_t commonPacket;
  colirone_payload_cmd_t colironePayloadCmd;
  commonPacket.type = 1;
  float altitude = 0.0f;
  float lastAltitude = 0.0f;
  float verticalVelocity = 0.0f;
  uint32_t logFileIndex = 0;

  uint32_t lastTimestamp = colirOne.getTimeStamp();
  uint32_t lastRxMode = colirOne.getTimeStamp();

  // get storage info
  printf("Storage Info:\n");
  colirone_storage_info_t storageInfo = colirOne.logger.getStorageInfo();
  printf("capacityInKiloByte: %ld KB\n", storageInfo.capacityInKiloByte);
  printf("Used Space: %ld bytes\n", colirOne.logger.getUsedSpace());
  printf("Free Space: %ld bytes\n", colirOne.logger.getFreeSpace());

  // Erase all logs if needed
  // colirOne.logger.eraseAllLogs();

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
  colirOne.logger.startLogging(); // start logging by default
	while(1){
        uint32_t timestamp = colirOne.getTimeStamp();
        XYZ_t accel = colirOne.imu.getAcceleration();
        printf("Acceleration: X: %.2f, Y: %.2f, Z: %.2f\n", accel.x, accel.y, accel.z);
        XYZ_t gyro = colirOne.imu.getGyroscope();
        printf("Gyroscope: X: %.2f, Y: %.2f, Z: %.2f\n", gyro.x, gyro.y, gyro.z);
        XYZ_t orientation = colirOne.imu.getOrientation();
        printf("Orientation: X: %.2f, Y: %.2f, Z: %.2f\n", orientation.x, orientation.y, orientation.z);
        Quaternion_t quaternion = colirOne.imu.getQuaternion();
        printf("Quaternion: W: %.2f, X: %.2f, Y: %.2f, Z: %.2f\n", quaternion.w, quaternion.x, quaternion.y, quaternion.z);

        float temp = colirOne.barometer.getTemperature();
        printf("Temperature: %.2f\n", temp);
        float pressure = colirOne.barometer.getPressure();
        printf("Pressure: %.2f\n", pressure);

        if(pressure > 10000){
            altitude = colirOne.barometer.calculateSeaLevelAltitude(pressure);
            printf("Altitude: %.2f\n", altitude);
            float deltaTime = (timestamp - lastTimestamp) / 1000.0f; // Convert to seconds
            if(deltaTime > 0){
                verticalVelocity = (altitude - lastAltitude) / deltaTime; // m/s
            }
            lastAltitude = altitude;
            lastTimestamp = timestamp;
            printf("Vertical Velocity: %.2f m/s\n", verticalVelocity);
        }

        colirOne.gps.run();
        double longitude = colirOne.gps.getLongitude();
        double latitude = colirOne.gps.getLatitude();
        int visibleSatellites = colirOne.gps.getVisibleSatellites();
        printf("GPS Longitude: %.6f, Latitude: %.6f, Visible Satellites: %d\n", longitude, latitude, visibleSatellites);

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

        if(colirOne.logger.checkEnableWriteLogs()){
          colirOne.logger.storeSensorLog(&sensorData);
        }
        // if(!colirOne.rf.isRxMode()){
        //     colirone_err_t err =  colirOne.rf.transmitSensorData(&sensorData, timestamp);
        //     if(err != COLIRONE_OK){
        //         printf("Error transmitting sensor data: %d\n", err);
        //     }
        //     sensorData = {0}; // Reset sensor data after transmission
        //     if(timestamp - lastRxMode > 100){
        //         printf("Switching to RX mode\n");
        //         commonPacket.is_rocket_rx = 1;
        //         colirOne.rf.transmitData((uint8_t*)&commonPacket, sizeof(commonPacket));
        //         colirOne.rf.setRxMode();
        //         lastRxMode = colirOne.getTimeStamp();
        //     }
        // }
        // else{
        //     if(colirOne.rf.hasReceivedData()){
        //         colirOne.rf.readColirOneCommand();
        //         colironePayloadCmd.lighterLaunchNumber = colirOne.rf.getLighterLaunchNumber();
        //         colironePayloadCmd.openShutes = colirOne.rf.getOpenShutes();
        //         colironePayloadCmd.closeShutes = colirOne.rf.getCloseShutes();
        //         colironePayloadCmd.startLogs = colirOne.rf.getStartLogs();
        //         colironePayloadCmd.writeLogs = colirOne.rf.getWriteLogs();
        //         colironePayloadCmd.resetAltitude = colirOne.rf.getResetAltitude();
        //         colironePayloadCmd.removeLogs = colirOne.rf.getRemoveLogs();
        //         printf("Received Command: Launch Number: %d, Open Shutes: %d, Close Shutes: %d, Start Logs: %d, Write Logs: %d, Reset Altitude: %d, Remove Logs: %d\n",
        //                colironePayloadCmd.lighterLaunchNumber, 
        //                colironePayloadCmd.openShutes, 
        //                colironePayloadCmd.closeShutes, 
        //                colironePayloadCmd.startLogs, 
        //                colironePayloadCmd.writeLogs, 
        //                colironePayloadCmd.resetAltitude, 
        //                colironePayloadCmd.removeLogs);
        //         if(colirOne.logger.checkEnableWriteLogs()){
        //             colirOne.logger.storeCommandLog(&colironePayloadCmd);
        //         }
        //         if(colironePayloadCmd.openShutes){
        //           colirOne.servo.setServoAngle(7, 40);
        //         }
        //         else if(colironePayloadCmd.closeShutes){
        //           colirOne.servo.setServoAngle(7, 80);
        //         }
        //         else if(colironePayloadCmd.lighterLaunchNumber > 0){
        //           colirOne.lighter.fireLighter(colironePayloadCmd.lighterLaunchNumber);
        //         }
        //         else if(colironePayloadCmd.startLogs){
        //           colirOne.logger.startLogging();
        //         }
        //         else if(colironePayloadCmd.writeLogs){
        //           colirOne.logger.updateLogFileIndex();
        //           colirOne.logger.getLogFileIndex(&logFileIndex);
        //           snprintf(newLogFileName, sizeof(newLogFileName), "log_%ld.txt", logFileIndex);
        //           colirOne.logger.initLogFile(LOG_FOLDER_NAME, newLogFileName);
        //           if(colirOne.logger.getUsedSpace() > 0){
        //             colirOne.logger.writeAllLogsFile(LOG_FOLDER_NAME, newLogFileName);
        //           }
        //         }
        //         else if(colironePayloadCmd.resetAltitude){
        //             altitude = 0.0f;
        //             lastAltitude = 0.0f;
        //             verticalVelocity = 0.0f;
        //         }
        //         else if(colironePayloadCmd.removeLogs){
        //             colirOne.logger.eraseAllLogs();
        //         }
        //     }
        //     else if(timestamp - lastRxMode > 100){
        //         printf("Switching to TX mode\n");
        //         colirOne.rf.setTxMode();
        //         commonPacket.is_rocket_rx = 0;
        //         colirOne.rf.transmitData((uint8_t*)&commonPacket, sizeof(commonPacket));
        //         lastRxMode = colirOne.getTimeStamp();
        //     }
        // }
  }
	return 0;
}