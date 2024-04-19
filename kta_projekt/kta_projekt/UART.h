#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <stdio.h>

// Makra pro nastavení USART
#define BAUD_RATE 9600  // Nastavení baudové rychlosti, může být upraveno podle potřeby

// Prototypy funkcí
void UART_Init(uint16_t speed, uint8_t parity, uint8_t stopBit);
uint8_t UART_get();              // Přijme jeden byte z USART
void UART_write(uint8_t ch);     // Odešle jeden byte do USART
void UART_write_array(const uint8_t *data, size_t len); // Odešle pole bytů do USART
int usart_putchar_printf(char c, FILE *stream);         // Funkce pro podporu printf přes USART

#endif // UART_H_
