/*
 * PasswordOperation.c
 *
 * Created: 18.04.2023 21:48:25
 *  Author: Antos Jan
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "PasswordOperation.h"
#include "timer.h"
#include "ADC.h"
#include "LCD.h"
#include "DataOperation.h"
#include "EEPROM.h"
#include "Rele.h"
#include "RTC.h"
#include "USART.h"

/*nastaveni rozsahu hodnot pro tlacitka LCD*/
#define UpMaximal 150
#define UpMinimal 50
#define SelectMaximal 700
#define SelectMininal 600
#define DownMaximal 300
#define DownMinimal 200
#define RightMaximal 50
#define RightMinimal 0
#define LeftMaximal 450
#define LeftMinimal 350
#define NothingPush 950

#define ADRESS 0x0020
#define OFFSET 0x0140 // prvni pozice automatickeho nastaveni

static FILE LCDOut = FDEV_SETUP_STREAM(LCDPutchar, NULL, _FDEV_SETUP_WRITE);
static FILE mystdout = FDEV_SETUP_STREAM(USART_putchar, NULL, _FDEV_SETUP_WRITE);

char Open [7] = "Unlock";
char Close [7] = "Lock  ";
char OffAlarm [10] = "Alarm OFF";
char OnAlarm [16] = "Alarm ON ->lock";


/* funkce ComparePassword pro porovnani hesel zda/li se jiz nachazeji v pameti*/
uint8_t ComparePassword(uint8_t* Pass, uint8_t i)
{
	uint8_t DataName [16];
	uint8_t DataPassword [17];
	
	/* Nacteni adres jednotlivych uzivatelu, v pripade ze je to validni uzivatel dojde k porovnani hesel */
	for (uint8_t position = 0; position < 10; position++)
	{
		EEPROMRead(DataName,position*ADRESS,16);
		EEPROMRead(DataPassword,(position*ADRESS)+0x0010,16);
		
		/* Kontrola, zda-li je pocatecni znak # , jelikoz kazdy uzivatel zacina s timto znakem...pro lepsi validaci*/
		if (DataName [0] == '#')
		{
			for (uint8_t k = 1; k < 16; k++)
			{
				/* pokud heslo nema plnou delku, obsahuje znak #, ktery je nahrazen \0 */
				if (DataPassword[k] == '#') 
				{
					DataPassword[k] = '\0';
					break;
				}
			}
			 /* pokud ma heslo maximalni delku, tak je az za heslo dan konec */
			DataPassword[16] = '\0'; 
			
			/* Samotne porovnani hesel (zadane vs EEPROM)*/
			if(StringComparation(i+1,DataPassword,Pass) == 0)
			{
				return 0; // Pokud je nalezena shoda
			}
		}
	}
	return 1;
}

/* funkce Password pro zadavani hesla & kontrolu */
uint8_t Password (uint8_t CountToAlarm)
{
	uint16_t ADValue = 0;
	uint8_t Value = 0;
	uint8_t BuffPassword [16] = {0};
	uint8_t i = 0;
	
	while (1)
	{
		ReadTime(); //vypis noveho casu na LCD
		ADValue = GetADCValue(); // prevod AD
		
		/* zadavani hesla pomoci tlacitek pod LCD displajem*/
		/*----sipka dolu---- */
		if ((ADValue >= DownMinimal) & (ADValue <= DownMaximal))
		{
			if (Value == 0)
			Value = 9;
			else
			Value--;
		}
		
		/*----sipka nahoru---- */
		else if ((ADValue >= UpMinimal) & (ADValue <= UpMaximal))
		{
			if (Value == 9)
			Value = 0;
			else
			Value++;
		}
		
		/*----sipka doprava---- */
		if ((ADValue >= RightMinimal) & (ADValue <= RightMaximal))
		{
			if (i < 15)
			{
				BuffPassword[i] = Value;
				Value = 0;
				i++;
			}
		}
		
		/*----sipka doleva---- */
		if ((ADValue >= LeftMinimal) & (ADValue <= LeftMaximal))
		{
			if (i > 0)
			{
				i--;
				Value = BuffPassword[i];
			}
		}
		
		/*----select tlacitko => potrvzeni hesla---- */
		if ((ADValue >= SelectMininal) & (ADValue <= SelectMaximal))
		{
			BuffPassword[i] = Value;
			for (uint8_t j = 0;j <= i ; j++)
			BuffPassword[j] = BuffPassword[j] + 48;// +48 -> do EEPROM je to ukladano v ascii...pricteme 0 (48 v ascii)
			
			/* Kontrola zadaneho hesla s hesly v EEPROM...pokud nejsou shodne zvysi se CountToAlarm */
			if (ComparePassword(BuffPassword,i) == 0)
			{
				return 0;
			}
			return CountToAlarm + 1;
		}
		
		LCDSetCursor(i,1); // vypis hesla na spodni radek LCD
		fprintf(&LCDOut,"%u ",Value);
		
		/* WaitLoop pro uvolneni tlacitka*/
		while (ADValue < NothingPush) // cekaci smycka na uvolneni tlacitka
		{
			ADValue = GetADCValue();
			ReadTime(); //kontrola casu
			Delay(30000); //delaz pro filtaci zakmitu tlacitka
		}
	}
	return 0;
}

