/*
 * usb_com.h
 *
 *  Created on: April 28, 2025
 *      Author: Dat Hoang
 */
#ifndef __USB_COM_H__
#define __USB_COM_H__

#include <stdint.h>
#include <stdio.h>

#define USB_DEBUG(log_msg) (printf("USB_Debug: %s\n", log_msg))

#define MAX_RECEIVE_BUFFER_LENGTH    ( 2048 )
#define MAX_TRANSMIT_BUFFER_LENGTH   ( 2048 )

typedef struct
{
	unsigned char rx_buffer[MAX_RECEIVE_BUFFER_LENGTH];
	unsigned char tx_buffer[MAX_TRANSMIT_BUFFER_LENGTH];
}usb_com_t;

#ifdef __cplusplus
extern "C" {
#endif
void usb_transmit(uint8_t *tx_buffer, uint16_t tx_len);
void usb_receive(uint8_t *rx_buffer, uint32_t rx_len);
#ifdef __cplusplus
}
#endif

#endif