/*
 * DataOperation.h
 *
 * Created: 17.04.2023 15:59:57
 *  Author: Antos Jan
 */ 
#pragma once

void Init();
uint8_t StringComparation (uint8_t i, uint8_t* Buf, uint8_t* Buf2);
uint8_t SetTime (uint8_t* BufTime);
int8_t ReadFromConsole (uint8_t* Buf, uint8_t i,uint8_t iMax, uint8_t MinValue, uint8_t MaxValue);
uint8_t FindNameOrEvent (uint8_t* Data, uint8_t* BufName, uint8_t i, uint16_t OffSet);
uint8_t ConsoleReact (uint16_t Tmp); 
void UserCreate (); 
void ShowActiveUserOrAutoEvent(uint16_t OffSet);
void CreateAutoEvent (); 
void Delete (uint16_t OffSet);
void Manual ();
 




