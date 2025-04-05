#include <SPI.h>
#include "printf.h"
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include "InputDebounce.h"
#include <U8g2lib.h>

/* ----------------------------------------------------------------MACROS AND DEFINES---------------------------------------------------------------- */
/**
 * @brief Button definition
 */
#define BUTTON_1                            13
#define BUTTON_2                            14
#define BUTTON_DEBOUNCE_DELAY               80
#define LONG_DURATION_PRESSED               5000

/**
 * @brief Led strip definition
 */
#define LED_PIN                             17
#define LED_COUNT                           1

/**
 * @brief RF24 definition
 */
#define RF24_CE_PIN                         4
#define RF24_CSN_PIN                        5

/**
 * @brief I2C definition
 */
#define I2C_OLED_ADDRESS                    0x3C
#define I2C_MASTER_CLK_PIN                  22
#define I2C_MASTER_SDA_PIN                  21

/**
 * @brief SPI definition
 */
#define SPI_MISO_PIN                        19
#define SPI_MOSI_PIN                        23
#define SPI_SCK_PIN                         18
#define SPI_DIN_PIN                         17

/**
 * @brief OLED(SSD1306) definition
 */
#define SCREEN_WIDTH                        128 // OLED display width, in pixels
#define SCREEN_HEIGHT                       64  // OLED display height, in pixels
#define OLED_RESET                          -1

#define FONT_HEIGHT                         12
#define FONT_WIDTH                          6
#define LINE_SPACING                        FONT_HEIGHT / 3

#define OLED_WIDTH                          128
#define OLED_HEIGHT                         64
#define OLED_MAX_SYMBOLS_IN_1_LINE          (OLED_WIDTH / FONT_WIDTH)
#define OLED_MAX_LINES                      (OLED_HEIGHT / FONT_HEIGHT)
#define OLED_CENTER                         (OLED_MAX_SYMBOLS_IN_1_LINE / 2)

/* ----------------------------------------------------------------MACROS AND DEFINES---------------------------------------------------------------- */

/* ----------------------------------------------------------------CONST VARIABLES---------------------------------------------------------------- */
/**
 * @brief OLED display Colir One Logo
 */
static const unsigned char PROGMEM logo[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
    0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFC,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFE, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xC0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF,
    0x80, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFC, 0x00,
    0x00, 0x00, 0x00, 0x3F, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0xFF, 0xFE, 0x00, 0x00,
    0x00, 0x00, 0x3F, 0xFF, 0xF8, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00,
    0x00, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00,
    0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x0F,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x07, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x03, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x03, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xFE, 0x00, 0x07, 0xFC, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xE0, 0x00, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0x00, 0x07, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1F, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1F, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/**
 * @brief RF24 CMD
 */
const char* RF24_LAUNCH_CMD        =        "l 1"; //fire lighter number 1
const char* RF24_OPEN_SHUTES_CMD   =        "s 8 40"; // set for servo number 8 angle = 40
const char* RF24_CLOSE_SHUTES_CMD  =        "s 8 80"; // set for servo number 8 angle = 80

/**
 * @brief RF24 TX-RX Adress
 */
const byte RxAddress[5]            =        {0xEE,0xDD,0xCC,0xBB,0xAA};
const byte TxAddress[5]            =        {0xAA,0xDD,0xCC,0xBB,0xAA};
/* ----------------------------------------------------------------CONST VARIABLES---------------------------------------------------------------- */

/* ----------------------------------------------------------------OBJECT---------------------------------------------------------------- */
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
RF24 radio(RF24_CE_PIN, RF24_CSN_PIN); // CE, CSN
U8G2_SSD1309_128X64_NONAME2_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 21);
InputDebounce openShutesButton;
InputDebounce closeShutesButton;
/* ----------------------------------------------------------------OBJECT---------------------------------------------------------------- */

