#include "colir_one.h"
#include "main.h"
#include "fatfs.h"

extern "C" {
  #include "NRF24L01.h"
  #include "bno055_stm32.h"
  #include "nmea_parse.h"
  #include "pca9685.h"
  #include "device.h"
  #include "bmp581.h"
}

#define RxBuffer_SIZE 64                //configure uart receive buffer size
#define DataBuffer_SIZE 512             //gather a few rxBuffer frames before parsing

I2C_HandleTypeDef hi2c2;

SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio_rx;
DMA_HandleTypeDef hdma_sdio_tx;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

uint8_t TxAddress[] = {0xEE,0xDD,0xCC,0xBB,0xAA};
uint8_t RxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xAA};
char TxData[32]; // Max message length 32 bytes
uint8_t RxData[32];

char cmdSymbol;
char cmdParams[5][6];

uint32_t value_adc;
uint8_t write_logs = 0;

// Functions for UART receiving, based on the DMA receive function, implementations may vary
uint16_t oldPos = 0;
uint16_t newPos = 0;
uint8_t RxBuffer[RxBuffer_SIZE];
uint8_t DataBuffer[DataBuffer_SIZE];
//create a GPS data structure
GPS myData;

float zeroAltitude = 0;
struct bmp5_sensor_data bmpData = {0,0};
float apogee = 0;

colir_one_rocket_state rState;

bool rxMode = false;
float VBatt;
float lastAltitude = 0;
float verticalVelocity = 0;
float altitude = 0;
flash_config* logsConfig;

double latitude = 0, longitude = 0;

uint32_t lastRxMode;
uint32_t lastTimestamp;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C2_Init(void);
static void MX_SPI3_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_SPI1_Init(void);
// static void MX_USART1_UART_INIT(void);
static void MX_USART2_UART_Init(void);
static void stm32_init(void);
static void FigherLighter(uint8_t lighterNumber);
static void LogData(char data[]);
static void ParseReceivedCommand(char cmd[], uint8_t payloadSize);
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);


static void stm32_init(void){
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C2_Init();
  MX_SPI3_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_SPI1_Init();
  // MX_USART1_UART_INIT();
  MX_USART2_UART_Init();
}

static void FigherLighter(uint8_t lighterNumber){
	uint16_t pinNumber = 0 | (1<<(6+lighterNumber));
	HAL_GPIO_WritePin(GPIOE, pinNumber, GPIO_PIN_SET);
	HAL_Delay(15);
	HAL_GPIO_WritePin(GPIOE, pinNumber, GPIO_PIN_RESET);
}

static void LogData(char data[]){
	if(write_logs == 1)
		log_data(data);
}


static void ParseReceivedCommand(char cmd[], uint8_t payloadSize)
{
	if(cmd[0] == '\0')
		return;

	cmdSymbol = cmd[0];
	int cmdCounter = 0;
	int cmdParamCounter = 0;
	for(uint8_t i = 2; i < payloadSize; i++){
		if(cmd[i] == ' '){
			cmdCounter++;
			cmdParamCounter = 0;
		}
		else{
			cmdParams[cmdCounter][cmdParamCounter] = cmd[i];
			cmdParamCounter++;
		}
	}

	if(cmdSymbol == 's'){ //format like "s {servoNumber} {servoAngle} {servoNumber} {servoAngle}"
		int servoNumber = atoi(cmdParams[0]);
		int servoAngle = atoi(cmdParams[1]);

		HAL_I2C_DeInit(&hi2c2);
		MX_I2C2_Init();

		  PCA9685_Init(&hi2c2);
      #ifndef PCA9685_SERVO_MODE
      PCA9685_SetPwmFrequency(50);
      #endif
		  // PCA9685_SetPwmFrequency(50);

		PCA9685_STATUS servoAngleStatus = PCA9685_SetServoAngle(servoNumber - 1, servoAngle);

		/*if(cmdParams[2] != 0){
			servoNumber = atoi(cmdParams[2]);
			servoAngle = atoi(cmdParams[3]);
			PCA9685_SetServoAngle(servoNumber - 1, servoAngle);
		}*/
	}
	else if(cmdSymbol == 'l'){ //format like "s {servoNumber} {servoAngle}"
		int lighterNumber = atoi(cmdParams[0]);
		write_logs = 1;
		rState = COLIRONE_CRUISE;
		FigherLighter(lighterNumber);
	}
	else if(cmdSymbol == 'w' && cmdParams[0][0] == 'e'){
		write_logs= 1;
	}
	else if(cmdSymbol == 'w' && cmdParams[0][0] == 'd'){
		write_logs= 0;
	}
	else if(cmdSymbol == 'w' && cmdParams[0][0] == 's'){
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
		read_logs_to_sd();
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
	}
	else if(cmdSymbol == 'r' && cmdParams[0][0] == 'a'){
		HAL_SPI_Init(&hspi1);
		int8_t err = getSensorData(&bmpData);
		zeroAltitude = calcAltitude(bmpData.pressure);
		apogee = 0;
		HAL_SPI_DeInit(&hspi1);
	}
	else if(cmdSymbol == 'r' && cmdParams[0][0] == 'l'){
		HAL_SPI_Init(&hspi1);
		reset_logs();
		HAL_SPI_DeInit(&hspi1);
	}
	LogData(cmd);
}

