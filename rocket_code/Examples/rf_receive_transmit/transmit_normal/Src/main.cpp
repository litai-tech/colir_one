#include "main.h"
#include "../../ColirOne/Inc/colir_one.h"

const uint8_t TxAddress[] = {0xAA,0xDD,0xCC,0xBB,0xAA};

int main(void){
  	ColirOne colirOne;
  	colirone_err_t err = colirOne.init();
    if(err != COLIRONE_OK){
        printf("Error initializing colirone: %d\n", err);
        return -1;
    }
    colirOne.rf.setTxAdress((uint8_t*)TxAddress);
    colirOne.rf.setTxMode();
    char* transmit_data = "Hello World!";
	while(1){
        colirOne.rf.transmitData((uint8_t*)transmit_data, strlen(transmit_data) + 1);
        HAL_Delay(1000);
	}
	return 0;
}