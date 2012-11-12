#ifndef _RS485_h
#define _RS485_h
#include "defines.h"

#include <Arduino.h>

#define MODE_DISABLED   0
#define MODE_RECEIVE    1
#define MODE_TRANSMIT   2


// Set up an RS485 chip.
class RS485 {
public:

  // Configure the direction of the transcieve
  // @param mode Direction to configure the chip for; can be MODE_DISABLED, MODE_RECEIVE, or MODE_TRANSMIT
  void setMode(uint8_t mode) {
    switch(mode) {
      case MODE_DISABLED:  // TODO: is this right?
        digitalWrite(R_ENABLE_PIN, HIGH);
        digitalWrite(D_ENABLE_PIN, LOW);
        break;
      case MODE_RECEIVE:
        digitalWrite(R_ENABLE_PIN, LOW);
        digitalWrite(D_ENABLE_PIN, LOW);
        break;
      case MODE_TRANSMIT:
        digitalWrite(R_ENABLE_PIN, HIGH);
        digitalWrite(D_ENABLE_PIN, HIGH);
        break;
    }
  }

  // Initialize the rs485 chip. 
  // @param baud Baud rate. Example: 57600
  // @param mode Direction to configure the chip for; can be MODE_DISABLED, MODE_RECEIVE, or MODE_TRANSMIT
  void init(unsigned int baud, uint8_t mode) {
    setMode(mode);
    
    pinMode(R_ENABLE_PIN, OUTPUT);
    pinMode(D_ENABLE_PIN, OUTPUT);
    
    Serial.begin(baud);
  }
};


#endif
