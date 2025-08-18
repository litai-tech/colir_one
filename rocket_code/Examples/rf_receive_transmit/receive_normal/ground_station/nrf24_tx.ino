#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(4, 5);  // CE, CSN

uint8_t TxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xAA};

void setup()
{
  Serial.begin(115200);
  
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(5,5);
  radio.disableCRC();

  radio.openWritingPipe(TxAddress);
  radio.stopListening();
}

void loop()
{
  const char text[] = "Hello World";
  bool success = radio.write(&text, sizeof(text));
  if(success){
    Serial.println("Transmit success");
  }
  delay(1000);
}