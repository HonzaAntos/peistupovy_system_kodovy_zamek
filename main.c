/*
 * main.c
 *
 * Created: 15.04.2023 17:37:24
 * Author : Antos Jan
 */  

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "LCD.h"
#include "timer.h"
#include "ADC.h"
#include "I2C.h"
#include "Rele.h"
#include "Text.h"
#include "RTC.h"
#include "USART.h"
#include "EEPROM.h"
#include "DataOperation.h"
#include "PasswordOperation.h"


static FILE LCDOut = FDEV_SETUP_STREAM(LCDPutchar, NULL, _FDEV_SETUP_WRITE);

#define LeftMaximal 450 
#define LeftMinimal 350

int main(void)
{
	uint8_t Time [8];
	uint8_t EventFirst [4];
	uint16_t Count = 0;
	uint16_t ADCValue = 0;
	uint8_t CountAlarm = 0;
	uint8_t Tmp = 0;
	uint8_t DisplayRefresh = 30;
		
	/* Inicializace komponent a rozhrani */
	TimerInit();
	I2CInit();
	LCDInit();
	USART_Init();
	ADC_Init();
	Rele_Init();
	RTCInit();
	
	sei();
	
	/* nastaveni pro spusteni */
	Usart_DumbRead(); 
	do
	{
		SetTimes (); // just text
		InfoAboutTime(); // just text
	}
	
	/* cekaci smycka -> ceka, dokud neni nastaven spravne cas*/
	while(SetTime(Time) != 8); 
	
	/* prevod ASCII -> num.(0-9) */
	for (uint8_t i = 0; i < 9; i++) 
		Time [i] = Time [i] - 48; 
	RTCSetTime(Time); // nastaveni casu
	
	fprintf(&LCDOut,"Lock  |");
	LCDSetCursor(16,0);
	fprintf(&LCDOut, "<<<<Settings>>>>");

	/* nalezeni prvniho automatickeho eventu */
	FirstEvent(EventFirst);
    while (1) 
    {
		/* pokud se zmackne leve tlacitko -> zadavani hesla */
		if ((ADCValue <= LeftMaximal) & (ADCValue >= LeftMinimal))
		{
			Tmp = CountAlarm;
			CountAlarm = Password(CountAlarm); // zadavani hesla
			ResultOfPassword(CountAlarm,Tmp); // vysledek podle zadaneho hesla
			FirstEvent(EventFirst); // nalezeni prvniho auto eventu  -> behem zadavani hesla neni mozne aby byl event proveden
			Usart_DumbRead(); // vycisteni USART -> zamezeni komunikace behem zadavani hesla
		}	
		
		/* testovani, zda prisel znak z USART komunikace */
		if ((UCSR0A & (1 << RXC0)) != 0)
		{
			/* opakuje se dokud neprijme ukoncovaci sekvenci /End */
			while (ConsoleReact(Count++) != 0); 
			LCDShiftRight(16); // prepnuti zpet na zadavaci obrazovku
			Count = 0; 
			FirstEvent(EventFirst); // nalezeni prvniho automatickeho eventu - mohl zde byt zmenen cas nebo pridan/odebran event
		}
		
		ADCValue = GetADCValue(); // zmereni analog. hodnoty na tlacitkach u LCD
		
		/* Zobrazeni casu na LCD displeji */
		if ((ReadTime() == 1)) 
		{
			/* overeni, zda je mozne event provest = alarm OFF*/
			if (CountAlarm < 2) 
			{
				/* overeni, zda jiz je cas k provedeni eventu */
				if (CheckEvent(EventFirst) == 0) 
				{
					FirstEvent(EventFirst); // po provedeni eventu, ulozit nasledujici
				}
			}
			/* obnoveni obsahu na displeji -> reset */
			DisplayRefresh = ResetLCD(DisplayRefresh,CountAlarm); 
		}
    }
}



