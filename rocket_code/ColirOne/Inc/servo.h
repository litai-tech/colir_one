#ifndef __SERVO_H__
#define __SERVO_H__

#include "stdio.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C"{
#endif
class Servo {
    public: 
        Servo();
        uint8_t init(void);
        uint8_t setServoAngle(uint8_t servoNumber, float servoAngle);
};

#ifdef __cplusplus
}
#endif

#endif