#ifndef __RF_H__
#define __RF_H__

#include "stdint.h"
#include "stdio.h"
#include "app.h"
#include "imu.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct __attribute__((packed)) {
	struct {
		uint8_t number;
	} lighter_launch;
	uint8_t open_shutes;
	uint8_t start_logs;
	uint8_t write_logs;
	uint8_t reset_altitude;
	uint8_t remove_logs;
} colirone_payload_cmd_t;

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
    uint8_t index;      
    uint32_t timestamp; // 4 bytes for timestamp
    uint8_t data[32 - 1 - 4]; // 32 bytes total, minus 1 byte for index and 4 bytes for timestamp
} sensor_packet_t;

class RF {
    public:
        RF(void);
        colirone_err_t init(void);
        colirone_err_t setTxAdress(uint8_t *address);
        colirone_err_t setRxAdress(uint8_t *address);
        colirone_err_t setTxRxAdress(uint8_t *txAddress, uint8_t *rxAddress);
        colirone_err_t setTxMode(void);
        colirone_err_t setRxMode(void);
        colirone_err_t sendData(uint8_t *data, uint8_t size); //max 32 bytes
        bool hasReceivedData(void);
        uint8_t* getReceivedData(void);
        bool isTxMode(void);
        bool isRxMode(void);

        // Default packet for colir one
        void readColirOneCommand(void);
        uint8_t getLighterLaunchNumber(void);
        uint8_t getOpenShutes(void);
        uint8_t getStartLogs(void);
        uint8_t getWriteLogs(void);
        uint8_t getResetAltitude(void);
        uint8_t getRemoveLogs(void);

        colirone_err_t transmitSensorData(colirone_payload_sensor_t *sensor, uint32_t timestamp);
        
        private:
        uint8_t txAddress[5];
        uint8_t rxAddress[5];
        bool txMode;
        bool rxMode;
        colirone_payload_cmd_t colirone_payload_cmd;
        colirone_payload_sensor_t colirone_payload_sensor;
};
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#endif

#endif