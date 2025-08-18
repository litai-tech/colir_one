/**
 * @brief Button definition
 */
#define BUTTON_1                            13
#define BUTTON_2                            14
#define BUTTON_DEBOUNCE_DELAY               80
#define LONG_DURATION_PRESSED               5000

/**
 * @brief RF24 definition
 */
#define RF24_CE_PIN                         4
#define RF24_CSN_PIN                        5

/**
 * @brief WS2812B definition
 */
#define WS2812B_PIN                         17
#define WS2812B_COUNT                       1

/**
 * @brief I2C definition
 */
#define I2C_OLED_ADDRESS                    0x3C
#define I2C_MASTER_CLK_PIN                  22
#define I2C_MASTER_SDA_PIN                  21

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

#define NRF24_QUEUE_SIZE                    10