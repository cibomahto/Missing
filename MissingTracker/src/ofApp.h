/*
x do blob detection and tracking
x export data over osc
* add final stage for rotating data
* farther data has fewer points (use triangles area: |ABxAC|/2)
* mask noisy areas around edges
* use an iir filter on depth image
* add second kinect
*/

#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxAutoControlPanel.h"
#include "ofxCv.h"
#include "Conversion.h"
#include "ofxOsc.h"

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
	
	ofImage result, background;
	
	ofFloatImage presence;
	ofMesh foregroundFlat;
	ofMesh foregroundCloud;
	ofEasyCam cam;
	
	ofxCv::ContourFinder contourFinder;
	
	ofxOscSender osc;
};
