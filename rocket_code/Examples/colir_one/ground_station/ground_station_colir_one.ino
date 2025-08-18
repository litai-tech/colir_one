#include "config.h"
#include "nrf24Driver.h"
#include "buttonDriver.h"
#include "oledDriver.h"
#include "ws2812bDriver.h"
#include "basic.h"

void setup()
{
  Serial.begin(115200);
  ws2812bInit();
  nrf24Init();
  buttonInit();
  oledInit();
  ws2812bStartFade(WS2812B_RED);
  delay(1000);
}

void loop()
{
    uint8_t buffer[32];
    unsigned long now = millis();
    buttonProcess(now);
    
    if (nrf24Available()) {
        ws2812bSetColor(WS2812B_GREEN);
        nrf24Read(buffer, sizeof(buffer));
      
        uint8_t packet_type = buffer[0];
        switch (packet_type) {
            case 1: {
                colirone_common_packet_t common_packet;
                memcpy(&common_packet, buffer, sizeof(colirone_common_packet_t));
                Serial.print("Received a common packet: ");
                Serial.print("Type: "); Serial.println(common_packet.type);
                Serial.print("Is Rocket RX: "); Serial.println(common_packet.is_rocket_rx);
                if(common_packet.is_rocket_rx) nrf24ProcessCMD();
                break;
            }
            case 0: { //sensor packet
                colirone_payload_sensor_t sensor_data;
                sensor_packet_t sensor_packet;
                memcpy(&sensor_packet, buffer, sizeof(sensor_packet_t));
                switch(sensor_packet.index) {
                    case RF_ACCELERATION:
                        memcpy(&sensor_data.acceleration, sensor_packet.data, sizeof(XYZ_t));
                        Serial.print("Accel: ");
                        Serial.print(sensor_data.acceleration.x); Serial.print(", ");
                        Serial.print(sensor_data.acceleration.y); Serial.print(", ");
                        Serial.println(sensor_data.acceleration.z);
                        break;
                    case RF_GYROSCOPE:
                        memcpy(&sensor_data.gyroscope, sensor_packet.data, sizeof(XYZ_t));
                        Serial.print("Gyro: ");
                        Serial.print(sensor_data.gyroscope.x); Serial.print(", ");
                        Serial.print(sensor_data.gyroscope.y); Serial.print(", ");
                        Serial.println(sensor_data.gyroscope.z);
                        break;
                    case RF_ORIENTATION:
                        memcpy(&sensor_data.orientation, sensor_packet.data, sizeof(XYZ_t));
                        Serial.print("Orientation: ");
                        Serial.print(sensor_data.orientation.x); Serial.print(", ");
                        Serial.print(sensor_data.orientation.y); Serial.print(", ");
                        Serial.println(sensor_data.orientation.z);
                        break;
                    case RF_QUATERNION:
                        memcpy(&sensor_data.quaternion, sensor_packet.data, sizeof(Quaternion_t));
                        Serial.print("Quaternion: ");
                        Serial.print(sensor_data.quaternion.w); Serial.print(", ");
                        Serial.print(sensor_data.quaternion.x); Serial.print(", ");
                        Serial.print(sensor_data.quaternion.y); Serial.print(", ");
                        Serial.println(sensor_data.quaternion.z);
                        break;
                    case RF_BAROMETER:
                        memcpy(&sensor_data.barometer, sensor_packet.data, sizeof(sensor_data.barometer));
                        setAltitude(sensor_data.barometer.altitude);
                        Serial.print("Barometer: ");
                        Serial.print(sensor_data.barometer.temperature); Serial.print(", ");
                        Serial.print(sensor_data.barometer.pressure); Serial.print(", ");
                        Serial.println(sensor_data.barometer.altitude);
                        break;
                    case RF_GPS:
                        memcpy(&sensor_data.gps, sensor_packet.data, sizeof(sensor_data.gps));
                        Serial.print("GPS: ");
                        Serial.print(sensor_data.gps.longitude, 6); Serial.print(", ");
                        Serial.print(sensor_data.gps.latitude, 6); Serial.print(", ");
                        Serial.println(sensor_data.gps.visible_satellites);
                        break;
                    case RF_VERTICAL_VELOCITY:
                        memcpy(&sensor_data.vertical_velocity, sensor_packet.data, sizeof(float));
                        setVerticalVelocity(sensor_data.vertical_velocity);
                        Serial.print("Vertical velocity: ");
                        Serial.println(sensor_data.vertical_velocity);
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                Serial.println("Unknown packet type received.");
                break;
        }
        if(getCurrentScreen() != CONFIRM_SCREEN){
          oledMainScreenDisplay(getVerticalVelocity(), getAltitude());
        }
    }
    else{
      ws2812bFadeUpdate();
      connectingScreenDisplayBounce();
    }
    delay(1);
}

/* ----------------------------------------------------------------MAIN PROGRAM---------------------------------------------------------------- */