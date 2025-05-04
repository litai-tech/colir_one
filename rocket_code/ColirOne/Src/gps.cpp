/*
 * gps.cpp
 *
 *  Created on: April 21, 2025
 *      Author: Dat Hoang
 */

#include "gps.h"

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;

uint8_t DataBuffer[512];
uint8_t RxBuffer[64];
uint16_t oldPos = 0;
uint16_t newPos = 0;

GPS::GPS(void){
    myGPS = {0};
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    oldPos = newPos; //keep track of the last position in the buffer
    if(oldPos + Size > 512){ //if the buffer is full, parse it, then reset the buffer

        uint16_t datatocopy = 512-oldPos;  // find out how much space is left in the main buffer
        memcpy ((uint8_t *)DataBuffer+oldPos, RxBuffer, datatocopy);  // copy data in that remaining space

        oldPos = 0;  // point to the start of the buffer
        memcpy ((uint8_t *)DataBuffer, (uint8_t *)RxBuffer+datatocopy, (Size-datatocopy));  // copy the remaining data
        newPos = (Size-datatocopy);  // update the position
    }
    else{
        memcpy((uint8_t *)DataBuffer+oldPos, RxBuffer, Size); //copy received data to the buffer
        newPos = Size+oldPos; //update buffer position

    }
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)RxBuffer, 64); //re-enable the DMA interrupt
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT); //disable the half transfer interrupt
}

void GPS::init(void){
    // Initialize GPS module if needed
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)RxBuffer, 64);
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
}

void GPS::run(void){
    nmea_parse(&myGPS, DataBuffer);
}

double GPS::getLongitude(void){
    return myGPS.longitude;
}

double GPS::getLatitude(void){
    return myGPS.latitude;
}

int GPS::getVisibleSatellites(void){
    return myGPS.satelliteCount;
}