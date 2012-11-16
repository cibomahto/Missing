#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofxMidi.h"
#include "ofxTiming.h"
#include "ofxAutoControlPanel.h"
#include "ofxCv.h"

#include "KinectTracker.h"
#include "DriverInterface.h"
#include "Speaker.h"
#include "MiniFont.h"

class MissingApp : public ofBaseApp {
public:
	void setupControlPanel();
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	void setupTracker();
	void updateTracker();
	void drawTracker();
	
	void setupControl();
	void updateControl();
	void drawControl();
	
	void drawScene(bool showLabels);
	void drawPlan(float x, float y, float side);
	void drawSection(float x, float y, float side);
	void drawPerspective();
	
	void buildWires();
	void buildSpeakers();
	
	ofxAutoControlPanel gui;
	
	bool connected, calibrating, clearBackground;
	float calibrationStart, connectedStart;
	
	KinectTracker kinectSw, kinectNe;
	
	ofFloatImage presence;
	ofxCv::ContourFinder contourFinder;
	
	ofEasyCam cam;
	ofVboMesh wiresCloud, centersCloud;
	ofVboMesh wires;
	
	vector<Speaker> speakers;
	vector<ofVec2f> listeners, realListeners;
	
	DriverInterface driver;
	
	ofxMidiOut midi;
	
	bool rawPresence;
	Hysteresis presenceHysteresis;
	FadeTimer volume;
	
	bool enableMidi;
};