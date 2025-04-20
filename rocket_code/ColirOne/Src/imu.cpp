#include "imu.h"
#include "../../Module/Inc/bno055_stm32.h"

#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef hi2c2;

IMU::IMU(void){
}

void IMU::init(void){
    bno055_assignI2C(&hi2c2);
    bno055_setup();
    bno055_setOperationModeNDOF();
}

XYZ_t IMU::getAcceleration(void){
    XYZ_t acceleration;
    bno055_vector_t linearAccel = bno055_getVectorLinearAccel();
    acceleration.x = linearAccel.x;
    acceleration.y = linearAccel.y;
    acceleration.z = linearAccel.z;
    return acceleration;
}

XYZ_t IMU::getGyroscope(void){
    XYZ_t gyroscope;
    bno055_vector_t gyro = bno055_getVectorGyroscope();
    gyroscope.x = gyro.x;
    gyroscope.y = gyro.y;
    gyroscope.z = gyro.z;
    return gyroscope;
}

XYZ_t IMU::getOrientation(void){
    XYZ_t orientation;
    bno055_vector_t orientationVector = bno055_getVectorEuler();
    orientation.x = orientationVector.x;
    orientation.y = orientationVector.y;
    orientation.z = orientationVector.z;
    return orientation;
}

Quaternion_t IMU::getQuaternion(void){
    Quaternion_t quaternion;
    bno055_vector_t quaternionVector = bno055_getVectorQuaternion();
    quaternion.w = quaternionVector.w;
    quaternion.x = quaternionVector.x;
    quaternion.y = quaternionVector.y;
    quaternion.z = quaternionVector.z;
    return quaternion;
}
