#ifndef __NRF24_DRIVER_H__
#define __NRF24_DRIVER_H__

#include "config.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

void nrf24Init(const uint8_t* RxAddress, const uint8_t* TxAddress);
bool nrf24Available(void);
void nrf24Read(uint8_t* buffer, size_t len);

void nrf24EnqueueCMD(colirone_payload_cmd_t command);
void nrf24ProcessCMD(void);

#ifdef __cplusplus
}
#endif

#endif