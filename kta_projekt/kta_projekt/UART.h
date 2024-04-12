/*
 * UART.h
 *
 * Created: 29.02.2024 12:46:14
 *  Author: Tomáš
 */ 


#pragma once

#include <avr/io.h>
#define F_CPU 16000000UL // Clock Speed 
#include <stdio.h>

void UART_Init(uint16_t baud);

uint8_t UART_get();

void UART_write(uint8_t ch);
  
int usart_putchar_printf(char c, FILE *stream);
