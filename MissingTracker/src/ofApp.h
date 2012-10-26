#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxAutoControlPanel.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	ofxKinect kinect;
	ofxAutoControlPanel panel;
	bool calibrating, clearBackground;
	float calibrationStart;
	
	ofImage result;
	ofImage background;
};
