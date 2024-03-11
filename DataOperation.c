/*
 * DataOperation.c
 *
 * Created: 17.04.2023 15:58:49
 *  Author: Antos Jan
 */ 
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "USART.h"
#include "EEPROM.h"
#include "DataOperation.h"
#include "timer.h"
#include "Text.h"
#include "RTC.h"
#include "LCD.h"

#define ADRESS 0x0020 // nasobek pameti kam ukladam data (pohyb po strance 16 bitu jmeno a 16 bitu heslo uzivatele / cas automatickeho nastveni)
#define OFFSET 0x0140 // prvni pozice automatickeho nastaveni

/* pomocne texty pro prechod do ruznych stavu */
uint8_t Inicializace [5] = "/Init";
uint8_t TimeSet [8] = "/SetTime";
uint8_t InfoAuto [9] = "/InfoAuto";
uint8_t InfoUser [9] = "/InfoUser";
uint8_t DeleteUser [11] = "/DeleteUser";
uint8_t CreateUser [11] = "/CreateUser";
uint8_t CrtAuto [8] = "/CrtAuto";
uint8_t DltAuto [8] = "/DltAuto";
uint8_t Stop [5] = "/Stop";
uint8_t Help [5] = "/Help";

static FILE mystdout = FDEV_SETUP_STREAM(USART_putchar, NULL, _FDEV_SETUP_WRITE);


/* vymazani textu v EEPROM */
void Init ()
{
	uint8_t Data [1] = {0};
	for (uint8_t i = 0; i < 20;i++)
	EEPROMWrite(Data,ADRESS*i,1);
	return;
}

/* funkce StringComparation pro porovnani retezcu (pom. fce. pro ConsoleReact) */
uint8_t StringComparation (uint8_t i, uint8_t* Buf, uint8_t* Buf2)
{
	uint8_t j = 0;
	for(;j < i; j++)
	{
		if (Buf[j] != Buf2 [j])
			break;
	}
	if ((Buf[j] == '\0') & (i == j)) // shodne pouze pokud je prvni buf prazdny a doslo k porovnani plneho poctu znaku
		return 0;
	return 1;
}

/* Funkce SetTime pro precteni casu z konzole */
uint8_t SetTime (uint8_t* BufTime)
{
	int8_t i = 0;
	
	
	/* zde dojde k precteni hesla z konzole */
	i = ReadFromConsole(BufTime,0,8,48,58); // ':' = 58(asci)...hned za 9
	
	/* Pokud dojde k presahnuti maximaniho poctu znaku -> vypise se chybove hlaseni a ukonceni vytvareni uzivatele */
	if (i > 8)
	{
		OverLenght (); // just text
		StartAgain (); // just text
		Usart_DumbRead();
		return 0;
	}
	
	/* pokud zadany znak neni cislo v rozmezi 0-9 -> ukonceni vytvareni uzivatele */
	if (i < 0)
	{
		WrongChar (); // just text
		USART_transmit(BufTime[(-1 * i) - 1]); // i je zaporne -> pridani minus, aby bylo kladne
		StartAgain () ; // just text
		Usart_DumbRead();
		return 0;
	}
	/*  Pokud se nedosahne poctu povolenych znaku -> vypisi se chyby a ukonci se vytvareni uzivatele*/
	if (i < 8)
	{
		ShortPassword (); // just text
		StartAgain (); // just text
		Usart_DumbRead();
		return 0;
	}
	
	for (uint8_t k = 0; k < 8; k++)
	{
		/* jestli je znak ':' na pozicich 2 & 5 v timestringu  */
		if ((BufTime [k] != ':') & ((k % 3) == 2)) // zde resim zda znak : je prave na pozicich 2 a 5
		{
			WrongTime ();
			Usart_DumbRead();
			return 0;
		}
		/* jestli neni znak ':' na jinych pozicich, nez 2 & 5 v timestringu  */
		if ((BufTime [k] == ':') & ((k % 3) != 2))
		{
			WrongTime ();
			Usart_DumbRead();
			return 0;
		}
	}
	return i;
}

