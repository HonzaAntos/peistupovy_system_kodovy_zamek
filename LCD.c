/*
 * LCD.c
 *
 * Created: 15.03.2023 00:34:28
 *  Author: Antos Jan
 */ 

#include <avr/io.h>
#include <stdio.h>

#include "LCD.h"
#include "timer.h"

#define RS (1<<PINB0)
#define ENABLE (1<<PINB1)

void LCDClear ()
{
	PORTB &= ~RS; // zapis instrukci
	PORTD &= ~0xf0; // 0000
	PORTB |= ENABLE; // aktivace bitu Enable
	BusyDelay(1640); // wait 1640 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(1640); // wait 1640 us
	PORTD |= 0x10; // 0001
	PORTB |= ENABLE; // aktivace bitu Enable
	BusyDelay(1640); // wait 1640 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(1640); // wait 1640 us
}

void LCDRst()
{
	PORTB &= ~(ENABLE | RS); // vynulovani bitu PB0 a PB1
	PORTD &= ~0xC0; // 00XX
	PORTD |= 0x30; // 0011
	PORTB |= ENABLE; // aktivace bitu Enable
	BusyDelay(10000); // wait 10 ms
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(10000); // wait 10 ms
	
	PORTB |= ENABLE; // aktivace bitu Enable
	BusyDelay(200); // wait 200 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(200); // wait 200 us
	
	PORTB |= ENABLE; // aktivace bitu Enable
	BusyDelay(200); // wait 200 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(200); // wait 200 us
}

/* nastaveni komunikace na 4 bity */
void LCD4bit ()
{
	PORTB &= ~(ENABLE | RS); // vynulovani bitu PB0 a PB1
	PORTD &= ~0xf0; // 0000
	PORTD |= 0x20; // 0010
	PORTB |= ENABLE; // zaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
}

void LCDFunctionSet ()
{
		PORTB &= ~(ENABLE | RS); // vynulovani bitu PB0 a PB1
		PORTD &= ~0xf0; // 0000
		PORTD |= 0x20; // 0010
		PORTB |= ENABLE; // zaktivovani bitu Enable
		BusyDelay(40); // wait 40 us
		PORTB &= ~ENABLE; // deaktivovani bitu Enable
		BusyDelay(40); // wait 40 us
		PORTD &= ~0x70; // X000
		PORTD |= 0x80; // 1000
		PORTB |= ENABLE; // zaktivovani bitu Enable
		BusyDelay(40); // wait 40 us
		PORTB &= ~ENABLE; // deaktivovani bitu Enable
		BusyDelay(40); // wait 40 us
}

void LCDDispley()
{
	PORTB &= ~(ENABLE | RS); // vynulovani bitu PB0 a PB1
	PORTD &= ~0xf0; // 0000
	PORTB |= ENABLE; // zaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
	PORTD |= 0xC0; // 1100
	PORTB |= ENABLE; // zaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
}

void LCDEntryMode()
{
	PORTB &= ~(ENABLE | RS); // vynulovani bitu PB0 a PB1
	PORTD &= ~0xf0; // 0000
	PORTB |= ENABLE; // zaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
	PORTD |= 0x60; // 0111
	PORTB |= ENABLE; // zaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
}

void LCDInit()
{
	DDRB |= (ENABLE | RS); // nastaveni vystupu PB0 a PB1
	DDRD |= 0xf0; // nastaveni vystupu PD4 az PD7
	
	PORTB &= ~(ENABLE | RS); // vynulovani bitu PB0 a PB1
	PORTD &= ~0xf0; // vynulovani bitu PD4 az PD7
	BusyDelay(100000); // wait 100 ms
	
	LCDRst();
	LCD4bit();
	LCDFunctionSet();
	LCDDispley();	
	LCDClear();
	LCDEntryMode();
	
}

