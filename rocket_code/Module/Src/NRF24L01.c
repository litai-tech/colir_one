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
#include "NRF24L01.h"

extern SPI_HandleTypeDef hspi3;
#define NRF24_SPI &hspi3


// THESE VALUES ARE MANUALLY CONFIGURED FOR SPECIFIC STM32!!!!!!!!
#define NRF24_CE_PORT   GPIOB
#define NRF24_CE_PIN    GPIO_PIN_6
#define NRF24_CSN_PORT   GPIOB
#define NRF24_CSN_PIN    GPIO_PIN_7
#define nRF24_TEST_ADDR  "nRF24"


// HAL_StatusTypeDef hal_result;
static void CS_Select(void)
{
	HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_RESET);
}

static void CS_UnSelect(void)
{
	HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_SET);
}

static void CE_Enable(void)
{
	HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_SET);
}

static void CE_Disable(void)
{
	HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_RESET);
}

// write a single byte to the particular register
static HAL_StatusTypeDef nrf24_write(uint8_t Reg, uint8_t Data)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t buf[2];
	buf[0] = Reg|1<<5;
	buf[1] = Data;

	// Pull the CS Pin LOW to select the device
	CS_Select();

	status |= HAL_SPI_Transmit(NRF24_SPI, buf, 2, 1000);

	// Pull the CS HIGH to release the device
	CS_UnSelect();

	return status;
}

//write multiple bytes starting from a particular register
static HAL_StatusTypeDef nrf24_writeMulti(uint8_t Reg, uint8_t *data, int size)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t buf[2];
	buf[0] = Reg|1<<5;
//	buf[1] = Data;

	// Pull the CS Pin LOW to select the device
	CS_Select();

	status |= HAL_SPI_Transmit(NRF24_SPI, buf, 1, 100);
	status |= HAL_SPI_Transmit(NRF24_SPI, data, size, 1000);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
	return status;
}


static HAL_StatusTypeDef nrf24_read(uint8_t reg, uint8_t* data)
{
	// uint8_t data=0;

	// Pull the CS Pin LOW to select the device
	HAL_StatusTypeDef status = HAL_OK;
	CS_Select();

	status |= HAL_SPI_Transmit(NRF24_SPI, &reg, 1, 100);
	status |= HAL_SPI_Receive(NRF24_SPI, data, 1, 100);

	// Pull the CS HIGH to release the device
	CS_UnSelect();

	return status;
}


/* Read multiple bytes from the register */
static HAL_StatusTypeDef nrf24_readMulti(uint8_t Reg, uint8_t *data, int size)
{
	HAL_StatusTypeDef status = HAL_OK;
	// Pull the CS Pin LOW to select the device
	CS_Select();

	status |= HAL_SPI_Transmit(NRF24_SPI, &Reg, 1, 100);
	status |= HAL_SPI_Receive(NRF24_SPI, data, size, 1000);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
	return status;
}


// send the command to the NRF
static HAL_StatusTypeDef nrf24_send_cmd(uint8_t cmd)
{
	HAL_StatusTypeDef status = HAL_OK;
	// Pull the CS Pin LOW to select the device
	CS_Select();

	status |= HAL_SPI_Transmit(NRF24_SPI, &cmd, 1, 100);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
	return status;
}

