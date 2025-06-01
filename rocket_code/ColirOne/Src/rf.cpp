#include "rf.h"

#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "string.h"
#include "../../Module/Inc/nrf24.h"
#include "../../Module/Inc/uart.h"

#define MAX_PAYLOAD_SIZE 32

RF::RF(void) {
}

void RF::init(void) {
  csn_high();
  ce_high();

  HAL_Delay(5);

  ce_low();

  nrf24_init();

  nrf24_auto_ack_all(no_auto_ack);
  nrf24_en_ack_pld(disable);
  nrf24_dpl(disable);

  nrf24_set_crc(no_crc, _1byte);

  nrf24_tx_pwr(_0dbm);
  nrf24_data_rate(_250kbps);
  nrf24_set_channel(76);
  nrf24_set_addr_width(5);

  nrf24_set_rx_dpl(0, disable);
  nrf24_set_rx_dpl(1, disable);
  nrf24_set_rx_dpl(2, disable);
  nrf24_set_rx_dpl(3, disable);
  nrf24_set_rx_dpl(4, disable);
  nrf24_set_rx_dpl(5, disable);

  nrf24_pipe_pld_size(0, MAX_PAYLOAD_SIZE);

  nrf24_auto_retr_delay(4);
  nrf24_auto_retr_limit(10);

  ce_high();
}

void RF::setTxAdress(uint8_t *address) {
    memcpy(txAddress, address, 5);
    ce_low();
    nrf24_open_tx_pipe(txAddress);
    ce_high();
}

void RF::setRxAdress(uint8_t *address) {
    memcpy(rxAddress, address, 5);
    ce_low();
    nrf24_open_rx_pipe(0, rxAddress);
    ce_high();
}

void RF::setTxRxAdress(uint8_t *txAddress, uint8_t *rxAddress) {
    memcpy(this->txAddress, txAddress, 5);
    memcpy(this->rxAddress, rxAddress, 5);
    ce_low();
    nrf24_open_tx_pipe(this->txAddress);
    nrf24_open_rx_pipe(0, this->rxAddress);
    ce_high();
}

void RF::setTxMode(void) {
    ce_low();
    nrf24_stop_listen();
    ce_high();
    txMode = true;
    rxMode = false;
}

void RF::setRxMode(void) {
    ce_low();
    nrf24_listen();
    ce_high();
    rxMode = true;
    txMode = false;
}

void RF::listen(void){
    nrf24_listen();
}

void RF::stopListen(void){
    nrf24_stop_listen();
}

colirone_err_t RF::transmitData(uint8_t *data, uint8_t size) {
    if (size > 32) {
        printf("Error: Data size exceeds 32 bytes.\n");
        return COLIRONE_ERROR;
    }
    colirone_err_t ret = nrf24_transmit(data, size);
    return ret;
}

bool RF::hasReceivedData(void) {
    return nrf24_data_available();
}

void RF::getReceivedData(uint8_t* rcv_data) {
    uint8_t dataR[32];
    nrf24_receive(dataR, MAX_PAYLOAD_SIZE);

    char tmp[50];
    sprintf(tmp, "Data received: %s\n", (char*)dataR);
    uart1_transmit((uint8_t*)tmp, strlen(tmp), 1000);
}

bool RF::isTxMode(void) {
    return txMode;
}

bool RF::isRxMode(void) {
    return rxMode;
}

void RF::readColirOneCommand(void) {
    nrf24_receive((uint8_t*)&colirone_payload_cmd, MAX_PAYLOAD_SIZE);
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
    colirone_err_t status = COLIRONE_OK;
    sensor_packet_t packet = {};
    packet.timestamp = timestamp;
    packet.packet_type = RF_ACCELERATION;
    memcpy(packet.data, &sensor->acceleration, sizeof(sensor->acceleration));
    status = nrf24_transmit((uint8_t*)&packet, sizeof(packet));
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    packet.packet_type = RF_GYROSCOPE;
    memcpy(packet.data, &sensor->gyroscope, sizeof(sensor->gyroscope));
    status = nrf24_transmit((uint8_t*)&packet, sizeof(packet));
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    packet.packet_type = RF_ORIENTATION;
    memcpy(packet.data, &sensor->orientation, sizeof(sensor->orientation));
    status = nrf24_transmit((uint8_t*)&packet, sizeof(packet));
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    packet.packet_type = RF_QUATERNION;
    memcpy(packet.data, &sensor->quaternion, sizeof(sensor->quaternion));
    status = nrf24_transmit((uint8_t*)&packet, sizeof(packet));
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    packet.packet_type = RF_BAROMETER;
    memcpy(packet.data, &sensor->barometer, sizeof(sensor->barometer));
    status = nrf24_transmit((uint8_t*)&packet, sizeof(packet));
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    packet.packet_type = RF_GPS;
    memcpy(packet.data, &sensor->gps, sizeof(sensor->gps));
    status = nrf24_transmit((uint8_t*)&packet, sizeof(packet));
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    packet.packet_type = RF_VERTICAL_VELOCITY;
    packet.data[0] = (uint8_t)(sensor->vertical_velocity);
    status = nrf24_transmit((uint8_t*)&packet, sizeof(packet));
    COLIRONE_CHECK_ERROR(status);
    HAL_Delay(15);

    return COLIRONE_OK;
}