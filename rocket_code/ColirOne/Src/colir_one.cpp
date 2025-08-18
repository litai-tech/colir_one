/*
 * colir_one.cpp
 *
 *  Created on: April 21, 2025
 *      Author: Dat Hoang
 */

#include "colir_one.h"
#include "../../Module/Inc/init.h"

ColirOne::ColirOne() {
    // Constructor implementation
}

colirone_err_t ColirOne::init(void) {
    // Initialize all components
    stm32_init();
    colirone_err_t err = COLIRONE_OK;
    err |= imu.init();
    if(err != COLIRONE_OK) {
        printf("IMU initialization failed\n");
        return err;
    }
    printf("IMU initialized.\n");
    err |= barometer.init();
    if(err != COLIRONE_OK) {
        printf("Barometer initialization failed\n");
        return err;
    }
    printf("Barometer initialized.\n");
    gps.init();
    printf("GPS initialized.\n");
    rf.init();
    printf("RF initialized.\n");
    err |= logger.init();
    if(err != COLIRONE_OK) {
        printf("W25Qxx initialization failed\n");
        return err;
    }
    printf("W25Qxx initialized.\n");
    return err;
}

uint32_t ColirOne::getTimeStamp(void) {
    return HAL_GetTick();
}