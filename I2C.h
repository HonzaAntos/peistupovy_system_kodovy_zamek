/*
 * I2C.h
 *
 * Created: 27.03.2023 19:52:27
 *  Author: Antos Jan
 */ 

#pragma once

void I2CInit ();
uint8_t I2CStart ();
void I2CRepeatStart ();
void I2CStop ();
void I2CWrite (uint8_t Data);
uint8_t I2CRead (uint8_t ack);