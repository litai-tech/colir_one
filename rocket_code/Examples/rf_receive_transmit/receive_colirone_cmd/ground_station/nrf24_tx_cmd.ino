#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(4, 5);  // CE, CSN
uint8_t TxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xAA};
typedef struct __attribute__((packed)) {
    uint8_t lighter_launch_number;
    uint8_t open_shutes;
    uint8_t start_logs;
    uint8_t write_logs;
    uint8_t reset_altitude;
    uint8_t remove_logs;
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
    colirone_payload_cmd.lighter_launch_number= 4; 
    colirone_payload_cmd.open_shutes = 1;           
    colirone_payload_cmd.start_logs = 0;             
    colirone_payload_cmd.write_logs = 0;             
    colirone_payload_cmd.reset_altitude = 1;         
    colirone_payload_cmd.remove_logs = 0;             

    // Send the colirone_payload_cmd
    radio.write((uint8_t*)&colirone_payload_cmd, sizeof(colirone_payload_cmd));
    delay(1000); // Delay for 1 second before sending again
}