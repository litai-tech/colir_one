#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(4, 5);  // CE, CSN

//address through which two modules communicate.
uint8_t RxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xAA};

void setup()
{
  Serial.begin(115200);
  
  radio.begin();
  
  //set the address
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(0, RxAddress);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(5,5);
  radio.disableCRC();

  // radio.openWritingPipe(TxAddress);
  // radio.stopListening();
  
  //Set module as receiver
  radio.startListening();
}

void loop()
{
  if(radio.available()){
    char text[32] = {0};
    uint8_t len = radio.getDynamicPayloadSize();
    if(len >= 1){
      radio.read(&text, len);
      text[len < 32 ? len : 31] = '\0';
      Serial.println(text);    
    }
  }
}