/* funkce ReadFromConsole pro vycteni dat z console */
int8_t ReadFromConsole (uint8_t* Buf, uint8_t i,uint8_t iMax, uint8_t MinValue, uint8_t MaxValue)
{
	unsigned char rec = 0;
	
	/*dokud se nevycte znak o ukonceni radku \r a \n */
	while (rec != 10) 
	{
		rec=USART_receive();
		if (rec == '#')// pro uzivatele
			return -100;
			
		/* neprecetl se prazdny znak? */
		if (rec != '\0') // testuje se zde zda se neprecetl prazdny znak
		{
			/* neprecetlo se ukonceni radku? */
			if (!((rec == 13) | (rec  == 10))) 
			{
				Buf[i] = rec;
				i++;
				/* testovani, zda-li je znak povoleny (v nastavenem rozsahu -> pro hesla) */
				if ((rec < MinValue) | (rec > MaxValue))
				{
					JustEnter (); // text
					Usart_DumbRead(); // vycisteni UDR0 zasobniku
					return -i;
				}
				USART_transmit(rec);
			}
		}
		if (i > iMax) // pri presahnuti povolenoho poctu znaku, dojde k vyskoceni z cteni dat
		{
			JustEnter (); // '/n do konzole'
			Usart_DumbRead(); // vycisteni usart
			return i;
		}
	}
	JustEnter (); 
	Usart_DumbRead(); 
	return i;
}

/* Funkce FindNameOrEvent pro hledani daneho uzivatele nebo automatickeho nastaveni */
uint8_t FindNameOrEvent (uint8_t* Data, uint8_t* BufName, uint8_t i, uint16_t OffSet)
{
	uint8_t position = 0;
	
	/* kontrola prideleneho mista v EEPROM */
	for (; position < 10; position++) 
	{
		EEPROMRead(Data,(position*ADRESS)+OffSet,16); 
		for (uint8_t k = 1; k < 16; k++)
		{
			/* nahrazeni # za znak \0 (konce) pro kontrolu v StringComparation */
			if (Data[k] == '#') //# - znak znaci neuplne zadane jmena nebo hesla
			{
				Data[k] = '\0';
			}
		}
		if (StringComparation(i,Data,BufName) == 0)
		break;
	}
	return position;
}

/* Funkce ConsoleReact slouzi pro precteni zadaneho prikazu z konzole -> nasledne se volaji dalsi funkce prodle precteneho prikazu */
uint8_t ConsoleReact (uint16_t Tmp)
{
	uint8_t UnknownText = 1;
	uint8_t Time [8];
	int8_t message = 0;
	uint8_t Buf [12] = {"\0"};
	
	/* precteni USART komunikace:
								Buf - pole jako zasobnik
								0 - pocatecni misto pro ukladani
								11 - max. pocet ulozenych dat
								0 - min. hodnota ASCII 
								255 - max. hodnota ASCII*/
	message = ReadFromConsole(Buf,0,11,0,255);
	
	/* prvni pruchod -> shitfnuti displeje => ukazuje 'setting' */
	if (Tmp == 0)
		LCDShiftLeft(16);
	
	/* zjisteni o jakou zpravu se jedna (porovnani s preddefinovanymi) */
	switch (message)
	{
		case 5:
		{
			if((StringComparation(message,Buf,Help)) == 0)
			{
				Manual();
				UnknownText = 0;
				break;
			}
			if((StringComparation(message,Buf,Inicializace)) == 0)
			{
				Init();
				UnknownText = 0;
				break;
			}
			if((StringComparation(message,Buf,Stop)) == 0)
			{
				SaveDone ();
				return 0;
			}
			break;
		}
		case 8:
		{
			if(StringComparation(message,Buf,CrtAuto) == 0)
			{
				CreateAutoEvent();
				UnknownText = 0;
				break;
			}
			if(StringComparation(message,Buf,DltAuto) == 0)
			{
				Delete (OFFSET);
				UnknownText = 0;
				break;
			}
			if(StringComparation(message,Buf,TimeSet) == 0)
			{
				SetTimes ();// just text
				InfoAboutTime ();
				if (SetTime(Time) == 8)
				{
					/* prevod ASCII -> num (0-9) */
					for (uint8_t i = 0; i < 9; i++) 
						Time [i] = Time [i] - 48;
					RTCSetTime(Time);
				}
				UnknownText = 0;
				break;
			}
			break;
		}
		case 9:
		{
			if(StringComparation(message,Buf,InfoUser) == 0)
			{
				UserList ();
				ShowActiveUserOrAutoEvent(0x0000);
				UnknownText = 0;
				break;
			}
			if(StringComparation(message,Buf,InfoAuto) == 0)
			{
				AutoList ();
				ShowActiveUserOrAutoEvent(OFFSET);
				UnknownText = 0;
				break;
			}
			break;
		}
		case 11: 
		{
			if(StringComparation(message,Buf,DeleteUser) == 0)
			{
				Delete (0x0000);
				UnknownText = 0;
				break;
			}
			if(StringComparation(message,Buf,CreateUser) == 0)
			{
				UserCreate();
				UnknownText = 0;
				break;
			}
			break;
		}
		default:
		{
			break;
		}
			
	}
	/* Pokud se nejedna o zadnou znamou akci -> vypise se chybove hlaseni*/
	if (UnknownText == 1) 
	{
		UnknownAction (); // just text
	}		
	return 1;
}

