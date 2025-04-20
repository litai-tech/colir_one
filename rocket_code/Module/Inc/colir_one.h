// Filename: Core/Inc/ColirOne.h
//   Author: Dat Hoang
//  Created: 11/28/2024 22:30:00 +07:00
#ifndef __ColirOne__
#define __ColirOne__

#include "stdio.h"
#include "stdint.h"

#ifdef __cplusplus

typedef struct {
    double x;
    double y;
    double z;
} XYZ_t;

typedef struct {
    double w;
    double x;
    double y;
    double z;
} Quaternion_t;

typedef struct {
    double longitude;
    double latitude;
    int visibleSatellites;
} GPS_State_t;

class GPS_State {
    public:
        double longitude;
        double latitude;
        int visibleSatellites;
};

class ImuState{
    public:
        XYZ_t Orientation;
        XYZ_t Acceleration;
        XYZ_t Gyroscope;
        XYZ_t Magnetometer;
        Quaternion_t quaternion;    
};

class IMU {
    public: 
        XYZ_t getOrientation();
        XYZ_t getAcceleration();
        XYZ_t getGyroscope();
        XYZ_t getMagnetometer();
        Quaternion_t getQuaternion();
};

class Barometer {
    public:
        int64_t getPressure();
        double calculateAltitude();
        double calculateSeaLevelAltitude(int64_t pressure);
        void resetZeroAltitude();
        int64_t getTemperature();
    private :
        double zeroAltitude;
};

class LoggerState {
    public: 
        bool WritingToLogsEnabled;
        uint8_t StoredLogsSessions;
};

class Logger {
    public: 
        void logData(char* data);//max 32 bytes
        bool writeLogsToSD();//return true if write to SD succeeded
        void eraseLogs();
};

class RfState {
    public: 
        bool IsTxMode;
        bool IsRxMode;
        char* RxAddress;
        char* TxAddress;
};

class RF {
    public: 
        void setTxAddress(char* txAddress);//max 5 bytes
        void setRxAddress(char* rxAddress);//max 5 bytes
        void setTxMode();
        void setRxMode();
        void sendData(char* data);//max 32 bytes
        bool hasReceivedData();
        char* getReceivedData();
};

class Servo {
    public: 
        void setServoAngle(uint8_t servoNumber, int servoAngle);
};

class Lighter {
    public: 
        void FireLighter(uint8_t lighterNumber);
};


class ColirOne{
    public:
        ColirOne();
        IMU imu;
        Barometer barometer;
        GPS_State gps_state;
        Logger logger;
        RF rf;
        Servo servo;
        Lighter lighter;
        void init(void);
        void base(void);
    private:
        void setState(void);
};

#endif

#ifdef __cplusplus
extern "C"{
#endif


#ifdef __cplusplus
}
#endif

#endif