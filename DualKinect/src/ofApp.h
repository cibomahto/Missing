#pragma once

#include "ofMain.h"
#include "ofxKinect.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	ofxKinect kinect1, kinect2;
	ofVec3f smoothDown1, smoothDown2;
};