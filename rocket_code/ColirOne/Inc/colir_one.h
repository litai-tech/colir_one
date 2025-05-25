/*
 * colir_one.h
 *
 *  Created on: April 21, 2025
 *      Author: Dat Hoang
 */
#ifndef __ColirOne__
#define __ColirOne__

#include "stdio.h"
#include "stdint.h"
#include "imu.h"
#include "gps.h"
#include "barometer.h"
#include "servo.h"
#include "lighter.h"
#include "rf.h"

#ifdef __cplusplus

#endif

#ifdef __cplusplus
extern "C"{
#endif

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

class ColirOne{
    public:
        ColirOne();
        IMU imu;
        Barometer barometer;
        GPS gps;
        Logger logger;
        RF rf;
        Servo servo;
        Lighter lighter;
        colirone_err_t init(void);
        void base(void);
        uint32_t getTimeStamp(void);
    private:
        void setState(void);
};


#ifdef __cplusplus
}
#endif

#endif