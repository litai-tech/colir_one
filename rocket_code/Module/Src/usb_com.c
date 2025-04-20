#include "usb_com.h"
#include "stdlib.h"
#include "usbd_cdc_if.h"

usb_com_t usb_com;

void usb_transmit(uint8_t *tx_buffer, uint16_t tx_len)
{
	// printf("Transmitting Frame to USB: ");
	CDC_Transmit_FS(tx_buffer, tx_len);
}

void usb_receive(uint8_t *rx_buffer, uint32_t rx_len)
{
	memset(usb_com.rx_buffer, 0, MAX_RECEIVE_BUFFER_LENGTH);
	memcpy((char*)usb_com.rx_buffer, (char*)rx_buffer, rx_len);

	printf("Received Frame from USB: ");

	for (int idx = 0; idx < rx_len; idx++)
	{
		printf("%c", usb_com.rx_buffer[idx]);
	}

	printf("\n");

}