/* Funkce UserCreate pro vytvoreni uzivatele pomoci USART komunikace */
void UserCreate ()
{	
	/* zapisuje se 15 hodnot od 1-15 na pozici 0, prijde validacni znak # */
	uint8_t BufName [16]; 
	uint8_t BufPassword [16];
	uint8_t Hash [1];
	int8_t i = 0;
	int8_t j = 0;
	uint8_t position = 0;

	/* Hledani volneho mista pro ulozeni uzivatele */
	for (; position < 10; position++)
	{
		EEPROMRead(Hash,position*ADRESS,1);
		/*Kontrola, zda-li je pocatecni znak # , jelikoz kazdy uzivatel zacina s timto znakem -> pokud tam neni, nejedna se aktualniho uzivatele*/
		if (Hash [0] != '#') 
		{
			break;
		}
	}
	if (position == 10)
	{
		MaxCountUsers (); // just text
		return;
	}
	
	/* vycteni jmena uzivatele z konzole */
	SetName (); // just text
	Usart_DumbRead(); 
	
	i = ReadFromConsole(BufName,1,16,0,255); // cteni z USART
	
	/* Pokud dojde k presahnuti maximaniho poctu znaku -> vypise se chybove hlaseni a ukonceni vytvareni uzivatele */
	if (i > 16)
	{
		OverLenght (); // just text
		StartAgain (); // just text
		Usart_DumbRead();
		return;
	}
	/* kontrola, jestli je zadane jmeno*/
	if (i == 1) 
	{
		RequiredName (); //just text
		return;
	}
	
	if (i == -100) // neni povoleno pouzit # v jmene 
	{
		JustEnter(); // just text
		WrongChar (); // just text
		fprintf(&mystdout,"#");
		StartAgain () ; // just text
		Usart_DumbRead();
		return;
	}
	/* podminky pro heslo a vypsani jmen uzivatelu do konzole */
	SetPassword (); // just text
	for (uint8_t k = 1; k < i; k++)
		USART_transmit(BufName[k]);
	InfoAboutPassword (); //just text
	
	/* Precteni hesla z konzole */
	j = ReadFromConsole(BufPassword,0,16,48,57);
	
	/* Pokud dojde k presahnuti maximaniho poctu znaku -> vypise se chybove hlaseni a ukonceni vytvareni uzivatele */
	if (j > 16) 
	{
		OverLenght (); // just text
		StartAgain (); // just text
		Usart_DumbRead();
		return;
	}
	
	/* pokud zadany znak neni cislo v rozmezi 0-9 -> ukonceni vytvareni uzivatele */
	if (j < 0) 
	{
		WrongChar (); // just text
		USART_transmit(BufPassword[(-1 * j) - 1]); // i je zaporne -> pridani minus, aby bylo kladne
		StartAgain (); // just text
		Usart_DumbRead();
		return;
	}
	
	/*  Pokud se nedosahne poctu povolenych znaku -> vypisi se chyby a ukonci se vytvareni uzivatele*/
	if (j < 4) 
	{
		ShortPassword (); // just text
		StartAgain (); // just text
		Usart_DumbRead();
		return;
	}
		
	/* Ulozeni uzivatele do EEPROM */
	BufName[0] = '#';
	/*pokud pro jmeno neni vyuzito max. pocet znaku, prida se znak # ->  pro lepsi validaci */
	if (i < 16)
		BufName[i] = '#';  
	/*to same plati pro heslo (viz. comment o 3 radky vyse)*/	
	if (j < 16)
		BufPassword[j] = '#'; // neuplna hesla jsou doplnena na svuj konec o #, pro snazsi cteni
	EEPROMWrite(BufName,position*ADRESS,16);
	EEPROMWrite(BufPassword,(position*ADRESS)+0x0010,16);
	
	SaveDone (); // just text
}

