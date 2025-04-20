#include "usbd_cdc_if.h"

#include "app.h"
#include "fifo.h"
#include "usb_com.h"
#include "uart.h"

#if DEBUG_LEVEL>0

// void _putchar(char c)
// {
// #if NON_BLOCK_PRINTING
//     fifo_debug_add(&c);
// #else
//     while (! LL_USART_IsActiveFlag_TXE(DEBUG_UART))
//         ;
//     LL_USART_TransmitData8(DEBUG_UART,c);
// #endif
// }


int _write (int fd, const void *buf, unsigned long int len)
{
    uint8_t * data = (uint8_t *) buf;
#if DEBUG_LEVEL>0
	// static uint8_t logbuf[1000] = {0};
	// static int logbuflen = 0;
	// if (logbuflen + len > sizeof(logbuf)) {
	// 	int mlen = logbuflen + len - sizeof(logbuf);
	// 	memmove(logbuf, &logbuf[mlen], sizeof(logbuf) - mlen);
	// 	logbuflen -= mlen;
	// }
	// memcpy(&logbuf[logbuflen], data, len);
	// logbuflen += len;

	// Send out USB serial
	usb_transmit(data, len);
	// if (res == USBD_OK)
	// 	logbuflen = 0;
#endif
#ifdef ENABLE_SERIAL_PRINTING
    // Send out UART serial
    // while(len--)
    // {
    //     _putchar(*data++);
    // }
    uart1_transmit(data, len, 1000);
#endif
    return 0;
}
#endif
