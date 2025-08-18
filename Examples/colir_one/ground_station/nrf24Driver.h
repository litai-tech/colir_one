#ifndef __NRF24_DRIVER_H__
#define __NRF24_DRIVER_H__

#include "config.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief RF24 TX,RX Address
 */
const uint8_t RxAddress[]            =        {0xAA,0xDD,0xCC,0xBB,0xAA};
const uint8_t TxAddress[]            =        {0xEE,0xDD,0xCC,0xBB,0xAA};

void nrf24Init(void);
bool nrf24Available(void);
void nrf24Read(uint8_t* buffer, size_t len);

void nrf24EnqueueCMD(colirone_payload_cmd_t command);
void nrf24ProcessCMD(void);

#ifdef __cplusplus
}
#endif

#endif