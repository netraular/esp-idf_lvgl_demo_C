#ifndef CONFIG_H
#define CONFIG_H

#include <driver/gpio.h> // Necesario para gpio_num_t

// Pines de la pantalla
// Usa GPIO_NUM_NC para pines no conectados, y GPIO_NUM_x para los pines conectados
#define TFT_MISO  GPIO_NUM_NC
#define TFT_MOSI  GPIO_NUM_11
#define TFT_SCLK  GPIO_NUM_13
#define TFT_CS    GPIO_NUM_10
#define TFT_DC    GPIO_NUM_9
#define TFT_RST   GPIO_NUM_14
#define TFT_BL    GPIO_NUM_8

// Pines de los botones (¡ya no se usan, pero los dejamos como ejemplo!)
// Usa GPIO_NUM_x para los pines de los botones
#define BUTTON_1_PIN GPIO_NUM_4
#define BUTTON_2_PIN GPIO_NUM_5
#define BUTTON_3_PIN GPIO_NUM_6
#define BUTTON_4_PIN GPIO_NUM_7

// Resolución de la pantalla
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

// Configuracion SPI
#define LCD_HOST                SPI2_HOST
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (40 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL

#endif // CONFIG_H