/*
 * UART buffer handler based on the DMA receive function, every implementation is valid,
 * as long as you pass a sufficiently long receive buffer to the library.
 * */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    oldPos = newPos; //keep track of the last position in the buffer
    if(oldPos + Size > DataBuffer_SIZE){ //if the buffer is full, parse it, then reset the buffer

        uint16_t datatocopy = DataBuffer_SIZE-oldPos;  // find out how much space is left in the main buffer
        memcpy ((uint8_t *)DataBuffer+oldPos, RxBuffer, datatocopy);  // copy data in that remaining space

        oldPos = 0;  // point to the start of the buffer
        memcpy ((uint8_t *)DataBuffer, (uint8_t *)RxBuffer+datatocopy, (Size-datatocopy));  // copy the remaining data
        newPos = (Size-datatocopy);  // update the position
    }
    else{
        memcpy((uint8_t *)DataBuffer+oldPos, RxBuffer, Size); //copy received data to the buffer
        newPos = Size+oldPos; //update buffer position

    }
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)RxBuffer, RxBuffer_SIZE); //re-enable the DMA interrupt
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT); //disable the half transfer interrupt
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV8;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDIO_SD_Init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 4;
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

// static void MX_USART1_UART_INIT(void){
//   huart1.Instance = USART1;
//   huart1.Init.BaudRate = 9600;
//   huart1.Init.WordLength = UART_WORDLENGTH_8B;
//   huart1.Init.StopBits = UART_STOPBITS_1;
//   huart1.Init.Parity = UART_PARITY_NONE;
//   huart1.Init.Mode = UART_MODE_TX_RX;
//   huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
//   huart1.Init.OverSampling = UART_OVERSAMPLING_16;
//   if (HAL_UART_Init(&huart1) != HAL_OK)
//   {
//     Error_Handler();
//   }
// }

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, NRF_CE_Pin|NRF_CSN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 PE7 PE8
                           PE9 PE10 PE11 PE12 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PC4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : NRF_CE_Pin NRF_CSN_Pin */
  GPIO_InitStruct.Pin = NRF_CE_Pin|NRF_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

ColirOne::ColirOne(){
    
}

void ColirOne::init(void){
    stm32_init();

    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)RxBuffer, RxBuffer_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

    //HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&value_adc,1);

    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);

    colir_one_init(&hspi1);
    HAL_Delay(100);

    read_logs_to_sd();

    NRF24_Init();
    NRF24_TxRxMode(TxAddress, RxAddress, 76);
    NRF24_RxMode();

    bno055_assignI2C(&hi2c2);
    bno055_setup();
    bno055_setOperationModeNDOF();

    PCA9685_Init(&hi2c2);
    #ifndef PCA9685_SERVO_MODE
    PCA9685_SetPwmFrequency(50);
    #endif

    //PCA9685_SetServoAngle(0, 90);
    //PCA9685_SetServoAngle(1, 87);
    //PCA9685_SetServoAngle(2, 83);
    //PCA9685_SetServoAngle(3, 84);

    //PCA9685_SetServoAngle(7, 90);

    HAL_SPI_Init(&hspi1);
    while(beginSPI() != BMP5_OK)
    {
        // Wait a bit to see if connection is established
        HAL_Delay(1000);
    }
    HAL_Delay(1000);
    int8_t err = getSensorData(&bmpData);
    zeroAltitude = calcAltitude(bmpData.pressure);
    HAL_SPI_DeInit(&hspi1);

    rState = COLIRONE_READY_TO_LAUNCH;
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);

    lastRxMode = HAL_GetTick();
    lastTimestamp = HAL_GetTick();

    logsConfig = get_logs_config();
}