/* Funkce ResultOfPassword resi ovladani rele a vypis vysledku na LCD*/
void ResultOfPassword (uint8_t CountToAlarm, uint8_t Tmp)
{
	LCDSetCursor(0,1);
	/* vymazani hesla z displaje */
	for (uint8_t i = 0; i < 16; i++)
		fprintf(&LCDOut," ");
		
	/* stav 1: zruseni alarmu, nezmeni se stav zamku (unlock/lock)...vrati se do stavu predchoziho -> Lock*/
	if ((CountToAlarm == 0) & (Tmp > 2))
	{
		LCDCursorHome();
		ReleSetDown(4);
		if ((PORTB & (1 << PORTB4)) == 0)
			fprintf(&LCDOut,"%s",Open);
		else
			fprintf(&LCDOut,"%s",Close);
		ConsoleReport(OffAlarm);
	}
	
	/* stav 2: zmena stavu vystupu -> dvere unlock/lock */
	if ((CountToAlarm == 0) & (Tmp < 3))
	{
		LCDCursorHome();
		if (ReleToggle(3) == 1)
		{
			fprintf(&LCDOut,"%s",Close); // text
			ConsoleReport(Close); // text
		}
		else
		{
			fprintf(&LCDOut,"%s",Open); // text
			ConsoleReport(Open); // text
		}
	}
	
	/* stav 3: sepnuti alarmu */
	if ((CountToAlarm > 2) & (Tmp == 2))
	{
		ReleSetDown(3);
		ReleSetUp(4);
		LCDCursorHome();
		fprintf(&LCDOut,"Alarm ");
		ConsoleReport(OnAlarm);
	}
	return;
}

/* funkce ReadTime pro zapis aktualniho casu na LCD */
uint8_t ReadTime ()
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	if (RTCChange() == 1)
	{
		RTCChangeClear();
		sec = RTCRead(0x00);
		min = RTCRead(0x01);
		hour = RTCRead(0x02);
		
		LCDSetCursor(8,0);
		fprintf(&LCDOut,"%2u:%2u:%2u", hour,min,sec);
		return 1;
	}
	return 0;
}

