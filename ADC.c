/*
 * ADC.c
 *
 * Created: 16.04.2023 17:56:34
 *  Author: Antos Jan
 */ 

#include <avr/io.h>
#include <stdio.h>

#include "ADC.h"

void ADC_Init()
{
	DIDR0 |= (1<<ADC0D); // vypnuti digitalniho vstupu -> usetreni el. energie
	ADCSRA |= (1<<ADEN)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2); // ADEN - zapnuti ADC, delicka je zapnuta na 128 (125 KHz), interrupt
	ADMUX |= (01<<REFS0); // zdroj reference external AREF
	ADMUX &= ~(1<<MUX3|1<<MUX2|1<<MUX1|1<<MUX0);
}

uint16_t GetADCValue()
{
	ADCSRA |= (1<<ADSC); // zapnuti prevodu AD prevodniku
	while((ADCSRA & (1<<ADIF)) == 0); // cekaci smycka dokud neni prevod ukoncen
	ADCSRA |= (1<<ADIF); // vynulovani flagu
	return ADC;
}

