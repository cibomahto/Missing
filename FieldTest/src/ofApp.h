#pragma once

#include "ofMain.h"

#include "ofxAssimpModelLoader.h"
#include "DriverInterface.h"
#include "Speaker.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	void buildWires();
	void buildSpeakers();
	
	bool autorun;
	ofEasyCam cam;
	ofMesh wiresCloud, centersCloud;
	ofMesh wires;
	
	vector<Speaker> speakers;
	vector<ofVec2f> listeners;
	
	DriverInterface driver;
	ofTrueTypeFont font;
};