static HAL_StatusTypeDef nrf24_reset(uint8_t reg)
{
	HAL_StatusTypeDef status = HAL_OK;
	if (reg == STATUS)
	{
		status |= nrf24_write(STATUS, 0x00);
	}

	else if (reg == FIFO_STATUS)
	{
		status |= nrf24_write(FIFO_STATUS, 0x11);
	}

	else {
	status |= nrf24_write(CONFIG, 0x08);
	status |= nrf24_write(EN_AA, 0x3F);
	status |= nrf24_write(EN_RXADDR, 0x03);
	status |= nrf24_write(SETUP_AW, 0x03);
	status |= nrf24_write(SETUP_RETR, 0x00);
	status |= nrf24_write(RF_CH, 0x02);
	status |= nrf24_write(RF_SETUP, 0x0E);
	status |= nrf24_write(STATUS, 0x00);
	status |= nrf24_write(OBSERVE_TX, 0x00);
	status |= nrf24_write(CD, 0x00);
	uint8_t rx_addr_p0_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	status |= nrf24_writeMulti(RX_ADDR_P0, rx_addr_p0_def, 5);
	uint8_t rx_addr_p1_def[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
	status |= nrf24_writeMulti(RX_ADDR_P1, rx_addr_p1_def, 5);
	status |= nrf24_write(RX_ADDR_P2, 0xC3);
	status |= nrf24_write(RX_ADDR_P3, 0xC4);
	status |= nrf24_write(RX_ADDR_P4, 0xC5);
	status |= nrf24_write(RX_ADDR_P5, 0xC6);
	uint8_t tx_addr_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	status |= nrf24_writeMulti(TX_ADDR, tx_addr_def, 5);
	status |= nrf24_write(RX_PW_P0, 0);
	status |= nrf24_write(RX_PW_P1, 0);
	status |= nrf24_write(RX_PW_P2, 0);
	status |= nrf24_write(RX_PW_P3, 0);
	status |= nrf24_write(RX_PW_P4, 0);
	status |= nrf24_write(RX_PW_P5, 0);
	status |= nrf24_write(FIFO_STATUS, 0x11);
	status |= nrf24_write(DYNPD, 0);
	status |= nrf24_write(FEATURE, 0);
	}
	return status;
}


HAL_StatusTypeDef nrf24_init(void)
{
	HAL_StatusTypeDef status = HAL_OK;
	// disable the chip before configuring the device
	CE_Disable();


	// reset everything
	status |= nrf24_reset (0);

	status |= nrf24_write(CONFIG, 0);  // will be configured later

	status |= nrf24_write(EN_AA, 0);  // No Auto ACK

	status |= nrf24_write (EN_RXADDR, 0);  // Not Enabling any data pipe right now

	status |= nrf24_write (SETUP_AW, 0x03);  // 5 Bytes for the TX/RX address

	status |= nrf24_write (SETUP_RETR, 0);   // No retransmission

	status |= nrf24_write (RF_CH, 0);  // will be setup during Tx or RX

	// nrf24_write (RF_SETUP, 0x0C);   // Power= 0db, data rate = 2Mbps
	status |= nrf24_write (RF_SETUP, 0x27);   // Power= 0db,data rate = 250kbps

	// Enable the chip after configuring the device
	CE_Enable();
	return status;
}


// set up the Tx mode

HAL_StatusTypeDef nrf24_TxMode_with_ACK_payload(uint8_t *address, uint8_t channel)
{
	HAL_StatusTypeDef status = HAL_OK;
	// disable the chip before configuring the device
	CE_Disable();

	status |= nrf24_write (RF_CH, channel);  // select the channel

	status |=  nrf24_writeMulti(TX_ADDR, address, 5);  // Write the TX address

	// Configure ACK recieve address for 0 pipe
	uint8_t en_rxaddr;
	status |= nrf24_read(EN_RXADDR, &en_rxaddr);
	en_rxaddr = en_rxaddr | 1;
	status |= nrf24_write (EN_RXADDR, en_rxaddr);
	status |= nrf24_writeMulti(RX_ADDR_P0, address, 5);

	// power up the device
	uint8_t config;
	status |= nrf24_read(CONFIG, &config);
	config = config | (1<<1);   // write 1 in the PWR_UP bit
	config = config | (1<<3);   // write 1 in EN_CRC to enable CRC
	config = config | (1<<2);   // write 1 in CRCO to set encoding scheme CRC to 2 bytes
	status |= nrf24_write (CONFIG, config);

	status |= nrf24_write (EN_AA,  0x3f); // Activate auto ack for all pipes
	status |= nrf24_write (SETUP_RETR, 0xFF); // 0 retransmission attempts with 1000us delay
	status |=  nrf24_write(FEATURE, (1<<1)|(1<<2)); // Enable dynamic payload length and payload with ACK

	status |= nrf24_write(DYNPD, (1<<1)|1); // Enable dynamic payload length for 1&2 pipes

	// Activate R_RX_PL_WID register feature
	uint8_t cmd[] = {ACTIVATE, 0x73};
	status |= HAL_SPI_Transmit(NRF24_SPI, cmd, 2, 100);

	// Enable the chip after configuring the device
	CE_Enable();
	return status;
}

HAL_StatusTypeDef flush_tx_fifo(void) {
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t cmd = FLUSH_TX;
	status |= nrf24_send_cmd(cmd);
	// reset FIFO_STATUS
	status |= nrf24_reset (FIFO_STATUS);
	return status;
}

HAL_StatusTypeDef flush_rx_fifo(void) {
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t cmd = FLUSH_RX;
	status |= nrf24_send_cmd(cmd);
	// reset FIFO_STATUS
	status |= nrf24_reset (FIFO_STATUS);
	return status;
}

HAL_StatusTypeDef nrf24_TxRxMode(uint8_t *TxAddress, uint8_t *RxAddress, uint8_t channel)
{
	HAL_StatusTypeDef status = HAL_OK;

	// disable the chip before configuring the device
	CE_Disable();

	status |= nrf24_write (RF_CH, channel);  // select the channel

	//TX Setup
	status |= nrf24_writeMulti(TX_ADDR, TxAddress, 5);  // Write the TX address

	status |= nrf24_write (SETUP_RETR, 0x33);

	//RX Setup
	uint8_t en_rxaddr = 0x3F;
		//en_rxaddr = en_rxaddr | (1<<1);
		status |= nrf24_write (EN_RXADDR, en_rxaddr);

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
		status |= nrf24_writeMulti(RX_ADDR_P1, RxAddress, 5);  // Write the Pipe1 address
		//nrf24_write(RX_ADDR_P2, 0xEE);  // Write the Pipe2 LSB address

		status |= nrf24_write (RX_PW_P0, 32);   // 32 bit payload size for pipe 2
		status |= nrf24_write (RX_PW_P1, 32);
		status |= nrf24_write (RX_PW_P2, 32);
		status |= nrf24_write (RX_PW_P3, 32);
		status |= nrf24_write (RX_PW_P4, 32);
		status |= nrf24_write (RX_PW_P5, 32);

	// Enable the chip after configuring the device
	CE_Enable();
	status |= flush_tx_fifo();
	return status;
}

HAL_StatusTypeDef nrf24_TxMode(void)
{
	HAL_StatusTypeDef status = HAL_OK;
	// disable the chip before configuring the device
	CE_Disable();

	// power up the device
	uint8_t config = 0;
	config = config | (1<<1);   // write 1 in the PWR_UP bit
	config = config | (1<<3);   // write 1 in EN_CRC to enable CRC
	config = config | (1<<2);   // write 1 in CRCO to set encoding scheme CRC to 2 bytes
	//config = config & (0xF2);    // write 0 in the PRIM_RX, and 1 in the PWR_UP, and all other bits are masked
	status |= nrf24_write (CONFIG, config);

	// Enable the chip after configuring the device
	CE_Enable();
	status |= flush_tx_fifo();
	return status;
}

HAL_StatusTypeDef nrf24_RxMode(void)
{
	HAL_StatusTypeDef status = HAL_OK;
	// disable the chip before configuring the device
	CE_Disable();

	// power up the device in Rx mode
	uint8_t config = 0;
	config = config | (1<<1) | (1<<0);
	config = config | (1<<3);   // write 1 in EN_CRC to enable CRC
	config = config | (1<<2);   // write 1 in CRCO to set encoding scheme CRC to 2 bytes
	status |= nrf24_write (CONFIG, config);

	// Enable the chip after configuring the device
	CE_Enable();
	status |= flush_rx_fifo();
	return status;
}

// transmit the data
HAL_StatusTypeDef nrf24_transmit(uint8_t *data)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t cmd = 0;
	// select the device
	CS_Select();

	// payload command
	cmd = W_TX_PAYLOAD;
	status |= HAL_SPI_Transmit(NRF24_SPI, &cmd, 1, 100);

	// send the payload
	status |= HAL_SPI_Transmit(NRF24_SPI, data, 32, 1000);

	// Unselect the device
	CS_UnSelect();

	HAL_Delay(1);
	uint8_t nrf24_status;
	status |= nrf24_read(STATUS, &nrf24_status);
		if (nrf24_status & (1 << 4)) {
			nrf24_status = (nrf24_status | (1 << 4)); // Actually not needed because this bit is already set, but reset is done by writing 1
			status |= nrf24_write(STATUS, nrf24_status);
			status |= flush_tx_fifo();
		}

	uint8_t fifo_status;
	status |= nrf24_read(FIFO_STATUS, &fifo_status);

	// check the fourth bit of FIFO_STATUS to know if the TX fifo is empty
	if ((fifo_status&(1<<4)) && (!(fifo_status&(1<<3))))
	{
		status |= flush_tx_fifo();
		// return 1;
	}

	return status;
}


