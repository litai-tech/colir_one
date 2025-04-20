#include "uart.h"
#include "stm32f4xx_hal_uart.h"

extern UART_HandleTypeDef huart1;

void uart1_transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout){
    HAL_UART_Transmit(&huart1, pData, Size, Timeout);
}
