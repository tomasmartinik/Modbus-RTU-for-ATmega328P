/*
 * ADC.h
 *
 * Created: 05.03.2024 14:22:07
 *  Author: Tomáš
 */ 


#ifndef ADC_H_
#define ADC_H_
#include <avr/io.h>

void adc_init(void);
uint16_t adc_get_value(uint8_t channel);
float get_temperature(uint8_t channel);





#endif /* ADC_H_ */