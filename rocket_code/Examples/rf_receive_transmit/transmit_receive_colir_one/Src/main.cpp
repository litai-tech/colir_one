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
    if(err != COLIRONE_OK){
        printf("Error setting TX/RX address: %d\n", err);
        return -1;
    }
    colirOne.rf.setTxMode();
    if(err != COLIRONE_OK){
        printf("Error setting TX mode: %d\n", err);
        return -1;
    }

    colirone_payload_sensor_t sensorData;
    colirone_common_packet_t commonPacket;
    commonPacket.type = 1;
    float altitude = 0.0f;
    float lastAltitude = 0.0f;
    float verticalVelocity = 0.0f;

    uint32_t lastTimestamp = colirOne.getTimeStamp();
    uint32_t lastRxMode = colirOne.getTimeStamp();

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

        if(!colirOne.rf.isRxMode()){
            sensorData.acceleration = accel;
            sensorData.gyroscope = gyro;
            sensorData.orientation = orientation;
            sensorData.quaternion = quaternion;
            sensorData.barometer.temperature = temp;
            sensorData.barometer.pressure = pressure;
            sensorData.barometer.altitude = altitude;
            sensorData.vertical_velocity = verticalVelocity;
            sensorData.gps.longitude = longitude;
            sensorData.gps.latitude = latitude;
            sensorData.gps.visible_satellites = visibleSatellites;
    
            colirone_err_t err =  colirOne.rf.transmitSensorData(&sensorData, timestamp);
            if(err != COLIRONE_OK){
                printf("Error transmitting sensor data: %d\n", err);
            }
            sensorData = {0}; // Reset sensor data after transmission
            if(timestamp - lastRxMode > 100){
                printf("Switching to RX mode\n");
                commonPacket.is_rocket_rx = 1;
                colirOne.rf.transmitData((uint8_t*)&commonPacket, sizeof(commonPacket));
                colirOne.rf.setRxMode();
                lastRxMode = colirOne.getTimeStamp();
            }
        }
        else{
            if(colirOne.rf.hasReceivedData()){
                colirOne.rf.readColirOneCommand();
                uint8_t launchNumber = colirOne.rf.getLighterLaunchNumber();
                uint8_t openShutes = colirOne.rf.getOpenShutes();
                uint8_t closeShutes = colirOne.rf.getCloseShutes();
                uint8_t startLogs = colirOne.rf.getStartLogs();
                uint8_t writeLogs = colirOne.rf.getWriteLogs();
                uint8_t resetAltitude = colirOne.rf.getResetAltitude();
                uint8_t removeLogs = colirOne.rf.getRemoveLogs();

                printf("Received Command: Launch Number: %d, Open Shutes: %d, Close Shutes: %d, Start Logs: %d, Write Logs: %d, Reset Altitude: %d, Remove Logs: %d\n",
                       launchNumber, openShutes, closeShutes, startLogs, writeLogs, resetAltitude, removeLogs);
            }
            else if(timestamp - lastRxMode > 500){
                printf("Switching to TX mode\n");
                colirOne.rf.setTxMode();
                commonPacket.is_rocket_rx = 0;
                colirOne.rf.transmitData((uint8_t*)&commonPacket, sizeof(commonPacket));
                lastRxMode = colirOne.getTimeStamp();
            }
        }
        
        // HAL_Delay(200);
	}
	return 0;
}