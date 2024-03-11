/*
 * USART.h
 *
 * Created: 27.03.2023 20:33:18
 *  Author: Antos Jan
 */ 

#pragma once

void USART_Init(void);
unsigned char USART_receive(void);
void USART_transmit(unsigned char data);
int USART_putchar(char c, FILE *stream);
void Usart_DumbRead ();
