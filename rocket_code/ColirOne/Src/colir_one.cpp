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

void ColirOne::init(void) {
    // Initialize all components
    stm32_init();
    imu.init();
    barometer.init();
    gps.init();
}