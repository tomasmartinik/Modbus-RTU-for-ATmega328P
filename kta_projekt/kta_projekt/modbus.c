/*
 * modbus.c
 *
 * Created: 12.04.2024 19:32:33
 *  Author: Tomáš
 */ 

#include "modbus.h"
#include "UART.h"
#include "timer.h"
#include <stdint.h>
#include <string.h>

// Definice a konstanty pro Modbus
#define SLAVE_ID 0x01
#define FUNC_READ_HOLDING_REGISTERS 0x03
#define FUNC_WRITE_SINGLE_REGISTER 0x06
#define MODBUS_MAX_FRAME 256  // Maximální velikost rámu
#define MODBUS_TIMEOUT 1500   // Timeout v mikrosekundách pro Modbus RTU

// Předpokládejme, že máme pole s registrama
#define MAX_REGISTER 10
uint16_t registers[MAX_REGISTER] = {0};
	
void modbus_init(void) {
	// Nastavení UART na požadované parametry pro Modbus RTU
	UART_Init(9600);
}


uint16_t modbus_read_register(uint16_t register_address) {
	if (register_address < MAX_REGISTER) {
		return registers[register_address];
		} else {
		// Zde bys mohl nastavit Modbus výjimku nebo chybový kód
		return 0xFFFF; // Modbus konvence pro chybu
	}
}

void modbus_write_register(uint16_t register_address, uint16_t value) {
	if (register_address < MAX_REGISTER) {
		registers[register_address] = value;
		// Můžeš přidat další logiku, pokud změna hodnoty vyžaduje nějakou akci
		} else {
		// Zde bys mohl nastavit Modbus výjimku nebo chybový kód
	}
}

uint16_t crc16(const uint8_t* data, uint16_t length) {
	uint16_t crc = 0xFFFF;
	for (uint16_t i = 0; i < length; i++) {
		crc ^= (uint16_t)data[i];
		for (uint8_t j = 0; j < 8; j++) {
			if (crc & 0x0001) {
				crc >>= 1;
				crc ^= 0xA001;
				} else {
				crc >>= 1;
			}
		}
	}
	return crc;
}

void modbus_process(void) {
	uint8_t frame[MODBUS_MAX_FRAME];
	uint8_t response[MODBUS_MAX_FRAME];
	uint8_t bytes_received = 0;
	uint16_t address, value, crc;
	uint8_t function_code;

	// Inicializace časovače pro měření timeoutu
	timerInit();

	// Příjem dat do bufferu frame
	while (1) {
		if (UART_data_received()) {
			// Pokud byla přijata data, načti je do bufferu
			frame[bytes_received] = UART_get();
			bytes_received++;

			// Restartuj timeout časovače
			timerReset();
			} else if (timerElapsed(MODBUS_TIMEOUT)) {
			// Pokud vypršel timeout, přeruš příjem
			break;
		}
		
		if (bytes_received >= MODBUS_MAX_FRAME) {
			// Pokud buffer přetekl, přeruš příjem
			break;
		}
	}

	// Pokud jsme neobdrželi žádnou zprávu, vrať se
	if (bytes_received == 0) return;

	// Ověření, že CRC je správné
	if (bytes_received > 2) {
		crc = (frame[bytes_received - 2] << 8) | frame[bytes_received - 1];
		if (crc16(frame, bytes_received - 2) != crc) {
			// CRC nesouhlasí - ignoruj zprávu nebo odesli chybový stav
			return;
		}
		} else {
		// Nemáme dost dat pro platnou zprávu
		return;
	}

	// Získání adresy zařízení
	uint8_t addr = frame[0];
	if (addr != SLAVE_ID) {
		// Adresa zařízení nesouhlasí - ignoruj zprávu
		return;
	}

	// Získání funkčního kódu
	function_code = frame[1];
 // Předpokládejme, že funkce se volá z místa, kde jsme již ověřili CRC a identifikovali zařízení
 function_code = frame[1];
 switch (function_code) {
	 case FUNC_READ_HOLDING_REGISTERS: {
		 address = (frame[2] << 8) | frame[3];
		 value = (frame[4] << 8) | frame[5];  // počet registrů k čtení

		 if (address + value > MAX_REGISTER) {
			 // Zpráva o chybě, pokud je požadavek mimo rozsah
			 generate_error_response(frame, response, 2); // Např. 2 je chybový kód pro "adresu mimo rozsah"
			 } else {
			 response[0] = SLAVE_ID;
			 response[1] = FUNC_READ_HOLDING_REGISTERS;
			 response[2] = value * 2;  // Počet bajtů dat: každý registr je 2 bajty
			 int response_idx = 3;
			 for (int i = 0; i < value; i++) {
				 uint16_t reg_val = registers[address + i];
				 response[response_idx++] = reg_val >> 8;
				 response[response_idx++] = reg_val & 0xFF;
			 }
			 uint16_t response_crc = crc16(response, response_idx);
			 response[response_idx++] = response_crc & 0xFF;
			 response[response_idx++] = response_crc >> 8;
			 send_response(response, response_idx);
		 }
		 break;
	 }

	 case FUNC_WRITE_SINGLE_REGISTER: {
		 address = (frame[2] << 8) | frame[3];
		 value = (frame[4] << 8) | frame[5];  // hodnota k zapsání do registru

		 if (address >= MAX_REGISTER) {
			 // Zpráva o chybě, pokud je požadavek mimo rozsah
			 generate_error_response(frame, response, 2); // Např. 2 je chybový kód pro "adresu mimo rozsah"
			 } else {
			 registers[address] = value;  // Zápis hodnoty do registru

			 // Vytvoření potvrzující odpovědi
			 memcpy(response, frame, 6); // Vracíme původní příkaz jako potvrzení
			 uint16_t response_crc = crc16(response, 6);
			 response[6] = response_crc & 0xFF;
			 response[7] = response_crc >> 8;
			 send_response(response, 8);
		 }
		 break;
	 }

	 default:
	 // Zpráva o chybě pro neznámý funkční kód
	 generate_error_response(frame, response, 1); // Např. 1 je chybový kód pro "neznámý funkční kód"
	 break;
 }

 // Pomocné funkce
 void generate_error_response(uint8_t *request_frame, uint8_t *response_frame, uint8_t error_code) {
	 response_frame[0] = SLAVE_ID;
	 response_frame[1] = request_frame[1] | 0x80; // Nastavíme nejvyšší bit na 1 pro chybovou odpověď
	 response_frame[2] = error_code; // Kód chyby
	 uint16_t crc = crc16(response_frame, 3);
	 response_frame[3] = crc & 0xFF;
	 response_frame[4] = crc >> 8;
	 send_response(response_frame, 5);
 }

 void send_response(uint8_t *response, int length) {
	 for (int i = 0; i < length; i++) {
		 UART_write(response[i]);
	 }
 }