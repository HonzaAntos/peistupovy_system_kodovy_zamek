/*
 * EEPROM.c
 *
 * Created: 28.03.2023 21:59:54
 *  Author: Antos Jan
 */ 
#include <avr/io.h>
#include <stdio.h>

#include "timer.h"
#include "EEPROM.h"
#include "I2C.h"

#define Read 0x01
#define Write 0x00 
#define EEPROMAdress 0xA0 // adresa jedne z bunek v pameti jeji velikost by mela byt 4KB tedy pro tuto aplikaci dostatecne velika tudiz neni nutne se prepinat do jinych bunek
#define EEPROMPage 32 // velikost stranky 

void EEPROMWrite(uint8_t Data[], uint16_t Adresa ,uint8_t Velikost)
{
	uint8_t AdresaUp = Adresa>>8;
	uint8_t AdresaDown = Adresa;
	uint8_t Tmp = Adresa % 32; // pro zjisteni kolik hodnot zbyva zapsat na aktualni stranku, nez dojde k posunu na dalsi stranku
	
	while (I2CStart() == 1); // cekaci smycka na start, pokud by komunikoval nekdo jiny ( ceka se dokud nedostane CPU povoleni - 0)
	I2CWrite(EEPROMAdress|Write); // urceni slave zarizeni
	I2CWrite(AdresaUp); // horni adresa v EEPROM
	I2CWrite(AdresaDown);// dolni adresa v EEPROM
	
	for(uint8_t i = 0; i <= (Velikost-1); i++)
	{
		if ((Tmp + i) >= EEPROMPage) // kontrolovani zda jiz neni nutne aby doslo k prechodu na dalsi stranku
		{
			Adresa = Adresa + i; // urceni nove adresy (melo by se jednat o nasobky 0x0020) tedy 32 bytu
			AdresaUp = Adresa>>8; 
			AdresaDown = Adresa;
			Tmp = 0;
			Delay(10000); // maximalni doba pro Self-Timed Write Cycle (10 ms)
			I2CRepeatStart(); // opetovne start
			I2CWrite(EEPROMAdress|Write); // urceni slave zarizeni
			I2CWrite(AdresaUp); // opetovne zadani Up a Down adresy
			I2CWrite(AdresaDown);
		}
		I2CWrite(Data[i]); // zapis dat do EEPROM
	}
	I2CStop(); // ukonceni komunikace
	Delay(10000); // cekani nez dojde k zapisu dat v EEPROM 
}

void EEPROMRead(uint8_t* Data, uint16_t Adresa ,uint8_t Velikost)
{
	uint8_t AdresaUp = Adresa>>8;
	uint8_t AdresaDown = Adresa;
	uint8_t Tmp = Adresa % 32; // pro zjisteni kolik hodnot zbyva precist na aktualni stranku, nez dojde k posunu na dalsi stranku
	uint8_t ACK = 1; // ACK pro inkrementaci pointru v pameti 
	
	while (I2CStart() == 1); // cekaci smycka na start, pokud by komunikoval nekdo jiny ( ceka se dokud nedostane CPU povoleni - 0)
	I2CWrite(EEPROMAdress|Write);  // urceni slave zarizeni
	I2CWrite(AdresaUp); // nastaveni pointru v EEPROM
	I2CWrite(AdresaDown); // nastaveni pointru v EEPROM
	I2CRepeatStart(); // opetovne start
	I2CWrite(EEPROMAdress|Read); // zahajeni cteni
	
	for(uint8_t i = 0; i <= (Velikost-1); i++)
	{
		if ((Tmp + i) >= EEPROMPage)
		{
			Adresa = Adresa + i; // urceni nove adresy (melo by se jednat o nasobky 0x0020) tedy 32 bytu
			AdresaUp = Adresa>>8;
			AdresaDown = Adresa;
			Tmp = 0;
			I2CRepeatStart();// opetovne start
			I2CWrite(EEPROMAdress|Write); // urceni slave zarizeni
			I2CWrite(AdresaUp);// nastaveni pointru v EEPROM
			I2CWrite(AdresaDown);// nastaveni pointru v EEPROM
			I2CRepeatStart();// opetovne start
			I2CWrite(EEPROMAdress|Read);// zahajeni cteni
			ACK = 1;
		}
		
		if ((i == (Velikost - 1))| ((Tmp + i + 1) == EEPROMPage)) // pri poslednim cteni nebo pri docteni stranky se jiz ACK od mastru neposle
		{
			ACK = 0;
		}
		Data[i] = I2CRead(ACK); // cteni dat
	}
	I2CStop(); // ukonceni komunikace
}