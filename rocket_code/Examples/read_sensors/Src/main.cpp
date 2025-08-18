#include "main.h"
#include "../../ColirOne/Inc/colir_one.h"

int main(void){
  ColirOne colirOne;
  colirOne.init();
	while(1){
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
      float altitude = colirOne.barometer.calculateSeaLevelAltitude(pressure);
      printf("Altitude: %.2f\n", altitude);
    }

    colirOne.gps.run();
    double longitude = colirOne.gps.getLongitude();
    double latitude = colirOne.gps.getLatitude();
    int visibleSatellites = colirOne.gps.getVisibleSatellites();
    printf("GPS Longitude: %.6f, Latitude: %.6f, Visible Satellites: %d\n", longitude, latitude, visibleSatellites);
    HAL_Delay(5000);
	}
	return 0;
}