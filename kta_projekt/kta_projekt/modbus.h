#ifndef MODBUS_H_
#define MODBUS_H_

#include <avr/io.h>
#include <stdint.h>

#define NUM_HOLDING_REGISTERS 4

extern uint16_t holdingRegisters[NUM_HOLDING_REGISTERS];  // Přidat extern


void modbus_init(uint16_t speed, uint8_t parity, uint8_t stopBit);
uint16_t computeCRC(const uint8_t* data, uint16_t length);
void sendModbusException(uint8_t functionCode, uint8_t exceptionCode);
void handleReadHoldingRegisters(uint16_t startAddress, uint16_t numRegisters);
void sendModbusResponse(uint8_t* data, uint16_t length);
void sendReadRegistersResponse(uint16_t startAddress, uint16_t numRegisters);
void processIncomingRequests();
void handleWriteSingleRegister(uint16_t registerAddress, uint16_t value);
void sendWriteSingleRegisterResponse(uint16_t registerAddress, uint16_t value);



#endif /* MODBUS_H_ */
