#include "lcd.h"

uint8_t fch, fcl, bch, bcl;
uint8_t orient;
//int disp_x_size, disp_y_size;
bool _transparent;

uint16_t disp_x_size = 239;
uint16_t disp_y_size = 319;
static void pin_mode_output(GPIO_TypeDef *GPIOx, uint32_t pin)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  HAL_GPIO_WritePin(GPIOx, pin, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}
static void lcd_data_output(void){
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	GPIOA->CRL = 0x33333333;
}
//static void lcd_data_input(void){
//	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
//	GPIOA->CRL = 0x88888888;
//}
static void lcd_pulse_low(void){
	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_WR, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_WR, GPIO_PIN_SET);
}
static void lcd_bus_write(uint8_t vh, uint8_t vl){
	GPIOA->BSRR = vh; // set true values
	GPIOA->BSRR = (~vh)<<16;
	lcd_pulse_low();
	GPIOA->BSRR = vl; // set true values
	GPIOA->BSRR = (~vl)<<16;
	lcd_pulse_low();
}
void lcd_write_cmd(uint8_t vl){
	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS, GPIO_PIN_RESET);
	lcd_bus_write(0x00, vl);
}

void lcd_write_data(uint8_t vh, uint8_t vl){
	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS, GPIO_PIN_SET);
	lcd_bus_write(vh, vl);
}

//void lcd_write_data(uint8_t vl){
//	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS, GPIO_PIN_SET);
//	lcd_bus_write(0x00, vl);
//}
void lcd_write_cmd_data(uint8_t cmd, uint16_t data){
	lcd_write_cmd(cmd);
	lcd_write_data(data>>8, data&0xff );
}


void lcd_set_color(uint8_t r, uint8_t g, uint8_t b)
{
	fch=((r&248)|g>>5);
	fcl=((g&28)<<3|b>>3);
}

uint32_t lcd_get_color()
{
	return (fch<<8) | fcl;
}


void lcd_set_bg_color(uint8_t r, uint8_t g, uint8_t b)
{
	bch=((r&248)|g>>5);
	bcl=((g&28)<<3|b>>3);
	_transparent=false;
}

void lcd_set_xy(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
//	if (orient==LANDSCAPE)
//	{
//		swap(word, x1, y1);
//		swap(word, x2, y2)
//		y1=disp_y_size-y1;
//		y2=disp_y_size-y2;
//		swap(word, y1, y2)
//	}
	lcd_write_cmd_data(0x46,(x2 << 8) | x1);
	lcd_write_cmd_data(0x47,y2);
	lcd_write_cmd_data(0x48,y1);
	lcd_write_cmd_data(0x20,x1);
	lcd_write_cmd_data(0x21,y1);
	lcd_write_cmd(0x22);
}

void lcd_clear_xy()
{
//	if (orient==PORTRAIT)
		lcd_set_xy(0,0,disp_x_size,disp_y_size);
//	else
//		setXY(0,0,disp_y_size,disp_x_size);
}

uint32_t lcd_get_bg_color()
{
	return (bch<<8) | bcl;
}

void lcd_fast_fill_8(uint16_t ch, long pix)
{
	long blocks;
	GPIOA->BSRR = ch; // set 1 values
	GPIOA->BSRR = (~ch)<<16; // set 0 values
	int i;
	blocks = pix/16;
	for (i=0; i<blocks; i++)
	{
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
		lcd_pulse_low();lcd_pulse_low();
	}
	if ((pix % 16) != 0)
		for (int i=0; i<(pix % 16)+1; i++)
		{
			lcd_pulse_low();lcd_pulse_low();
		}
}


void lcd_clear_screen()
{
	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_CS, GPIO_PIN_RESET);
	lcd_clear_xy();
	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS, GPIO_PIN_SET);
	lcd_fast_fill_8(0,((disp_x_size+1)*(disp_y_size+1)));
	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_CS, GPIO_PIN_SET);
}


