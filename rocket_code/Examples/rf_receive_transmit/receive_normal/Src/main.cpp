#include "main.h"
#include "../../ColirOne/Inc/colir_one.h"

const uint8_t RxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xAA};

int main(void){
  	ColirOne colirOne;
  	colirone_err_t err = colirOne.init();
    if(err != COLIRONE_OK){
        printf("Error initializing colirone: %d\n", err);
        return -1;
    }
    colirOne.rf.setRxAdress((uint8_t*)RxAddress);
    colirOne.rf.setRxMode();
    uint8_t rcv_data[32] = {0}; // Buffer to hold received data
	while(1){
        if(colirOne.rf.hasReceivedData()){
            colirOne.rf.getReceivedData(rcv_data);
            // printf("Received data: %s\n", (char*)rcv_data);
        }
	}
	return 0;
}