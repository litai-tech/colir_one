#include "stdio.h"

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
        int visible_satellites;
    } gps;
    float vertical_velocity; // m/s
} colirone_payload_sensor_t;

typedef struct __attribute__((packed)) {
  uint8_t lighter_launch_number;
	uint8_t close_shutes;
	uint8_t open_shutes;
	uint8_t start_logs;
	uint8_t write_logs;
	uint8_t reset_altitude;
	uint8_t remove_logs;
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