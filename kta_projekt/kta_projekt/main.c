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
#include "sensor.h"

#define TEMP_REGISTER      0  // Aktuální teplota
#define UPPER_LIMIT        1  // Horní limit teploty
#define LOWER_LIMIT        2  // Dolní limit teploty
#define STATUS_WORD        3  // Status word

#define WATCHDOG_BIT       (1 << 0)
#define TEMP_VALID_BIT     (1 << 1)
#define UPPER_LIMIT_BIT    (1 << 2)
#define LOWER_LIMIT_BIT    (1 << 3)

int main(void) {
	// Inicializace Modbus komunikace
	UART_Init(9600, 0, 1);
	modbus_init(9600, 0, 1);  // Nastaví baud rate na 9600, bez parity, 1 stop bit

	// Předpokládejme, že máme holding registry již inicializované
	//holdingRegisters[0] = 1;
	//holdingRegisters[1] = 2;
	//holdingRegisters[2] = 3;
	//holdingRegisters[3] = 4;

	timerInit();
	initializeHoldingRegisters();

	while(1) {
	
		updateSystemState();
		processIncomingRequests();
		
		delay(1000000);

		
	}

	return 0;
}