/* ----------------------------------------------------------------GLOBAL VARIABLES---------------------------------------------------------------- */
int counter = 0;
String incomingCmd = "";
uint8_t dataToSend[32];
// uint8_t cmd[] = {1, 255, 7, 255, 15};
bool sendResult;
unsigned long lastTx;
String lastAltitude = "";
String lastVerticalVelocity = "";
unsigned long lastPackageTime = 0;
bool connected = false;
bool isConfirmScreen = false;
unsigned long button1StartPress = 0;
unsigned long button2StartPress = 0;

/* ----------------------------------------------------------------GLOBAL VARIABLES---------------------------------------------------------------- */

/* ----------------------------------------------------------------GLOBAL FUNCTION PROTOTYPES---------------------------------------------------------------- */
/**
 * @brief Button Pressed Call Back
 *
 * @param       pinIn: Button PIN call back
 */
void buttonPressedCallback(uint8_t pinIn);

/**
 * @brief Button Released Call Back
 *
 * @param       pinIn: Button PIN call back
 */
void buttonReleasedCallback(uint8_t pinIn);

/**
 * @brief clear buffer
 *
 * @param       buff: Buffer Array
 * @param       buffSize: Buffer size
 */
void clearBuffer(uint8_t buff[], int buffSize);

/**
 * @brief RF24 parse receive command
 *
 * @param       data: RF24 Data receive
 * @param       ackCmd:  ACK CMD
 */
void parseCommand(String data, uint8_t ackCmd[]);

/**
 * @brief RF24 get value form command
 *
 * @param       data: command receieve
 * @param       separator: separator character
 * @param       index: index of character
 */
String getValue(String data, char separator, int index);

/**
 * @brief RF24 send command to device
 *
 * @param       cmd: command send to device
 */
bool sendCommand(const char* cmd);

/**
 * @brief OLED main screen display
 *
 * @param       verticalVelocity: vertical velocity receive from device
 * @param       altitude: altitude receive from device
 */
void oledMainScreenDisplay(String verticalVelocity, String altitude);

 /**
 * @brief OLED Cofirm lauch command screen display
 */
void confirmLauchCommandDisplay(void);

/* ----------------------------------------------------------------GLOBAL FUNCTION PROTOTYPES---------------------------------------------------------------- */

/* ----------------------------------------------------------------MAIN PROGRAM---------------------------------------------------------------- */
void setup() {
  
  Serial.begin(115200);
  // Led stip init
  strip.begin();           // Initialize NeoPixel object
  strip.setBrightness(10); // Set BRIGHTNESS to about 4% (max = 255)
  strip.setPixelColor(0, 255, 0, 0);
  strip.show();

  // Radio init
  if(!radio.begin()) {
    Serial.println("RF24 init failed");
    while(1) {}
  }
  // Button init
  openShutesButton.registerCallbacks(buttonPressedCallback, buttonReleasedCallback);
  closeShutesButton.registerCallbacks(buttonPressedCallback, buttonReleasedCallback);
  openShutesButton.setup(BUTTON_1, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);
  closeShutesButton.setup(BUTTON_2, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);

  // oled init
  u8g2.setBusClock(1000000);
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x12_t_symbols);
  u8g2.enableUTF8Print();
  u8g2.setFontMode(1);
  u8g2.clearBuffer();
  u8g2.drawBitmap(4, 12, 15, 39, logo);// cnt = wid / 8, h = height
  u8g2.sendBuffer();

  // radio config
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, RxAddress);
  radio.openWritingPipe(TxAddress);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(5,5);
  printf_begin();
  //radio.printDetails();
  radio.startListening();
}

