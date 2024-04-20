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
    uint8_t response[256];  // Buffer pro odpověď
    int index = 0;

    // Sestavení hlavičky odpovědi
    response[index++] = 0x01;  // Slave ID
    response[index++] = 0x03;  // Funkční kód
    response[index++] = numRegisters * 2;  // Počet bajtů dat

    // Přidání dat registru
    for (uint16_t i = startAddress; i < startAddress + numRegisters; i++) {
        response[index++] = (holdingRegisters[i] >> 8) & 0xFF;  // Vyšší bajt hodnoty registru
        response[index++] = holdingRegisters[i] & 0xFF;         // Nižší bajt hodnoty registru
    }

    // Výpočet CRC
    uint16_t crc = computeCRC(response, index);
    response[index++] = crc & 0xFF;
    response[index++] = (crc >> 8) & 0xFF;

    // Logování odesílané odpovědi
    //printf("Sending response: ");
    for (int i = 0; i < index; i++) {
      //  printf("%02X ", response[i]);
    }
    //printf("\n");

    // Odeslání odpovědi
    UART_write_array(response, index);
}



void processIncomingRequests() {
    uint8_t buffer[256];
    int numBytes = UART_read_frame(buffer, 256);  // Čeká na příjem celého rámcového bufferu
    uint16_t crc;
    uint8_t slaveId, functionCode;

    if (numBytes > 0) {
        //printf("Received %d bytes.\n", numBytes);  // Zobrazit, kolik bajtů bylo přijato
        crc = ((uint16_t)buffer[numBytes - 2]) | ((uint16_t)buffer[numBytes - 1] << 8);
        
        if (computeCRC(buffer, numBytes - 2) == crc) {
            slaveId = buffer[0];
            functionCode = buffer[1];
            //printf("Slave ID: %02X, Function Code: %02X\n", slaveId, functionCode);  // Zobrazit ID a funkční kód

            uint16_t startAddress, numRegisters, value;

            switch (functionCode) {
                case 0x03:  // Čtení holding registrů
                    startAddress = (buffer[2] << 8) | buffer[3];
                    numRegisters = (buffer[4] << 8) | buffer[5];
                    //printf("Reading Holding Registers: Start Address: %u, Number of Registers: %u\n", startAddress, numRegisters);
                    handleReadHoldingRegisters(startAddress, numRegisters);
                    break;
                case 0x06:  // Zápis jednoho registru
                    startAddress = (buffer[2] << 8) | buffer[3];
                    value = (buffer[4] << 8) | buffer[5];
                    //printf("Writing Single Register: Address: %u, Value: %u\n", startAddress, value);
                    handleWriteSingleRegister(startAddress, value);
                    break;
                default:  // Neplatná funkce
                    //printf("Invalid Function Code: %02X\n", functionCode);
                    sendModbusException(slaveId, 1);
                    break;
            }
        } else {
            //printf("CRC Error: Calculated CRC: %04X, Expected CRC: %04X\n", computeCRC(buffer, numBytes - 2), crc);
            sendModbusException(buffer[0], 3);  // Chyba CRC
        }
    }
}

void sendWriteSingleRegisterResponse(uint16_t registerAddress, uint16_t value) {
    uint8_t response[8];
    int index = 0;

    response[index++] = 0x01;  // Slave ID
    response[index++] = 0x06;  // Funkční kód pro zápis jednoho registru
    response[index++] = (registerAddress >> 8) & 0xFF;  // Vyšší byte adresy
    response[index++] = registerAddress & 0xFF;  // Nižší byte adresy
    response[index++] = (value >> 8) & 0xFF;  // Vyšší byte hodnoty
    response[index++] = value & 0xFF;  // Nižší byte hodnoty

    uint16_t crc = computeCRC(response, index);  // Vypočítat CRC
    response[index++] = crc & 0xFF;  // Nižší byte CRC
    response[index++] = (crc >> 8) & 0xFF;  // Vyšší byte CRC

    UART_write_array(response, index);  // Odeslání odpovědi
}


void handleWriteSingleRegister(uint16_t registerAddress, uint16_t value) {
	if (registerAddress >= NUM_HOLDING_REGISTERS) {
		sendModbusException(0x06, 0x02);  // ILLEGAL DATA ADDRESS
		return;
	}

	holdingRegisters[registerAddress] = value;  // Zápis hodnoty do registru
	sendWriteSingleRegisterResponse(registerAddress, value);  // Odeslat potvrzení o zápisu
}