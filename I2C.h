/*
 * I2C.h
 *
 * Created: 2023-08-29 오전 3:50:33
 *  Author: CTM
 */ 


#ifndef I2C_H_
#define I2C_H_

#define F_SCL 100000UL // SCL 주파수
#define Prescaler 1
#define TWBR_val (((F_CPU / F_SCL) - 16) / 2)

#define I2C_READ 0X01
#define i2C_WRITE 0X00
typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
void i2c_init(void);
void i2c_start(uint8_t adress);
void i2c_transmit(uint8_t data);
uint8_t i2c_receive_ack(void);
uint8_t i2c_receive_nack(void);
void i2c_transmit_nbytes(uint8_t adress, uint8_t* data, uint16_t length);
void i2c_receive_nbytes(uint8_t adress, uint8_t* data, uint16_t length);
void i2c_stop(void);

void i2c_init(void)
{
	PORTC |= (1 << 0 )| (1 << 1);
	TWBR = (uint8_t)TWBR_val;
}
void i2c_start(uint8_t adress)
{
	TWCR = (1 << TWINT) | (1<<TWSTA) | (1<<TWEN);
	while( !(TWCR & (1<<TWINT)));
	
	TWDR = adress;
	TWCR = (1<<TWINT) | (1<<TWEN);
	
	while (! (TWCR&(1<<TWINT)));
}
void i2c_transmit(uint8_t data)
{
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	
	while(!(TWCR & (1<< TWINT)));
}
void i2c_transmit_nbytes(uint8_t adress, uint8_t* data, uint16_t length)
{
	i2c_start(adress | i2C_WRITE);
	for(uint16_t i =0; i<length; i++)
	{
		i2c_transmit(data[i]);
		_delay_ms(1);
	}
	i2c_stop();
}
uint8_t i2c_receive_ack(void){
	
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR &(1<<TWINT)));
	return TWDR;
}
uint8_t i2c_receive_nack(void){
	TWDR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	
	return TWDR;
}
void i2c_stop(void){
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}

#endif /* I2C_H_ */