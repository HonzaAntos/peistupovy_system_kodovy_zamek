/*
 * LCD.h
 *
 * Created: 15.03.2023 00:35:41
 *  Author: Antos Jan
 */ 

#pragma once

void LCDClear ();
void LCDRst();
void LCDReset();
void LCDInit();
void LCDCursorHome();
void LCDEntryMode();
void LCDDispley();
void LCDFunctionSet();
void LCD4bit();
void LCDSetCursor(char Col, uint8_t Row);
void LCDShiftLeft(uint8_t Tmp);
void LCDShiftRight(uint8_t Tmp);

int LCDPutchar(char c, FILE *stream);
