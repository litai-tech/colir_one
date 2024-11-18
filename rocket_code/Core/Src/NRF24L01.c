/*
  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************

  File:		  NRF24L01.c
  Author:     ControllersTech.com
  Updated:    30th APRIL 2021

  ***************************************************************************************************************
  Copyright (C) 2017 ControllersTech.com

  This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
  of the GNU General Public License version 3 as published by the Free Software Foundation.
  This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
  or indirectly by this software, read more about this on the GNU General Public License.

  ***************************************************************************************************************
*/
#include "stm32f4xx_hal.h"
#include "NRF24L01.h"

extern SPI_HandleTypeDef hspi3;
#define NRF24_SPI &hspi3


// THESE VALUES ARE MANUALLY CONFIGURED FOR SPECIFIC STM32!!!!!!!!
#define NRF24_CE_PORT   GPIOB
#define NRF24_CE_PIN    GPIO_PIN_6
#define NRF24_CSN_PORT   GPIOB
#define NRF24_CSN_PIN    GPIO_PIN_7
#define nRF24_TEST_ADDR  "nRF24"


HAL_StatusTypeDef hal_result;
void CS_Select (void)
{
	HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_RESET);
}

void CS_UnSelect (void)
{
	HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_SET);
}


void CE_Enable (void)
{
	HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_SET);
}

void CE_Disable (void)
{
	HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_RESET);
}

// write a single byte to the particular register
void nrf24_WriteReg (uint8_t Reg, uint8_t Data)
{
	uint8_t buf[2];
	buf[0] = Reg|1<<5;
	buf[1] = Data;

	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_SPI_Transmit(NRF24_SPI, buf, 2, 1000);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}

//write multiple bytes starting from a particular register
void nrf24_WriteRegMulti (uint8_t Reg, uint8_t *data, int size)
{
	uint8_t buf[2];
	buf[0] = Reg|1<<5;
//	buf[1] = Data;

	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_SPI_Transmit(NRF24_SPI, buf, 1, 100);
	HAL_SPI_Transmit(NRF24_SPI, data, size, 1000);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}


uint8_t nrf24_ReadReg (uint8_t Reg)
{
	uint8_t data=0;

	// Pull the CS Pin LOW to select the device
	CS_Select();

	hal_result = HAL_SPI_Transmit(NRF24_SPI, &Reg, 1, 100);
	hal_result = HAL_SPI_Receive(NRF24_SPI, &data, 1, 100);

	// Pull the CS HIGH to release the device
	CS_UnSelect();

	return data;
}


/* Read multiple bytes from the register */
void nrf24_ReadReg_Multi (uint8_t Reg, uint8_t *data, int size)
{
	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_SPI_Transmit(NRF24_SPI, &Reg, 1, 100);
	HAL_SPI_Receive(NRF24_SPI, data, size, 1000);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}


// send the command to the NRF
void nrfsendCmd (uint8_t cmd)
{
	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_SPI_Transmit(NRF24_SPI, &cmd, 1, 100);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}