void loop() {
  unsigned long now = millis();
  if(now - lastPackageTime > 1000)
    connected = false;
  unsigned int openShutesButtonState = openShutesButton.process(now);
  unsigned int closeShutesButtonState = closeShutesButton.process(now);
  if(connected){
    strip.setPixelColor(0, 0, 255, 0);
    strip.show();
    if(!isConfirmScreen){
      oledMainScreenDisplay(lastVerticalVelocity, lastAltitude);
    }
    // Serial.print("vertical: ");
    // Serial.println(lastVerticalVelocity);
    // Serial.print("alt: ");
    // Serial.println(lastAltitude);
  }
  else{
    strip.setPixelColor(0, 255, 0, 0);
    strip.show();
  }

  if (Serial.available() > 0) {
    incomingCmd = Serial.readString();
    
    clearBuffer(dataToSend, sizeof(dataToSend));
    parseCommand(incomingCmd, dataToSend);
  }
  if (radio.available()) {
    lastPackageTime = now;
    connected = true;
    char text[33] = {0}; // Max size is 32 but last byte is for debug string ending.
    radio.read(&text, sizeof(text));
    Serial.println(text);

    if(text[0] == 'c'){
      // radio.stopListening();
      // lastTx = millis();
      // sendResult = radio.write(&dataToSend, sizeof(dataToSend));
      // delay(50);
      // radio.startListening();
      // if (sendResult) {
      //   Serial.print("Command sent ");
      //   Serial.println((char *)dataToSend);
      //   clearBuffer(dataToSend, sizeof(dataToSend));
      // }
      // else {
      //   Serial.println("Tx failed");
      // }
    }
    else if(text[0] == 'h'){
      lastAltitude = getValue(text, ' ', 2);
    }
    else if(text[0] == 'v'){
      lastVerticalVelocity = getValue(text, ' ', 2);
    }
  }
  delay(1);
}
/* ----------------------------------------------------------------MAIN PROGRAM---------------------------------------------------------------- */

/* ----------------------------------------------------------------GLOBAL FUNCTIONS---------------------------------------------------------------- */
void oledMainScreenDisplay(String verticalVelocity, String altitude){
  u8g2.clearBuffer();
  //Line 1
  u8g2.drawGlyph(0*FONT_WIDTH, FONT_HEIGHT, 0x2195); // 8 symbols before
  u8g2.setCursor(2*FONT_WIDTH, FONT_HEIGHT);
  u8g2.print("Velocity");
  u8g2.setCursor(10*FONT_WIDTH, FONT_HEIGHT); // 10 symbols before
  u8g2.print(":");
  u8g2.setCursor(13*FONT_WIDTH, FONT_HEIGHT); //13 or 14
  u8g2.print(verticalVelocity + "m/s");

  //Line 2
  u8g2.setCursor(0*FONT_WIDTH, 2*FONT_HEIGHT + LINE_SPACING);
  u8g2.print("Altitude:");
  u8g2.setCursor(13*FONT_WIDTH, 2*FONT_HEIGHT + LINE_SPACING); //13 or 14
  u8g2.print(altitude + "m");

  //Line 3
  const char* line3_str = "Shutes";
  u8g2.setCursor(OLED_CENTER*FONT_WIDTH - (strlen(line3_str) / 2)*FONT_WIDTH, 3*FONT_HEIGHT + LINE_SPACING*2);
  u8g2.print(line3_str);
  //Line 4
  u8g2.drawGlyph(0*FONT_WIDTH, 4*FONT_HEIGHT + LINE_SPACING*3, 0x2193);
  u8g2.setCursor(1*FONT_WIDTH, 4*FONT_HEIGHT + LINE_SPACING*3);
  u8g2.print("Open");
  const char* close_str = "Close";
  u8g2.setCursor((OLED_MAX_SYMBOLS_IN_1_LINE)*FONT_WIDTH - 1*FONT_WIDTH - (strlen(close_str)*FONT_WIDTH), 4*FONT_HEIGHT + LINE_SPACING*3);
  u8g2.print(close_str);
  u8g2.drawGlyph((OLED_MAX_SYMBOLS_IN_1_LINE)*FONT_WIDTH - 1*FONT_WIDTH, 4*FONT_HEIGHT + LINE_SPACING*3, 0x2193);
  u8g2.sendBuffer();
}

