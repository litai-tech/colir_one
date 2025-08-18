#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "InputDebounce.h"

/* ----------------------------------------------------------------MACROS AND DEFINES---------------------------------------------------------------- */
/**
 * @brief Button definition
 */
#define BUTTON_1                            13
#define BUTTON_2                            14
#define BUTTON_DEBOUNCE_DELAY               80
#define LONG_DURATION_PRESSED               5000

/**
 * @brief RF24 definition
 */
#define RF24_CE_PIN                         4
#define RF24_CSN_PIN                        5

#define QUEUE_SIZE                          10

/* ----------------------------------------------------------------MACROS AND DEFINES---------------------------------------------------------------- */

/* ----------------------------------------------------------------CONST VARIABLES---------------------------------------------------------------- */
const uint8_t RxAddress[]            =        {0xAA,0xDD,0xCC,0xBB,0xAA};
const uint8_t TxAddress[]            =        {0xEE,0xDD,0xCC,0xBB,0xAA};
/* ----------------------------------------------------------------CONST VARIABLES---------------------------------------------------------------- */
/* ----------------------------------------------------------------OBJECT---------------------------------------------------------------- */
typedef enum{
    RF_ACCELERATION = 0,
    RF_GYROSCOPE,
    RF_ORIENTATION,
    RF_QUATERNION,
    RF_BAROMETER,
    RF_GPS,
    RF_VERTICAL_VELOCITY,
} rf_packet_type_t;

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
  uint8_t lighterLaunchNumber;
	uint8_t closeShutes;
	uint8_t openShutes;
	uint8_t startLogs;
	uint8_t writeLogs;
	uint8_t resetAltitude;
	uint8_t removeLogs;
} colirone_payload_cmd_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t index;      
    uint32_t timestamp;
    uint8_t data[26]; // 32 - 1 - 5 = 26
} sensor_packet_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t is_rocket_rx;
} colirone_common_packet_t;

colirone_payload_sensor_t sensor_data;
colirone_common_packet_t common_packet;
RF24 radio(RF24_CE_PIN, RF24_CSN_PIN); // CE, CSN
InputDebounce openShutesButton;
InputDebounce closeShutesButton;
/* ----------------------------------------------------------------OBJECT---------------------------------------------------------------- */

/* ----------------------------------------------------------------GLOBAL FUNCTION PROTOTYPES---------------------------------------------------------------- */
void transmit_launch_cmd(void);
void transmit_open_shutes_cmd(void);
void transmit_close_shutes_cmd(void);

/* ----------------------------------------------------------------GLOBAL FUNCTION PROTOTYPES---------------------------------------------------------------- */

/* ----------------------------------------------------------------GLOBAL VARIABLES---------------------------------------------------------------- */
colirone_payload_cmd_t colirone_payload_cmd_queue[QUEUE_SIZE];
int queueHead = 0;
int queueTail = 0;
/* ----------------------------------------------------------------GLOBAL VARIABLES---------------------------------------------------------------- */

/* ----------------------------------------------------------------MAIN PROGRAM---------------------------------------------------------------- */
void setup()
{
  Serial.begin(115200);
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(5,5);
  radio.disableCRC();
  radio.openReadingPipe(0, RxAddress);
  radio.openWritingPipe(TxAddress);
  radio.startListening();
    // Button init
  openShutesButton.registerCallbacks(buttonPressedCallback, buttonReleasedCallback);
  closeShutesButton.registerCallbacks(buttonPressedCallback, buttonReleasedCallback);
  openShutesButton.setup(BUTTON_1, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);
  closeShutesButton.setup(BUTTON_2, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);
}

void loop()
{
    uint8_t buffer[32];
    unsigned long now = millis();
    openShutesButton.process(now);
    closeShutesButton.process(now);
    
    if (radio.available()) {
        radio.read(buffer, sizeof(buffer));
      
        uint8_t packet_type = buffer[0];
        switch (packet_type) {
            case 1: {
                colirone_common_packet_t common_packet;
                memcpy(&common_packet, buffer, sizeof(colirone_common_packet_t));
                Serial.print("Received a common packet: ");
                Serial.print("Type: "); Serial.println(common_packet.type);
                Serial.print("Is Rocket RX: "); Serial.println(common_packet.is_rocket_rx);
                if(common_packet.is_rocket_rx) process_cmd();
                break;
            }
            case 0: { //sensor packet
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
                        Serial.println(sensor_data.gps.visibleSatellites);
                        break;
                    case RF_VERTICAL_VELOCITY:
                        memcpy(&sensor_data.verticalVelocity, sensor_packet.data, sizeof(float));
                        Serial.print("Vertical velocity: ");
                        Serial.println(sensor_data.verticalVelocity);
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
    }
    delay(1);
}

/* ----------------------------------------------------------------MAIN PROGRAM---------------------------------------------------------------- */

void buttonPressedCallback(uint8_t pinIn)
{
}

void buttonReleasedCallback(uint8_t pinIn)
{
  if(pinIn == BUTTON_1){
      colirone_payload_cmd_t colirone_payload_cmd = {0};
      colirone_payload_cmd.lighterLaunchNumber = 1;
      enqueue_cmd(colirone_payload_cmd);
  }
  else if(pinIn == BUTTON_2){
      colirone_payload_cmd_t colirone_payload_cmd = {0};
      colirone_payload_cmd.openShutes = 1;
      enqueue_cmd(colirone_payload_cmd);
  }
}

void enqueue_cmd(colirone_payload_cmd_t command) {
    if ((queueTail + 1) % QUEUE_SIZE != queueHead) {
        colirone_payload_cmd_queue[queueTail] = command;
        queueTail = (queueTail + 1) % QUEUE_SIZE;
    } else {
        Serial.println("queue is full!");
    }
}

void process_cmd() {
    if (queueHead != queueTail) {
        colirone_payload_cmd_t colirone_payload_cmd = colirone_payload_cmd_queue[queueHead]; 
        queueHead = (queueHead + 1) % QUEUE_SIZE;
        transmit_cmd(colirone_payload_cmd);
    }
}

void transmit_cmd(colirone_payload_cmd_t colirone_payload_cmd){
  radio.stopListening();
  int ret = radio.write((uint8_t*)&colirone_payload_cmd, sizeof(colirone_payload_cmd));
  if(ret == 0){
    Serial.println("transmit_launch_cmd success");
  }
  delay(50);
  radio.startListening();  
}

/* ----------------------------------------------------------------GLOBAL FUNCTIONS---------------------------------------------------------------- */