#include "main.h"
#include "../../ColirOne/Inc/colir_one.h"

const uint8_t RxAddress[] = {0xEE,0xDD,0xCC,0xBB,0xAA};
const uint8_t TxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xAA};

typedef struct __attribute__((packed)) {
    uint8_t type; // type != 0 for custom data
    uint8_t is_rocket_rx;
} colirone_common_packet_t;

int main(void){
  	ColirOne colirOne;
  	colirone_err_t err = colirOne.init();
    if(err != COLIRONE_OK){
        printf("Error initializing colirone: %d\n", err);
        return -1;
    }
    colirOne.rf.setTxRxAdress((uint8_t*)TxAddress, (uint8_t*)RxAddress);
    colirOne.rf.setTxMode();

    colirone_payload_sensor_t sensorData;
    colirone_common_packet_t commonPacket;
    commonPacket.type = 1;
    float altitude = 0.0f;
    float lastAltitude = 0.0f;
    float verticalVelocity = 0.0f;

    uint32_t lastTimestamp = colirOne.getTimeStamp();
    uint32_t lastRxMode = colirOne.getTimeStamp();
    uint32_t lastTxMode = colirOne.getTimeStamp();
    uint32_t txPacketCount = 0;
    uint32_t maxTxPackets = 5;

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
            // Quick check for received data - non-blocking
            if(colirOne.rf.hasReceivedData()){
                colirOne.rf.readColirOneCommand();
                uint8_t launchNumber = colirOne.rf.getLighterLaunchNumber();
                uint8_t openShutes = colirOne.rf.getOpenShutes();
                uint8_t closeShutes = colirOne.rf.getCloseShutes();
                uint8_t startLogs = colirOne.rf.getStartLogs();
                uint8_t writeLogs = colirOne.rf.getWriteLogs();
                uint8_t resetAltitude = colirOne.rf.getResetAltitude();
                uint8_t removeLogs = colirOne.rf.getRemoveLogs();

                printf("RX: CMD received - Launch:%d Open:%d Close:%d StartLog:%d WriteLog:%d ResetAlt:%d RemoveLog:%d\n",
                       launchNumber, openShutes, closeShutes, startLogs, writeLogs, resetAltitude, removeLogs);
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