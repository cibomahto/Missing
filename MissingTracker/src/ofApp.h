/*
 * do background thresholding with mm, don't display bg image
 * add iir to presence image
 */

#pragma once

#include "ofMain.h"
#include "ofxAutoControlPanel.h"
#include "ofxCv.h"
#include "Conversion.h"
#include "ofxOsc.h"
#include "KinectTracker.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	ofxAutoControlPanel gui;
	
	bool calibrating, clearBackground;
	float calibrationStart;
	
	KinectTracker kinectSw, kinectNe;
	
	ofFloatImage presence;
	ofxCv::ContourFinder contourFinder;
	ofxOscSender osc;
	
	ofEasyCam cam;
};
