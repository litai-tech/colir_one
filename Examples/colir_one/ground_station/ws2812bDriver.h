#ifndef __WS2812B_DRIVER_H__
#define __WS2812B_DRIVER_H__

#include "config.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
  WS2812B_RED,
  WS2812B_GREEN,
  WS2812B_BLUE
} ws2812b_color_t;

void ws2812bInit(void);
void ws2812bSetColor(ws2812b_color_t color);
void ws2812bSetBrightness(uint8_t brightness);

void ws2812bStartFade(ws2812b_color_t color);
void ws2812bStopFade(void);
void ws2812bFadeUpdate(void);
void ws2812bStartCustomFade(ws2812b_color_t color, uint8_t minBright, uint8_t maxBright, uint8_t speed);
void ws2812bBreathingUpdate(void);
void ws2812bRainbowFade(void);
bool ws2812bIsFading(void);

#ifdef __cplusplus
}
#endif

#endif
