/*
 * timer.c
 *
 * Created: 20.03.2023 17:38:17
 *  Author: Antos Jan
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#include "timer.h"

#define NOP() asm volatile ("nop"); 
volatile uint32_t Timer1OverFlowCount = 0; 

void TimerInit () // nastaveni casovace T1
{
	TCCR1B = 0x02; // nastaveni delicky - clk/8
	TCNT1 = 0; // resetovani casovace
	TIMSK1 = 0x01; // povoleni preruseni preteceni
	sei(); // povoleni globalnich preruseni
}

uint32_t GetTime () // funkce slouzi pro ziskani ubehnuteho casu od spusteni casovace
{
	cli();
	uint32_t ActualTime = Timer1OverFlowCount; // nacteni poctu preteceni
	ActualTime = ActualTime << 16; // posunuti 16 spodnich bitu do hornich 16 bitu
	ActualTime |= TCNT1; // priorovani hodnoty v casovaci
	sei();
	return ActualTime; // vracena hodnota
}

void BusyDelay(uint32_t us)
{
	us -= 6;
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	
	for (; us != 0; us--)
	{
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
		NOP();
	}
}

void Delay(uint32_t us)  //zpozdeni mikrokontroleru
{
	us = us << 1; // cas v casovaci bezi 2x rychleji tedy nas pozadavek na cekani chceme take dvojnasobny
	uint32_t StartTime = GetTime(); // nacteni pocatecni hodnoty casu
	while (1)
	{
		if (StartTime + us <= GetTime()) // porovnavani aktualniho casu s konecnym (pocatecni + pozadovany)
		{
			break;
		}
	}
}

ISR(TIMER1_OVF_vect) // obsluha preruseni pro preteceni casovace
{
	Timer1OverFlowCount++;
}