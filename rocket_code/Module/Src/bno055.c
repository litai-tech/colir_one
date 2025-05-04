#include "bno055.h"
#include <string.h>
#include <stm32f4xx_hal_i2c.h>

uint16_t accelScale = 100;
uint16_t tempScale = 1;
uint16_t angularRateScale = 16;
uint16_t eulerScale = 16;
uint16_t magScale = 16;
uint16_t quaScale = (1<<14);    // 2^14

I2C_HandleTypeDef *_bno055_i2c_port;

void bno055_assignI2C(I2C_HandleTypeDef *hi2c_device) {
  _bno055_i2c_port = hi2c_device;
}

static void bno055_delay(int time) {
#ifdef FREERTOS_ENABLED
  osDelay(time);
#else
  HAL_Delay(time);
#endif
}

static HAL_StatusTypeDef bno055_writeData(uint8_t reg, uint8_t data) {
  uint8_t txdata[2] = {reg, data};
  HAL_StatusTypeDef status;
  status = HAL_I2C_Master_Transmit(_bno055_i2c_port, BNO055_I2C_ADDR << 1,
                                   txdata, sizeof(txdata), 10);
  return status;
}

static HAL_StatusTypeDef bno055_readData(uint8_t reg, uint8_t *data, uint8_t len) {
  HAL_StatusTypeDef status = HAL_OK;
  status |= HAL_I2C_Master_Transmit(_bno055_i2c_port, BNO055_I2C_ADDR << 1, &reg, 1,
                          100);
  status |= HAL_I2C_Master_Receive(_bno055_i2c_port, BNO055_I2C_ADDR << 1, data, len,
                         100);
  return status;
}


static HAL_StatusTypeDef bno055_setPage(uint8_t page) { 
  HAL_StatusTypeDef status = bno055_writeData(BNO055_PAGE_ID, page); 
  return status;
}

HAL_StatusTypeDef bno055_getOperationMode(bno055_opmode_t *mode) {
  // bno055_opmode_t mode;
  HAL_StatusTypeDef status= bno055_readData(BNO055_OPR_MODE, mode, 1);
  return status;
}

HAL_StatusTypeDef bno055_setOperationMode(bno055_opmode_t mode) {
  HAL_StatusTypeDef status = bno055_writeData(BNO055_OPR_MODE, mode);
  if (mode == BNO055_OPERATION_MODE_CONFIG) {
    bno055_delay(19);
  } else {
    bno055_delay(7);
  }
  return status;
}

HAL_StatusTypeDef bno055_setOperationModeConfig(void) {
  HAL_StatusTypeDef status =  bno055_setOperationMode(BNO055_OPERATION_MODE_CONFIG);
  return status;
}

HAL_StatusTypeDef bno055_setOperationModeNDOF(void) {
  HAL_StatusTypeDef status = bno055_setOperationMode(BNO055_OPERATION_MODE_NDOF);
  return status;
}

static HAL_StatusTypeDef bno055_setExternalCrystalUse(bool state) {
  HAL_StatusTypeDef status = HAL_OK;
  status |= bno055_setPage(0);
  uint8_t tmp = 0;
  status |= bno055_readData(BNO055_SYS_TRIGGER, &tmp, 1);
  tmp |= (state == true) ? 0x80 : 0x0;
  status |= bno055_writeData(BNO055_SYS_TRIGGER, tmp);
  bno055_delay(700);
  return status;
}

HAL_StatusTypeDef bno055_enableExternalCrystal(void) { 
  HAL_StatusTypeDef status =  bno055_setExternalCrystalUse(true); 
  return status;
}
HAL_StatusTypeDef bno055_disableExternalCrystal(void) { 
  HAL_StatusTypeDef status = bno055_setExternalCrystalUse(false); 
  return status;
}

HAL_StatusTypeDef bno055_reset(void) {
  HAL_StatusTypeDef status = bno055_writeData(BNO055_SYS_TRIGGER, 0x20);
  bno055_delay(700);
  return status;
}

HAL_StatusTypeDef bno055_getTemp(uint8_t *temp) {
  HAL_StatusTypeDef status = HAL_OK;
  status |= bno055_setPage(0);
  status |= bno055_readData(BNO055_TEMP, temp, 1);
  return status;
}

