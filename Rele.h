/*
 * Rele.h
 *
 * Created: 19.04.2023 14:13:22
 *  Author: Antos Jan
 */ 

#pragma once

void Rele_Init ();
void ReleSetDown (uint8_t PBx);
void ReleSetUp (uint8_t PBx);
uint8_t ReleToggle (uint8_t PBx);