/*
build up a background image (use nearest filtering over time)
do that in the depth-image space
analyze current image for any points that are > certain distance from background image
project these points
-
rotate+translate these points so they are oriented on the platform xy plane
build up a low-resolution (binned) "presence graph"
do blob detection and tracking
*/

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
	
	ofImage result, background;
	
	ofMesh foregroundCloud;
	ofEasyCam cam;
};
