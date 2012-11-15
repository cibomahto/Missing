#pragma once

#include "ofMain.h"

class DriverInterface {
public:
	DriverInterface()
	:deadAngle(10)
	,connected(false) {
	}

	void setup(string port, int baud) {
		connected = serial.setup(port, baud);
	}
	void update(vector<float> angles) {
		packet.clear();
		
		// write the header
		const unsigned char header = 0xfe;
		packet.push_back(header);
		
		// write the remapped values
		const int minValue = 127, maxValue = 0;
		const float minAngle = -180 + deadAngle, maxAngle = +180 - deadAngle;
		vector<unsigned char> remapped(angles.size());
		for(int i = 0; i < angles.size(); i++) {
			remapped[i] = (unsigned char) ofMap(angles[i], minAngle, maxAngle, minValue, maxValue);
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
	void setDeadZone(float deadAngle) {
		this->deadAngle = deadAngle;
	}
	vector<unsigned char>& getPacket() {
		return packet;
	}
protected:
	ofSerial serial;
	bool connected;
	float deadAngle;
	vector<unsigned char> packet;
	
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