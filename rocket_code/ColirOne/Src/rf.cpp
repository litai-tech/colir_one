#include "rf.h"

#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "string.h"
#include "../../Module/Inc/NRF24L01.h"

RF::RF(void) {
}

colirone_err_t RF::init(void) {
    HAL_StatusTypeDef ret = nrf24_init();
    if (ret != HAL_OK) {
        return COLIRONE_ERROR;
    }
    return COLIRONE_OK;
}

colirone_err_t RF::setTxAdress(uint8_t *address) {
    memcpy(txAddress, address, 5);
    HAL_StatusTypeDef ret = nrf24_TxRxMode(txAddress, NULL, 0);
    if (ret != HAL_OK) {
        return COLIRONE_ERROR;
    }
    return COLIRONE_OK;
}

colirone_err_t RF::setRxAdress(uint8_t *address) {
    memcpy(rxAddress, address, 5);
    HAL_StatusTypeDef ret = nrf24_TxRxMode(NULL, rxAddress, 0);
    if (ret != HAL_OK) {
        return COLIRONE_ERROR;
    }
    return COLIRONE_OK;
}

colirone_err_t RF::setTxRxAdress(uint8_t *txAddress, uint8_t *rxAddress) {
    memcpy(this->txAddress, txAddress, 5);
    memcpy(this->rxAddress, rxAddress, 5);
    HAL_StatusTypeDef ret = nrf24_TxRxMode(this->txAddress, this->rxAddress, 0);
    if (ret != HAL_OK) {
        return COLIRONE_ERROR;
    }
    return COLIRONE_OK;
}

colirone_err_t RF::setTxMode(void) {
    HAL_StatusTypeDef ret = nrf24_TxMode();
    if (ret != HAL_OK) {
        return COLIRONE_ERROR;
    }
    txMode = true;
    rxMode = false;
    return COLIRONE_OK;
}

colirone_err_t RF::setRxMode(void) {
    HAL_StatusTypeDef ret = nrf24_RxMode();
    if (ret != HAL_OK) {
        return COLIRONE_ERROR;
    }
    rxMode = true;
    txMode = false;
    return COLIRONE_OK;
}

colirone_err_t RF::sendData(uint8_t *data, uint8_t size) {
    if (size > 32) {
        printf("Error: Data size exceeds 32 bytes.\n");
        return COLIRONE_ERROR;
    }
    HAL_StatusTypeDef ret = nrf24_transmit(data);
    if (ret != HAL_OK) {
        return COLIRONE_ERROR;
    }
    return COLIRONE_OK;
}

bool RF::hasReceivedData(void) {
    return nrf24_check_data_available();
}

uint8_t* RF::getReceivedData(void) {
    uint8_t data[32];
    memset(&data, 0, sizeof(data));
    HAL_StatusTypeDef ret = nrf24_receive(data);
    for(int i = 0; i < 32; i++) {
        if (data[i] == 0) {
            data[i] = '\0'; // Null-terminate the string
            break;
        }
        printf("0x%02X ", data[i]);
        // printf("\n");
        // printf("%d: %d ",i, data[i]);
    }
    printf("received data: %s\n", data);
    if (ret != HAL_OK) {
        return NULL;
    }
    return data;
}

bool RF::isTxMode(void) {
    return txMode;
}

bool RF::isRxMode(void) {
    return rxMode;
}

void RF::readColirOneCommand(void) {
    nrf24_receive((uint8_t*)&colirone_payload_cmd);
}

uint8_t RF::getLighterLaunchNumber(void) {
    return colirone_payload_cmd.lighter_launch.number;
}

uint8_t RF::getOpenShutes(void) {
    return colirone_payload_cmd.open_shutes;
}

uint8_t RF::getStartLogs(void) {
    return colirone_payload_cmd.start_logs;
}

uint8_t RF::getWriteLogs(void) {
    return colirone_payload_cmd.write_logs;
}

uint8_t RF::getResetAltitude(void) {
    return colirone_payload_cmd.reset_altitude;
}

uint8_t RF::getRemoveLogs(void) {
    return colirone_payload_cmd.remove_logs;
}


colirone_err_t RF::transmitSensorData(colirone_payload_sensor_t *sensor, uint32_t timestamp) {
    HAL_StatusTypeDef status = HAL_OK;
    sensor_packet_t packet = {0};
    packet.timestamp = timestamp;
    packet.index = 0;
    memcpy(packet.data, &sensor->acceleration, sizeof(sensor->acceleration));
    status = nrf24_transmit((uint8_t*)&packet);
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    packet.index = 1;
    memcpy(packet.data, &sensor->gyroscope, sizeof(sensor->gyroscope));
    status = nrf24_transmit((uint8_t*)&packet);
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    packet.index = 2;
    memcpy(packet.data, &sensor->orientation, sizeof(sensor->orientation));
    status = nrf24_transmit((uint8_t*)&packet);
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    packet.index = 3;
    memcpy(packet.data, &sensor->quaternion, sizeof(sensor->quaternion));
    status = nrf24_transmit((uint8_t*)&packet);
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    packet.index = 4;
    memcpy(packet.data, &sensor->barometer, sizeof(sensor->barometer));
    status = nrf24_transmit((uint8_t*)&packet);
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    packet.index = 5;
    memcpy(packet.data, &sensor->gps, sizeof(sensor->gps));
    status = nrf24_transmit((uint8_t*)&packet);
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    packet.index = 6;
    packet.data[0] = (uint8_t)(sensor->vertical_velocity);
    status = nrf24_transmit((uint8_t*)&packet);
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    return COLIRONE_OK;
}