HAL_StatusTypeDef bno055_setup(void) {
  HAL_StatusTypeDef status = HAL_OK;
  status |= bno055_reset();

  uint8_t id = 0;
  status |= bno055_readData(BNO055_CHIP_ID, &id, 1);
  if (id != BNO055_ID) {
    printf("Can't find BNO055, id: 0x%02x. Please check your wiring.\r\n", id);
  }
  status |= bno055_setPage(0);
  status |= bno055_writeData(BNO055_SYS_TRIGGER, 0x0);

  // Select BNO055 config mode
  status |= bno055_setOperationModeConfig();
  bno055_delay(10);
  return status;
}

HAL_StatusTypeDef bno055_getSWRevision(int16_t *swRev) {
  HAL_StatusTypeDef status = HAL_OK;
  status |= bno055_setPage(0);
  uint8_t buffer[2];
  status |= bno055_readData(BNO055_SW_REV_ID_LSB, buffer, 2);
  *swRev = (int16_t)((buffer[1] << 8) | buffer[0]);
  return status;
}

HAL_StatusTypeDef bno055_getBootloaderRevision(uint8_t* blRev) {
  HAL_StatusTypeDef status = HAL_OK;
  status |= bno055_setPage(0);
  status |= bno055_readData(BNO055_BL_REV_ID, blRev, 1);
  return status;
}

HAL_StatusTypeDef bno055_getSystemStatus(uint8_t *sysStatus) {
  HAL_StatusTypeDef status = HAL_OK;
  status |= bno055_setPage(0);
  status |= bno055_readData(BNO055_SYS_STATUS, sysStatus, 1);
  return status;
}

HAL_StatusTypeDef bno055_getSelfTestResult(bno055_self_test_result_t *result) {
  HAL_StatusTypeDef status = HAL_OK;
  status |= bno055_setPage(0);
  uint8_t tmp;
  status |= bno055_readData(BNO055_ST_RESULT, &tmp, 1);
  bno055_self_test_result_t _result ={0};
  _result.mcuState = (tmp >> 3) & 0x01;
  _result.gyrState = (tmp >> 2) & 0x01;
  _result.magState = (tmp >> 1) & 0x01;
  _result.accState = (tmp >> 0) & 0x01;

  *result = _result;
  return status;
}

HAL_StatusTypeDef bno055_getSystemError(uint8_t* sysError) {
  HAL_StatusTypeDef status = HAL_OK;
  status |= bno055_setPage(0);
  status |= bno055_readData(BNO055_SYS_ERR, sysError, 1);
  return status;
}

HAL_StatusTypeDef bno055_getCalibrationState(bno055_calibration_state_t* calState) {
  HAL_StatusTypeDef status = HAL_OK;
  status |= bno055_setPage(0);
  bno055_calibration_state_t _calState = {.sys = 0, .gyro = 0, .mag = 0, .accel = 0};
  uint8_t tmp = 0;
  status |= bno055_readData(BNO055_CALIB_STAT, &tmp, 1);
  _calState.sys = (tmp >> 6) & 0x03;
  _calState.gyro = (tmp >> 4) & 0x03;
  _calState.accel = (tmp >> 2) & 0x03;
  _calState.mag = tmp & 0x03;

  *calState = _calState;
  return status;
}


HAL_StatusTypeDef bno055_getCalibrationData(bno055_calibration_data_t *calData) {
  bno055_calibration_data_t _calData;
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t buffer[22];
  bno055_opmode_t operationMode = {0}; 
  status |= bno055_getOperationMode(&operationMode);
  status |= bno055_setOperationModeConfig();
  status |= bno055_setPage(0);

  status |= bno055_readData(BNO055_ACC_OFFSET_X_LSB, buffer, 22);

  // Assumes little endian processor
  memcpy(&_calData.offset.accel, buffer, 6);
  memcpy(&_calData.offset.mag, buffer + 6, 6);
  memcpy(&_calData.offset.gyro, buffer + 12, 6);
  memcpy(&_calData.radius.accel, buffer + 18, 2);
  memcpy(&_calData.radius.mag, buffer + 20, 2);

  *calData = _calData;

  bno055_setOperationMode(operationMode);

  return status;
}

HAL_StatusTypeDef bno055_setCalibrationData(bno055_calibration_data_t calData) {
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t buffer[22];
  bno055_opmode_t operationMode = {0};
  status |= bno055_getOperationMode(&operationMode);
  status |= bno055_setOperationModeConfig();
  status |= bno055_setPage(0);

  // Assumes litle endian processor
  memcpy(buffer, &calData.offset.accel, 6);
  memcpy(buffer + 6, &calData.offset.mag, 6);
  memcpy(buffer + 12, &calData.offset.gyro, 6);
  memcpy(buffer + 18, &calData.radius.accel, 2);
  memcpy(buffer + 20, &calData.radius.mag, 2);

  for (uint8_t i=0; i < 22; i++) {
    // TODO(oliv4945): create multibytes write
    status |= bno055_writeData(BNO055_ACC_OFFSET_X_LSB+i, buffer[i]);
  }

  status |= bno055_setOperationMode(operationMode);
  return status;
}