void confirmLauchCommandDisplay(void){
  u8g2.clearBuffer();

  u8g2.setCursor(1*FONT_WIDTH, (((OLED_MAX_LINES - 1)/2)*FONT_HEIGHT));
  u8g2.print("Send Launch Command?");

  // final line
  u8g2.drawGlyph(0*FONT_WIDTH, 4*FONT_HEIGHT + LINE_SPACING*3, 0x2193);
  u8g2.setCursor(1*FONT_WIDTH, 4*FONT_HEIGHT + LINE_SPACING*3);
  u8g2.print("No");
  const char* yes_str = "Yes";
  u8g2.setCursor((OLED_MAX_SYMBOLS_IN_1_LINE)*FONT_WIDTH - 1*FONT_WIDTH - (strlen(yes_str)*FONT_WIDTH), 4*FONT_HEIGHT + LINE_SPACING*3);
  u8g2.print(yes_str);
  u8g2.drawGlyph((OLED_MAX_SYMBOLS_IN_1_LINE)*FONT_WIDTH - 1*FONT_WIDTH, 4*FONT_HEIGHT + LINE_SPACING*3, 0x2193);

  u8g2.sendBuffer();
}

void buttonPressedCallback(uint8_t pinIn)
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
    if(isConfirmScreen){
      Serial.println("Back to Main Screen");
      isConfirmScreen = false; 
      oledMainScreenDisplay(lastVerticalVelocity, lastAltitude);
    }
    // Main screen
    else{
      unsigned long button2PressDuration = millis() - button2StartPress;
      if(button1PressDuration >= LONG_DURATION_PRESSED){
        // Main screen with long press >= 5000 ms -> change to confirm screen
        Serial.println("Open Confirm Screen");
        isConfirmScreen = true;
        confirmLauchCommandDisplay();
      }
      else{
        // Main screen with short press < 5000 ms -> send open shutes command
        bool ret = sendCommand(RF24_OPEN_SHUTES_CMD);
        if(ret){
          Serial.print("Command sent ");
          Serial.println(RF24_OPEN_SHUTES_CMD);
        }
        else{
          Serial.println("Send command RF24_OPEN_SHUTES_CMD FAILED");
        }
      }
    }
  }
  else if(pinIn == BUTTON_2){
    // Confirm Screen -> press Yes -> send launch command -> back to main screen
    if(isConfirmScreen){
      Serial.println("Send Launch Command");
      isConfirmScreen = false;
      // send launch command
      bool ret = sendCommand(RF24_LAUNCH_CMD);
      if(ret){
        Serial.print("Command sent ");
        Serial.println(RF24_LAUNCH_CMD);
      }
      else{
        Serial.println("Send command RF24_LAUNCH_CMD FAILED");
      }     
      //back to main screen
      oledMainScreenDisplay(lastVerticalVelocity, lastAltitude);
    }
    // Main Screen -> press Close -> send close shutes command
    else{
      Serial.println("Send close shutes Command");
      // send close shutes command
      bool ret = sendCommand(RF24_CLOSE_SHUTES_CMD);
      if(ret){
        Serial.print("Command sent ");
        Serial.println(RF24_CLOSE_SHUTES_CMD);
      }
      else{
        Serial.println("Send command RF24_CLOSE_SHUTES_CMD FAILED");
      }
    }
  }
}

void clearBuffer(uint8_t buff[], int buffSize) {
  for(int i = 0; i < buffSize; i++){
    buff[i] = {0};
  }
}

void parseCommand(String data, uint8_t ackCmd[]) {
  for(int i = 0; i < data.length(); i++){
    ackCmd[i] = data[i];
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

bool sendCommand(const char* cmd){
  radio.stopListening();
  bool ret = false;
  ret = radio.write(cmd, strlen(cmd));
  delay(50);
  radio.startListening();
  return ret;
}
/* ----------------------------------------------------------------GLOBAL FUNCTIONS---------------------------------------------------------------- */