/* funkce FirstEvent hleda nadchazejici event */
void FirstEvent (uint8_t* Time)
{
	/* pro docasne buffery */
	uint8_t TimeReal [3];
	uint8_t TimeWinning [4] = {100,100,100,100};
	uint8_t TimeCurrent [4];
		
	/* pro pomocne promenne */
	uint8_t HelpTmp = 0;
	uint8_t Diff = 0;
	uint8_t WinningTmp = 0;
	
	/* pro data z EEPROM */
	uint8_t Data [16]; 
	uint8_t DataTime [16];

	
	TimeReal[2] = RTCRead(0x00); // sec
	TimeReal[1] = RTCRead(0x01); // min
	TimeReal[0] = RTCRead(0x02); // hour
	
	/* Nacteni adres uzivatelu  */
	for (uint8_t position = 0; position < 10; position++)
	{
		Diff = 0;
		HelpTmp = 0;
		
		EEPROMRead(Data,position*ADRESS+OFFSET,16);
		EEPROMRead(DataTime,(position*ADRESS)+0x0010+OFFSET,16);
		
		/* Kontrola zda-li je uzivatel valid => kontrola prvniho znaku (musi byt '#') */
		if (Data [0] == '#')
		{
			for (uint8_t j = 0; j < 3; j++)
				TimeCurrent[j] = (DataTime[j+(2*j)] - '0') * 10 + (DataTime[j+1+(2*j)] - '0'); // premena ulozeneho casu do stejneho zapisu jako je TimeReal;
			TimeCurrent[3] = DataTime[9]; // zapis zda dojde k odemceni ci zamceni
						
			/* je realny cas 'TimeReal' vetsi nez cas 'TimeCurrent (kdy dojde k eventu)*/
			/* pokud ANO -> prida se k TimeReal 24 hodin... pro kontrolu eventu i po pulnoci*/
			
			/* hodiny jsou vetsi */
			if (TimeReal[0] > TimeCurrent[0]) 			
			{
				TimeCurrent[0] += 24;
				HelpTmp = 1;
			}
			/* hodiny jsou stejne */
			if (TimeReal[0] == TimeCurrent[0])  
			{
				/* minuty jsou vetsi */
				if (TimeReal[1] > TimeCurrent[1]) 
				{
					TimeCurrent[0] += 24;
					HelpTmp = 1;
				}
				/* minuty jsou stejne */
				if (TimeReal[1] == TimeCurrent[1])  
				{
					/* vteriny jsou vetsi nebo stejne */
					if (TimeReal[2] >= TimeCurrent[2]) 
					{
						TimeCurrent[0] += 24;
						HelpTmp = 1;
					}
				}
			}
			
			/* overovani, zda-li se nacteny event nevykona drive, nez jiz nalezeny */
			if (TimeWinning[0] > TimeCurrent [0])
			{
				Diff = 1;
			}
			if (TimeWinning[0] == TimeCurrent [0]) //hour
			{
				if (TimeWinning[1] > TimeCurrent [1]) //min
				{
					Diff = 1;
				}
				if (TimeWinning[1] == TimeCurrent[1])
				{
					if (TimeWinning[2] > TimeCurrent[2]) //sec
						Diff = 1;
				}
			}
			/* Prepis puvodne aktualniho casu na nove nalezeny...pokud je lepsi*/
			if (Diff == 1)
			{
				WinningTmp = HelpTmp;
				for (uint8_t k = 0; k < 4; k++)
					TimeWinning [k] = TimeCurrent [k];
			}
		}
	}
	
	/* Odebrani pridanych 24 hod, pokud je vitezny cas je az po pulnoci */
	if (WinningTmp == 1)
		TimeWinning[0] -= 24;
		/* Ulozeni vitezneho casu*/
	for (uint8_t k = 0; k < 4; k++)
		Time [k] = TimeWinning [k];
	return;
}

/* Funkce ResetLCD restartuje obsah displeje kazdou minutu --------------------------------------///NAVIC////*/
uint8_t ResetLCD (uint8_t RefreshCount, uint8_t CountAlarm)
{
	RefreshCount--;
	if (RefreshCount == 0)
	{
		RefreshCount = 30;
		LCDReset();
		LCDCursorHome();
		if (CountAlarm > 2)
		{
			fprintf(&LCDOut,"Alarm ");
		}
		else
		{
			if ((PORTB & (1 << 4)) != 0)
			{
				fprintf(&LCDOut,"Lock  |");
			}
			else
			{
				fprintf(&LCDOut,"Unlock|");
			}
		}
		LCDSetCursor(16,0);
		fprintf(&LCDOut, "<<<<Settings>>>>");
	}
	return RefreshCount;
}

/* Funkce CheckEvent kontroluje, zda-li je jiz cas na provedeni nadchazejiciho eventu*/
uint8_t CheckEvent (uint8_t* Time)
{
	uint8_t RealTime [3];
		
	RealTime[2] = RTCRead(0x00); //sec
	RealTime[1] = RTCRead(0x01); //min
	RealTime[0] = RTCRead(0x02); //hour
	
	/* Kontrola */
	for (uint8_t i = 0; i < 3; i++) 
	{ 
		if (RealTime[i] != Time[i])
			return 1;
	}
	
	/* vykonani eventu, pokud se jiz ma provest*/
	LCDCursorHome();
	if (Time[3] == 'u')	
	{
		ReleSetUp(3);
		fprintf(&LCDOut,"%s",Open);
		ConsoleReport(Open);
	}
	else 
	{
		ReleSetDown(3);	
		fprintf(&LCDOut,"%s",Close);
		ConsoleReport(Close);
	}	
	return 0;
} 

/* funkce ConsoleReport slouzi pro vypis prirazeneho 'textu' s aktualnim casem na konzolu */
void ConsoleReport (char* Text)
{
	uint8_t Time [3];
	Time[2] = RTCRead(0x00); // sec
	Time[1] = RTCRead(0x01); // min
	Time[0] = RTCRead(0x02); // hour
	/* vypis na konzolu */
	fprintf(&mystdout,"V case \n%2u:%2u:%2u se udalo =>  %s\n", Time[0],Time[1],Time[2],Text);
	return;
}
