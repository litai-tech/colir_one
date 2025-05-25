#include "main.h"
#include "../../ColirOne/Inc/colir_one.h"

const uint8_t TxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xAA};

int main(void){
  	ColirOne colirOne;
  	colirone_err_t err = colirOne.init();
    if(err != COLIRONE_OK){
        printf("Error initializing colirone: %d\n", err);
        return -1;
    }
    err = colirOne.rf.setTxAdress((uint8_t*)TxAddress);
    colirOne.rf.setTxMode();
    colirone_payload_sensor_t sensorData;
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
        sensorData = {0};
        HAL_Delay(1000);
	}
	return 0;
}