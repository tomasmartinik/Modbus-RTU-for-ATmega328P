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
		if (data == NULL || length == 0) {
			printf("Error: Invalid data or length in sendModbusResponse.\n");
			return;
		}

		// Debugging output to verify the data to be sent
		printf("Sending Modbus Response: ");
		for (int i = 0; i < length; i++) {
			printf("%02X ", data[i]);
		}
		printf("\n");

		// Send the array through UART
		UART_write_array(data, length);
	}


void sendReadRegistersResponse(uint16_t startAddress, uint16_t numRegisters) {
    uint8_t response[256];
    int index = 0;

    response[index++] = 0x01;
    response[index++] = 0x03;
    response[index++] = numRegisters * 2;

    for (uint16_t i = startAddress; i < startAddress + numRegisters; i++) {
        response[index++] = (holdingRegisters[i] >> 8) & 0xFF;
        response[index++] = holdingRegisters[i] & 0xFF;
    }

    uint16_t crc = computeCRC(response, index);
    response[index++] = crc & 0xFF;
    response[index++] = (crc >> 8) & 0xFF;

    // Debugovací výpis
    printf("Sending response: ");
    for (int i = 0; i < index; i++) {
        printf("%02X ", response[i]);
    }
    printf("\n");

	UART_write_array(response, index);
}



void processIncomingRequests() {
	uint8_t buffer[256];
	int numBytes = UART_read_frame(buffer, 256);  // Předpokládá, že tato funkce blokuje, dokud není přijat celý frame
	uint16_t crc, startAddress, numRegisters, value;
	uint8_t slaveId, functionCode;

	if (numBytes > 0) {
		crc = ((uint16_t)buffer[numBytes-2] | (uint16_t)buffer[numBytes-1] << 8);
		if (computeCRC(buffer, numBytes-2) == crc) {
			slaveId = buffer[0];
			functionCode = buffer[1];
			
			switch (functionCode) {
				case 0x03:
				// Zpracovat čtení holding registrů
				startAddress = (buffer[2] << 8) | buffer[3];
				numRegisters = (buffer[4] << 8) | buffer[5];
				handleReadHoldingRegisters(startAddress, numRegisters);
				break;
				case 0x06:
				// Zpracovat zápis jednoho registru
				startAddress = (buffer[2] << 8) | buffer[3];
				value = (buffer[4] << 8) | buffer[5];
				handleWriteSingleRegister(startAddress, value);
				break;
				default:
				sendModbusException(slaveId, 1); // Neplatná funkce
				break;
			}
			} else {
			// Chyba CRC, odeslat chybu
			sendModbusException(buffer[0], 3); // Chyba CRC
		}
	}
}


void handleWriteSingleRegister(uint16_t registerAddress, uint16_t value) {
    if (registerAddress < NUM_HOLDING_REGISTERS) {
        holdingRegisters[registerAddress] = value;
        sendWriteSingleRegisterResponse(registerAddress, value);  // Odpovědět potvrzením zápisu
    } else {
        sendModbusException(0x06, 0x02);  // ILLEGAL DATA ADDRESS
    }
}

void sendWriteSingleRegisterResponse(uint16_t registerAddress, uint16_t value) {
    uint8_t response[8];
    int index = 0;
    response[index++] = 0x01;
    response[index++] = 0x06;
    response[index++] = (registerAddress >> 8) & 0xFF;
    response[index++] = registerAddress & 0xFF;
    response[index++] = (value >> 8) & 0xFF;
    response[index++] = value & 0xFF;
    uint16_t crc = computeCRC(response, index);
    response[index++] = crc & 0xFF;
    response[index++] = (crc >> 8) & 0xFF;
    UART_write_array(response, index);
}


