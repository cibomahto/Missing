#ifndef _PARAMETERS_h
#define _PARAMETERS_h

#include <Arduino.h>

#define PARAMETER_MAGIC_NUMBER         2     // Magic number is 0xBE
#define PARAMETER_ADDRESS             10     // Address is a uint8_t
#define PARAMETER_REVERSED            11     // Reversed is a uint8_t (0=not, 1=reversed)

// Init the parameter machine; if 
void initParameter();

// Write a uint8_t to EEPROM
// @param addres Address to write to (should be a PARAMETER_ define)
// @param Value to write to
void writeUint8Parameter(uint8_t address, uint8_t value);

// Read a uint8_t from EEPROM
// @param addres Address to read from (should be a PARAMETER_ define)
// @return Value stored in the address
uint8_t readUint8Parameter(uint8_t address);

#endif

