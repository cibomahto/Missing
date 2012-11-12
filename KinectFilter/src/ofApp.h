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

class KinectFilter {
protected:
	unsigned int kinectFrameCount;
	vector<SimpleHysteresis> hysteresis;
	ofImage mask, masked;
public:
	KinectFilter()
	:kinectFrameCount(0)
	{
	}
	void update(ofPixels& pixels) {
		ofxCv::imitate(mask, pixels);
		unsigned char* kinectPixels = pixels.getPixels();
		unsigned char* maskPixels = mask.getPixels();
		int n = pixels.getWidth() * pixels.getHeight();
		if(hysteresis.size() != n) {
			hysteresis.resize(n);
		}
		for(int i = 0; i < n; i++) {
			maskPixels[i] = hysteresis[i].update(kinectPixels[i] > 0, kinectFrameCount) ? 255 : 0;
		}
		mask.update();
		ofxCv::bitwise_and(pixels, mask, masked);
		masked.update();
		kinectFrameCount++;
	}
	ofImage& getMask() {
		return mask;
	}
	ofImage& getMasked() {
		return masked;
	}
};

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	ofxKinect kinect;
	KinectFilter filter;
};