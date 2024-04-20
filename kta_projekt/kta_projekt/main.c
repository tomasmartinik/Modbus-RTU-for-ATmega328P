/*
 * kta_projekt.c
 *
 * Created: 12.04.2024 19:32:14
 * Author : Tomáš
 */ 

#include <avr/io.h>
#include "modbus.h"
#include "timer.h"
#include "UART.h"

int main(void) {
	// Inicializace Modbus komunikace
	UART_Init(9600, 0, 1);
	modbus_init(9600, 0, 1);  // Nastaví baud rate na 9600, bez parity, 1 stop bit

	// Předpokládejme, že máme holding registry již inicializované
	holdingRegisters[0] = 1;
	holdingRegisters[1] = 2;
	holdingRegisters[2] = 3;
	holdingRegisters[3] = 4;

	timerInit();

	// Neustálá smyčka, simulující provoz zařízení
	while(1) {
		// Simulace příchozí zprávy: Přečti registery od indexu 0, celkem 4 registry
		//handleReadHoldingRegisters(0, 4);
		
		processIncomingRequests();

		// Simulace chybného příchozího požadavku: Neplatná adresa
		//handleReadHoldingRegisters(5, 1);  // Tato adresa je mimo rozsah

		// Simulace chybného příchozího požadavku: Neplatný počet registrů
		//handleReadHoldingRegisters(2, 3);  // Tento požadavek přesahuje maximální počet registrů
		//printf("ssss\n");
		delay(1000000);

		
	}

	return 0;
}