static HAL_StatusTypeDef bno055_getVector(uint8_t vec, bno055_vector_t* xyz) {
  HAL_StatusTypeDef status = HAL_OK;
  status |= bno055_setPage(0);
  uint8_t buffer[8];    // Quaternion need 8 bytes

  if (vec == BNO055_VECTOR_QUATERNION)
    status |= bno055_readData(vec, buffer, 8);
  else
    status |= bno055_readData(vec, buffer, 6);

  double scale = 1;

  if (vec == BNO055_VECTOR_MAGNETOMETER) {
    scale = magScale;
  } else if (vec == BNO055_VECTOR_ACCELEROMETER ||
           vec == BNO055_VECTOR_LINEARACCEL || vec == BNO055_VECTOR_GRAVITY) {
    scale = accelScale;
  } else if (vec == BNO055_VECTOR_GYROSCOPE) {
    scale = angularRateScale;
  } else if (vec == BNO055_VECTOR_EULER) {
    scale = eulerScale;
  } else if (vec == BNO055_VECTOR_QUATERNION) {
    scale = quaScale;
  }

  bno055_vector_t _xyz = {.w = 0, .x = 0, .y = 0, .z = 0};
  if (vec == BNO055_VECTOR_QUATERNION) {
    _xyz.w = (int16_t)((buffer[1] << 8) | buffer[0]) / scale;
    _xyz.x = (int16_t)((buffer[3] << 8) | buffer[2]) / scale;
    _xyz.y = (int16_t)((buffer[5] << 8) | buffer[4]) / scale;
    _xyz.z = (int16_t)((buffer[7] << 8) | buffer[6]) / scale;
  } else {
    _xyz.x = (int16_t)((buffer[1] << 8) | buffer[0]) / scale;
    _xyz.y = (int16_t)((buffer[3] << 8) | buffer[2]) / scale;
    _xyz.z = (int16_t)((buffer[5] << 8) | buffer[4]) / scale;
  }

  *xyz = _xyz;
  return status;
}

HAL_StatusTypeDef bno055_getVectorAccelerometer(bno055_vector_t* xyz) {
  HAL_StatusTypeDef status = bno055_getVector(BNO055_VECTOR_ACCELEROMETER, xyz);
  return status;
}
HAL_StatusTypeDef bno055_getVectorMagnetometer(bno055_vector_t* xyz) {
  HAL_StatusTypeDef status = bno055_getVector(BNO055_VECTOR_MAGNETOMETER, xyz);
  return status;
}
HAL_StatusTypeDef bno055_getVectorGyroscope(bno055_vector_t* xyz) {
  HAL_StatusTypeDef status = bno055_getVector(BNO055_VECTOR_GYROSCOPE, xyz);
  return status;
}
HAL_StatusTypeDef bno055_getVectorEuler(bno055_vector_t* xyz) {
  HAL_StatusTypeDef status = bno055_getVector(BNO055_VECTOR_EULER, xyz);
  return status;
}
HAL_StatusTypeDef bno055_getVectorLinearAccel(bno055_vector_t* xyz) {
  HAL_StatusTypeDef status = bno055_getVector(BNO055_VECTOR_LINEARACCEL, xyz);
  return status;
}
HAL_StatusTypeDef bno055_getVectorGravity(bno055_vector_t* xyz) {
  HAL_StatusTypeDef status = bno055_getVector(BNO055_VECTOR_GRAVITY, xyz);
  return status;
}
HAL_StatusTypeDef bno055_getVectorQuaternion(bno055_vector_t* xyz) {
  HAL_StatusTypeDef status = bno055_getVector(BNO055_VECTOR_QUATERNION, xyz);
  return status;
}

HAL_StatusTypeDef bno055_setAxisMap(bno055_axis_map_t axis) {
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t axisRemap = (axis.z << 4) | (axis.y << 2) | (axis.x);
  uint8_t axisMapSign = (axis.x_sign << 2) | (axis.y_sign << 1) | (axis.z_sign);
  status |= bno055_writeData(BNO055_AXIS_MAP_CONFIG, axisRemap);
  status |= bno055_writeData(BNO055_AXIS_MAP_SIGN, axisMapSign);
  return status;
}
