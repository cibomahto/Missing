#pragma once

#include "ofMain.h"
#include "Speaker.h"

class DriverInterface {
public:
	DriverInterface()
	:connected(false)
	,header(0xfe)
	,configMaxSpeed(38)
	,configMinSpeed(100)
	,configStop(2)
	,configStart(10)
	,updateRate(0) {
	}

	void setup(string port, int baud) {
		connected = serial.setup(port, baud);
	}
	void setUpdateRate(unsigned int updateRate) {
		this->updateRate = updateRate;
	}
	void setConfigMaxSpeed(unsigned char configMaxSpeed) {
		this->configMaxSpeed = safe(configMaxSpeed);
	}
	void setConfigMinSpeed(unsigned char configMinSpeed) {
		this->configMinSpeed = safe(configMinSpeed);
	}
	void setConfigStop(unsigned char configStop) {
		this->configStop = safe(configStop);
	}
	void setConfigStart(unsigned char configStart) {
		this->configStart = safe(configStart);
	}
	void update(vector<Speaker>& speakers) {
		static unsigned int counter = 0;
		counter++;
		if(updateRate > 0 && counter % updateRate != 0) {
			return;
		}
		
		packet.clear();
		
		// write the header
		packet.push_back(header);
		packet.push_back(configMaxSpeed);
		packet.push_back(configMinSpeed);
		packet.push_back(configStop);
		packet.push_back(configStart);
		
		// write the remapped values
		static const float unitsPerDegree = 71.75/180.;
		vector<unsigned char> remapped(speakers.size());
		for(int i = 0; i < speakers.size(); i++) {
			Speaker& cur = speakers[i];
			float raw = cur.getAngle() * unitsPerDegree;
			raw += cur.getPosCenter();
			remapped[i] = ofClamp(raw, cur.getPosMin(), cur.getPosMax());
			cur.setRemapped(remapped[i]);
		}
		
		// make remapped values safe
		for(int i = 0; i < remapped.size(); i++) {
			remapped[i] = safe(remapped[i]);
		}
		packet.insert(packet.end(), remapped.begin(), remapped.end());
		
		// write the crc
		unsigned char crc = getCrc(remapped);
		unsigned char crc0 = (crc & 0xf0) >> 4;
		unsigned char crc1 = (crc & 0x0f) >> 0;
		packet.push_back(crc0);
		packet.push_back(crc1);
		
		if(connected) {
			serial.writeBytes(&packet.front(), packet.size());
		}
	}
	vector<unsigned char>& getPacket() {
		return packet;
	}
protected:
	ofSerial serial;
	bool connected;
	vector<unsigned char> packet;

	unsigned char
		header,
		configMaxSpeed,
		configMinSpeed,
		configStop,
		configStart;
		
	unsigned int updateRate;
		
	static unsigned char safe(unsigned char byte) {
		byte &= ~(1<<7);
		return byte;
	}
	
	static unsigned char getCrc(vector<unsigned char>& data) {
		uint8_t crc = 0;
		for(int i = 0; i < data.size(); i++) {
			crc = _crc_ibutton_update(crc, data[i]);
		}
		return crc;
	}
	
	static uint8_t _crc_ibutton_update (uint8_t crc, uint8_t data) {
    uint8_t i;		
		crc = crc ^ data;
		for (i = 0; i < 8; i++) {
			if (crc & 0x01)
				crc = (crc >> 1) ^ 0x8C;
			else
				crc >>= 1;
		}		
		return crc;
	}
};