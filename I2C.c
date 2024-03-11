/*
 * I2C.c
 *
 * Created: 27.03.2023 19:43:58
 *  Author: Antos Jan
 */ 

#include <avr/io.h>
#include <util/twi.h>

#include "timer.h"
#include "I2C.h"

#define MCU_CL 16000000
#define I2C_CL 100000
#define I2CSpeed ((MCU_CL/I2C_CL)-16)/2

void I2CInit ()
{
	TWSR = 0; 
	TWBR = I2CSpeed; // nastaveni rychlosti I2C
}

/* zahajeni komunikace, v pripade ze neuspeje vraci 1*/
uint8_t I2CStart ()
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); // odeslani start condition
	while(!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != TW_START)
	{
		return 1;
	}
	return 0;
}
/* opetovne zahajeni komunikace */
void I2CRepeatStart ()
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); // odeslani repeat start condition
	while(!(TWCR & (1<<TWINT)));
}

/* ukonceni komunikace */
void I2CStop ()
{
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
	Delay(5);
}

void I2CWrite (uint8_t Data)
{
	TWDR = Data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while(!(TWCR & (1<<TWINT)));

}

/* precteni hodnoty odeslane slave zarizenim*/
uint8_t I2CRead (uint8_t ack)
{
	if (ack == 1)
	{
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	}
	else
	{
		TWCR = (1<<TWINT)|(1<<TWEN);
	}
	while(!(TWCR & (1<<TWINT)));
	return TWDR;
}
