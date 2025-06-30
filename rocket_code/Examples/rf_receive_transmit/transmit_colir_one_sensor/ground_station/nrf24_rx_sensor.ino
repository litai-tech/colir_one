#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(4, 5);  // CE, CSN

uint8_t RxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xAA};

typedef struct __attribute__((packed)) {
    float x;
    float y;
    float z;
} XYZ_t;

typedef struct __attribute__((packed)) {
    float w;
    float x;
    float y;
    float z;
} Quaternion_t;

typedef struct __attribute__((packed)) {
    XYZ_t acceleration;
    XYZ_t gyroscope;
    XYZ_t orientation;
    Quaternion_t quaternion;
    struct {
        float temperature;
        float pressure;
        float altitude;
    } barometer;
    struct {
        double longitude;
        double latitude;
        int visibleSatellites;
    } gps;
    float verticalVelocity; // m/s
} colirone_payload_sensor_t;

typedef struct __attribute__((packed)) {
    uint8_t index;      
    uint32_t timestamp;
    uint8_t data[27]; // 32 - 1 - 4 = 27
} sensor_packet_t;

colirone_payload_sensor_t sensor_data;

typedef struct __attribute__((packed)) {
    uint8_t lighterLaunchNumber;
	uint8_t openShutes;
	uint8_t closeShutes;
	uint8_t startLogs;
	uint8_t writeLogs;
	uint8_t resetAltitude;
	uint8_t removeLogs;
} colirone_payload_cmd_t;

void setup()
{
  Serial.begin(115200);
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(0, RxAddress);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(5,5);
  radio.disableCRC();
  radio.startListening();
}

void loop()
{
    sensor_packet_t packet;
    if (radio.available()) {
        radio.read(&packet, sizeof(packet));
        switch(packet.index) {
            case RF_ACCELERATION:
                memcpy(&sensor_data.acceleration, packet.data, sizeof(XYZ_t));
                Serial.print("Accel: ");
                Serial.print(sensor_data.acceleration.x); Serial.print(", ");
                Serial.print(sensor_data.acceleration.y); Serial.print(", ");
                Serial.println(sensor_data.acceleration.z);
                break;
            case RF_GYROSCOPE:
                memcpy(&sensor_data.gyroscope, packet.data, sizeof(XYZ_t));
                Serial.print("Gyro: ");
                Serial.print(sensor_data.gyroscope.x); Serial.print(", ");
                Serial.print(sensor_data.gyroscope.y); Serial.print(", ");
                Serial.println(sensor_data.gyroscope.z);
                break;
            case RF_ORIENTATION:
                memcpy(&sensor_data.orientation, packet.data, sizeof(XYZ_t));
                Serial.print("Orientation: ");
                Serial.print(sensor_data.orientation.x); Serial.print(", ");
                Serial.print(sensor_data.orientation.y); Serial.print(", ");
                Serial.println(sensor_data.orientation.z);
                break;
            case RF_QUATERNION:
                memcpy(&sensor_data.quaternion, packet.data, sizeof(Quaternion_t));
                Serial.print("Quaternion: ");
                Serial.print(sensor_data.quaternion.w); Serial.print(", ");
                Serial.print(sensor_data.quaternion.x); Serial.print(", ");
                Serial.print(sensor_data.quaternion.y); Serial.print(", ");
                Serial.println(sensor_data.quaternion.z);
                break;
            case RF_BAROMETER:
                memcpy(&sensor_data.barometer, packet.data, sizeof(sensor_data.barometer));
                Serial.print("Barometer: ");
                Serial.print(sensor_data.barometer.temperature); Serial.print(", ");
                Serial.print(sensor_data.barometer.pressure); Serial.print(", ");
                Serial.println(sensor_data.barometer.altitude);
                break;
            case RF_GPS:
                memcpy(&sensor_data.gps, packet.data, sizeof(sensor_data.gps));
                Serial.print("GPS: ");
                Serial.print(sensor_data.gps.longitude, 6); Serial.print(", ");
                Serial.print(sensor_data.gps.latitude, 6); Serial.print(", ");
                Serial.println(sensor_data.gps.visibleSatellites);
                break;
            case RF_VERTICAL_VELOCITY:
                memcpy(&sensor_data.verticalVelocity, packet.data, sizeof(float));
                Serial.print("Vertical velocity: ");
                Serial.println(sensor_data.verticalVelocity);
                break;
            default:
                Serial.println("Unknown packet index!");
                break;
        }
    }
}