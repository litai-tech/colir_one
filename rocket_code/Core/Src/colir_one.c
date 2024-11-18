#include "main.h"
#include "w25qxx.h"
#include "fatfs.h"
#include "colir_one.h"

#define LOG_SIZE 32
#define LOG_BUF_SIZE 32
#define PAGE_SIZE LOG_SIZE * LOG_BUF_SIZE
#define CONFIG_SIZE 2048
#define SPI1_CS_GPIO_Port GPIOD
#define SPI1_CS_Pin GPIO_PIN_7

W25QXX_HandleTypeDef w25qxx;
uint32_t sectors;
uint32_t flash_size;

uint8_t buf[LOG_SIZE];
uint8_t log_buf[LOG_BUF_SIZE][LOG_SIZE];
uint8_t config_buf[CONFIG_SIZE];
flash_config* logs_config;

uint8_t logs_buf_counter = 0;

uint8_t current_log = 255;
uint32_t current_address;
uint16_t log_buf_counter = 0;

W25QXX_result_t res;

void colir_one_init(SPI_HandleTypeDef *hspi) {
	if(hspi->State != HAL_SPI_STATE_READY)
		HAL_SPI_Init(hspi);

	res = w25qxx_init(&w25qxx, hspi, SPI1_CS_GPIO_Port, SPI1_CS_Pin);

	sectors = w25qxx.block_count * w25qxx.sectors_in_block;
	flash_size = w25qxx.block_count * w25qxx.block_size;
	//w25qxx_chip_erase(&w25qxx);
	w25qxx_read(&w25qxx, 0, (uint8_t*)&config_buf, sizeof(config_buf));

	logs_config = (flash_config*)config_buf;
	if(logs_config->last_log == 0 || logs_config->last_log == 255){
		logs_config->last_log = 0;
		logs_config->last_address = CONFIG_SIZE;
		memset(&logs_config->log_address, 0, sizeof(logs_config->log_address));
	}

	HAL_SPI_DeInit(hspi);
}

void log_data(char cmd[]){
	memcpy(&log_buf[log_buf_counter], cmd, sizeof(log_buf[log_buf_counter]));
	log_buf_counter++;
	if(log_buf_counter >= LOG_BUF_SIZE){
		if(w25qxx.spiHandle->State != HAL_SPI_STATE_READY)
			HAL_SPI_Init(w25qxx.spiHandle);

		log_buf_counter = 0;

		current_address = logs_config->last_address;
		//logs_config->last_address = current_address;

		res = w25qxx_erase(&w25qxx, current_address, sizeof(log_buf));
		HAL_Delay(35);
		res = w25qxx_write(&w25qxx, current_address, (uint8_t*)&log_buf, sizeof(log_buf));
		HAL_Delay(35);
		w25qxx_read(&w25qxx, current_address, (uint8_t*)&log_buf, sizeof(log_buf));

		if(current_log == 255){
			current_log = logs_config->last_log + 1;
			if(current_log >= 32)
				current_log = 1;
			logs_config->last_log = current_log;
		}

		current_address += PAGE_SIZE;
		logs_config->last_address = current_address;
		logs_config->log_address[current_log - 1] = current_address;

		res = w25qxx_erase(&w25qxx, 0, sizeof(config_buf));
		if(res == W25QXX_Ok){

			memcpy(&config_buf, (uint8_t*)logs_config, sizeof(config_buf));
			res = w25qxx_write(&w25qxx, 0, (uint8_t*)&config_buf, sizeof(config_buf));
			HAL_Delay(35);
			w25qxx_read(&w25qxx, 0, (uint8_t*)&config_buf, sizeof(config_buf));
		}

		HAL_SPI_DeInit(w25qxx.spiHandle);
	}
}

FRESULT FR_Status;
char end_of_the_line[2] = "\n";

void read_logs_to_sd(){
	FR_Status = f_mount(&SDFatFS, SDPath, 1);
	if(FR_Status == FR_OK){
		DIR dir;
		uint32_t lastAddress = CONFIG_SIZE;
		uint32_t currentReadAdress = CONFIG_SIZE;
		FR_Status = f_opendir(&dir, SDPath);
		if(FR_Status == FR_OK) {
			if(w25qxx.spiHandle->State != HAL_SPI_STATE_READY)
				HAL_SPI_Init(w25qxx.spiHandle);

			for(uint8_t i = 1; i <= logs_config->last_log; i++){
				if(i < logs_config->last_log)
					lastAddress = logs_config->log_address[i];
				else
					lastAddress = logs_config->last_address;

				char fileName[8];
				memset(&fileName, 0, sizeof(fileName));
				sprintf(fileName, "%d.txt", i);
				FR_Status = f_open(&SDFile, fileName, FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
				if(FR_Status == FR_OK){
					currentReadAdress = logs_config->log_address[i - 1];
					for(uint32_t address = currentReadAdress; address < lastAddress; address += LOG_SIZE){
						w25qxx_read(&w25qxx, address, (uint8_t*)&buf, sizeof(buf));
						f_puts(&buf, &SDFile);
						f_puts(&end_of_the_line, &SDFile);
					}
					f_close(&SDFile);
				}
			}
			HAL_SPI_DeInit(w25qxx.spiHandle);
		}
		f_mount(&SDFatFS, (TCHAR const*)NULL, 0);
	}
}

void reset_logs(){
	if(w25qxx.spiHandle->State != HAL_SPI_STATE_READY)
		HAL_SPI_Init(w25qxx.spiHandle);

	w25qxx_chip_erase(&w25qxx);
	w25qxx_read(&w25qxx, 0, (uint8_t*)&config_buf, sizeof(config_buf));

	logs_config = (flash_config*)config_buf;
	if(logs_config->last_log == 0 || logs_config->last_log == 255){
		logs_config->last_log = 0;
		logs_config->last_address = CONFIG_SIZE;
		memset(&logs_config->log_address, 0, sizeof(logs_config->log_address));
	}
	HAL_SPI_DeInit(w25qxx.spiHandle);
}

flash_config* get_logs_config() {
	return logs_config;
}
