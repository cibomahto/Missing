#pragma once

#include "ofMain.h"

class Speaker {
protected:
	static ofVboMesh play3;
	static int count;
	
	ofVec3f position;
	ofVec2f orientation;
	float baseRotation;
	ofMesh wires;
	string label;
	
	float prevAngle, currentAngle, smoothAngle, actualAngle;  
  bool prevMoving, moving;
	
public:
	static void setupMesh();
	void setup(ofVec3f position, ofMesh& wires);
	void draw();
	void update(vector<ofVec2f>& listeners);
	float getAngle();
};
