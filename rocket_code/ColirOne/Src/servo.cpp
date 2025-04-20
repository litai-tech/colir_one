#include "servo.h"
#include "../../Module/Inc/pca9685.h"
#include "stm32f4xx_hal.h"
#include "../Inc/app.h"

extern I2C_HandleTypeDef hi2c2;

Servo::Servo(void){
}

uint8_t Servo::init(void){
    COLIRONE_CHECK_ERROR(PCA9685_Init(&hi2c2));
    #ifndef PCA9685_SERVO_MODE
        COLIRONR_CHECK_ERROR(PCA9685_SetPwmFrequency(50));
    #endif
    return COLIRONE_OK;
}

uint8_t Servo::setServoAngle(uint8_t servoNumber, float servoAngle){
    if(servoNumber > 8) return COLIRONE_ERROR;
    if(servoAngle < 0) servoAngle = 0;
    if(servoAngle > 180) servoAngle = 180;

    COLIRONE_CHECK_ERROR(PCA9685_SetServoAngle(servoNumber, (float)servoAngle));
    return COLIRONE_OK;
}