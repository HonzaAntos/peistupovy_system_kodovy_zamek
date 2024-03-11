/*
 * RTC.h
 *
 * Created: 27.03.2023 20:44:30
 *  Author: Antos Jan
 */ 

#pragma once

void RTCInit ();
uint8_t HexToBCD (uint8_t Hex);
uint8_t RTCRead (uint8_t Adres);
void RTCSetTime (uint8_t* Time);
void RTCSetDate (uint8_t day, uint8_t date, uint8_t month, uint8_t year);
void RTCChangeClear();
uint8_t RTCChange();
