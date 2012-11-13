/*
* add second kinect
* tune
*/

#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxAutoControlPanel.h"
#include "ofxCv.h"
#include "Conversion.h"
#include "ofxOsc.h"
#include "KinectFilter.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	ofxKinect kinect;
	ofxAutoControlPanel gui;
	bool calibrating, clearBackground;
	float calibrationStart;
	
	ofImage valid, background;
	
	ofFloatImage presence;
	ofMesh foregroundFlat, foregroundCloud;
	ofEasyCam cam;
	
	KinectFilter filter;
	
	ofxCv::ContourFinder contourFinder;
	
	ofxOscSender osc;
};