void lcd_fill_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
	if (x1>x2)
	{
		swap(uint16_t, x1, x2);
	}
	if (y1>y2)
	{
		swap(uint16_t, y1, y2);
	}

	if (fch==fcl)
	{
		HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_CS, GPIO_PIN_RESET);
		lcd_set_xy(x1, y1, x2, y2);
		HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS, GPIO_PIN_SET);
		lcd_fast_fill_8(fch, ((x2-x1)+1)*((y2-y1)+1) );
		HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_CS, GPIO_PIN_SET);

	}
//	else
//	{
//
//		for (int i=0; i<((y2-y1)/2)+1; i++)
//		{
//			drawHLine(x1, y1+i, x2-x1);
//			drawHLine(x1, y2-i, x2-x1);
//		}
//
//
//	}
}

void lcd_init(){
	pin_mode_output(LCD_CTRL_PORT, LCD_CS | LCD_RS | LCD_WR | LCD_RST);
	lcd_data_output();

	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RST, GPIO_PIN_SET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RST, GPIO_PIN_RESET);
	HAL_Delay(15);
	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RST, GPIO_PIN_SET);
	HAL_Delay(15);

	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_CS, GPIO_PIN_RESET);

	lcd_write_cmd_data(0x11,0x2004);
	lcd_write_cmd_data(0x13,0xCC00);
	lcd_write_cmd_data(0x15,0x2600);
	lcd_write_cmd_data(0x14,0x252A);
	lcd_write_cmd_data(0x12,0x0033);
	lcd_write_cmd_data(0x13,0xCC04);
	lcd_write_cmd_data(0x13,0xCC06);
	lcd_write_cmd_data(0x13,0xCC4F);
	lcd_write_cmd_data(0x13,0x674F);
	lcd_write_cmd_data(0x11,0x2003);
	lcd_write_cmd_data(0x30,0x2609);
	lcd_write_cmd_data(0x31,0x242C);
	lcd_write_cmd_data(0x32,0x1F23);
	lcd_write_cmd_data(0x33,0x2425);
	lcd_write_cmd_data(0x34,0x2226);
	lcd_write_cmd_data(0x35,0x2523);
	lcd_write_cmd_data(0x36,0x1C1A);
	lcd_write_cmd_data(0x37,0x131D);
	lcd_write_cmd_data(0x38,0x0B11);
	lcd_write_cmd_data(0x39,0x1210);
	lcd_write_cmd_data(0x3A,0x1315);
	lcd_write_cmd_data(0x3B,0x3619);
	lcd_write_cmd_data(0x3C,0x0D00);
	lcd_write_cmd_data(0x3D,0x000D);
	lcd_write_cmd_data(0x16,0x0007);
	lcd_write_cmd_data(0x02,0x0013);
	lcd_write_cmd_data(0x03,0x0003);
	lcd_write_cmd_data(0x01,0x0127);
	lcd_write_cmd_data(0x08,0x0303);
	lcd_write_cmd_data(0x0A,0x000B);
	lcd_write_cmd_data(0x0B,0x0003);
	lcd_write_cmd_data(0x0C,0x0000);
	lcd_write_cmd_data(0x41,0x0000);
	lcd_write_cmd_data(0x50,0x0000);
	lcd_write_cmd_data(0x60,0x0005);
	lcd_write_cmd_data(0x70,0x000B);
	lcd_write_cmd_data(0x71,0x0000);
	lcd_write_cmd_data(0x78,0x0000);
	lcd_write_cmd_data(0x7A,0x0000);
	lcd_write_cmd_data(0x79,0x0007);
	lcd_write_cmd_data(0x07,0x0051);
	lcd_write_cmd_data(0x07,0x0053);
	lcd_write_cmd_data(0x79,0x0000);
	lcd_write_cmd(0x22);
	HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_CS, GPIO_PIN_SET);
	lcd_set_color(255,255,255);
	lcd_set_bg_color(255,255,255);
	lcd_clear_screen();
	lcd_fill_rect(disp_x_size/2-25,disp_y_size/2-25,disp_x_size/2+25, disp_y_size/2+25);
}