void nrf24_reset(uint8_t REG)
{
	if (REG == STATUS)
	{
		nrf24_WriteReg(STATUS, 0x00);
	}

	else if (REG == FIFO_STATUS)
	{
		nrf24_WriteReg(FIFO_STATUS, 0x11);
	}

	else {
	nrf24_WriteReg(CONFIG, 0x08);
	nrf24_WriteReg(EN_AA, 0x3F);
	nrf24_WriteReg(EN_RXADDR, 0x03);
	nrf24_WriteReg(SETUP_AW, 0x03);
	nrf24_WriteReg(SETUP_RETR, 0x00);
	nrf24_WriteReg(RF_CH, 0x02);
	nrf24_WriteReg(RF_SETUP, 0x0E);
	nrf24_WriteReg(STATUS, 0x00);
	nrf24_WriteReg(OBSERVE_TX, 0x00);
	nrf24_WriteReg(CD, 0x00);
	uint8_t rx_addr_p0_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	nrf24_WriteRegMulti(RX_ADDR_P0, rx_addr_p0_def, 5);
	uint8_t rx_addr_p1_def[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
	nrf24_WriteRegMulti(RX_ADDR_P1, rx_addr_p1_def, 5);
	nrf24_WriteReg(RX_ADDR_P2, 0xC3);
	nrf24_WriteReg(RX_ADDR_P3, 0xC4);
	nrf24_WriteReg(RX_ADDR_P4, 0xC5);
	nrf24_WriteReg(RX_ADDR_P5, 0xC6);
	uint8_t tx_addr_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	nrf24_WriteRegMulti(TX_ADDR, tx_addr_def, 5);
	nrf24_WriteReg(RX_PW_P0, 0);
	nrf24_WriteReg(RX_PW_P1, 0);
	nrf24_WriteReg(RX_PW_P2, 0);
	nrf24_WriteReg(RX_PW_P3, 0);
	nrf24_WriteReg(RX_PW_P4, 0);
	nrf24_WriteReg(RX_PW_P5, 0);
	nrf24_WriteReg(FIFO_STATUS, 0x11);
	nrf24_WriteReg(DYNPD, 0);
	nrf24_WriteReg(FEATURE, 0);
	}
}




void NRF24_Init (void)
{
	// disable the chip before configuring the device
	CE_Disable();


	// reset everything
	nrf24_reset (0);

	nrf24_WriteReg(CONFIG, 0);  // will be configured later

	nrf24_WriteReg(EN_AA, 0);  // No Auto ACK

	nrf24_WriteReg (EN_RXADDR, 0);  // Not Enabling any data pipe right now

	nrf24_WriteReg (SETUP_AW, 0x03);  // 5 Bytes for the TX/RX address

	nrf24_WriteReg (SETUP_RETR, 0);   // No retransmission

	nrf24_WriteReg (RF_CH, 0);  // will be setup during Tx or RX

	nrf24_WriteReg (RF_SETUP, 0x0C);   // Power= 0db, data rate = 2Mbps

	// Enable the chip after configuring the device
	CE_Enable();

}


// set up the Tx mode

void NRF24_TxMode_with_ACK_Payload (uint8_t *Address, uint8_t channel)
{
	// disable the chip before configuring the device
	CE_Disable();

	nrf24_WriteReg (RF_CH, channel);  // select the channel

	nrf24_WriteRegMulti(TX_ADDR, Address, 5);  // Write the TX address

	// Configure ACK recieve address for 0 pipe
	uint8_t en_rxaddr = nrf24_ReadReg(EN_RXADDR);
	en_rxaddr = en_rxaddr | 1;
	nrf24_WriteReg (EN_RXADDR, en_rxaddr);
	nrf24_WriteRegMulti(RX_ADDR_P0, Address, 5);

	// power up the device
	uint8_t config = nrf24_ReadReg(CONFIG);
	config = config | (1<<1);   // write 1 in the PWR_UP bit
	config = config | (1<<3);   // write 1 in EN_CRC to enable CRC
	config = config | (1<<2);   // write 1 in CRCO to set encoding scheme CRC to 2 bytes
	nrf24_WriteReg (CONFIG, config);

	nrf24_WriteReg (EN_AA,  0x3f); // Activate auto ack for all pipes
	nrf24_WriteReg (SETUP_RETR, 0xFF); // 0 retransmission attempts with 1000us delay
	nrf24_WriteReg(FEATURE, (1<<1)|(1<<2)); // Enable dynamic payload length and payload with ACK

	nrf24_WriteReg(DYNPD, (1<<1)|1); // Enable dynamic payload length for 1&2 pipes

	// Activate R_RX_PL_WID register feature
	uint8_t cmdtosend[] = {ACTIVATE, 0x73};
	HAL_SPI_Transmit(NRF24_SPI, cmdtosend, 2, 100);

	// Enable the chip after configuring the device
	CE_Enable();
}

void flush_tx_fifo() {
	uint8_t cmdtosend = FLUSH_TX;
	nrfsendCmd(cmdtosend);
	// reset FIFO_STATUS
	nrf24_reset (FIFO_STATUS);
}

void flush_rx_fifo() {
	uint8_t cmdtosend = FLUSH_RX;
	nrfsendCmd(cmdtosend);
	// reset FIFO_STATUS
	nrf24_reset (FIFO_STATUS);
}

void NRF24_TxRxMode (uint8_t *TxAddress, uint8_t *RxAddress, uint8_t channel)
{
	// disable the chip before configuring the device
	CE_Disable();

	nrf24_WriteReg (RF_CH, channel);  // select the channel

	//TX Setup
	nrf24_WriteRegMulti(TX_ADDR, TxAddress, 5);  // Write the TX address

	nrf24_WriteReg (SETUP_RETR, 0x33);

	//RX Setup
	uint8_t en_rxaddr = 0x3F;
		//en_rxaddr = en_rxaddr | (1<<1);
		nrf24_WriteReg (EN_RXADDR, en_rxaddr);

		/* We must write the address for Data Pipe 1, if we want to use any pipe from 2 to 5
		 * The Address from DATA Pipe 2 to Data Pipe 5 differs only in the LSB
		 * Their 4 MSB Bytes will still be same as Data Pipe 1
		 *
		 * For Eg->
		 * Pipe 1 ADDR = 0xAABBCCDD11
		 * Pipe 2 ADDR = 0xAABBCCDD22
		 * Pipe 3 ADDR = 0xAABBCCDD33
		 *
		 */
		nrf24_WriteRegMulti(RX_ADDR_P1, RxAddress, 5);  // Write the Pipe1 address
		//nrf24_WriteReg(RX_ADDR_P2, 0xEE);  // Write the Pipe2 LSB address

		nrf24_WriteReg (RX_PW_P0, 32);   // 32 bit payload size for pipe 2
		nrf24_WriteReg (RX_PW_P1, 32);
		nrf24_WriteReg (RX_PW_P2, 32);
		nrf24_WriteReg (RX_PW_P3, 32);
		nrf24_WriteReg (RX_PW_P4, 32);
		nrf24_WriteReg (RX_PW_P5, 32);

	// Enable the chip after configuring the device
	CE_Enable();
	flush_tx_fifo();
}

void NRF24_TxMode (void)
{
	// disable the chip before configuring the device
	CE_Disable();

	// power up the device
	uint8_t config = 0;
	config = config | (1<<1);   // write 1 in the PWR_UP bit
	config = config | (1<<3);   // write 1 in EN_CRC to enable CRC
	config = config | (1<<2);   // write 1 in CRCO to set encoding scheme CRC to 2 bytes
	//config = config & (0xF2);    // write 0 in the PRIM_RX, and 1 in the PWR_UP, and all other bits are masked
	nrf24_WriteReg (CONFIG, config);

	// Enable the chip after configuring the device
	CE_Enable();
	flush_tx_fifo();
}

void NRF24_RxMode (void)
{
	// disable the chip before configuring the device
	CE_Disable();

	// power up the device in Rx mode
	uint8_t config = 0;
	config = config | (1<<1) | (1<<0);
	config = config | (1<<3);   // write 1 in EN_CRC to enable CRC
	config = config | (1<<2);   // write 1 in CRCO to set encoding scheme CRC to 2 bytes
	nrf24_WriteReg (CONFIG, config);

	// Enable the chip after configuring the device
	CE_Enable();
	flush_rx_fifo();
}

// transmit the data
uint8_t NRF24_Transmit (uint8_t *data)
{
	uint8_t cmdtosend = 0;
	// select the device
	CS_Select();

	// payload command
	cmdtosend = W_TX_PAYLOAD;
	hal_result = HAL_SPI_Transmit(NRF24_SPI, &cmdtosend, 1, 100);

	// send the payload
	hal_result = HAL_SPI_Transmit(NRF24_SPI, data, 32, 1000);

	// Unselect the device
	CS_UnSelect();

	HAL_Delay(1);

	uint8_t status = nrf24_ReadReg(STATUS);
		if (status & (1 << 4)) {
			status = (status | (1 << 4)); // Actually not needed because this bit is already set, but reset is done by writing 1
			nrf24_WriteReg(STATUS, status);
			flush_tx_fifo();
		}

	uint8_t fifostatus = nrf24_ReadReg(FIFO_STATUS);

	// check the fourth bit of FIFO_STATUS to know if the TX fifo is empty
	if ((fifostatus&(1<<4)) && (!(fifostatus&(1<<3))))
	{
		flush_tx_fifo();
		return 1;
	}

	return 0;
}


void NRF24_Receive_ACK_Payload(uint8_t *data, uint8_t* data_size) {
	uint8_t cmdtosend = 0;

	// select the device
	CS_Select();

//	 Receive the payload size
	cmdtosend = R_RX_PL_WID;
	HAL_SPI_Transmit(NRF24_SPI, &cmdtosend, 1, 100);
	HAL_SPI_Receive(NRF24_SPI, data_size, 1, 1000);
//	 Receive payload
	if ( *data_size > 0) {
		// Required CSN transition between different commands
		CS_UnSelect();
		CS_Select();

		cmdtosend = R_RX_PAYLOAD;
		HAL_SPI_Transmit(NRF24_SPI, &cmdtosend, 1, 100);
		HAL_SPI_Receive(NRF24_SPI, data, 32, 1000);
	}


	// Unselect the device
	CS_UnSelect();
	HAL_Delay(1);
	cmdtosend = FLUSH_RX;
	nrfsendCmd(cmdtosend);
}


uint8_t isDataAvailable ()
{
	uint8_t fifo = nrf24_ReadReg(FIFO_STATUS);
	uint8_t status = nrf24_ReadReg(STATUS);
	uint8_t config = nrf24_ReadReg(CONFIG);

	if ((status&(1<<6)))
	{
		nrf24_WriteReg(STATUS, (1<<6)); // Clear receive fifo bit
		return 1;
	}

	return 0;
}

void NRF24_Receive (uint8_t *data)
{
	uint8_t cmdtosend = 0;

	// select the device
	CS_Select();

	// payload command
	cmdtosend = R_RX_PAYLOAD;
	HAL_SPI_Transmit(NRF24_SPI, &cmdtosend, 1, 100);

	// Receive the payload
	HAL_SPI_Receive(NRF24_SPI, data, 32, 1000);

	// Unselect the device
	CS_UnSelect();

	HAL_Delay(1);

	cmdtosend = FLUSH_RX;
	nrfsendCmd(cmdtosend);
}

