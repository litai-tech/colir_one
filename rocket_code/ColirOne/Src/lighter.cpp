#include "lighter.h"
#include "stm32f4xx_hal.h"

Lighter::Lighter() {
}

void Lighter::fireLighter(uint8_t lighterNumber) {
    uint16_t pinNumber = 0 | (1<<(6+lighterNumber));
	HAL_GPIO_WritePin(GPIOE, pinNumber, GPIO_PIN_SET);
	HAL_Delay(15);
	HAL_GPIO_WritePin(GPIOE, pinNumber, GPIO_PIN_RESET);   
}