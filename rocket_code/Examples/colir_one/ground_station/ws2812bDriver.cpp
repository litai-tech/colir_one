#include "ws2812bDriver.h"
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>

static Adafruit_NeoPixel ws2812b(WS2812B_COUNT, WS2812B_PIN, NEO_GRB + NEO_KHZ800);

// Variables for fade effect
static unsigned long lastFadeTime = 0;
static uint8_t currentBrightness = 0;
static int8_t fadeDirection = 1; // 1 = fade in, -1 = fade out
static bool fadeEnabled = false;
static ws2812b_color_t fadeColor = WS2812B_RED;

// Fade settings
#define FADE_SPEED 10        // milliseconds between brightness changes
#define MIN_BRIGHTNESS 25    // minimum brightness
#define MAX_BRIGHTNESS 255  // maximum brightness
#define FADE_STEP 1         // brightness change per step

void ws2812bInit(void){
  ws2812b.begin();           // Initialize NeoPixel object
  ws2812bSetBrightness(10);
  ws2812bSetColor(WS2812B_RED);
}

void ws2812bSetColor(ws2812b_color_t color){
  fadeEnabled = false; // Disable fade when setting solid color
  
  switch(color){
    case WS2812B_RED:
      ws2812b.setPixelColor(0, 255, 0, 0);
      ws2812b.show();
    break;
    case WS2812B_GREEN:
      ws2812b.setPixelColor(0, 0, 255, 0);
      ws2812b.show();
    break;
    case WS2812B_BLUE:
      ws2812b.setPixelColor(0, 0, 0, 255);
      ws2812b.show();
    break;
    default:
    break;
  }
}

void ws2812bSetBrightness(uint8_t brightness){
  ws2812b.setBrightness(brightness); // Set BRIGHTNESS to about 4% (max = 255)
}

void ws2812bStartFade(ws2812b_color_t color) {
  fadeEnabled = true;
  fadeColor = color;
  currentBrightness = MIN_BRIGHTNESS;
  fadeDirection = 1;
  lastFadeTime = millis();
}

void ws2812bStopFade(void) {
  fadeEnabled = false;
  ws2812b.setPixelColor(0, 0, 0, 0); // Turn off LED
  ws2812b.show();
}

void ws2812bFadeUpdate(void) {
  if (!fadeEnabled) return;
  
  unsigned long currentTime = millis();
  
  // Check if it's time to update brightness
  if (currentTime - lastFadeTime >= FADE_SPEED) {
    lastFadeTime = currentTime;
    
    // Update brightness
    currentBrightness += (fadeDirection * FADE_STEP);
    
    // Check for direction change
    if (currentBrightness >= MAX_BRIGHTNESS) {
      currentBrightness = MAX_BRIGHTNESS;
      fadeDirection = -1; // Start fading out
    } else if (currentBrightness <= MIN_BRIGHTNESS) {
      currentBrightness = MIN_BRIGHTNESS;
      fadeDirection = 1;  // Start fading in
    }
    
    // Apply the fade effect
    uint8_t red = 0, green = 0, blue = 0;
    
    switch(fadeColor) {
      case WS2812B_RED:
        red = currentBrightness;
        break;
      case WS2812B_GREEN:
        green = currentBrightness;
        break;
      case WS2812B_BLUE:
        blue = currentBrightness;
        break;
    }
    
    ws2812b.setPixelColor(0, red, green, blue);
    ws2812b.show();
  }
}

// Advanced fade with custom parameters
void ws2812bStartCustomFade(ws2812b_color_t color, uint8_t minBright, uint8_t maxBright, uint8_t speed) {
  fadeEnabled = true;
  fadeColor = color;
  currentBrightness = minBright;
  fadeDirection = 1;
  lastFadeTime = millis();
}

// Breathing effect (smooth sine wave fade)
void ws2812bBreathingUpdate(void) {
  if (!fadeEnabled) return;
  
  unsigned long currentTime = millis();
  
  if (currentTime - lastFadeTime >= FADE_SPEED) {
    lastFadeTime = currentTime;
    
    // Use sine wave for smooth breathing effect
    float breath = (sin((currentTime * 0.01)) + 1.0) / 2.0; // 0.0 to 1.0
    currentBrightness = MIN_BRIGHTNESS + (breath * (MAX_BRIGHTNESS - MIN_BRIGHTNESS));
    
    uint8_t red = 0, green = 0, blue = 0;
    
    switch(fadeColor) {
      case WS2812B_RED:
        red = currentBrightness;
        break;
      case WS2812B_GREEN:
        green = currentBrightness;
        break;
      case WS2812B_BLUE:
        blue = currentBrightness;
        break;
    }
    
    ws2812b.setPixelColor(0, red, green, blue);
    ws2812b.show();
  }
}

// Rainbow fade effect
void ws2812bRainbowFade(void) {
  static uint16_t hue = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastFadeTime >= FADE_SPEED) {
    lastFadeTime = currentTime;
    
    // Convert HSV to RGB
    uint32_t color = ws2812b.gamma32(ws2812b.ColorHSV(hue));
    ws2812b.setPixelColor(0, color);
    ws2812b.show();
    
    hue += 256; // Adjust speed of color change
  }
}

bool ws2812bIsFading(void) {
  return fadeEnabled;
}