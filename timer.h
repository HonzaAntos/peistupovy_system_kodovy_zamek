/*
 * timer.h
 *
 * Created: 20.03.2023 17:41:31
 *  Author: Antos Jan
 */ 

#pragma once

void TimerInit ();
uint32_t GetTime ();
void BusyDelay(uint32_t us);
void Delay(uint32_t us);
