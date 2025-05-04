/*
 * imu.cpp
 *
 *  Created on: April 21, 2025
 *      Author: Dat Hoang
 */

#include "imu.h"
#include "../../Module/Inc/bno055.h"

#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef hi2c2;

IMU::IMU(void){
}

colirone_err_t IMU::init(void){
    bno055_assignI2C(&hi2c2);
    bno055_setup();
    bno055_setOperationModeNDOF();
    return COLIRONE_OK;
}

XYZ_t IMU::getAcceleration(void){
    XYZ_t acceleration;
    bno055_vector_t linearAccel;
    HAL_StatusTypeDef status = bno055_getVectorLinearAccel(&linearAccel);
    if(status != HAL_OK){
        printf("Error getting acceleration: %d\n", status);
    }
    acceleration.x = linearAccel.x;
    acceleration.y = linearAccel.y;
    acceleration.z = linearAccel.z;
    return acceleration;
}

XYZ_t IMU::getGyroscope(void){
    XYZ_t gyroscope;
    bno055_vector_t gyro;
    HAL_StatusTypeDef status = bno055_getVectorGyroscope(&gyro);
    if(status != HAL_OK){
        printf("Error getting gyroscope: %d\n", status);
    }
    gyroscope.x = gyro.x;
    gyroscope.y = gyro.y;
    gyroscope.z = gyro.z;
    return gyroscope;
}

XYZ_t IMU::getOrientation(void){
    XYZ_t orientation;
    bno055_vector_t orientationVector;
    HAL_StatusTypeDef status =  bno055_getVectorEuler(&orientationVector);
    if(status != HAL_OK){
        printf("Error getting orientation: %d\n", status);
    }
    orientation.x = orientationVector.x;
    orientation.y = orientationVector.y;
    orientation.z = orientationVector.z;
    return orientation;
}

Quaternion_t IMU::getQuaternion(void){
    Quaternion_t quaternion;
    bno055_vector_t quaternionVector;
    HAL_StatusTypeDef status = bno055_getVectorQuaternion(&quaternionVector);
    if(status != HAL_OK){
        printf("Error getting quaternion: %d\n", status);
    }
    quaternion.w = quaternionVector.w;
    quaternion.x = quaternionVector.x;
    quaternion.y = quaternionVector.y;
    quaternion.z = quaternionVector.z;
    return quaternion;
}
