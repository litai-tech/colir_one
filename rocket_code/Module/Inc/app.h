#ifndef _APP_H_
#define _APP_H_
#include <stdint.h>

#define DEBUG_UART      USART1

#ifndef DEBUG_LEVEL
// Enable the CDC ACM USB interface & debug logs (DEBUG_LEVEL > 0)
#define DEBUG_LEVEL     1
#endif

#define NON_BLOCK_PRINTING 0

#define ENABLE_SERIAL_PRINTING

#endif