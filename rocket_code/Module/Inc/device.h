#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <stdint.h>
/** Return a millisecond timestamp.  Does not need to be synchronized to anything.
 *  *Optional* to compile, but will not calculate delays correctly without a correct implementation.
*/
uint32_t millis();
 
typedef enum {
    COLIRONE_READY_TO_LAUNCH = 0b00000001,
    COLIRONE_CRUISE = 0b00000010,
    COLIRONE_MOTOR_STOPPED = 0b00000100,
    COLIRONE_FALLING = 0b00001000,
    COLIRONE_SHUTES_DEPLOYED = 0b00010000
} colir_one_rocket_state;

typedef struct {
    uint32_t last_address;
    uint32_t log_address[32];
    uint8_t last_log;
} flash_config;

void colir_one_init(SPI_HandleTypeDef *hspi);
void log_data(char cmd[]);
void read_logs_to_sd();
void reset_logs();
flash_config* get_logs_config();
  

#endif