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
    // err |= imu.init();
    // err |= barometer.init();
    // gps.init();
    rf.init();
    return err;
}

uint32_t ColirOne::getTimeStamp(void) {
    return HAL_GetTick();
}