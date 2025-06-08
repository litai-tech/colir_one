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
            int launch_num = colirOne.rf.getLighterLaunchNumber();
            uint8_t is_open_shutes = colirOne.rf.getOpenShutes();
            uint8_t is_start_logs = colirOne.rf.getStartLogs();
            uint8_t is_write_logs = colirOne.rf.getWriteLogs();
            uint8_t is_reset_alt = colirOne.rf.getResetAltitude();
            uint8_t is_remove_logs =  colirOne.rf.getRemoveLogs();
    
            printf("Lighter Launch Number: %d\n", launch_num);
            printf("Open Shutes: %d\n", is_open_shutes);
            printf("Start Logs: %d\n", is_start_logs);
            printf("Write Logs: %d\n", is_write_logs);
            printf("Reset Altitude: %d\n", is_reset_alt);
            printf("Remove Logs: %d\n", is_remove_logs);

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