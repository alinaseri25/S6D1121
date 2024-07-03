#ifndef LCD_H_
#define LCD_H_

/*
 * DB0:DB7 will be A0:A7
 *
 * */


#include  "stdbool.h"
#include "stm32f1xx_hal.h"
#define swap(type, i, j) {type t = i; i = j; j = t;}

#define LCD_CTRL_PORT GPIOB
#define LCD_DATA_PORT GPIOA


#define LCD_CS  GPIO_PIN_0
#define LCD_RS  GPIO_PIN_1
#define LCD_WR  GPIO_PIN_10
//#define LCD_RD  GPIO_PIN_11
#define LCD_RST  GPIO_PIN_11

void lcd_init(void);
void lcd_fill_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_clear_screen();
void lcd_set_bg_color(uint8_t r, uint8_t g, uint8_t b);
void lcd_set_color(uint8_t r, uint8_t g, uint8_t b);



#endif /* LCD_H_ */