/* Funkce ShowActiveUserOrAutoEvent vypise vsechny aktivni uzivatele nebo automatickych nastaveni na zaklade adresy */
void ShowActiveUserOrAutoEvent(uint16_t OffSet)
{
	uint8_t Data [32];
	uint8_t i = 1;
	
	/* Nacteni adres jednotlivych uzivatelu, v pripade ze je to validni uzivatel dojde k jeho vypsani do konzole */
	for (uint8_t position = 0; position < 10; position++)
	{
		Delay(100000);
		EEPROMRead(Data,position*ADRESS+OffSet,32);
		/* vytvoreny uzivatel zacina #...pokud neni na prvni pozici hash -> nejedna se o aktulniho uzivatele */
		if (Data [0] == '#') 
		{
			i = 1;
			while(Data[i] != '#')
			{
				USART_transmit(Data[i]);
				/* Pokud je jmeno uzivatele=size(15), tak nema validacni znak # nakonci*/
				if (i == 15) 
					break;
				i++;
			}
			fprintf(&mystdout, " ----> ");
			i = 16;
			while(Data[i] != '#')
			{
				USART_transmit(Data[i]);
				/* Pokud je HESLO uzivatele=size(16), tak nema validacni znak # nakonci*/
				if (i == 32) 
					break;
				i++;
			}
			JustEnter ();
		}
	}
}

/* funcke CreteAutoEvent pro vytvoreni automatickeho odemykani/zamykani dveri */
void CreateAutoEvent ()
{
	int8_t i = 0;
	int8_t j = 0;
	uint8_t position = 0;
	uint8_t BufName [16]; // zapisuje se 15 hodnot od 1-15 na pozici 0, prijde #
	uint8_t BufTime [16];
	uint8_t Hash [1];

	/* Hledani volneho mista pro ulozeni uzivatele */
	for (; position < 10; position++)
	{
		EEPROMRead(Hash,(position*ADRESS)+OFFSET,1);
		/* vytvoreny uzivatel zacina #...pokud neni na prvni pozici hash -> nejedna se o aktulniho uzivatele */
		if (Hash [0] != '#') 
		{
			break;
		}
	}
	if (position == 10)
	{
		MaxCountUsers ();
		return;
	}
	
	/* Vycteni jmena uzivatele z konzole */
	SetName ();
	Usart_DumbRead(); 
	
	i = ReadFromConsole(BufName,1,16,0,255);
	
	/* Pokud dojde k presahnuti maximaniho poctu znaku -> vypise se chybove hlaseni a ukonceni vytvareni uzivatele */
	if (i > 16) 
	{
		OverLenght (); //just text
		StartAgain (); //just text
		Usart_DumbRead(); 
		return;
	}
	
	/* kontrola, jestli je zadane jmeno*/
	if (i == 1)
	{
		RequiredName ();
		return;
	}
	
	if (i == -100)// neni povoleno pouzit # v jmene 
	{
		JustEnter();
		WrongChar ();
		fprintf(&mystdout,"#");
		StartAgain () ;
		Usart_DumbRead();
		return;
	}
	
	/* vypsani pozadavku na cas do konzole, opetovne vypsani jmena uzivatele*/
	SetTimes (); //just text
	for (uint8_t k = 1; k < i; k++)
	USART_transmit(BufName[k]);
	InfoAboutTime (); //just text
	
	j = SetTime(BufTime); // nastaveni casu
	if (j == 0)
		return;
	
	LockUnlock (); //just text
	
	j = ReadFromConsole(BufTime,9,11,0,255);
	
	if ((BufTime [9] == 'u') & (j == 10))
	{
		BufTime [10] = 'n';
		BufTime [11] = 'l';
		BufTime [12] = 'o';
		BufTime [13] = 'c';
		BufTime [14] = 'k';
		j = 15;
	}
	else if ((BufTime [9] == 'l') & (j == 10))
	{
		BufTime [10] = 'o';
		BufTime [11] = 'c';
		BufTime [12] = 'k';
		j = 13;
	}
	else
	{
		WrongChar();
		USART_transmit(BufTime[9]);
		JustEnter();
		Usart_DumbRead();
		return;
	}
	
	/* Ulozeni uzivatele do EEPROM */
	BufName[0] = '#';
	/*pokud pro jmeno neni vyuzito max. pocet znaku, prida se znak # ->  pro lepsi validaci */
	if (i < 16)
	BufName[i] = '#'; 
	/*to same plati pro heslo (viz. comment o 3 radky vyse)*/	
	if (j < 16)
	BufTime[j] = '#'; // neuplna hesla jsou doplnena na svuj konec o #, pro snazsi cteni
	BufTime[8] = ' ';
	EEPROMWrite(BufName,(position*ADRESS)+OFFSET,16);
	EEPROMWrite(BufTime,(position*ADRESS)+0x0010+OFFSET,16);
	SaveDone(); // just text
	Usart_DumbRead();
}

