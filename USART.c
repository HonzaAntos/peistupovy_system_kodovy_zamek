/*
 * USART.c
 *
 * Created: 27.03.2023 20:31:25
 *  Author: Antos Jan
 */ 

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "USART.h"

#define F_CPU 16000000
#define FOSC 16000000
#define BAUD 38400
#define UBRR_VALUE ((FOSC/(16*BAUD))-1)



/* Inicialiace komunikace USART */
void USART_Init(void)
{
	UBRR0L = (unsigned char) UBRR_VALUE;
	UBRR0H = (unsigned char) (UBRR_VALUE>>8);
	
	UCSR0B |= (1<<TXEN0) | (1<<RXEN0);
	UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);
}

/* Precteni prijmuteho znaku pres USART*/
unsigned char USART_receive(void)
{
	while (!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

/* Odeslani znaku pres USART */
void USART_transmit(unsigned char data)
{
	while (!(UCSR0A &(1<<UDRE0)));
	UDR0 = data;
}

/* Odeslani znaku pres USART do konzole */
int USART_putchar(char c, FILE *stream)
{
	if (c == '\n')
	USART_putchar('\r', stream);
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

/* slouzi k vyprazdneni zasobniku UDR0 -> precteni vsech zbylych dat, ktere nejsou potreba */
void Usart_DumbRead ()
{
	while ((UCSR0A & (1 << RXC0)) != 0)
		USART_receive();
	return;
}