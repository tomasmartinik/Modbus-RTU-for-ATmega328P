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
	UART_Init(9600, 0, 1);  // Nastaví baud rate na 9600, bez parity, 1 stop bit
	modbus_init(9600, 0, 1);

	// Předpokládejme, že máme holding registry již inicializované
	//holdingRegisters[0] = 1;
	//holdingRegisters[1] = 2;
	//holdingRegisters[2] = 3;
	//holdingRegisters[3] = 4;
	

	timerInit();

	// Neustálá smyčka, simulující provoz zařízení
	while(1) {
		// Zpracování příchozích Modbus požadavků
		processIncomingRequests();
		delay(1000);
		

		// Možné další úkoly, které může zařízení vykonávat, jako je monitorování senzorů, atd.
	}

	return 0;
}
