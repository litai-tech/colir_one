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
	while(1){
        if(colirOne.rf.hasReceivedData()){
            colirOne.rf.readColirOneCommand();
            int launchNumVal = colirOne.rf.getLighterLaunchNumber();
            uint8_t openShutesVal = colirOne.rf.getOpenShutes();
            uint8_t startLogsVal = colirOne.rf.getStartLogs();
            uint8_t writeLogsVal = colirOne.rf.getWriteLogs();
            uint8_t resetAltVal = colirOne.rf.getResetAltitude();
            uint8_t removeLogsVal =  colirOne.rf.getRemoveLogs();
    
            printf("Lighter Launch Number: %d\n", launchNumVal);
            printf("Open Shutes: %d\n", openShutesVal);
            printf("Start Logs: %d\n", startLogsVal);
            printf("Write Logs: %d\n", writeLogsVal);
            printf("Reset Altitude: %d\n", resetAltVal);
            printf("Remove Logs: %d\n", removeLogsVal);

            // uint8_t rcv_data[32] = {0}; // Buffer to hold received data
            // colirOne.rf.getReceivedData(rcv_data);
            // printf("Received data: ");
            // for(int i = 0; i < 32; i++) {
            //     printf("%02X ", rcv_data[i]);
            // }
            // printf("\n");
        }
	}
	return 0;
}