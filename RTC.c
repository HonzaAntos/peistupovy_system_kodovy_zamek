/*
 * RTC.c
 *
 * Created: 27.03.2023 20:39:13
 *  Author: Antos Jan
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer.h"
#include "I2C.h"
#include "RTC.h"

#define Write 0x00
#define Read 0x01
#define RTCAdress 0xD0

uint8_t Diff = 0;

/* Funkce RTCInit inicializuje RTC */
void RTCInit ()
{
	DDRD &= ~(1<<PIND2);
	PORTD |= (1<<PIND2);
	
	EICRA = 0x02;
	EIMSK = 0x01;
	
	if (I2CStart() == 0)
	{
		I2CWrite(RTCAdress|Write);
		I2CWrite(0x07);
		I2CWrite(0x10);
		I2CStop();
	}
}

/* Funkce HexToBCD prevadi hexa cislo na BCD */
uint8_t HexToBCD (uint8_t Hex)
{
	uint8_t BCDUp = ((Hex & 0xf0) >> 4);
	uint8_t BCDDown = (Hex & 0x0f);
	return BCDUp * 10 + BCDDown;
}

/* Funkce RTCRead precte z RTC hodnotu na zadane adrese */
uint8_t RTCRead(uint8_t Adres)
{
	uint8_t Tmp;
	if (I2CStart() == 0)
	{
		I2CWrite(RTCAdress|Write);
		I2CWrite(Adres);
		I2CRepeatStart();
		I2CWrite(RTCAdress|Read);
		Tmp = I2CRead(0);
		I2CStop();
		Tmp = HexToBCD(Tmp);
	}
	return Tmp;
}

/* Funkce RTCSetTime zapisuje do RTC cas */
void RTCSetTime (uint8_t* Time)
{
	uint8_t sec;
	uint8_t min;
	uint8_t hours;
	
	/* Vytvoreni hex cisla...pro RTC jako decimal */
	hours = Time [0] * 16 + Time [1]; 
	min = Time [3] * 16 + Time [4];
	sec = Time[6] * 16 + Time [7];
	
	if (I2CStart() == 0)
	{
		I2CWrite(RTCAdress|Write);
		I2CWrite(0x00);
		I2CWrite(sec);
		I2CWrite(min);
		I2CWrite(hours);
		I2CStop();
	}
}

/* Funkce RTCSetDate zapisuje do RTC datum */
void RTCSetDate (uint8_t day, uint8_t date, uint8_t month, uint8_t year)
{
	if (I2CStart() == 0)
	{
		I2CWrite(RTCAdress|Write);
		I2CWrite(0x03);
		I2CWrite(day);
		I2CWrite(date);
		I2CWrite(month);
		I2CWrite(year);
		I2CStop();
	}
}

/* Funkce RTCChangeClear nuluje informaci z prijateho signalu */
void RTCChangeClear()
{
	Diff = 0;
}

ISR (INT0_vect)
{
	Diff = 1;
}

/* Funkce RTCChange urcuje, zda-li prisel impuls urcujici zmenu na RTC */
uint8_t RTCChange()
{
	return Diff;
}