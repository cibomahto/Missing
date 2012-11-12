#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxCv.h"

class SimpleHysteresis {
protected:
	unsigned int lastTime;
	bool curValue;
	
	static const unsigned int delay;
public:
	SimpleHysteresis()
		:lastTime(0)
		,curValue(false)
	{
	}
	const bool& update(const bool& value, const unsigned int& curTime) {
		if(value != curValue) {
			lastTime = curTime;
		}
		curValue = value;
		return curTime - lastTime > delay;
	}
};

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	ofxKinect kinect;
	unsigned int kinectFrameCount;
	vector<SimpleHysteresis> hysteresis;
	ofImage mask;
	ofImage masked;
};