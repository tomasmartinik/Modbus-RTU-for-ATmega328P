/*
 * timer.h
 *
 * Created: 28.02.2024 11:17:09
 *  Author: Tomáš
 */ 


#ifndef TIMER_H_
#define TIMER_H_


#include <avr/io.h>
#include <avr/interrupt.h>

void timerInit(void);
uint32_t getTime(void);
void busyDelay(uint32_t us);
void delay(uint32_t us);


#endif /* TIMER_H_ */