/*
 * PasswordOperation.h
 *
 * Created: 18.04.2023 21:49:57
 *  Author: Antos Jan
 */ 

#pragma once

uint8_t ComparePassword(uint8_t* Pass, uint8_t i);
uint8_t Password(uint8_t CountToAlarm);
void ResultOfPassword (uint8_t CountAlarm, uint8_t Tmp);
uint8_t ReadTime ();
uint8_t ResetLCD (uint8_t ObnovaDispleje, uint8_t CountAlarm);
void FirstEvent (uint8_t* Time);
uint8_t CheckEvent (uint8_t* Time);
void ConsoleReport (char* Text);