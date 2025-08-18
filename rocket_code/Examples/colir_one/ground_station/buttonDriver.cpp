#include "InputDebounce.h"
#include "buttonDriver.h"
#include "config.h"
#include "nrf24Driver.h"
#include "basic.h"
#include "oledDriver.h"

static InputDebounce openShutesButton;
static InputDebounce closeShutesButton;

static unsigned long button1StartPress = 0;
static unsigned long button2StartPress = 0;

static void buttonPressedCallback(uint8_t pinIn)
{
  // handle pressed state
  if(pinIn == BUTTON_1){
    button1StartPress = millis();
  }
  else{
    button2StartPress = millis();
  }
}

void buttonReleasedCallback(uint8_t pinIn)
{
  if(pinIn == BUTTON_1){
    unsigned long button1PressDuration = millis() - button1StartPress;
    Serial.print("button1 press duration: ");
    Serial.println(button1PressDuration);
    // Confirm Screen -> press No -> back to main screen
    if(getCurrentScreen() == CONFIRM_SCREEN){
      Serial.println("Back to Main Screen");
      oledMainScreenDisplay(getVerticalVelocity(), getAltitude());
    }
    // Main screen
    else if(getCurrentScreen() == MAIN_SCREEN){
      unsigned long button2PressDuration = millis() - button2StartPress;
      if(button1PressDuration >= LONG_DURATION_PRESSED){
        // Main screen with long press >= 5000 ms -> change to confirm screen
        Serial.println("Open Confirm Screen");
        confirmLauchCommandDisplay();
      }
      else{
        // Main screen with short press < 5000 ms -> send open shutes command
        colirone_payload_cmd_t payload_cmd = {0};
        payload_cmd.open_shutes = 1;
        nrf24EnqueueCMD(payload_cmd);
      }
    }
  }
  else if(pinIn == BUTTON_2){
    // Confirm Screen -> press Yes -> send launch command -> back to main screen
    if(getCurrentScreen() == CONFIRM_SCREEN){
      Serial.println("Send Launch Command");
      colirone_payload_cmd_t payload_cmd = {0};
      payload_cmd.lighter_launch_number = 1;
      nrf24EnqueueCMD(payload_cmd);     
      oledMainScreenDisplay(getVerticalVelocity(), getAltitude());
    }
    // Main Screen -> press Close -> send close shutes command
    else if(getCurrentScreen() == MAIN_SCREEN){
      Serial.println("Send close shutes Command");
      // send close shutes command
        colirone_payload_cmd_t payload_cmd = {0};
        payload_cmd.close_shutes = 1;
        nrf24EnqueueCMD(payload_cmd);
    }
  }
}

void buttonInit(void){
  openShutesButton.registerCallbacks(buttonPressedCallback, buttonReleasedCallback);
  closeShutesButton.registerCallbacks(buttonPressedCallback, buttonReleasedCallback);
  openShutesButton.setup(BUTTON_1, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);
  closeShutesButton.setup(BUTTON_2, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);
}

void buttonProcess(unsigned long now){
    openShutesButton.process(now);
    closeShutesButton.process(now); 
}