#pragma once

#include "ofMain.h"
#include "ofxAutoControlPanel.h"
#include "ofxCv.h"
#include "Conversion.h"
#include "ofxOsc.h"
#include "KinectTracker.h"

class MissingTracker : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	ofxAutoControlPanel gui;
	
	bool connected, calibrating, clearBackground;
	float calibrationStart;
	
	KinectTracker kinectSw, kinectNe;
	
	ofFloatImage presence;
	ofxCv::ContourFinder contourFinder;
	ofxOscSender osc;
	
	ofEasyCam cam;
};
