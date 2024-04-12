/*
 * modbus.h
 *
 * Created: 12.04.2024 19:32:48
 *  Author: Tomáš
 */ 


#ifndef MODBUS_H_
#define MODBUS_H_

void modbus_init(void);
uint16_t modbus_read_register(uint16_t register_address);
void modbus_write_register(uint16_t register_address, uint16_t value);
void modbus_process(void);


#endif /* MODBUS_H_ */