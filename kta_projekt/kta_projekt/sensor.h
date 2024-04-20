/*
 * sensor.h
 *
 * Created: 20.04.2024 23:27:59
 *  Author: Tomáš
 */ 


#ifndef SENSOR_H_
#define SENSOR_H_

#include <stdint.h>

#define TEMP_REGISTER      0  // Index for current temperature
#define UPPER_LIMIT        1  // Index for upper temperature limit
#define LOWER_LIMIT        2  // Index for lower temperature limit
#define STATUS_WORD        3  // Index for status word

#define WATCHDOG_BIT       (1 << 0)
#define TEMP_VALID_BIT     (1 << 1)
#define UPPER_LIMIT_BIT    (1 << 2)
#define LOWER_LIMIT_BIT    (1 << 3)

#define UPPER_TEMPERATURE_LIMIT 35
#define LOWER_TEMPERATURE_LIMIT 5

#define UPPER_TEMPERATURE_LIMIT 35
#define LOWER_TEMPERATURE_LIMIT 5

#define TEMP_CHANNEL 0

void initializeHoldingRegisters(void);
uint16_t simulateTemperature(void);
void updateTemperature(uint16_t temperature);
void toggleWatchdog(void);
void setStatusBit(uint16_t bitMask);
void clearStatusBit(uint16_t bitMask);
uint8_t checkStatusBit(uint16_t bitMask);
void updateSystemState(void);


#endif /* SENSOR_H_ */