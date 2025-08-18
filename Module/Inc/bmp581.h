/*
 * bmp581.h
 *
 *  Created on: Jun 14, 2024
 *      Author: Administrator
 */
#include "bmp5.h"
#include "bmp5_defs.h"

#ifndef INC_BMP581_H_
#define INC_BMP581_H_

#define BMP5_E_INVALID_SETTING (BMP5_E_NVM_NOT_READY-1)

#define BMP581_CSN_PORT   GPIOC
#define BMP581_CSN_PIN    GPIO_PIN_4

#ifdef __cplusplus
extern "C" {
#endif

int8_t beginSPI();
int8_t init();
int8_t setMode(enum bmp5_powermode mode);
int8_t getMode(enum bmp5_powermode* mode);
int8_t enablePress(uint8_t pressEnable);

// Data acquisistion
int8_t getSensorData(struct bmp5_sensor_data* data);
float calcAltitude(float pressure);
float kalman_filter(float inputValue);

// Output data rate (ODR) control
int8_t setODRFrequency(uint8_t odr);
int8_t getODRFrequency(uint8_t* odr);

// Oversampling (OSR) control
int8_t setOSRMultipliers(struct bmp5_osr_odr_press_config* config);
int8_t getOSRMultipliers(struct bmp5_osr_odr_press_config* config);
int8_t getOSREffective(struct bmp5_osr_odr_eff* osrOdrEffective);

// IIR filter control
int8_t setFilterConfig(struct bmp5_iir_config* iirConfig);
int8_t setOORConfig(struct bmp5_oor_press_configuration* oorConfig);

// Interrupt control
//int8_t setInterruptConfig(BMP581_InterruptConfig* config);
//int8_t getInterruptStatus(uint8_t* status);

// FIFO control
int8_t setFIFOConfig(struct bmp5_fifo* fifoConfig);
int8_t getFIFOLength(uint8_t* numData);
int8_t getFIFOData(struct bmp5_sensor_data* data, uint8_t numData);
int8_t flushFIFO();

// NVM control
int8_t readNVM(uint8_t addr, uint16_t* data);
int8_t writeNVM(uint8_t addr, uint16_t data);

#ifdef __cplusplus
}
#endif

#endif /* INC_BMP581_H_ */