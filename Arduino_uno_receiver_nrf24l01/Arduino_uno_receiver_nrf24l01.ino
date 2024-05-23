#include <SPI.h>
#include "printf.h"
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN

const byte RxAddress[5] = {0xEE,0xDD,0xCC,0xBB,0xAA};
const byte TxAddress[5] = {0xAA,0xDD,0xCC,0xBB,0xAA};
int counter = 0;

String incomingCmd = "";
uint8_t dataToSend[32];

uint8_t cmd[] = {1, 255, 7, 255, 15};

bool sendResult;
unsigned long lastTx;

void setup() {
  Serial.begin(9600);
  Serial.println("Start setup.");
  if(!radio.begin()) {
    Serial.println("Radio is not responding!!!!");
    while(1) {}
  }
  radio.setAutoAck(false);
  radio.setDataRate(RF24_2MBPS);
  radio.openReadingPipe(1, RxAddress);
  radio.openWritingPipe(TxAddress);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(5,5);
  printf_begin();
  //radio.printDetails();
  radio.startListening();
}

void loop() {
  if (Serial.available() > 0) {
    incomingCmd = Serial.readString();
    
    clearBuffer(dataToSend, sizeof(dataToSend));
    parseCommand(incomingCmd, dataToSend);
  }
  if (radio.available()) {
    char text[33] = {0}; // Max size is 32 but last byte is for debug string ending.
    radio.read(&text, sizeof(text));
    Serial.println(text);

    if(text[0] == 'c'){
      radio.stopListening();
      lastTx = millis();
      while(millis() - lastTx < 200){
        sendResult = radio.write(&dataToSend, sizeof(dataToSend));
        delayMicroseconds(100);
      }
      radio.startListening();
      if (sendResult) {
        //Serial.print("Command sent ");
        //Serial.println(dataToSend);
        clearBuffer(dataToSend, sizeof(dataToSend));
      }
      else {
        Serial.println("Tx failed");
      }
    }
  }
}

void clearBuffer(uint8_t buff[], int buffSize) {
  for(int i = 0; i < buffSize; i++){
    buff[i] = {0};
  }
}

void parseCommand(String data, uint8_t ackCmd[]) {
  for(int i = 0; i < data.length(); i++){
    ackCmd[i] = data[i];
  }
}
