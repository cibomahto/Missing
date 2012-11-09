/*
 z axis is up/down, everything is on the x/y plane
 +z is up, -z is down
 everything on screen is done in millimeters, though data is diverse
 */

#include "ofApp.h"

float centimetersToMillimeters(float centimeters) {
	return 10 * centimeters;
}

float inchesToMillimeters(float inches) {
	return 25.4 * inches;
}

float feetToInches(float inches) {
	return 12 * inches;
}

float feetToMillimeters(float feet) {
	return inchesToMillimeters(feetToInches(feet));
}

float feetInchesToMillimeters(float feet, float inches) {
	return inchesToMillimeters(feetToInches(feet) + inches);
}

void scale(ofMesh& mesh, float amount) {
	for(int i = 0; i < mesh.getNumVertices(); i++) {
		mesh.getVertices()[i] *= amount;
	}
}

float stageSize = feetInchesToMillimeters(15, 8);
float stageHeight = feetToMillimeters(16);
float eyeLevel = feetInchesToMillimeters(5, 9);
ofVec2f play3Orientation = ofVec2f(-1, 0);

ofVboMesh Speaker::play3;
int Speaker::count = 0;
void Speaker::setupMesh() {
	ofxAssimpModelLoader model;
	model.loadModel("play3.obj");
	play3 = model.getMesh(0);
	scale(play3, centimetersToMillimeters(1));
}

ofVec3f referencePoint;
bool distanceToReference(const ofVec3f& a, const ofVec3f& b) {
	return a.distance(referencePoint) < b.distance(referencePoint);
}

void Speaker::setup(ofVec3f position, ofMesh& wiresCloud) {
	this->position = position;
	label = ofToString(count++);
	
	vector<ofVec3f> centers = wiresCloud.getVertices();
	referencePoint = position;
	ofSort(centers, distanceToReference);
	ofVec2f a = centers[0], b = centers[1], c = centers[2];
	wires.addVertex(a);
	wires.addVertex(b);
	wires.addVertex(c);
	// could also determine the real orientation from wires, but its' more work
	orientation = -position;
	orientation.normalize();
	baseRotation = play3Orientation.angle(orientation);
}

void Speaker::draw() {
	ofPushMatrix();
	wires.drawWireframe();
	ofTranslate(position.x, position.y, 0);
	ofDrawBitmapString(label, 0, 0);
	ofTranslate(0, 0, position.z);
	ofLine(ofVec2f(0, 0), orientation * feetToMillimeters(1));
	ofPushMatrix();
	ofRotateZ(baseRotation + rotation);
	play3.draw();
	ofPopMatrix();
	ofPopMatrix();
}

ofVec2f getClosestPoint(vector<ofVec2f>& points, ofVec2f target) {
	float minimum;
	ofVec2f closest;
	for(int i = 0; i < points.size(); i++) {
		float cur = points[i].distance(target);
		if(i == 0 || cur < minimum) {
			minimum = cur;
			closest = points[i];
		}
	}
	return closest;
}

void Speaker::update(vector<ofVec2f>& listeners) {
	ofVec2f closest = getClosestPoint(listeners, position);
	ofVec2f direction = closest - position;
	rotation = orientation.angle(direction);
}

void ofApp::setup() {
	ofSetVerticalSync(true);
	ofEnableAlphaBlending();
	
	Speaker::setupMesh();
	
	wiresCloud.setMode(OF_PRIMITIVE_POINTS);
	wiresCloud.load("wires.ply");
	scale(wiresCloud, inchesToMillimeters(1));
	
	centersCloud.setMode(OF_PRIMITIVE_POINTS);
	centersCloud.load("centers.ply");
	scale(centersCloud, inchesToMillimeters(1));
	
	buildWires();
	buildSpeakers();
}

void ofApp::buildWires() {
	wires.setMode(OF_PRIMITIVE_LINES);
	for(int i = 0; i < wiresCloud.getNumVertices(); i++) {
		ofVec3f wireCenter = wiresCloud.getVertex(i);
		wires.addVertex(wireCenter);
		wires.addVertex(wireCenter + ofVec3f(0, 0, feetToMillimeters(16)));
	}
}

void ofApp::buildSpeakers() {
	ofSeedRandom(0);
	for(int i = 0; i < centersCloud.getNumVertices(); i++) {
		Speaker speaker;
		ofVec3f position = centersCloud.getVertex(i);
		position.z = feetToMillimeters(ofRandom(2, 8));
		speaker.setup(position, wiresCloud);
		speakers.push_back(speaker);
	}
}

void ofApp::update() {
	listeners.clear();
	if(ofGetKeyPressed('q')) {
		listeners.push_back(ofVec2f(0, 0));
	}
	if(ofGetKeyPressed('w')) {
		listeners.push_back(ofVec2f(
			ofMap(mouseX, 0, ofGetWidth(), -stageSize / 2, stageSize / 2),
			ofMap(mouseY, 0, ofGetHeight(), stageSize / 2, -stageSize / 2)));
	}
	
	for(int i = 0; i < speakers.size(); i++) {
		speakers[i].update(listeners);
	}
}

void ofApp::draw() {
	ofBackground(0);
	ofSetColor(255, 128);
	
	cam.begin();
	ofScale(.2, .2, .2);
	
	ofTranslate(0, 0, -eyeLevel);
	ofPushMatrix();
	ofNoFill();
	ofRect(-stageSize / 2, -stageSize / 2, stageSize, stageSize);
	ofTranslate(0, 0, stageHeight);
	ofRect(-stageSize / 2, -stageSize / 2, stageSize, stageSize);
	ofPopMatrix();
	
	for(int i = 0; i < speakers.size(); i++) {
		speakers[i].draw();
	}
	
	for(int i = 0; i < listeners.size(); i++) {
		ofCircle(listeners[i], feetToMillimeters(1));
	}
	
	wiresCloud.draw();
	centersCloud.draw();
	wires.draw();
	cam.end();
}