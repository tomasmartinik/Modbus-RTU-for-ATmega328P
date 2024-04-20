/*
 * ADC.c
 *
 * Created: 05.03.2024 14:21:55
 *  Author: Tomáš
 */ 

#include "ADC.h"
#include <math.h>

const float T0 = 298.15;  // 25 stupnu v kelvinech
const float B = 3895.0;   // B-koeficient termistoru
const float R0 = 10000.0; // odpor termistoru pri T0
const float VR1 = 20000.0; //odpor trimru
const float R3 = 1500.0; // odpor R3

void adc_init(void){
	ADMUX |= (1 << REFS0); // referencni napati ADC
	ADCSRA |= (1 << ADEN); // enable ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); 
}

uint16_t adc_get_value(uint8_t channel){
	ADMUX &= ~(MUX3 | MUX2 | MUX1 | MUX0);	
	ADMUX |= channel;	//nastaveni kanalu
	
	ADCSRA |= (1 << ADSC);	
	
	while (ADCSRA & (1 << ADSC));
	
	return ADC;
}

float get_temperature(uint8_t channel){
	uint16_t adc_value = adc_get_value(channel);
	
	float A0 = adc_value / 1024.0 * 5.0 ;	
	float R = A0/(5.0 - A0)*VR1 - R3;	
	return (B/(log(R / R0) + B/T0)) - 273.15;	//teplota
}


