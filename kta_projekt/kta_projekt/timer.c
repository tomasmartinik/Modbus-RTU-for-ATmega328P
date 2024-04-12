/*
 * timer.c
 *
 * Created: 28.02.2024 11:20:24
 *  Author: Tomáš
 */ 

#include "timer.h"

#define NOP() asm volatile ("nop");

volatile uint16_t overflowCount = 0;uint32_t startTime = 0;ISR(TIMER1_OVF_vect){ //pocitadlo preteceni casovace	overflowCount++;}

void timerInit(void){	TCNT1 = 0;		// vynulovani casovace	TIFR1 = (1<<TOV1) | (1<<OCF1A);	// smazani vlajek preruseni	TCCR1B |= (1<<CS11); // zapnuti casovace, preddelicka nastavena na 8	TIMSK1 |= (1<<TOIE1); //povoluji preruseni od preteceni	sei();//povoli vsechny preruseni (globalne)}
uint32_t getTime(void){
	//cli();
	return (((uint32_t)overflowCount * 0xFFFF)+TCNT1)>>1;
	//sei();
	//return cas;
}

void busyDelay(uint32_t us){
	//pro rezii potrebujeme 2 us, odecteme z celku
	us -= 2;
	
	// 7x NOP, aby celkova rezie byla 2 us
	NOP();NOP();NOP();NOP();NOP();NOP();NOP();
	
	// for trva 0.6250 us, do 1 us potrebujeme 6x NOP
	for (;us!=0;--us)
	{
		NOP();NOP();NOP();NOP();NOP();NOP();
	}
}

void delay(uint32_t us){	startTime = getTime() + us;	while (getTime() < startTime);}