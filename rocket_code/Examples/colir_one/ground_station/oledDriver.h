#ifndef __OLED_DRIVER_H__
#define __OLED_DRIVER_H__

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
  MAIN_SCREEN,
  CONFIRM_SCREEN,
  CONNECTING_SCREEN
} oled_screen_t;

void oledInit(void);
void oledMainScreenDisplay(float verticalVelocity, float altitude);
void confirmLauchCommandDisplay(void);
void connectingScreenDisplayBounce(void);
void setCurrentScreen(oled_screen_t screen);
oled_screen_t getCurrentScreen(void);

#ifdef __cplusplus
}
#endif

#endif
