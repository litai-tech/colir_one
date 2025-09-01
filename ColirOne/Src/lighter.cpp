/*
 * lighter.cpp
 *
 *  Created on: April 21, 2025
 *      Author: Dat Hoang
 */

#include "lighter.h"
#include "stm32f4xx_hal.h"

Lighter::Lighter() {
}

//Assuming lighterNumber is 1-8
void Lighter::fireLighter(uint8_t lighterNumber) {
    uint16_t pinNumber = 0 | (1<<(6+lighterNumber - 1)); // Assuming lighterNumber is 1-8, and pins are PE6 to PE13
	HAL_GPIO_WritePin(GPIOE, pinNumber, GPIO_PIN_SET);
	HAL_Delay(15);
	HAL_GPIO_WritePin(GPIOE, pinNumber, GPIO_PIN_RESET);   
}