void LCDCursorHome()
{
	PORTB &= ~RS; // zapis instrukci
	PORTD &= ~0xf0; // 0000
	PORTB |= ENABLE; // aktivace bitu Enable
	BusyDelay(1640); // wait 1640 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(1640); // wait 1640 us
	PORTD |= 0x20; // 0010
	PORTB |= ENABLE; // aktivace bitu Enable
	BusyDelay(1640); // wait 1640 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(1640); // wait 1640 us
}

void LCDSetCursor(char Col, uint8_t Row)
{
	char Position;
	if (Row == 0)
	{
		Position = Col;
	}
	else
	{
		Position = Col + 0x40;
	}
	PORTB &= ~RS; // instrukce
	PORTD &= ~0xf0; // 0000
	PORTD |= (0x80 | (Position & 0xf0)); // up 4 bits from Position
	PORTB |= ENABLE; // aktivace bitu Enable
	BusyDelay(40); // wait 40 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
	PORTD &= ~0xf0; // 0000
	PORTD |= ((Position & 0x0f)<<4); // down 4 bits from Position
	PORTB |= ENABLE; // aktivace bitu Enable
	BusyDelay(40); // wait 40 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
}

/* LCD shift to left | Tmp --> x shift*/
void LCDShiftLeft(uint8_t Tmp)
{
	PORTB &= ~RS; // instrukce
	while(1)
	{
		PORTD &= ~0xf0; // 0000
		PORTD |= 0x10; // 0001
		PORTB |= ENABLE; // aktivace bitu Enable
		BusyDelay(40); // wait 40 us
		PORTB &= ~ENABLE; // deaktivovani bitu Enable
		BusyDelay(40); // wait 40 us
		PORTD &= ~0xf0; // 0000
		PORTD |= 0x80; // 1000
		PORTB |= ENABLE; // aktivace bitu Enable
		BusyDelay(40); // wait 40 us
		PORTB &= ~ENABLE; // deaktivovani bitu Enable
		BusyDelay(40); // wait 40 us
		Tmp--;
		if (Tmp == 0)
		{
			break;
		}
	}
}
/* LCD shift to Right | Tmp --> x shift*/
void LCDShiftRight(uint8_t Tmp)
{
	PORTB &= ~RS; // instrukce
	while(1)
	{
		PORTD &= ~0xf0; // 0000
		PORTD |= 0x10; // 0001
		PORTB |= ENABLE; // aktivace bitu Enable
		BusyDelay(40); // wait 40 us
		PORTB &= ~ENABLE; // deaktivovani bitu Enable
		BusyDelay(40); // wait 40 us
		PORTD &= ~0xf0; // 0000
		PORTD |= 0xC0; // 1100
		PORTB |= ENABLE; // aktivace bitu Enable
		BusyDelay(40); // wait 40 us
		PORTB &= ~ENABLE; // deaktivovani bitu Enable
		BusyDelay(40); // wait 40 us
		Tmp--;
		if (Tmp == 0)
		{
			break;
		}
	}
}

/* Reset LCD */
void LCDReset()
{
		PORTB &= ~(ENABLE | RS); // vynulovani bitu PB0 a PB1
		PORTD &= ~0xf0; // vynulovani bitu PD4 az PD7
				
		LCDRst();
		LCD4bit();
		LCDFunctionSet();
		LCDDispley();
		LCDClear();
		LCDEntryMode();

}

int LCDPutchar(char c, FILE *stream)
{
	PORTB |= RS; // zapis dat
	PORTD &= ~0xf0; // 0000
	PORTD |= (c & 0xf0); // up 4 bits from c
	PORTB |= ENABLE; // aktivace bitu Enable
	BusyDelay(40); // wait 40 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
	PORTD &= ~0xf0; // 0000
	PORTD |= ((c & 0x0f) << 4); // down 4 bits from c
	PORTB |= ENABLE; // aktivace bitu Enable
	BusyDelay(40); // wait 40 us
	PORTB &= ~ENABLE; // deaktivovani bitu Enable
	BusyDelay(40); // wait 40 us
	return 0;
}