void ColirOne::base(void){
	uint32_t timestamp = HAL_GetTick();
	bno055_vector_t orientation = bno055_getVectorEuler();
	bno055_vector_t linearAccel = bno055_getVectorLinearAccel();
	bno055_vector_t quaternion = bno055_getVectorQuaternion();
	bno055_vector_t gyro = bno055_getVectorGyroscope();
	logsConfig = get_logs_config();

	HAL_SPI_Init(&hspi1);
	int8_t err = getSensorData(&bmpData);
	HAL_SPI_DeInit(&hspi1);
	if(bmpData.pressure > 10000){
		altitude = calcAltitude(bmpData.pressure) - zeroAltitude;
		if(altitude > apogee)
			apogee = altitude;

		float deltaT = timestamp - lastTimestamp;
		if(deltaT != 0)
			verticalVelocity = (altitude - lastAltitude) / (deltaT / 1000);

		lastAltitude = altitude;
		lastTimestamp = timestamp;

		if(verticalVelocity < -3 && altitude > 5 && altitude < 150 && (rState == COLIRONE_CRUISE || rState == COLIRONE_SHUTES_DEPLOYED)){
			rState = COLIRONE_SHUTES_DEPLOYED;
			ParseReceivedCommand("s 8 90", 32);
		}
	}

	VBatt = (value_adc * 2 / 4095) * 7.4;

	nmea_parse(&myData, DataBuffer);
	latitude = myData.latitude;
	longitude = myData.longitude;
	if(longitude > 1000)
		longitude = longitude / 1000;
	if(myData.lonSide == 'W')
		longitude = -longitude;
	if(myData.latSide == 'S')
		latitude = -latitude;
	//ParseReceivedCommand("s 8 90", 32);
	HAL_Delay(10);
	if(!rxMode){
		NRF24_TxMode();
		memset(&TxData, 0, sizeof(TxData));
		sprintf(TxData, "p %d %f %f %d", timestamp, latitude, longitude, myData.satelliteCount);
		NRF24_Transmit((uint8_t*)TxData);
		LogData(TxData);

		HAL_Delay(15);
		memset(&TxData, 0, sizeof(TxData));
		sprintf(TxData, "h %d %.2f", timestamp, altitude);
		NRF24_Transmit((uint8_t*)TxData);
		LogData(TxData);

		HAL_Delay(15);
		memset(&TxData, 0, sizeof(TxData));
		sprintf(TxData, "v %d %.2f", timestamp, verticalVelocity);
		NRF24_Transmit((uint8_t*)TxData);
		LogData(TxData);

		HAL_Delay(15);
		memset(&TxData, 0, sizeof(TxData));
		sprintf(TxData, "o %d %.2f %.2f %.2f", timestamp, -orientation.x, -orientation.y, -orientation.z);
		NRF24_Transmit((uint8_t*)TxData);
		LogData(TxData);

		HAL_Delay(15);
		memset(&TxData, 0, sizeof(TxData));
		sprintf(TxData, "g %d %.2f %.2f %.2f", timestamp, gyro.x, gyro.y, gyro.z);
		NRF24_Transmit((uint8_t*)TxData);
		LogData(TxData);

		HAL_Delay(15);
		memset(&TxData, 0, sizeof(TxData));
		sprintf(TxData, "a %d %.2f %.2f %.2f", timestamp, linearAccel.x, linearAccel.y, linearAccel.z);
		NRF24_Transmit((uint8_t*)TxData);
		LogData(TxData);

		HAL_Delay(15);
		memset(&TxData, 0, sizeof(TxData));
		sprintf(TxData, "q %d %.2f %.2f %.2f %.2f", timestamp, quaternion.x, quaternion.y, quaternion.z, quaternion.w);
		NRF24_Transmit((uint8_t*)TxData);
		LogData(TxData);

		HAL_Delay(15);
		memset(&TxData, 0, sizeof(TxData));
		sprintf(TxData, "s %d %d %d %d", timestamp, write_logs, rState, logsConfig->last_log);
		NRF24_Transmit((uint8_t*)TxData);
		LogData(TxData);

		HAL_Delay(15);
		if(timestamp - lastRxMode > 500){
			memset(&TxData, 0, sizeof(TxData));
			sprintf(TxData, "c");
			NRF24_Transmit((uint8_t*)TxData);
			rxMode = true;
			NRF24_RxMode();
			lastRxMode = HAL_GetTick();
		}
	}
	else{
		if (isDataAvailable() == 1)
		{
			memset(&RxData, 0, sizeof(RxData));
			NRF24_Receive(RxData);
			rxMode = false;
			lastRxMode = HAL_GetTick();
			ParseReceivedCommand((char*)RxData, sizeof(RxData));
		}
		else if(timestamp - lastRxMode > 250){
			rxMode = false;
			lastRxMode = HAL_GetTick();
		}
	}

	HAL_Delay(70);
}

XYZ IMU::getOrientation(){
    XYZ orientation;
    bno055_vector_t orientationVector = bno055_getVectorEuler();
    orientation.x = orientationVector.x;
    orientation.y = orientationVector.y;
    orientation.z = orientationVector.z;
    return orientation;
}
