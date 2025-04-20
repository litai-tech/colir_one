#include "barometer.h"
#include "stm32f4xx_hal.h"
#include "../Inc/app.h"

extern SPI_HandleTypeDef hspi1;

Barometer::Barometer() {
    zeroAltitude = 0;
    bmpData = {0};
}

uint8_t Barometer::init(void) {
    HAL_SPI_Init(&hspi1);
    
    uint32_t startTime = HAL_GetTick();
    while (beginSPI() != BMP5_OK) {
        // Check if 5 seconds have passed
        if (HAL_GetTick() - startTime > 10000) {
            HAL_SPI_DeInit(&hspi1);
            return COLIRONE_ERROR;
        }
        HAL_Delay(5000);
    }
    COLIRONE_CHECK_ERROR(getSensorData(&bmpData));
    zeroAltitude = calcAltitude(bmpData.pressure);
    
    HAL_SPI_DeInit(&hspi1);
    return COLIRONE_OK;
}

float Barometer::getPressure(void) {
    HAL_SPI_Init(&hspi1);
    COLIRONE_CHECK_ERROR(getSensorData(&bmpData));
    HAL_SPI_DeInit(&hspi1);
    return bmpData.pressure;
}

float Barometer::getTemperature(void) {
    HAL_SPI_Init(&hspi1);
    COLIRONE_CHECK_ERROR(getSensorData(&bmpData));
    HAL_SPI_DeInit(&hspi1);
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