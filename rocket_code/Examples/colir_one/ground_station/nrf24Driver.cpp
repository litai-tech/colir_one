#include "nrf24Driver.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

static RF24 radio(RF24_CE_PIN, RF24_CSN_PIN); // CE, CSN

static colirone_payload_cmd_t colirone_payload_cmd_queue[NRF24_QUEUE_SIZE];
static int queueHead = 0;
static int queueTail = 0;

void nrf24Init(void){
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(5,5);
  radio.disableCRC();
  radio.openReadingPipe(0, RxAddress);
  radio.openWritingPipe(TxAddress);
  radio.startListening();
}

bool nrf24Available(void){
  return radio.available();
}

void nrf24Read(uint8_t* buffer, size_t len){
  radio.read(buffer, len);
}

static void transmit_cmd(colirone_payload_cmd_t colirone_payload_cmd){
  radio.stopListening();
  int ret = radio.write((uint8_t*)&colirone_payload_cmd, sizeof(colirone_payload_cmd));
  if(ret == 0){
    Serial.println("transmit_launch_cmd success");
  }
  delay(50);
  radio.startListening();  
}

void nrf24EnqueueCMD(colirone_payload_cmd_t command) {
    if ((queueTail + 1) % NRF24_QUEUE_SIZE != queueHead) {
        colirone_payload_cmd_queue[queueTail] = command;
        queueTail = (queueTail + 1) % NRF24_QUEUE_SIZE;
    }
}

void nrf24ProcessCMD(void) {
    if (queueHead != queueTail) {
        colirone_payload_cmd_t colirone_payload_cmd = colirone_payload_cmd_queue[queueHead]; 
        queueHead = (queueHead + 1) % NRF24_QUEUE_SIZE;
        transmit_cmd(colirone_payload_cmd);
    }
}