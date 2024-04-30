/*
 * sensor.c
 *
 * Created: 20.04.2024 23:27:49
 *  Author: Tomáš
 */ 
#include "sensor.h"
#include "modbus.h"
#include "ADC.h"
#include <stdlib.h>

extern uint16_t holdingRegisters[NUM_HOLDING_REGISTERS];

void initializeHoldingRegisters(void) {
	holdingRegisters[UPPER_LIMIT] = UPPER_TEMPERATURE_LIMIT;
	holdingRegisters[LOWER_LIMIT] = LOWER_TEMPERATURE_LIMIT;
	holdingRegisters[STATUS_WORD] = 0;  // Clear status word initially
}

uint16_t simulateTemperature(void) {
	return 20;  // Vrátí pevně nastavenou hodnotu 20
}

void updateTemperature(uint16_t temperature) {
    holdingRegisters[TEMP_REGISTER] = temperature;

    // Vymazání všech relevantních bitů před nastavením nového stavu
    clearStatusBit(UPPER_LIMIT_BIT | LOWER_LIMIT_BIT | TEMP_VALID_BIT);

    if (temperature > holdingRegisters[UPPER_LIMIT]) {
        // Nastavit bit pro překročení horního limitu
        setStatusBit(UPPER_LIMIT_BIT);
    } else if (temperature < holdingRegisters[LOWER_LIMIT]) {
        // Nastavit bit pro spadnutí pod dolní limit
        setStatusBit(LOWER_LIMIT_BIT);
    } else {
        // Nastavit bit pro platnou teplotu, pokud teplota není ani nad horním, ani pod dolním limitem
        setStatusBit(TEMP_VALID_BIT);
    }
}

void toggleWatchdog(void) {
	if (checkStatusBit(WATCHDOG_BIT)) {
		clearStatusBit(WATCHDOG_BIT);
		} else {
		setStatusBit(WATCHDOG_BIT);
	}
}

void setStatusBit(uint16_t bitMask) {
	holdingRegisters[STATUS_WORD] |= bitMask;
}

void clearStatusBit(uint16_t bitMask) {
	holdingRegisters[STATUS_WORD] &= ~bitMask;
}

uint8_t checkStatusBit(uint16_t bitMask) {
	return (holdingRegisters[STATUS_WORD] & bitMask) != 0;
}

void updateSystemState(void) {
	// Simulace získání teploty
	//uint16_t currentTemperature = get_temperature(1);
	uint16_t currentTemperature = 20;

	// Aktualizace teploty v registru
	updateTemperature(currentTemperature);

	// Aktualizace stavového slova a dalších vlastností
	toggleWatchdog();  // Volitelné: změna stavu watchdog
}
