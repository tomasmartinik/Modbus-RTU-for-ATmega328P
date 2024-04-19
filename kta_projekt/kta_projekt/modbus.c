/*
 * modbus.c
 * Created: 12.04.2024 19:32:33
 * Author: Tomáš
 */

#include "modbus.h"
#include "UART.h"
#include "timer.h"
#include <stdint.h>
#include <string.h>

#define NUM_HOLDING_REGISTERS 4

uint16_t holdingRegisters[NUM_HOLDING_REGISTERS] = {0};

// Initialize Modbus using UART, set its speed, parity and stopBit
void modbus_init(uint16_t speed, uint8_t parity, uint8_t stopBit) {
	UART_Init(speed,parity,stopBit);
}


uint16_t computeCRC(const uint8_t* data, uint16_t length) {
	uint16_t crc = 0xFFFF;  // Počáteční hodnota CRC
	for (uint16_t pos = 0; pos < length; pos++) {
		crc ^= (uint16_t)data[pos];  // XOR byte do crc

		for (int i = 8; i != 0; i--) {  // Pro každý bit
			if ((crc & 0x0001) != 0) {  // Pokud je LSB 1
				crc >>= 1;  // Posunout vpravo o 1
				crc ^= 0xA001;  // XOR s polynomiální konstantou
				} else {
				crc >>= 1;  // Pouze posunout vpravo o 1
			}
		}
	}
	return crc;
}

void sendModbusException(uint8_t functionCode, uint8_t exceptionCode) {
	uint8_t response[5];  // Malý buffer pro chybovou zprávu
	int index = 0;

	response[index++] = 0x01;  // Adresa zařízení (slave ID)
	response[index++] = functionCode | 0x80;  // Přidání 0x80 k funkčnímu kódu pro indikaci chyby
	response[index++] = exceptionCode;  // Kód výjimky

	// Výpočet CRC pro zprávu
	uint16_t crc = computeCRC(response, index);
	response[index++] = crc & 0xFF;  // Nižší bajt CRC
	response[index++] = (crc >> 8) & 0xFF;  // Vyšší bajt CRC

	// Odeslání odpovědi přes UART
	UART_write_array(response, index);
}

void handleReadHoldingRegisters(uint16_t startAddress, uint16_t numRegisters) {
	if (startAddress >= NUM_HOLDING_REGISTERS) {
		sendModbusException(0x03, 0x02); // ILLEGAL DATA ADDRESS, exception code 0x02
		return;
	}
	if (numRegisters == 0 || startAddress + numRegisters > NUM_HOLDING_REGISTERS) {
		sendModbusException(0x03, 0x03); // ILLEGAL DATA VALUE, exception code 0x03
		return;
	}

	sendReadRegistersResponse(startAddress, numRegisters);
}


void sendModbusResponse(uint8_t* data, uint16_t length){
	UART_write_array(data, length);
}


void sendReadRegistersResponse(uint16_t startAddress, uint16_t numRegisters) {
	uint8_t response[256];  // Předpokládáme dostatečně velký buffer
	int index = 0;

	// Přidání Modbus RTU hlavičky a dat
	response[index++] = 0x01;  // Adresa zařízení
	response[index++] = 0x03;  // Funkční kód pro čtení holding registrů
	response[index++] = numRegisters * 2;  // Počet bajtů následujících dat

	for (uint16_t i = startAddress; i < startAddress + numRegisters; i++) {
		response[index++] = (holdingRegisters[i] >> 8) & 0xFF;  // Vyšší bajt hodnoty registru
		response[index++] = holdingRegisters[i] & 0xFF;         // Nižší bajt hodnoty registru
	}

	uint16_t crc = computeCRC(response, index);
	response[index++] = crc & 0xFF;
	response[index++] = (crc >> 8) & 0xFF;

	// Odeslání odpovědi
	sendModbusResponse(response, index);
}
