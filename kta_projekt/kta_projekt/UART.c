/*
 * UART.c
 * Created: 29.02.2024 12:50:31
 * Author: Tomáš
 */ 

#include "UART.h"
#include "timer.h"
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>

#define TIMEOUT_MICROSECONDS 1750  // 3.5 znakové intervaly při 9600 baud = 364 us; 1750 je bezpečná hodnota pro zahrnutí nějaké rezervy
#define RX_BUFFER_SIZE 256

volatile uint8_t rxBuffer[RX_BUFFER_SIZE];
volatile uint16_t rxWritePos = 0;
volatile uint16_t rxReadPos = 0;

ISR(USART_RX_vect) {
    uint8_t data = UDR0;  // Přečíst data z přijímacího registru
    uint16_t nextPos = (rxWritePos + 1) % RX_BUFFER_SIZE;
    if (nextPos != rxReadPos) {  // Zkontrolovat, zda buffer není plný
        rxBuffer[rxWritePos] = data;
        rxWritePos = nextPos;
    }
}

// Setup stream to enable printf to use UART for output
static FILE myuart = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);

void UART_Init(uint16_t speed, uint8_t parity, uint8_t stopBit)
{
	// Calculate the UBRR register value for the baud rate
	uint16_t ubrr0_speed = 16000000 / 16 / speed - 1;

	// Set baud rate
	UBRR0H = (uint8_t)(ubrr0_speed >> 8);
	UBRR0L = (uint8_t)ubrr0_speed;

	// Set frame format: 8 data bits, and configure parity and stop bits
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  // 8 data bits
	switch (parity) {
		case 0:  // No parity
		break;
		case 1:  // Odd parity
		UCSR0C |= (1 << UPM01) | (1 << UPM00);
		break;
		case 2:  // Even parity
		UCSR0C |= (1 << UPM01);
		break;
	}

	if (stopBit == 2) {
		UCSR0C |= (1 << USBS0);  // 2 stop bits
	} // Default is 1 stop bit, so no need to modify the register for 1 stop bit

	// Enable receiver and transmitter
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	// Redirect stdout to UART for printf functionality
	stdout = &myuart;
}
uint8_t UART_get()
{
    // Wait for data to be received
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void UART_write(uint8_t ch)
{
    // Wait for the transmit buffer to be empty
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = ch;
}

void UART_write_array(const uint8_t *data, size_t len)
{
    printf("UART Sending: ");
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);  // Zobrazí každý bajt v hexadecimálním formátu
        UART_write(data[i]);       // Odešle bajt přes UART
    }
    printf("\n");  // Nový řádek pro lepší přehlednost výstupu
}

int usart_putchar_printf(char c, FILE *stream)
{
    if (c == '\n') {
        UART_write('\r');
    }
    UART_write(c);
    return 0;
}
int UART_read_frame(uint8_t *buffer, int buffer_length) {
	int count = 0;
	uint32_t last_byte_time = 0;
	uint8_t byte;

	while (count < buffer_length) {
		if (UCSR0A & (1 << RXC0)) {
			byte = UART_get();
			buffer[count++] = byte;
			last_byte_time = 0;
			} else {
			if (last_byte_time > TIMEOUT_MICROSECONDS) {
				break;
			}
			delay(10);
			last_byte_time += 10;
		}
	}
	return count;
}
