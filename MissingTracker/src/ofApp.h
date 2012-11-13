/*
* add second kinect
* tune/calibrate
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
	
	ofxAutoControlPanel gui;
	
	bool calibrating, clearBackground;
	float calibrationStart;
	
	// each kinect
	ofxKinect kinect;
	ofImage valid, background;
	KinectFilter filter;
	
	// combined
	ofMesh foregroundCloud, foregroundFlat;
	ofFloatImage presence;	
	ofxCv::ContourFinder contourFinder;
	ofxOscSender osc;
	
	ofEasyCam cam;
};
