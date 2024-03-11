 /*
 * Rele.h
 *
 * Created: 19.04.2023 14:11:16
 *  Author: Antos Jan
 */

#include <avr/io.h>
#include <stdio.h>

#include "Rele.h"

/* Inicializace Rele */
void Rele_Init ()
{
	DDRB |= (1 << PINB3)|(1 << PINB4); //01
	PORTB |= (1 << PINB3)|(1 << PINB4);
}
/* Nastaveni bitu do 0*/
void ReleSetDown (uint8_t PBx)
{
	PORTB |= (1 << PBx);
}
/* Nastaveni bitu do 1*/
void ReleSetUp (uint8_t PBx)
{
	PORTB &= ~(1 << PBx);
}
/* Zmena bitu*/
uint8_t ReleToggle (uint8_t PBx)
{
	PORTB ^= (1 << PBx); 
	if ((PORTB & (1 << PBx)) == 0)
		return 0;
	return 1;
}