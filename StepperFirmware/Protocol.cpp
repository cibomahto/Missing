#include "Protocol.h"
#include "defines.h"

#define MIN_POSITION 100
#define MAX_POSITION 1000

void Protocol::init(uint8_t address) {
  m_address = address;
  reset();
}

void Protocol::reset() {
  for(uint8_t i = 0; i < MAX_ADDRESS + 1; i++) {
    m_packetData[i] = 0;
  }
  
  m_crc = 0;
  m_packetLength = 0;
  m_mode = MODE_NOPACKET;
  
  digitalWrite(LED_PIN, LOW);  // LED on
}

void Protocol::readByte(uint8_t data, int16_t& targetPosition) {
  if(data & 0x80) {
    // Reset the mode, no matter what
    switch(data) {
      case MODE_UPDATETARGET:
        reset();
        digitalWrite(LED_PIN, HIGH);  // LED on
        
        m_mode = MODE_UPDATETARGET;
        m_expectedLength = MAX_ADDRESS + 3;
        m_packetData[m_packetLength] = data;
        m_packetLength +=1;
        updateCRC(data);
        break;
      default:
        reset();
        break;
    }
  }
  else if(m_mode != MODE_NOPACKET) {
    if(m_packetLength < m_expectedLength - 2) {
      m_packetData[m_packetLength] = data;
      m_packetLength +=1;
  
      updateCRC(data);
    }
    else if(m_packetLength < m_expectedLength) {
      m_packetData[m_packetLength] = data;
      m_packetLength +=1;
    }
    
    if(m_packetLength == m_expectedLength) {
      // TODO: Check CRC here.
      
      switch(m_mode) {
        case MODE_UPDATETARGET:
        
          targetPosition = map(m_packetData[m_address], 0, 127, MIN_POSITION, MAX_POSITION);
          break;
      }
      
      reset();
    }
  }
  // Otherwise don't do anything.
}

void Protocol::updateCRC(uint8_t data) {
   uint8_t i;

    m_crc = m_crc ^ data;
    for (i = 0; i < 8; i++)
    {
      if (m_crc & 0x01) {
        m_crc = (m_crc >> 1) ^ 0x8C;
      }
      else {
        m_crc >>= 1;
      }
    }
}
  
