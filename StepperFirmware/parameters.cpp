#include "defines.h"
#include "parameters.h"

#include <EEPROM.h>

void initParameter() {
  if(0xBE != readUint8Parameter(PARAMETER_MAGIC_NUMBER)) {
    for(uint8_t i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(300);
      digitalWrite(LED_PIN, LOW);
      delay(300);
    }
    
    writeUint8Parameter(PARAMETER_ADDRESS,      11);
    writeUint8Parameter(PARAMETER_REVERSED,     0x01);
    writeUint8Parameter(PARAMETER_MAGIC_NUMBER, 0xBE);
  }
}

void writeUint8Parameter(uint8_t address, uint8_t value) {
  EEPROM.write(address, value);
}


uint8_t readUint8Parameter(uint8_t address) {
  return EEPROM.read(address);
}

