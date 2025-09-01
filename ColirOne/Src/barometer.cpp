/*
 * barometer.cpp
 *
 *  Created on: April 21, 2025
 *      Author: Dat Hoang
 */


#include "barometer.h"
#include "stm32f4xx_hal.h"
#include "../Inc/app.h"

extern SPI_HandleTypeDef hspi1;

Barometer::Barometer() {
    zeroAltitude = 0;
    bmpData = {0};
}

colirone_err_t Barometer::init(void) {
    beginSPI();
    int8_t err = getSensorData(&bmpData);
    if(err != BMP5_OK) {
        return COLIRONE_ERROR;
    }
    zeroAltitude = calcAltitude(bmpData.pressure);
    return COLIRONE_OK;
}

float Barometer::getPressure(void) {
    int8_t err = getSensorData(&bmpData);
    if(err != BMP5_OK) {
        printf("Error getting pressure: %d\n", err);
    }
    return bmpData.pressure;
}

float Barometer::getTemperature(void) {
    int8_t err = getSensorData(&bmpData);
    if(err != BMP5_OK) {
        printf("Error getting pressure: %d\n", err);
    }
    return bmpData.temperature;
}

float Barometer::calculateAltitude(float pressure) {
    return calcAltitude(pressure);
}

void Barometer::resetZeroAltitude(void) {
    float pressure = getPressure();
    zeroAltitude = calculateAltitude(pressure);
}

float Barometer::calculateSeaLevelAltitude(float pressure) {
    return calculateAltitude(pressure) - zeroAltitude;
}