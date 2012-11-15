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
	
	int posMin, posCenter, posMax;
	
	float prevAngle, currentAngle, smoothAngle, actualAngle;  
  bool prevMoving, moving;
	
public:
	static void setupMesh();
	void setup(ofVec3f position, ofMesh& wires, int posMin, int posCenter, int posMax);
	void draw(bool showLabel);
	void update(vector<ofVec2f>& listeners);
	float getAngle() const;
	float getPosMin() const;
	float getPosCenter() const;
	float getPosMax() const;
};
