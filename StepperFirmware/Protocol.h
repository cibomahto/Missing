#ifndef _PROTOCOL_h
#define _PROTOCOL_h

#include <Arduino.h>

#define MAX_ADDRESS         50
#define MAX_PACKETLENGTH    MAX_ADDRESS + 3

#define MODE_NOPACKET       0x0      ///< Waiting for a high bit to start a packet
#define MODE_UPDATETARGET   0xFE     ///< Receibing an UpdateTarget packet


class Protocol {
private:
  uint8_t m_address;

  uint8_t m_packetData[MAX_PACKETLENGTH];  ///< Data received in this packet
  
  uint8_t m_crc;           ///< Current CRC measurement
  uint8_t m_mode;          ///< Current operational mode
  uint8_t m_packetLength;  ///< Number of bytes we have received in this packet
  uint8_t m_expectedLength; ///< Number of bytes expected in the current packet

  // add a new byte to the CRC
  void updateCRC(uint8_t data);

public:
  // Initialize the packet processor
  // @param address Device address (must be less than MAX_ADDRESS)
  void init(uint8_t address);
  
  // Reset the packet processor state machine
  void reset();
  
  // Read a byte into the packet processor, and handle it as possible.
  // @param data Data byte to read in
  // @param targetPosition (HACK) new target position.
  void readByte(uint8_t data, int16_t& targetPosition);
};

#endif
