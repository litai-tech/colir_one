/*
 * bmp581.c
 *
 *  Created on: Jun 14, 2024
 *      Author: Administrator
 */
#include "stm32f4xx_hal.h"
#include "bmp581.h"

extern SPI_HandleTypeDef hspi1;
#define BMP5_SPI &hspi1

// Reference to the sensor
struct bmp5_dev sensor;

// Place to store OSR/ODR config values
struct bmp5_osr_odr_press_config osrOdrConfig;

// Place to store FIFO config values
struct bmp5_fifo fifo;

HAL_StatusTypeDef bmp5_hal_result;


BMP5_INTF_RET_TYPE readRegisters(uint8_t regAddress, uint8_t* dataBuffer, uint32_t numBytes, void* interfacePtr)
{
    // Make sure the number of bytes is valid
    if(numBytes == 0)
    {
        return BMP5_E_COM_FAIL;
    }

    // Start transmission
    HAL_GPIO_WritePin(BMP581_CSN_PORT, BMP581_CSN_PIN, GPIO_PIN_RESET);

    // Read all requested bytes
    bmp5_hal_result = HAL_SPI_Transmit(BMP5_SPI, &regAddress, 1, 100);
    bmp5_hal_result = HAL_SPI_Receive(BMP5_SPI, dataBuffer, numBytes, 1000);

    // End transmission
    HAL_GPIO_WritePin(BMP581_CSN_PORT, BMP581_CSN_PIN, GPIO_PIN_SET);

    return BMP5_OK;
}

BMP5_INTF_RET_TYPE writeRegisters(uint8_t regAddress, const uint8_t* dataBuffer, uint32_t numBytes, void* interfacePtr)
{
    // Make sure the number of bytes is valid
    if(numBytes == 0)
    {
        return BMP5_E_COM_FAIL;
    }

    // Begin transmission
    HAL_GPIO_WritePin(BMP581_CSN_PORT, BMP581_CSN_PIN, GPIO_PIN_RESET);

    // Write all the data
    HAL_SPI_Transmit(BMP5_SPI, &regAddress, 1, 100);
    HAL_SPI_Transmit(BMP5_SPI, dataBuffer, numBytes, 1000);

    // End transmission
    HAL_GPIO_WritePin(BMP581_CSN_PORT, BMP581_CSN_PIN, GPIO_PIN_SET);

    return BMP5_OK;
}

void usDelay(uint32_t period, void* interfacePtr)
{
    HAL_Delay(period);
}

int8_t begin()
{
    // Variable to track errors returned by API calls
    int8_t err = BMP5_OK;

    // Initialize the sensor
    err = init();
    if(err != BMP5_OK)
    {
        return err;
    }

    // Enable both pressure and temperature sensors
    err = enablePress(BMP5_ENABLE);
    if(err != BMP5_OK)
    {
        return err;
    }

    // Set to normal mode
    return setMode(BMP5_POWERMODE_NORMAL);
}

int8_t beginSPI()
{
    // Set up chip select pin
    HAL_GPIO_WritePin(BMP581_CSN_PORT, BMP581_CSN_PIN, GPIO_PIN_SET);

    // Initialize sensor
    return begin();
}

int8_t init()
{
    // Variable to track errors returned by API calls
    int8_t err = BMP5_OK;

    // Initialize config values
    osrOdrConfig = (struct bmp5_osr_odr_press_config){0,0,0,0};
    fifo = (struct bmp5_fifo){0,0,0,0,0,0,0,0,0};

    // Set helper function pointers
    sensor.read = readRegisters;
    sensor.write = writeRegisters;
    sensor.delay_us = usDelay;
    //sensor.intf_ptr = &interfaceData;

    // Reset the sensor
    err = bmp5_soft_reset(&sensor);
    if(err != BMP5_OK)
    {
        return err;
    }

    // Initialize the sensor
    return bmp5_init(&sensor);
}

int8_t setMode(enum bmp5_powermode mode)
{
    return bmp5_set_power_mode(mode, &sensor);
}

