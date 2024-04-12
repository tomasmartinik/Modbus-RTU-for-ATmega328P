/*
 * UART.c
 *
 * Created: 29.02.2024 12:50:31
 *  Author: Tomáš
 */ 


#include "UART.h"


static FILE myuart = FDEV_SETUP_STREAM(usart_putchar_printf, NULL,_FDEV_SETUP_WRITE);	

void UART_Init(uint16_t speed)
{
	uint16_t ubrr0_speed = 16000000/16/speed-1;
	
	//nastaveni rychlosti
	UBRR0H = (uint8_t)(ubrr0_speed>>8);
	UBRR0L = (uint8_t)ubrr0_speed;
	
	//asynchronni mod
	UCSR0A &= ~(1<<U2X0);	
	
	//8 bitu se bude posilat
	UCSR0C |= (1<<UCSZ01);
	UCSR0C |= (1<<UCSZ00);
	
	//1 stop bit
	UCSR0C &= ~(1<<USBS0);
	
	//povoleni vysilace a prijimace
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);	

	stdout = &myuart;
}


uint8_t UART_get()
{
	while ( !(UCSR0A & (1<<RXC0)));
	return UDR0;
}

void UART_write(uint8_t ch)
{
	while ( !(UCSR0A & (1<< UDRE0)));
	UDR0 = ch;
	
}

int usart_putchar_printf(char c, FILE *stream) {
	
	if (c == '\n'){
		usart_putchar_printf('\r', stream);
	}
	UART_write(c);	
	return 0;
}



