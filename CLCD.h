/*
 * CLCD.h
 *
 * Created: 2023-08-29 오전 3:53:24
 *  Author: CTM
 */ 
#include "I2C.h"
#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>

#ifndef CLCD_H_
#define CLCD_H_

#ifndef F_CPU
#define F_CPU 16000000L
#endif

#define RS1_EN1 0x05
#define RS1_EN0 0x01
#define RS0_EN1 0x04
#define RS0_EN0 0x00
#define Backlight 0x08

uint8_t I2C_addr_PCF8574 = (0x27 << 1);

void twi_init(void);
void i2c_lcd_init(void);
void i2c_lcd_command(uint8_t command);
void i2c_lcd_command_8(uint8_t command);
void i2c_lcd_data(uint8_t data);
void i2c_lcd_goto_XY(uint8_t row, uint8_t col);
void i2c_lcd_write_string(char string[]);


void i2c_lcd_init(void)
{
	i2c_init();
	_delay_ms(50);
	
	i2c_lcd_command_8(0x30); _delay_ms(5);
	i2c_lcd_command_8(0x30); _delay_us(100);
	i2c_lcd_command_8(0x30); _delay_us(100);
	i2c_lcd_command_8(0x20); _delay_us(100);
	
	i2c_lcd_command(0x28); _delay_us(50);
	i2c_lcd_command(0x08); _delay_us(50); //display on/off conrol
	i2c_lcd_command(0x01); _delay_ms(3); // clear display
	i2c_lcd_command(0x06); _delay_us(50);
	i2c_lcd_command(0x0c); _delay_us(50); // display on , cursor & cursor blink off
}
void i2c_lcd_command_8(uint8_t command)
{
	uint8_t c_buf[4];
	
	c_buf[0] = (command&0xf0) | RS0_EN1 | Backlight;
	c_buf[1] = (command&0XF0) | RS0_EN0 | Backlight;
	
	i2c_transmit_nbytes(I2C_addr_PCF8574,c_buf,2);
}
void i2c_lcd_command(uint8_t command)
{
	uint8_t c_buf[4];
	
	c_buf[0] = (command&0xf0) | RS0_EN1 | Backlight;
	c_buf[1] = (command&0XF0) | RS0_EN0 | Backlight;
	c_buf[2] = ((command<<4)&0XF0) | RS0_EN1 |Backlight;
	c_buf[3] = ((command<<4)&0XF0) | RS0_EN0 |Backlight;
	
	i2c_transmit_nbytes(I2C_addr_PCF8574,c_buf,4);
}
void i2c_lcd_data(uint8_t data){
	uint8_t d_buf[4];
	
	d_buf[0] = (data & 0xf0) | RS1_EN1 | Backlight;
	d_buf[1] = (data & 0xf0) | RS1_EN0 | Backlight;
	d_buf[2] = ((data << 4) & 0xf0) | RS1_EN1 | Backlight;
	d_buf[3] = ((data << 4) & 0xf0) | RS1_EN0 | Backlight;
	
	i2c_transmit_nbytes(I2C_addr_PCF8574, d_buf, 4);
}

/*lcd창 커서 이동함수*/
void i2c_lcd_goto_XY(uint8_t row, uint8_t col)
{
	uint8_t address = (0x40 * row) + col;
	uint8_t command = 0x80 | address;
	
	i2c_lcd_command(command);
}

/*문자열 받는 함수*/
void i2c_lcd_write_string(char *string){
	while(*string){
		i2c_lcd_data(*string++);
	}
}

#endif /* CLCD_H_ */