int8_t getMode(enum bmp5_powermode* mode)
{
    return bmp5_get_power_mode(mode, &sensor);
}

int8_t enablePress(uint8_t pressEnable)
{
    osrOdrConfig.press_en = pressEnable;
    return bmp5_set_osr_odr_press_config(&osrOdrConfig, &sensor);
}

int8_t getSensorData(struct bmp5_sensor_data* data)
{
    return bmp5_get_sensor_data(data, &osrOdrConfig, &sensor);
}

float kalman_filter(float inputValue)
{
    float x_k1_k1,x_k_k1;
    float Z_k;
    static float P_k1_k1;

    static float Q = 0.0001;//Q: Regulation noise, Q increases, dynamic response becomes faster, and convergence stability becomes worse
    static float R = 0.005; //R: Test noise, R increases, dynamic response becomes slower, convergence stability becomes better
    static float Kg = 0;
    static float P_k_k1 = 1;

    float kalman_value;
    static float kalman_old=0;
    Z_k = inputValue;
    x_k1_k1 = kalman_old;

    x_k_k1 = x_k1_k1;
    P_k_k1 = P_k1_k1 + Q;

    Kg = P_k_k1/(P_k_k1 + R);

    kalman_value = x_k_k1 + Kg * (Z_k - kalman_old);
    P_k1_k1 = (1 - Kg)*P_k_k1;
    P_k_k1 = P_k1_k1;

    kalman_old = kalman_value;

    return kalman_value;
}

float calcAltitude(float pressure) {
    float A = pressure / 101325;
    float B = 1 / 5.25588;
    float C = pow(A, B);
    C = 1.0 - C;
    C = C / 0.0000225577;
    return C;
}

int8_t setODRFrequency(uint8_t odr)
{
    // Check whether ODR is valid
    if(odr > BMP5_ODR_0_125_HZ)
    {
        return BMP5_E_INVALID_SETTING;
    }

    osrOdrConfig.odr = odr;
    return bmp5_set_osr_odr_press_config(&osrOdrConfig, &sensor);
}

int8_t getODRFrequency(uint8_t* odr)
{
    *odr = osrOdrConfig.odr;
    return BMP5_OK;
}

int8_t setOSRMultipliers(struct bmp5_osr_odr_press_config* config)
{
    // Check whether OSR multipliers are valid
    if(config->osr_t > BMP5_OVERSAMPLING_128X
        || config->osr_p > BMP5_OVERSAMPLING_128X)
    {
        return BMP5_E_INVALID_SETTING;
    }

    // TODO - Check whether this OSR is compatible with current ODR

    osrOdrConfig.osr_t = config->osr_t;
    osrOdrConfig.osr_p = config->osr_p;
    return bmp5_set_osr_odr_press_config(&osrOdrConfig, &sensor);
}

int8_t getOSRMultipliers(struct bmp5_osr_odr_press_config* config)
{
    config->osr_t = osrOdrConfig.osr_t;
    config->osr_p = osrOdrConfig.osr_p;
    return BMP5_OK;
}

int8_t getOSREffective(struct bmp5_osr_odr_eff* osrOdrEffective)
{
    return bmp5_get_osr_odr_eff(osrOdrEffective, &sensor);
}

int8_t setFilterConfig(struct bmp5_iir_config* iirConfig)
{
    return bmp5_set_iir_config(iirConfig, &sensor);
}

int8_t setOORConfig(struct bmp5_oor_press_configuration* oorConfig)
{
    return bmp5_set_oor_configuration(oorConfig, &sensor);
}