HAL_StatusTypeDef nrf24_receive_ACK_payload(uint8_t *data, uint8_t* data_size) {
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t cmd = 0;

	// select the device
	CS_Select();

//	 Receive the payload size
	cmd = R_RX_PL_WID;
	status |= HAL_SPI_Transmit(NRF24_SPI, &cmd, 1, 100);
	status |= HAL_SPI_Receive(NRF24_SPI, data_size, 1, 1000);
//	 Receive payload
	if ( *data_size > 0) {
		// Required CSN transition between different commands
		CS_UnSelect();
		CS_Select();

		cmd = R_RX_PAYLOAD;
		status |= HAL_SPI_Transmit(NRF24_SPI, &cmd, 1, 100);
		status |= HAL_SPI_Receive(NRF24_SPI, data, 32, 1000);
	}


	// Unselect the device
	CS_UnSelect();
	HAL_Delay(1);
	cmd = FLUSH_RX;
	status |= nrf24_send_cmd(cmd);
	return status;
}


HAL_StatusTypeDef nrf24_check_data_available(void)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t fifo_status;
	status |= nrf24_read(FIFO_STATUS, &fifo_status);
	uint8_t nrf24_status;
	status |= nrf24_read(STATUS, &nrf24_status);
	uint8_t config;
	status |= nrf24_read(CONFIG, &config);

	if ((nrf24_status&(1<<6)))
	{
		status |= nrf24_write(STATUS, (1<<6)); // Clear receive fifo bit
		// return 1;
	}
	return status;
	// return 0;
}

HAL_StatusTypeDef nrf24_receive(uint8_t *data)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t cmd = 0;

	// select the device
	CS_Select();

	// payload command
	cmd = R_RX_PAYLOAD;
	status |= HAL_SPI_Transmit(NRF24_SPI, &cmd, 1, 100);

	// Receive the payload
	status |= HAL_SPI_Receive(NRF24_SPI, data, 32, 1000);

	// Unselect the device
	CS_UnSelect();

	HAL_Delay(1);

	cmd = FLUSH_RX;
	status |= nrf24_send_cmd(cmd);
	return status;
}