/* Funkce Delete slouzi k odstraneni uzivatele/Automatickeho nastaveni*/
void Delete (uint16_t OffSet)
{
	uint8_t position = 0;
	uint8_t Data [16];
	uint8_t BufName [16];
	int8_t i = 0;
	
	/* Vyber uzivatel pro smazani */
	Usart_DumbRead();
	UserList ();
	ShowActiveUserOrAutoEvent(OffSet);
	NameDelete ();
	
	i = ReadFromConsole(BufName,1,255,0,255);
	
	/* pridani '#' pred jmeno jelikoz takto jsou jmena ulozena v pameti*/
	BufName [0] = '#';
	
	/* Hledani, zda je uzivatel v pameti(EEPROM)*/
	position = FindNameOrEvent(Data,BufName,i,OffSet);
	if (position == 10)
	{
		DeleteFailed ();
		return;
	}
	
	/* Pro odstraneni uzivatele staci pouze smazat validacni znak ('#') z pocatku*/
	EEPROMWrite(0,(position*ADRESS)+OffSet,1);
	DeleteDone ();
	return;
}

/* Funkce UserHelp pro zobrazeni moznych prikazu v konzoli + jejich popis */
void Manual ()
{
	fprintf(&mystdout,"/Init -> Inicializuje se pamet (tovarni nastaveni)\n");
	Delay(50000);
	fprintf(&mystdout,"/CreateUser -> Vytvoreni noveho uzivatele (max. 10)\n");
	Delay(50000);
	fprintf(&mystdout,"/DeleteUser -> Smazani existujiciho uzivatele\n");
	Delay(50000);
	fprintf(&mystdout,"/InfoUser -> Vypsani vsech ulozenych uzivatelu + jejich hesla\n");
	Delay(50000);
	fprintf(&mystdout,"/InfoAuto -> Vypsani vsech automatickych nastaveni\n");
	Delay(50000);
	fprintf(&mystdout,"/CrtAuto -> Vytvoreni automatickeho ovladani dveri (podle aktualniho casu)\n");
	Delay(50000);
	fprintf(&mystdout,"/DltAuto -> Smazani automatickeho ovladani dveri (podle aktualniho casu)\n");
	Delay(50000);
	fprintf(&mystdout,"/Stop -> Vypnuti nastavovani v konzoli\n");
	Delay(50000);
	fprintf(&mystdout,"/SetTime -> Nastaveni noveho casu\n");
	Delay(50000);

	return;
}