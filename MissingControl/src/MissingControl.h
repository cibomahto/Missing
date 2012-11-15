#pragma once

#include "ofMain.h"

#include "ofxAssimpModelLoader.h"
#include "ofxMidi.h"
#include "ofxTiming.h"
#include "ofxOsc.h"
#include "ofxAutoControlPanel.h"

#include "DriverInterface.h"
#include "Speaker.h"
#include "MiniFont.h"

class MissingControl : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	void drawScene(bool showLabels);
	void drawPlan(float x, float y, float side);
	void drawSection(float x, float y, float side);
	void drawPerspective();
	
	void buildWires();
	void buildSpeakers();
	
	ofxAutoControlPanel gui;
	
	bool autorun;
	ofEasyCam cam;
	ofVboMesh wiresCloud, centersCloud;
	ofVboMesh wires;
	
	vector<Speaker> speakers;
	vector<ofVec2f> listeners, oscListeners;
	
	DriverInterface driver;
	
	ofxMidiOut midi;
	ofxOscReceiver osc;
	
	bool rawPresence;
	Hysteresis presence;
	FadeTimer volume;
};