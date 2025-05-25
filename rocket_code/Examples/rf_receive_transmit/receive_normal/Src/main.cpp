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
    err = colirOne.rf.setRxAdress((uint8_t*)RxAddress);
    colirOne.rf.setRxMode();
	while(1){
        if(colirOne.rf.hasReceivedData()){
            uint8_t* data = colirOne.rf.getReceivedData();
                // printf("Received data: %s\n", data);
        }
	}
	return 0;
}