/*
 * Text.c
 *
 * Created: 18.04.2023 20:53:25
 *  Author: Antos Jan
 */ 

#include <avr/io.h>
#include <stdio.h>

#include "Text.h"
#include "USART.h"

static FILE mystdout = FDEV_SETUP_STREAM(USART_putchar, NULL, _FDEV_SETUP_WRITE);

void WrongChar ()
{
	fprintf(&mystdout, "spatne zadany znak: ");
}
void SetTimes ()
{
	fprintf(&mystdout, "\nZadejte prosim cas: ");
}
void InfoAboutTime ()
{
	fprintf(&mystdout, "\nve tvaru HH:mm:ss -> napr. 12:35:10\n");
}
void LockUnlock ()
{
	fprintf(&mystdout, "Chcete?:\n uzamykat -> l (lock) \n odemykat -> u (unlock)");
}	
void DeleteFailed ()
{
	fprintf(&mystdout, "Tento prvek nebyl nalezen\n");
}
void DeleteDone ()
{
	fprintf(&mystdout, "OK\nPrvek byl uspesne odstranen \n");
}
void UnknownAction ()
{
	fprintf(&mystdout, "Byla zadana neznama akce\n");
}
void MaxCountUsers ()
{
	fprintf(&mystdout,"Byl dosazen maximlni pocet. Neni jiz mozne pridavat dalsi\n");
}
void SetName ()
{
	fprintf(&mystdout, "Zadejte prosim uzivatelske jmeno: \n");
}
void OverLenght ()
{
	fprintf(&mystdout, "\nByla presazena maximalni delka\n");
}
void RequiredName ()
{
	fprintf(&mystdout,"Je vyzadovano jmeno\n");
}
void SetPassword ()
{
	fprintf(&mystdout, "\nZadejte prosim heslo pro uzivatele: ");
}
void InfoAboutPassword ()
{
	fprintf(&mystdout, "\n--> 4 az 16 cisel)\n");
}
void WrongTime () 
{
	fprintf(&mystdout, "Cas je spatne zadan\n"); 
}
void JustEnter () 
{
	fprintf(&mystdout, "\n");
}
void UserList ()
{
	fprintf(&mystdout, "Uzivatele + hesla: \n");
}
void AutoList ()
{
	fprintf(&mystdout, "Automaticke eventy: \n");
}
void StartAgain () 
{
	fprintf(&mystdout, "\nZacnete prosim znovu\n");
}
void ShortPassword ()  
{
	fprintf(&mystdout, "\nMoc kratke\n");
}
void SaveDone () 
{
	fprintf(&mystdout, "OK\nNastaveni probehlo uspesne\n");
}
void NameDelete () 
{
	fprintf(&mystdout, "Napiste prosim jmeno prvku, ktery ma byt odstranen: \n");
}