int8_t setFIFOConfig(struct bmp5_fifo* fifoConfig)
{
    // Variable to track errors returned by API calls
    int8_t err = BMP5_OK;

    // Copy desired config
    memcpy(&fifo, fifoConfig, sizeof(struct bmp5_fifo));

    // The sensor must be in stanby mode for the FIFO config to
    // be set, grab the current power mode
    enum bmp5_powermode originalMode;
    err = getMode(&originalMode);
    if(err != BMP5_OK)
    {
        return err;
    }

    // Now set to standby
    err = setMode(BMP5_POWERMODE_STANDBY);
    if(err != BMP5_OK)
    {
        return err;
    }

    // Now we can set the FIFO config
    err = bmp5_set_fifo_configuration(&fifo, &sensor);
    if(err != BMP5_OK)
    {
        return err;
    }

    // Finally, set back to original power mode
    return setMode(originalMode);
}

int8_t getFIFOLength(uint8_t* numData)
{
    // Variable to track errors returned by API calls
    int8_t err = BMP5_OK;

    uint16_t numBytes = 0;
    err = bmp5_get_fifo_len(&numBytes, &fifo, &sensor);
    if(err != BMP5_OK)
    {
        return err;
    }

    // Determine number of bytes per sample
    uint8_t bytesPerSample = 1;
    if(fifo.frame_sel == BMP5_FIFO_PRESS_TEMP_DATA)
    {
        bytesPerSample = 6;
    }
    else if((fifo.frame_sel == BMP5_FIFO_TEMPERATURE_DATA) || (fifo.frame_sel == BMP5_FIFO_PRESSURE_DATA))
    {
        bytesPerSample = 3;
    }

    // Compute number of samples in the FIFO buffer
    *numData = numBytes / bytesPerSample;

    return BMP5_OK;
}

int8_t getFIFOData(struct bmp5_sensor_data* data, uint8_t numData)
{
    // Variable to track errors returned by API calls
    int8_t err = BMP5_OK;

    // Determine the number of bytes per data frame, 3 bytes per sensor
    uint8_t bytesPerFrame;
    if(fifo.frame_sel == BMP5_FIFO_TEMPERATURE_DATA
        || fifo.frame_sel == BMP5_FIFO_PRESSURE_DATA)
    {
        bytesPerFrame = 3;
    }
    else
    {
        bytesPerFrame = 6;
    }

    // Set number of bytes to read based on user's buffer size. If this is larger
    // than the number of bytes in the buffer, bmp5_get_fifo_data() will
    // automatically limit it
    fifo.length = numData * bytesPerFrame;

    // Create buffer to hold all bytes of FIFO buffer
    uint8_t byteBuffer[fifo.length];
    fifo.data = byteBuffer;

    // Get all bytes out of FIFO buffer
    err = bmp5_get_fifo_data(&fifo, &sensor);
    if(err != BMP5_OK)
    {
        return err;
    }

    // Parse raw data into temperature and pressure data
    return bmp5_extract_fifo_data(&fifo, data);
}

int8_t flushFIFO()
{
    // Variable to track errors returned by API calls
    int8_t err = BMP5_OK;

    // There isn't a simple way to flush the FIFO buffer unfortunately. However the
    // FIFO is automatically flushed when certain config settings change, such as
    // the power mode. We can simply change the power mode twice to flush the buffer

    // Grab the current power mode
    enum bmp5_powermode originalMode;
    err = getMode(&originalMode);
    if(err != BMP5_OK)
    {
        return err;
    }

    // Change the power mode to something else
    if(originalMode != BMP5_POWERMODE_STANDBY)
    {
        // Sensor is not in standby mode, so default to that
        err = setMode(BMP5_POWERMODE_STANDBY);
        if(err != BMP5_OK)
        {
            return err;
        }
    }
    else
    {
        // Already in standby, switch to forced instead
        err = setMode(BMP5_POWERMODE_FORCED);
        if(err != BMP5_OK)
        {
            return err;
        }
    }

    // Finally, set back to original power mode
    return setMode(originalMode);
}

int8_t readNVM(uint8_t addr, uint16_t* data)
{
    return bmp5_nvm_read(addr, data, &sensor);
}

int8_t writeNVM(uint8_t addr, uint16_t data)
{
    return bmp5_nvm_write(addr, &data, &sensor);
}

