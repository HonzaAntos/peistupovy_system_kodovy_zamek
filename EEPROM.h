/*
 * EEPROM.h
 *
 * Created: 28.03.2023 22:00:45
 *  Author: Antos Jan
 */  

#pragma once

void EEPROMRead(uint8_t* Data, uint16_t Adresa ,uint8_t Velikost);
void EEPROMWrite(uint8_t Data[], uint16_t Adresa ,uint8_t Velikost);