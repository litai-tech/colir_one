#ifndef __UART_H__
#define __UART_H__

#include "stdio.h"
#include "stdint.h"
#include "stm32f4xx_hal.h"
#ifdef __cplusplus
extern "C" {
#endif
void uart1_transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout);
#ifdef __cplusplus
}
#endif
#endif