#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(4, 5);  // CE, CSN
uint8_t TxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xAA};
typedef struct __attribute__((packed)) {
    uint8_t lighterLaunchNumber;
    uint8_t openShutes;
    uint8_t startLogs;
    uint8_t writeLogs;
    uint8_t resetAltitude;
    uint8_t removeLogs;
} colirone_payload_cmd_t;

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
    // Create an instance of payload_t and populate it with data
    colirone_payload_cmd_t colirone_payload_cmd;
    colirone_payload_cmd.lighterLaunchNumber= 4; 
    colirone_payload_cmd.openShutes = 1;           
    colirone_payload_cmd.startLogs = 0;             
    colirone_payload_cmd.writeLogs = 0;             
    colirone_payload_cmd.resetAltitude = 1;         
    colirone_payload_cmd.removeLogs = 0;             

    // Send the colirone_payload_cmd
    radio.write((uint8_t*)&colirone_payload_cmd, sizeof(colirone_payload_cmd));
    delay(1000); // Delay for 1 second before sending again
}