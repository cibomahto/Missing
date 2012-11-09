#pragma once

#include "ofMain.h"

#include "ofxAssimpModelLoader.h"

class Speaker {
protected:
	static ofVboMesh play3;
	static int count;
	
	ofVec3f position;
	ofVec2f orientation;
	float baseRotation, rotation;
	ofMesh wires;
	string label;
public:
	static void setupMesh();
	void setup(ofVec3f position, ofMesh& wires);
	void draw();
	void update(vector<ofVec2f>& listeners);
};

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	void buildWires();
	void buildSpeakers();
	
	ofEasyCam cam;
	ofMesh wiresCloud, centersCloud;
	ofMesh wires;
	
	vector<Speaker> speakers;
	vector<ofVec2f> listeners;
};